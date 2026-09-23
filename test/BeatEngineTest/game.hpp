#pragma once

#include <BeatEngine/Application.hpp>

class Game : public Application {
private:
    int m_Argc{};
    char** m_Argv{};
public:
    Game(int argc, char** argv);
    ~Game() override = default;
public:
    void Init() override;
};
