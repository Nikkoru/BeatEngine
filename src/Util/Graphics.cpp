#include "BeatEngine/Util/Graphics.hpp"

namespace {
struct PairHash{
    template<class T1, class T2>
    size_t operator()(const std::pair<T1, T2>& p) const {
        auto h1 = std::hash<T1>{}(p.first);       
        auto h2 = std::hash<T2>{}(p.second);       

        return h1 ^ (h2 + 0x9E3779B9 + (h1 << 6) + (h1 >> 2));
    }
};
}

VertexArray Util::GetOutlineVertices(const VertexArray &vertices) {
    std::unordered_map<std::pair<uint32_t, uint32_t>, int, PairHash> edgeMap;

    for (size_t i = 0; i < vertices.GetSize(); i += 3) {
        auto v0 = i;
        auto v1 = i + 1;
        auto v2 = i + 2;

        std::pair<uint32_t, uint32_t> e0 { std::min(v0, v1), std::max(v0, v1) };
        std::pair<uint32_t, uint32_t> e1 { std::min(v1, v2), std::max(v1, v2) };
        std::pair<uint32_t, uint32_t> e2 { std::min(v2, v0), std::max(v2, v0) };

        edgeMap[e0]++;
        edgeMap[e1]++;
        edgeMap[e2]++;
    }

    std::unordered_map<uint32_t, uint32_t> idMap;
    uint32_t nextOutputIndex{};
    VertexArray out{};

    auto getOrCreateOutputVertex = [&](uint32_t originalID) -> uint32_t {
        auto it = idMap.find(originalID);

        if (it != idMap.end()) {
            return it->second;
        }

        out.EmplaceBack(vertices[originalID]);
        idMap[originalID] = nextOutputIndex;
        return nextOutputIndex++;
    };

    for (const auto& [edge, count] : edgeMap) {
        getOrCreateOutputVertex(edge.first);
        getOrCreateOutputVertex(edge.second);
    }

    return out;
}
