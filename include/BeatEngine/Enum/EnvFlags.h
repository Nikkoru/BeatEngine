#pragma once

using EnvFlags = int;

enum EnvFlags_ {
    EnvFlags_None                      = 0,
    EnvFlags_Debug                     = 1 << 0,
    EnvFlags_TestBuild                 = 1 << 1
};
