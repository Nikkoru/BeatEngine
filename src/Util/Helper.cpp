#include "BeatEngine/Util/Helper.h"
#include <cstring>

bool Helper::ParseBoolFromString(const char *str) {
    return strlen(str) == 5;
}
