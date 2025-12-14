#pragma once

#include "BeatEngine/Base/Signal.h"

class GameExitSignal : public Base::Signal {    
public:
    GameExitSignal() : Base::Signal(typeid(GameExitSignal)) {}
};
