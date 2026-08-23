#pragma once

#include "MathTypes.h"
#include "WeakGameObjectPtr.h"

#include <vector>

namespace gm
{
	class EffectSpawner;

	class EffectInstance
	{
	public:
		bool IsValid() const;
		bool SetWorldTransform(const Matrix& world);
		bool SetOpacity(float opacity);
		void Stop();

	private:
		friend EffectSpawner;
		void SetTrackObjects(std::vector<WeakGameObjectPtr> trackObjects);

		std::vector<WeakGameObjectPtr> _trackObjects{};
	};
}
