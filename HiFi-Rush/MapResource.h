#pragma once

#include "MapTypes.h"
#include <Resource.h>

#include <memory>

namespace gm
{
	class MapResource final : public Resource
	{
	public:
		static std::shared_ptr<MapResource> Create(MapData&& data);

		static inline ResourceType Type = ResourceType::Map;
		ResourceType GetType() const override { return Type; }

		const MapData& GetData() const { return _data; }

	private:
		explicit MapResource(MapData&& data);

	private:
		MapData _data{};
	};
}
