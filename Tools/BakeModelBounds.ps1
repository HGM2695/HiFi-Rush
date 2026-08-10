param(
    [string]$RootPath = (Join-Path $PSScriptRoot "..\HiFi-Rush\Resources\Model\Binary"),
    [switch]$VerifyOnly
)

# Static/Skeletal Model 바이너리 본문 뒤에 로컬 바운드 메타데이터를 기록합니다.
# 기본 실행은 전체 베이킹이며, -VerifyOnly는 저장값과 모델에서 다시 계산한 값을 비교합니다.

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

if (("ModelBoundsBaker" -as [type]) -eq $null)
{
    Add-Type -Language CSharp -TypeDefinition @"
using System;
using System.Collections.Generic;
using System.IO;

public static class ModelBoundsBaker
{
    const int ModelTypeStatic = 0;
    const int ModelTypeSkeletal = 1;
    const int MatrixBytes = 64;
    const int VertexMeshBytes = 44;
    const int LegacyTextureTypeCount = 18;
    const int BoundsFooterBytes = 24;
    const int AnimationSamplesPerSecond = 60;
    const float SkeletalBoundsPaddingRatio = 0.01f;

    struct Point3
    {
        public float X;
        public float Y;
        public float Z;

        public Point3(float x, float y, float z)
        {
            X = x;
            Y = y;
            Z = z;
        }
    }

    struct Quaternion4
    {
        public float X;
        public float Y;
        public float Z;
        public float W;
    }

    struct Matrix4
    {
        public float M11, M12, M13, M14;
        public float M21, M22, M23, M24;
        public float M31, M32, M33, M34;
        public float M41, M42, M43, M44;
    }

    struct KeyFrame
    {
        public Point3 Scale;
        public Quaternion4 Rotation;
        public Point3 Position;
        public float TrackPosition;
    }

    struct SkinnedVertex
    {
        public Point3 Position;
        public uint Index0, Index1, Index2, Index3;
        public float Weight0, Weight1, Weight2, Weight3;
    }

    sealed class Bone
    {
        public Matrix4 Transform;
        public int ParentIndex;
    }

    sealed class AnimationChannel
    {
        public uint BoneIndex;
        public KeyFrame[] KeyFrames;
    }

    sealed class AnimationClip
    {
        public float Duration;
        public float TicksPerSecond;
        public AnimationChannel[] Channels;
    }

    sealed class PointBounds
    {
        public bool HasPoints;
        public Point3 Min;
        public Point3 Max;

        public void Include(Point3 point)
        {
            if (!HasPoints)
            {
                Min = point;
                Max = point;
                HasPoints = true;
                return;
            }

            Min = new Point3(
                Math.Min(Min.X, point.X),
                Math.Min(Min.Y, point.Y),
                Math.Min(Min.Z, point.Z));
            Max = new Point3(
                Math.Max(Max.X, point.X),
                Math.Max(Max.Y, point.Y),
                Math.Max(Max.Z, point.Z));
        }
    }

    struct ModelBounds
    {
        public Point3 BoxCenter;
        public Point3 BoxExtents;
    }

    sealed class ParsedModel
    {
        public int ModelType;
        public long CoreLength;
        public long OriginalLength;
        public int VertexCount;
        public bool HasBounds;
        public ModelBounds StoredBounds;
        public ModelBounds CalculatedBounds;
        public Bone[] Bones;
        public AnimationClip[] Animations;
        public PointBounds[] BonePointBounds;
        public int AnimationSampleCount;
    }

    public sealed class Result
    {
        public bool IsStatic;
        public bool HadBounds;
        public bool WasWritten;
        public int VertexCount;
        public long OriginalBytes;
        public long WrittenBytes;
        public int AnimationSampleCount;
    }

    public static Result Process(string path, bool verifyOnly)
    {
        ParsedModel model = Parse(path);
        bool isStatic = model.ModelType == ModelTypeStatic;
        bool wasWritten = false;

        if (verifyOnly)
        {
            if (!model.HasBounds)
                throw new InvalidDataException("바운드 메타데이터가 없습니다.");

            ValidateStoredBounds(model.StoredBounds, model.CalculatedBounds);
        }
        else
        {
            WriteBounds(path, model.CoreLength, model.CalculatedBounds);
            wasWritten = true;
        }

        return new Result
        {
            IsStatic = isStatic,
            HadBounds = model.HasBounds,
            WasWritten = wasWritten,
            VertexCount = model.VertexCount,
            OriginalBytes = model.OriginalLength,
            WrittenBytes = wasWritten ? model.CoreLength + BoundsFooterBytes : model.OriginalLength,
            AnimationSampleCount = model.AnimationSampleCount
        };
    }

    static ParsedModel Parse(string path)
    {
        var points = new List<Point3>();
        var parsed = new ParsedModel();

        using (var input = new BinaryReader(File.Open(path, FileMode.Open, FileAccess.Read, FileShare.Read)))
        {
            parsed.OriginalLength = input.BaseStream.Length;
            parsed.ModelType = input.ReadInt32();
            if (parsed.ModelType != ModelTypeStatic && parsed.ModelType != ModelTypeSkeletal)
                throw new InvalidDataException("지원하지 않는 모델 타입입니다.");

            Skip(input, MatrixBytes);
            if (parsed.ModelType == ModelTypeSkeletal)
            {
                parsed.Bones = ReadBones(input);
                parsed.Animations = ReadAnimations(input, parsed.Bones.Length);
                parsed.BonePointBounds = new PointBounds[parsed.Bones.Length];
                for (int boneIndex = 0; boneIndex < parsed.BonePointBounds.Length; ++boneIndex)
                    parsed.BonePointBounds[boneIndex] = new PointBounds();
            }

            ReadMeshes(input, parsed, points);
            SkipMaterials(input);
            parsed.CoreLength = input.BaseStream.Position;

            long remainBytes = input.BaseStream.Length - parsed.CoreLength;
            if (remainBytes == BoundsFooterBytes)
            {
                parsed.StoredBounds = ReadBounds(input);
                parsed.HasBounds = true;
            }
            else if (remainBytes != 0)
            {
                throw new InvalidDataException("모델 본문 뒤에 알 수 없는 데이터가 있습니다. bytes=" + remainBytes);
            }
        }

        if (parsed.ModelType == ModelTypeStatic)
        {
            if (points.Count == 0)
                throw new InvalidDataException("Static Model 정점 데이터가 비어 있습니다.");

            parsed.VertexCount = points.Count;
            parsed.CalculatedBounds = CalculateBounds(points);
        }
        else
        {
            parsed.CalculatedBounds = CalculateSkeletalBounds(parsed);
        }

        return parsed;
    }

    static void ReadMeshes(BinaryReader input, ParsedModel model, List<Point3> points)
    {
        uint meshCount = input.ReadUInt32();
        for (uint meshIndex = 0; meshIndex < meshCount; ++meshIndex)
        {
            SkipString(input);
            Skip(input, sizeof(uint));

            uint vertexCount = input.ReadUInt32();
            if (model.ModelType == ModelTypeStatic)
            {
                for (uint vertexIndex = 0; vertexIndex < vertexCount; ++vertexIndex)
                {
                    points.Add(new Point3(input.ReadSingle(), input.ReadSingle(), input.ReadSingle()));
                    Skip(input, VertexMeshBytes - sizeof(float) * 3);
                }
            }
            else
            {
                int checkedVertexCount = checked((int)vertexCount);
                var vertices = new SkinnedVertex[checkedVertexCount];
                for (int vertexIndex = 0; vertexIndex < vertices.Length; ++vertexIndex)
                    vertices[vertexIndex] = ReadSkinnedVertex(input);

                uint meshBoneCount = input.ReadUInt32();
                int checkedMeshBoneCount = checked((int)meshBoneCount);
                var boneIndices = new uint[checkedMeshBoneCount];
                var offsetMatrices = new Matrix4[checkedMeshBoneCount];
                for (int paletteIndex = 0; paletteIndex < checkedMeshBoneCount; ++paletteIndex)
                    boneIndices[paletteIndex] = input.ReadUInt32();
                for (int paletteIndex = 0; paletteIndex < checkedMeshBoneCount; ++paletteIndex)
                    offsetMatrices[paletteIndex] = ReadMatrix(input);

                for (int vertexIndex = 0; vertexIndex < vertices.Length; ++vertexIndex)
                    AddVertexInfluences(model, vertices[vertexIndex], boneIndices, offsetMatrices);

                model.VertexCount = checked(model.VertexCount + checkedVertexCount);
            }

            uint indexCount = input.ReadUInt32();
            Skip(input, CheckedByteCount(indexCount, sizeof(uint)));
        }
    }

    static Bone[] ReadBones(BinaryReader input)
    {
        uint boneCount = input.ReadUInt32();
        var bones = new Bone[checked((int)boneCount)];
        for (int boneIndex = 0; boneIndex < bones.Length; ++boneIndex)
        {
            SkipString(input);
            var bone = new Bone();
            bone.Transform = ReadMatrix(input);
            bone.ParentIndex = input.ReadInt32();
            if (bone.ParentIndex < -1 || bone.ParentIndex >= boneIndex)
                throw new InvalidDataException("Bone 부모 인덱스가 올바르지 않습니다. bone=" + boneIndex + ", parent=" + bone.ParentIndex);

            bones[boneIndex] = bone;
        }

        if (bones.Length == 0)
            throw new InvalidDataException("Skeletal Model Bone 데이터가 비어 있습니다.");

        return bones;
    }

    static AnimationClip[] ReadAnimations(BinaryReader input, int boneCount)
    {
        uint animationCount = input.ReadUInt32();
        var animations = new AnimationClip[checked((int)animationCount)];
        for (int animationIndex = 0; animationIndex < animations.Length; ++animationIndex)
        {
            SkipString(input);
            var animation = new AnimationClip();
            animation.Duration = input.ReadSingle();
            animation.TicksPerSecond = input.ReadSingle();

            uint channelCount = input.ReadUInt32();
            animation.Channels = new AnimationChannel[checked((int)channelCount)];
            for (int channelIndex = 0; channelIndex < animation.Channels.Length; ++channelIndex)
            {
                SkipString(input);
                var channel = new AnimationChannel();
                channel.BoneIndex = input.ReadUInt32();
                if (channel.BoneIndex >= boneCount)
                    throw new InvalidDataException("Animation Channel Bone 인덱스가 범위를 벗어났습니다.");

                uint keyFrameCount = input.ReadUInt32();
                channel.KeyFrames = new KeyFrame[checked((int)keyFrameCount)];
                for (int keyFrameIndex = 0; keyFrameIndex < channel.KeyFrames.Length; ++keyFrameIndex)
                    channel.KeyFrames[keyFrameIndex] = ReadKeyFrame(input);

                animation.Channels[channelIndex] = channel;
            }

            animations[animationIndex] = animation;
        }

        return animations;
    }

    static SkinnedVertex ReadSkinnedVertex(BinaryReader input)
    {
        var vertex = new SkinnedVertex();
        vertex.Position = ReadPoint(input);
        Skip(input, VertexMeshBytes - sizeof(float) * 3);
        vertex.Index0 = input.ReadUInt32();
        vertex.Index1 = input.ReadUInt32();
        vertex.Index2 = input.ReadUInt32();
        vertex.Index3 = input.ReadUInt32();
        vertex.Weight0 = input.ReadSingle();
        vertex.Weight1 = input.ReadSingle();
        vertex.Weight2 = input.ReadSingle();
        vertex.Weight3 = input.ReadSingle();
        return vertex;
    }

    static void AddVertexInfluences(ParsedModel model, SkinnedVertex vertex, uint[] boneIndices, Matrix4[] offsetMatrices)
    {
        float weightSum = vertex.Weight0 + vertex.Weight1 + vertex.Weight2 + vertex.Weight3;
        if (vertex.Weight0 < 0f || vertex.Weight1 < 0f || vertex.Weight2 < 0f || vertex.Weight3 < 0f ||
            Math.Abs(weightSum - 1f) > 0.001f)
            throw new InvalidDataException("Skeletal Vertex BlendWeight가 올바르지 않습니다. sum=" + weightSum);

        AddVertexInfluence(model, vertex.Position, vertex.Index0, vertex.Weight0, boneIndices, offsetMatrices);
        AddVertexInfluence(model, vertex.Position, vertex.Index1, vertex.Weight1, boneIndices, offsetMatrices);
        AddVertexInfluence(model, vertex.Position, vertex.Index2, vertex.Weight2, boneIndices, offsetMatrices);
        AddVertexInfluence(model, vertex.Position, vertex.Index3, vertex.Weight3, boneIndices, offsetMatrices);
    }

    static void AddVertexInfluence(ParsedModel model, Point3 position, uint paletteIndex, float weight, uint[] boneIndices, Matrix4[] offsetMatrices)
    {
        if (weight <= 0f)
            return;

        if (paletteIndex >= (uint)boneIndices.Length)
            throw new InvalidDataException("Skeletal Vertex BlendIndex가 Mesh Bone Palette 범위를 벗어났습니다.");

        uint boneIndex = boneIndices[paletteIndex];
        if (boneIndex >= (uint)model.BonePointBounds.Length)
            throw new InvalidDataException("Mesh Bone 인덱스가 Skeleton Bone 범위를 벗어났습니다.");

        model.BonePointBounds[boneIndex].Include(TransformPoint(position, offsetMatrices[paletteIndex]));
    }

    static KeyFrame ReadKeyFrame(BinaryReader input)
    {
        var keyFrame = new KeyFrame();
        keyFrame.Scale = ReadPoint(input);
        keyFrame.Rotation = new Quaternion4
        {
            X = input.ReadSingle(),
            Y = input.ReadSingle(),
            Z = input.ReadSingle(),
            W = input.ReadSingle()
        };
        keyFrame.Position = ReadPoint(input);
        keyFrame.TrackPosition = input.ReadSingle();
        return keyFrame;
    }

    static Matrix4 ReadMatrix(BinaryReader input)
    {
        var matrix = new Matrix4();
        matrix.M11 = input.ReadSingle(); matrix.M12 = input.ReadSingle(); matrix.M13 = input.ReadSingle(); matrix.M14 = input.ReadSingle();
        matrix.M21 = input.ReadSingle(); matrix.M22 = input.ReadSingle(); matrix.M23 = input.ReadSingle(); matrix.M24 = input.ReadSingle();
        matrix.M31 = input.ReadSingle(); matrix.M32 = input.ReadSingle(); matrix.M33 = input.ReadSingle(); matrix.M34 = input.ReadSingle();
        matrix.M41 = input.ReadSingle(); matrix.M42 = input.ReadSingle(); matrix.M43 = input.ReadSingle(); matrix.M44 = input.ReadSingle();
        return matrix;
    }

    static void SkipMaterials(BinaryReader input)
    {
        uint materialCount = input.ReadUInt32();
        for (uint materialIndex = 0; materialIndex < materialCount; ++materialIndex)
        {
            uint totalTextureCount = input.ReadUInt32();
            if (totalTextureCount == 0)
                continue;

            uint[] textureCounts = new uint[LegacyTextureTypeCount];
            for (int textureType = 0; textureType < LegacyTextureTypeCount; ++textureType)
                textureCounts[textureType] = input.ReadUInt32();

            for (int textureType = 0; textureType < LegacyTextureTypeCount; ++textureType)
            {
                for (uint textureIndex = 0; textureIndex < textureCounts[textureType]; ++textureIndex)
                    SkipString(input);
            }
        }
    }

    static ModelBounds CalculateSkeletalBounds(ParsedModel model)
    {
        if (model.VertexCount == 0)
            throw new InvalidDataException("Skeletal Model 정점 데이터가 비어 있습니다.");

        var accumulatedBounds = new PointBounds();
        IncludePoseBounds(model.BonePointBounds, BuildBoneModelMatrices(model.Bones, null, 0f), accumulatedBounds);
        int sampleCount = 1;

        for (int animationIndex = 0; animationIndex < model.Animations.Length; ++animationIndex)
        {
            AnimationClip animation = model.Animations[animationIndex];
            List<float> samplePositions = BuildSamplePositions(animation);
            for (int sampleIndex = 0; sampleIndex < samplePositions.Count; ++sampleIndex)
            {
                Matrix4[] boneModelMatrices = BuildBoneModelMatrices(model.Bones, animation, samplePositions[sampleIndex]);
                IncludePoseBounds(model.BonePointBounds, boneModelMatrices, accumulatedBounds);
                ++sampleCount;
            }
        }

        if (!accumulatedBounds.HasPoints)
            throw new InvalidDataException("Skeletal Model 바운드를 계산할 수 없습니다.");

        model.AnimationSampleCount = sampleCount;

        Point3 boxCenter = Midpoint(accumulatedBounds.Min, accumulatedBounds.Max);
        Point3 boxExtents = new Point3(
            (accumulatedBounds.Max.X - accumulatedBounds.Min.X) * 0.5f,
            (accumulatedBounds.Max.Y - accumulatedBounds.Min.Y) * 0.5f,
            (accumulatedBounds.Max.Z - accumulatedBounds.Min.Z) * 0.5f);
        float padding = Math.Max(0.001f, Math.Max(boxExtents.X, Math.Max(boxExtents.Y, boxExtents.Z)) * SkeletalBoundsPaddingRatio);
        boxExtents.X += padding;
        boxExtents.Y += padding;
        boxExtents.Z += padding;

        return new ModelBounds
        {
            BoxCenter = boxCenter,
            BoxExtents = boxExtents
        };
    }

    static List<float> BuildSamplePositions(AnimationClip animation)
    {
        var positions = new SortedSet<float>();
        positions.Add(0f);

        if (animation.Duration > 0f)
        {
            positions.Add(animation.Duration);
            if (animation.TicksPerSecond > 0f)
            {
                double seconds = animation.Duration / animation.TicksPerSecond;
                int intervalCount = Math.Max(1, checked((int)Math.Ceiling(seconds * AnimationSamplesPerSecond)));
                for (int intervalIndex = 1; intervalIndex < intervalCount; ++intervalIndex)
                    positions.Add(animation.Duration * intervalIndex / intervalCount);
            }
        }

        for (int channelIndex = 0; channelIndex < animation.Channels.Length; ++channelIndex)
        {
            KeyFrame[] keyFrames = animation.Channels[channelIndex].KeyFrames;
            for (int keyFrameIndex = 0; keyFrameIndex < keyFrames.Length; ++keyFrameIndex)
            {
                float trackPosition = keyFrames[keyFrameIndex].TrackPosition;
                if (trackPosition >= 0f && (animation.Duration <= 0f || trackPosition <= animation.Duration))
                    positions.Add(trackPosition);
            }
        }

        return new List<float>(positions);
    }

    static Matrix4[] BuildBoneModelMatrices(Bone[] bones, AnimationClip animation, float trackPosition)
    {
        var localMatrices = new Matrix4[bones.Length];
        for (int boneIndex = 0; boneIndex < bones.Length; ++boneIndex)
            localMatrices[boneIndex] = bones[boneIndex].Transform;

        if (animation != null)
        {
            for (int channelIndex = 0; channelIndex < animation.Channels.Length; ++channelIndex)
            {
                AnimationChannel channel = animation.Channels[channelIndex];
                KeyFrame keyFrame = SampleKeyFrame(channel, trackPosition);
                localMatrices[channel.BoneIndex] = CreateTransformMatrix(keyFrame.Position, keyFrame.Rotation, keyFrame.Scale);
            }
        }

        var modelMatrices = new Matrix4[bones.Length];
        for (int boneIndex = 0; boneIndex < bones.Length; ++boneIndex)
        {
            int parentIndex = bones[boneIndex].ParentIndex;
            modelMatrices[boneIndex] = parentIndex >= 0
                ? Multiply(localMatrices[boneIndex], modelMatrices[parentIndex])
                : localMatrices[boneIndex];
        }

        return modelMatrices;
    }

    static KeyFrame SampleKeyFrame(AnimationChannel channel, float trackPosition)
    {
        KeyFrame[] keyFrames = channel.KeyFrames;
        if (keyFrames.Length == 0)
            return IdentityKeyFrame(trackPosition);

        if (keyFrames.Length == 1 || trackPosition <= keyFrames[0].TrackPosition)
            return keyFrames[0];

        int low = 0;
        int high = keyFrames.Length - 1;
        while (low + 1 < high)
        {
            int middle = low + (high - low) / 2;
            if (trackPosition < keyFrames[middle].TrackPosition)
                high = middle;
            else
                low = middle;
        }

        if (trackPosition >= keyFrames[high].TrackPosition)
            return keyFrames[high];

        KeyFrame current = keyFrames[low];
        KeyFrame next = keyFrames[high];
        float frameLength = next.TrackPosition - current.TrackPosition;
        if (frameLength <= 0f)
            return current;

        float ratio = (trackPosition - current.TrackPosition) / frameLength;
        return new KeyFrame
        {
            Scale = Lerp(current.Scale, next.Scale, ratio),
            Rotation = Slerp(current.Rotation, next.Rotation, ratio),
            Position = Lerp(current.Position, next.Position, ratio),
            TrackPosition = trackPosition
        };
    }

    static KeyFrame IdentityKeyFrame(float trackPosition)
    {
        return new KeyFrame
        {
            Scale = new Point3(1f, 1f, 1f),
            Rotation = new Quaternion4 { W = 1f },
            Position = new Point3(0f, 0f, 0f),
            TrackPosition = trackPosition
        };
    }

    static void IncludePoseBounds(PointBounds[] bonePointBounds, Matrix4[] boneModelMatrices, PointBounds output)
    {
        for (int boneIndex = 0; boneIndex < bonePointBounds.Length; ++boneIndex)
        {
            PointBounds boneBounds = bonePointBounds[boneIndex];
            if (!boneBounds.HasPoints)
                continue;

            for (int x = 0; x < 2; ++x)
            {
                for (int y = 0; y < 2; ++y)
                {
                    for (int z = 0; z < 2; ++z)
                    {
                        Point3 corner = new Point3(
                            x == 0 ? boneBounds.Min.X : boneBounds.Max.X,
                            y == 0 ? boneBounds.Min.Y : boneBounds.Max.Y,
                            z == 0 ? boneBounds.Min.Z : boneBounds.Max.Z);
                        output.Include(TransformPoint(corner, boneModelMatrices[boneIndex]));
                    }
                }
            }
        }
    }

    static Matrix4 CreateTransformMatrix(Point3 position, Quaternion4 rotation, Point3 scale)
    {
        rotation = Normalize(rotation);
        float xx = rotation.X * rotation.X;
        float yy = rotation.Y * rotation.Y;
        float zz = rotation.Z * rotation.Z;
        float xy = rotation.X * rotation.Y;
        float xz = rotation.X * rotation.Z;
        float yz = rotation.Y * rotation.Z;
        float xw = rotation.X * rotation.W;
        float yw = rotation.Y * rotation.W;
        float zw = rotation.Z * rotation.W;

        var matrix = new Matrix4();
        matrix.M11 = (1f - 2f * (yy + zz)) * scale.X;
        matrix.M12 = (2f * (xy + zw)) * scale.X;
        matrix.M13 = (2f * (xz - yw)) * scale.X;
        matrix.M21 = (2f * (xy - zw)) * scale.Y;
        matrix.M22 = (1f - 2f * (xx + zz)) * scale.Y;
        matrix.M23 = (2f * (yz + xw)) * scale.Y;
        matrix.M31 = (2f * (xz + yw)) * scale.Z;
        matrix.M32 = (2f * (yz - xw)) * scale.Z;
        matrix.M33 = (1f - 2f * (xx + yy)) * scale.Z;
        matrix.M41 = position.X;
        matrix.M42 = position.Y;
        matrix.M43 = position.Z;
        matrix.M44 = 1f;
        return matrix;
    }

    static Matrix4 Multiply(Matrix4 lhs, Matrix4 rhs)
    {
        var result = new Matrix4();
        result.M11 = lhs.M11 * rhs.M11 + lhs.M12 * rhs.M21 + lhs.M13 * rhs.M31 + lhs.M14 * rhs.M41;
        result.M12 = lhs.M11 * rhs.M12 + lhs.M12 * rhs.M22 + lhs.M13 * rhs.M32 + lhs.M14 * rhs.M42;
        result.M13 = lhs.M11 * rhs.M13 + lhs.M12 * rhs.M23 + lhs.M13 * rhs.M33 + lhs.M14 * rhs.M43;
        result.M14 = lhs.M11 * rhs.M14 + lhs.M12 * rhs.M24 + lhs.M13 * rhs.M34 + lhs.M14 * rhs.M44;
        result.M21 = lhs.M21 * rhs.M11 + lhs.M22 * rhs.M21 + lhs.M23 * rhs.M31 + lhs.M24 * rhs.M41;
        result.M22 = lhs.M21 * rhs.M12 + lhs.M22 * rhs.M22 + lhs.M23 * rhs.M32 + lhs.M24 * rhs.M42;
        result.M23 = lhs.M21 * rhs.M13 + lhs.M22 * rhs.M23 + lhs.M23 * rhs.M33 + lhs.M24 * rhs.M43;
        result.M24 = lhs.M21 * rhs.M14 + lhs.M22 * rhs.M24 + lhs.M23 * rhs.M34 + lhs.M24 * rhs.M44;
        result.M31 = lhs.M31 * rhs.M11 + lhs.M32 * rhs.M21 + lhs.M33 * rhs.M31 + lhs.M34 * rhs.M41;
        result.M32 = lhs.M31 * rhs.M12 + lhs.M32 * rhs.M22 + lhs.M33 * rhs.M32 + lhs.M34 * rhs.M42;
        result.M33 = lhs.M31 * rhs.M13 + lhs.M32 * rhs.M23 + lhs.M33 * rhs.M33 + lhs.M34 * rhs.M43;
        result.M34 = lhs.M31 * rhs.M14 + lhs.M32 * rhs.M24 + lhs.M33 * rhs.M34 + lhs.M34 * rhs.M44;
        result.M41 = lhs.M41 * rhs.M11 + lhs.M42 * rhs.M21 + lhs.M43 * rhs.M31 + lhs.M44 * rhs.M41;
        result.M42 = lhs.M41 * rhs.M12 + lhs.M42 * rhs.M22 + lhs.M43 * rhs.M32 + lhs.M44 * rhs.M42;
        result.M43 = lhs.M41 * rhs.M13 + lhs.M42 * rhs.M23 + lhs.M43 * rhs.M33 + lhs.M44 * rhs.M43;
        result.M44 = lhs.M41 * rhs.M14 + lhs.M42 * rhs.M24 + lhs.M43 * rhs.M34 + lhs.M44 * rhs.M44;
        return result;
    }

    static Point3 TransformPoint(Point3 point, Matrix4 matrix)
    {
        return new Point3(
            point.X * matrix.M11 + point.Y * matrix.M21 + point.Z * matrix.M31 + matrix.M41,
            point.X * matrix.M12 + point.Y * matrix.M22 + point.Z * matrix.M32 + matrix.M42,
            point.X * matrix.M13 + point.Y * matrix.M23 + point.Z * matrix.M33 + matrix.M43);
    }

    static Point3 Lerp(Point3 from, Point3 to, float ratio)
    {
        return new Point3(
            from.X + (to.X - from.X) * ratio,
            from.Y + (to.Y - from.Y) * ratio,
            from.Z + (to.Z - from.Z) * ratio);
    }

    static Quaternion4 Slerp(Quaternion4 from, Quaternion4 to, float ratio)
    {
        from = Normalize(from);
        to = Normalize(to);
        float dot = from.X * to.X + from.Y * to.Y + from.Z * to.Z + from.W * to.W;
        if (dot < 0f)
        {
            dot = -dot;
            to.X = -to.X;
            to.Y = -to.Y;
            to.Z = -to.Z;
            to.W = -to.W;
        }

        if (dot > 0.9995f)
        {
            return Normalize(new Quaternion4
            {
                X = from.X + (to.X - from.X) * ratio,
                Y = from.Y + (to.Y - from.Y) * ratio,
                Z = from.Z + (to.Z - from.Z) * ratio,
                W = from.W + (to.W - from.W) * ratio
            });
        }

        dot = Math.Max(-1f, Math.Min(1f, dot));
        double theta = Math.Acos(dot);
        double sinTheta = Math.Sin(theta);
        float fromRatio = (float)(Math.Sin((1f - ratio) * theta) / sinTheta);
        float toRatio = (float)(Math.Sin(ratio * theta) / sinTheta);
        return new Quaternion4
        {
            X = from.X * fromRatio + to.X * toRatio,
            Y = from.Y * fromRatio + to.Y * toRatio,
            Z = from.Z * fromRatio + to.Z * toRatio,
            W = from.W * fromRatio + to.W * toRatio
        };
    }

    static Quaternion4 Normalize(Quaternion4 value)
    {
        float length = (float)Math.Sqrt(
            (double)value.X * value.X + (double)value.Y * value.Y +
            (double)value.Z * value.Z + (double)value.W * value.W);
        if (length <= 0f)
            return new Quaternion4 { W = 1f };

        float inverseLength = 1f / length;
        value.X *= inverseLength;
        value.Y *= inverseLength;
        value.Z *= inverseLength;
        value.W *= inverseLength;
        return value;
    }

    static ModelBounds CalculateBounds(List<Point3> points)
    {
        Point3 boxMin = points[0];
        Point3 boxMax = points[0];

        for (int i = 1; i < points.Count; ++i)
        {
            Point3 point = points[i];
            boxMin.X = Math.Min(boxMin.X, point.X);
            boxMin.Y = Math.Min(boxMin.Y, point.Y);
            boxMin.Z = Math.Min(boxMin.Z, point.Z);
            boxMax.X = Math.Max(boxMax.X, point.X);
            boxMax.Y = Math.Max(boxMax.Y, point.Y);
            boxMax.Z = Math.Max(boxMax.Z, point.Z);
        }

        return new ModelBounds
        {
            BoxCenter = Midpoint(boxMin, boxMax),
            BoxExtents = new Point3(
                (boxMax.X - boxMin.X) * 0.5f,
                (boxMax.Y - boxMin.Y) * 0.5f,
                (boxMax.Z - boxMin.Z) * 0.5f)
        };
    }

    static ModelBounds ReadBounds(BinaryReader input)
    {
        return new ModelBounds
        {
            BoxCenter = ReadPoint(input),
            BoxExtents = ReadPoint(input)
        };
    }

    static void WriteBounds(string path, long coreLength, ModelBounds bounds)
    {
        string temporaryPath = path + ".bounds-bake.tmp";
        if (File.Exists(temporaryPath))
            File.Delete(temporaryPath);

        try
        {
            using (var input = File.Open(path, FileMode.Open, FileAccess.Read, FileShare.Read))
            using (var output = File.Create(temporaryPath))
            {
                CopyBytes(input, output, coreLength);
                using (var writer = new BinaryWriter(output))
                {
                    WritePoint(writer, bounds.BoxCenter);
                    WritePoint(writer, bounds.BoxExtents);
                    writer.Flush();
                }
            }

            File.Replace(temporaryPath, path, null);
        }
        finally
        {
            if (File.Exists(temporaryPath))
                File.Delete(temporaryPath);
        }
    }

    static void ValidateStoredBounds(ModelBounds stored, ModelBounds calculated)
    {
        if (!NearlyEqual(stored.BoxCenter, calculated.BoxCenter) ||
            !NearlyEqual(stored.BoxExtents, calculated.BoxExtents))
            throw new InvalidDataException("저장된 바운드와 정점에서 계산한 바운드가 일치하지 않습니다.");
    }

    static bool NearlyEqual(Point3 lhs, Point3 rhs)
    {
        return NearlyEqual(lhs.X, rhs.X) && NearlyEqual(lhs.Y, rhs.Y) && NearlyEqual(lhs.Z, rhs.Z);
    }

    static bool NearlyEqual(float lhs, float rhs)
    {
        float scale = Math.Max(1f, Math.Max(Math.Abs(lhs), Math.Abs(rhs)));
        return Math.Abs(lhs - rhs) <= scale * 0.00001f;
    }

    static Point3 ReadPoint(BinaryReader input)
    {
        return new Point3(input.ReadSingle(), input.ReadSingle(), input.ReadSingle());
    }

    static void WritePoint(BinaryWriter output, Point3 point)
    {
        output.Write(point.X);
        output.Write(point.Y);
        output.Write(point.Z);
    }

    static Point3 Midpoint(Point3 lhs, Point3 rhs)
    {
        return new Point3(
            (lhs.X + rhs.X) * 0.5f,
            (lhs.Y + rhs.Y) * 0.5f,
            (lhs.Z + rhs.Z) * 0.5f);
    }

    static long CheckedByteCount(uint count, int stride)
    {
        return checked((long)count * stride);
    }

    static void SkipString(BinaryReader input)
    {
        uint length = input.ReadUInt32();
        Skip(input, length);
    }

    static void Skip(BinaryReader input, long count)
    {
        if (count < 0 || input.BaseStream.Position + count > input.BaseStream.Length)
            throw new EndOfStreamException("바이너리 모델 데이터가 예상보다 짧습니다.");

        input.BaseStream.Seek(count, SeekOrigin.Current);
    }

    static void CopyBytes(Stream input, Stream output, long count)
    {
        byte[] buffer = new byte[1024 * 1024];
        long remain = count;
        while (remain > 0)
        {
            int readSize = input.Read(buffer, 0, (int)Math.Min(buffer.Length, remain));
            if (readSize <= 0)
                throw new EndOfStreamException("바이너리 모델 복사 중 파일 끝에 도달했습니다.");

            output.Write(buffer, 0, readSize);
            remain -= readSize;
        }
    }
}
"@
}

$resolvedRoot = (Resolve-Path -LiteralPath $RootPath).Path
$modelFiles = @(if (Test-Path -LiteralPath $resolvedRoot -PathType Leaf)
    {
        Get-Item -LiteralPath $resolvedRoot
    }
    else
    {
        Get-ChildItem -LiteralPath $resolvedRoot -Recurse -File -Filter "*.bin" | Sort-Object FullName
    })

$staticCount = 0
$skeletalCount = 0
$writtenCount = 0
$staticVertexCount = 0L
$skeletalVertexCount = 0L
$animationSampleCount = 0L
$failures = [System.Collections.Generic.List[string]]::new()

for ($index = 0; $index -lt $modelFiles.Count; $index++)
{
    $file = $modelFiles[$index]
    Write-Progress -Activity "모델 바운드 베이킹" -Status $file.Name -PercentComplete (($index + 1) * 100 / $modelFiles.Count)

    try
    {
        $result = [ModelBoundsBaker]::Process($file.FullName, $VerifyOnly.IsPresent)
        if ($result.IsStatic)
        {
            $staticCount++
            $staticVertexCount += $result.VertexCount
        }
        else
        {
            $skeletalCount++
            $skeletalVertexCount += $result.VertexCount
            $animationSampleCount += $result.AnimationSampleCount
        }

        if ($result.WasWritten)
        {
            $writtenCount++
        }
    }
    catch
    {
        $failures.Add("$($file.FullName): $($_.Exception.Message)")
    }
}

Write-Progress -Activity "모델 바운드 베이킹" -Completed

[pscustomobject]@{
    RootPath = $resolvedRoot
    Mode = if ($VerifyOnly) { "Verify" } else { "Bake" }
    StaticModelCount = $staticCount
    SkeletalModelCount = $skeletalCount
    WrittenModelCount = $writtenCount
    TotalStaticVertexCount = $staticVertexCount
    TotalSkeletalVertexCount = $skeletalVertexCount
    TotalAnimationSampleCount = $animationSampleCount
    FailureCount = $failures.Count
}

if ($failures.Count -gt 0)
{
    $failures | ForEach-Object { Write-Error $_ }
    throw "모델 바운드 처리에 실패한 파일이 있습니다. count=$($failures.Count)"
}
