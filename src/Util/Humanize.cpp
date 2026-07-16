#include "BeatEngine/Util/Humanize.hpp"
#include <format>

std::string Humanize::FromSeconds(int secs, bool withLetter) {
    auto min = static_cast<int>(secs / 60);
    auto sec = secs - min * 60;
    auto hour = static_cast<int>(min / 60);

    if (min / 60 >= 1)
        hour += static_cast<int>(min / 60);

    std::string str{};

    if (hour >= 1) {
        std::string letter = "";
        withLetter ? letter = "h" : letter = "";
        str.append(std::format("{:02}{}:", hour, letter));
    }

    std::string letter = "";
    withLetter ? letter = "m" : letter = "";
    str.append(std::format("{:02}{}:", min, letter));

    withLetter ? str.append(std::format("{:02}s", sec)) : str.append(std::format("{:02}", sec));

    return str;
}
