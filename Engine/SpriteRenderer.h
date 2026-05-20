#pragma once

#include "Component.h"
#include "SpriteFrame.h"

namespace gm
{
    class Transform;
    class Texture;
    class Material;
    struct MaterialDesc;

    class SpriteRenderer : public Component
    {
    public:
        SpriteRenderer();
        virtual ~SpriteRenderer();

        virtual TickGroup	        GetTickGroup() const { return TickGroup::RenderSubmit; }

        void                        SetTexture(const std::shared_ptr<Texture>& texture);
        void                        SetMaterial(const MaterialDesc& desc);
        void                        SetMaterial(const Material& material);

        void                        SetSourceRect(const SpriteFrame& frame);
        void                        DisableSourceRect();

        std::shared_ptr<Texture>    GetTexture(uint32 slot = 0) const;
        Material*                   GetMaterial() const { return _material.get(); }

    protected:
        virtual void    OnInitialize() override;
        virtual void    OnRender() override;

    private:
        void            EnsureDefaultMaterial();

    private:
        std::unique_ptr<Material>   _material;
        Transform*                  _ownerTransform{};
        bool                        _useSourceRect = false;
        SpriteFrame                 _sourceFrame{};
    };
}
