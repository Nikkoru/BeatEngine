#pragma once

#include "BeatEngine/Base/Signal.h"
#include "BeatEngine/Base/Settings.h"

#include <memory>
#include <typeindex>
 
class SetSettingsSignal : public Base::Signal {
public:
    std::type_index SettingsID;
    std::shared_ptr<Base::Settings> Settings;     
public:
    SetSettingsSignal(std::type_index id, std::shared_ptr<Base::Settings> settings) : Base::Signal(typeid(SetSettingsSignal)),  SettingsID(id), Settings(settings) {}
};
