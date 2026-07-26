#pragma once

#include "BeatEngine/Util/Optional.hpp"
#include <filesystem>
#include <fstream>
class DataStream {
public:
    enum class Type {
        None = 0,
        File,
        Memory
    };
public:
    DataStream() = default;
    DataStream(void* data, size_t sizeInBytes) { StartReadForMemory(data, sizeInBytes); }
    DataStream(const std::filesystem::path& path) { StartReadForFile(path); }

    bool StartReadForFile(const std::filesystem::path& path);
    void StartReadForMemory(void* data, size_t sizeInBytes);

    Optional<size_t> Read(void* readData, size_t sizeToRead);
    Optional<size_t> Seek(size_t pos);
    Optional<size_t> TellPos();

    size_t GetSize() { return m_Size; };
private:
    Type m_Type{ Type::None };
    std::ifstream m_FileStream{};
    std::byte* m_MemoryPointData{ nullptr };
    size_t m_Size{};
    size_t m_Offset{};
};
