#pragma once

#include "Component.h"
#include <memory>

namespace gm
{
    class Transform;
    class Texture;

    class SpriteRenderer : public Component
    {
    public:
        SpriteRenderer();
        virtual ~SpriteRenderer();

        virtual void    OnInitialize() override;
        virtual void    OnUpdate() override;
        virtual void    OnLateUpdate() override;
        virtual void    OnRender(HDC hDC) override;

        void                        SetTexture(const std::shared_ptr<Texture>& texture) { _texture = texture; }
        std::shared_ptr<Texture>    GetTexture() const { return _texture; }

    private:
        std::shared_ptr<Texture> _texture{};

        Transform* _transform{};
    };
}