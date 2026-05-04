#pragma once

#include <unordered_map>
#include <string>
#include <type_traits>

#include "Resource.h"

namespace gm
{
    class Resources
    {
    public:
        Resources() = default;
        ~Resources() = default;

        // 없으면 nullptr / 타입 다르면 nullptr
        template<typename T>
        std::shared_ptr<T> Find(const std::wstring& key) const
        {
            static_assert(std::is_base_of_v<Resource, T>, "T는 반드시 Resource를 상속해야 합니다.");

            auto base = FindBase(key);
            if (base == nullptr || base->GetType() != T::Type)
                return nullptr;

            return std::static_pointer_cast<T>(base);
        }

        // 있으면 기존 반환, 없으면 생성 + Load + 캐싱
        template<typename T, typename TDesc>
        std::shared_ptr<T> Load(const std::wstring& key, const TDesc& desc)
        {
            static_assert(std::is_base_of_v<Resource, T>, "T는 반드시 Resource를 상속해야 합니다.");

            auto base = FindBase(key);
            if (base)
            {
                GM_ASSERT_RETURN_VAL(base->GetType() == T::Type, nullptr, "존재하지만 타입이 일치하지 않습니다.");
                return std::static_pointer_cast<T>(base);
            }

            std::shared_ptr<Resource> resource = T::Create(desc);
            GM_ASSERT_RETURN_VAL(resource, nullptr, "Load 실패");

            resource->SetName(key);

            _resourceList.emplace(key, resource);
            return std::static_pointer_cast<T>(resource);
        }

        bool    Add(const std::wstring& key, const std::shared_ptr<Resource>& resource);
        bool    Unload(const std::wstring& key);
        void    Clear();
        size_t  Count() const { return _resourceList.size(); }

    private:
        std::shared_ptr<Resource> FindBase(const std::wstring& key) const;

    private:
        std::unordered_map<std::wstring, std::shared_ptr<Resource>> _resourceList;
    };
}
