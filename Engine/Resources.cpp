#include "Resources.h"

namespace gm
{
    bool Resources::Add(const std::wstring& key, const std::shared_ptr<Resource>& resource)
    {
        GM_ASSERT_RETURN_VAL(key.empty() == false, false, "리소스 키가 비어 있습니다.");
        GM_ASSERT_RETURN_VAL(resource, false, "resource가 nullptr입니다.");
        GM_ASSERT_RETURN_VAL(_resourceList.find(key) == _resourceList.end(), false, "이미 같은 이름의 리소스가 존재합니다.");

        resource->SetName(key);
        _resourceList.emplace(key, resource);
        return true;
    }

    std::shared_ptr<Resource> Resources::FindBase(const std::wstring& key) const
    {
        auto it = _resourceList.find(key);
        if (it == _resourceList.end())
            return nullptr;

        return it->second;
    }

    bool Resources::Unload(const std::wstring& key)
    {
        return _resourceList.erase(key) > 0;
    }

    void Resources::Clear()
    {
        _resourceList.clear();
    }
}