#include "Resources.h"

namespace gm
{
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