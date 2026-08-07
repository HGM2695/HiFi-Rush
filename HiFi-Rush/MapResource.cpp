#include "MapResource.h"

#include <utility>

namespace gm
{
	std::shared_ptr<MapResource> MapResource::Create(MapData&& data)
	{
		GM_ASSERT_RETURN_VAL(data.objects.empty() == false, nullptr, "MapResource의 환경 오브젝트 데이터가 비어 있습니다.");
		return std::shared_ptr<MapResource>(new MapResource(std::move(data)));
	}

	MapResource::MapResource(MapData&& data)
		: _data(std::move(data))
	{}
}
