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

        void                        SetTexture(const std::shared_ptr<Texture>& texture) { _texture = texture; }
        std::shared_ptr<Texture>    GetTexture() const { return _texture; }

    protected:
        virtual void    OnInitialize() override;
        virtual void    OnRender(HDC hDC) override;

    private:
        std::shared_ptr<Texture> _texture{};

        Transform* _transform{};
    };
}