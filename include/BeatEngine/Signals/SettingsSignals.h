#pragma once

#include "BeatEngine/Base/Signal.h"
#include "BeatEngine/Base/Settings.h"

#include <typeindex>
#include <memory>
 
class SetSettingsSignal : public Base::Signal {
public:
    std::type_index SettingsID;
    std::shared_ptr<Base::Settings> Settings;     
public:
    SetSettingsSignal(std::type_index id, std::shared_ptr<Base::Settings> settings) : SettingsID(id), Settings(settings) {}
};
