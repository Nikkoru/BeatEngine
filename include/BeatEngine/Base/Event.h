#pragma once

#include <memory>
#include <type_traits>
#include <typeindex>

namespace Base {
	class Event {
	public:
		Event() : ID(typeid(nullptr)) {}
		Event(std::type_index id) : ID(id) {}
		virtual ~Event() = default;
    public:
        template<typename TEvent>
            requires(std::is_base_of_v<Base::Event, TEvent>)
        bool Is() { return ID == typeid(TEvent); };

        template<typename TEvent>
            requires(std::is_base_of_v<Base::Event, TEvent>)
        std::shared_ptr<TEvent> GetIf() { if (Is<TEvent>()) return std::static_pointer_cast<TEvent>(this); }
	public:
		std::type_index ID;
	};
}
