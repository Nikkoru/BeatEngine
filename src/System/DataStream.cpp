#include "BeatEngine/System/DataStream.hpp"
#include "BeatEngine/Logger.h"
#include "BeatEngine/Util/Optional.hpp"
#include <cstring>
#include <filesystem>
#include <fstream>

bool DataStream::StartReadForFile(const std::filesystem::path& path) {
    m_MemoryPointData = nullptr;
    m_Type = Type::File;

    m_FileStream = std::ifstream{ path, std::ios::binary };

    if (!m_FileStream.is_open()) {
        Logger::AddError("", "Failed to create DataStream, cannot open file \"{}\"", path.string());
        return false;
    }

    m_FileStream.seekg(0, std::ios::end);
    m_Size = m_FileStream.tellg();
    m_FileStream.seekg(0, std::ios::beg);
    m_Offset = 0;

    return true;
}

void DataStream::StartReadForMemory(void* data, size_t sizeInBytes) {
    if (m_FileStream.is_open()) {
        m_FileStream.close();
        m_FileStream = {};
    }
    m_Type = Type::Memory;

    m_MemoryPointData = static_cast<std::byte*>(data);
    m_Size = sizeInBytes;
}

Optional<size_t> DataStream::Read(void* readData, size_t sizeToRead) {
    switch (m_Type) {
    default:
    case Type::None:
        return std::nullopt;
    case Type::File:
        if (!m_FileStream.is_open()) return std::nullopt;

        m_FileStream.read(static_cast<char*>(readData), sizeToRead);
        return static_cast<size_t>(m_FileStream.gcount()); 
    case Type::Memory:
        if (!m_MemoryPointData) return std::nullopt;

        const std::size_t count = std::min(sizeToRead, m_Size - m_Offset);
        if (count > 0) {
            memcpy(readData, m_MemoryPointData + m_Offset, sizeToRead);
            m_Offset += count;
        }

        return static_cast<size_t>(count);
    };
}

Optional<size_t> DataStream::Seek(size_t pos) {
    switch (m_Type) {
    default:
    case Type::None:
        return std::nullopt;
    case Type::File:
        if (!m_FileStream.is_open()) return std::nullopt;
        m_FileStream.clear();
        m_FileStream.seekg(pos, std::ios::beg);
        if (auto realPos = TellPos().Value(); realPos > 0)
            m_Offset = realPos;
        return TellPos();
    case Type::Memory:
        if (!m_MemoryPointData) return std::nullopt;

        m_Offset = pos < m_Size ? pos : m_Size;
        break;
    };

    return m_Offset;
}

Optional<size_t> DataStream::TellPos() {
    switch (m_Type) {
    default:
    case Type::None:
        return std::nullopt;
    case Type::File:
        if (!m_FileStream.is_open()) return std::nullopt;
        auto pos = m_FileStream.tellg();
        return pos < 0 ? std::nullopt : Optional<size_t>(static_cast<size_t>(pos));
    }
}
