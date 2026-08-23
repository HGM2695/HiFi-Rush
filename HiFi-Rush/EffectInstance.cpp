#include "EffectInstance.h"

#include "EffectRuntimeObject.h"
#include "GameObject.h"

#include <utility>

namespace gm
{
	bool EffectInstance::IsValid() const
	{
		for (const WeakGameObjectPtr& trackObject : _trackObjects)
		{
			if (trackObject)
				return true;
		}
		return false;
	}

	bool EffectInstance::SetWorldTransform(const Matrix& world)
	{
		bool hasValidTrack = false;
		for (const WeakGameObjectPtr& trackObject : _trackObjects)
		{
			EffectRuntimeObject* runtimeObject = dynamic_cast<EffectRuntimeObject*>(trackObject.Get());
			if (runtimeObject == nullptr)
				continue;
			runtimeObject->SetWorldTransform(world);
			hasValidTrack = true;
		}
		return hasValidTrack;
	}

	bool EffectInstance::SetOpacity(float opacity)
	{
		bool hasValidTrack = false;
		for (const WeakGameObjectPtr& trackObject : _trackObjects)
		{
			EffectRuntimeObject* runtimeObject = dynamic_cast<EffectRuntimeObject*>(trackObject.Get());
			if (runtimeObject == nullptr)
				continue;
			runtimeObject->OverrideOpacity(opacity);
			hasValidTrack = true;
		}
		return hasValidTrack;
	}

	void EffectInstance::Stop()
	{
		for (WeakGameObjectPtr& trackObject : _trackObjects)
		{
			GameObject* object = trackObject.Get();
			if (object)
				object->Destroy();
			trackObject.Reset();
		}
		_trackObjects.clear();
	}

	void EffectInstance::SetTrackObjects(std::vector<WeakGameObjectPtr> trackObjects)
	{
		Stop();
		_trackObjects = std::move(trackObjects);
	}
}
