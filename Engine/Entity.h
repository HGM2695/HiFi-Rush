#pragma once
#include "string"

namespace gm
{
	class Entity
	{
	public:
		Entity() = default;
		virtual ~Entity() = default;

		void				SetName(const std::wstring& name) { _name = name; }
		const std::wstring& GetName() const { return _name; }

	private:
		std::wstring _name{};
	};
}


