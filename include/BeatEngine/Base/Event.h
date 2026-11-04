#pragma once

#include "BeatEngine/Util/Optional.hpp"
#include <optional>
#include <type_traits>
#include <typeindex>

namespace Base {
struct Event {
    Event() : ID(typeid(nullptr)) {}
    Event(std::type_index id) : ID(id) {}
    virtual ~Event() = default;

    template<typename TEvent>
        requires(std::is_base_of_v<Base::Event, TEvent>)
    bool Is() { return ID == typeid(TEvent); };

    template<typename TEvent>
        requires(std::is_base_of_v<Base::Event, TEvent>)
    static Optional<TEvent> GetIf(Optional<Event> e) {
        if (e.HasValue() && e->Is<TEvent>()) {
            return e.StaticCastTo<TEvent>();
        }
        else return std::nullopt;
    }

    std::type_index ID;
};
}
