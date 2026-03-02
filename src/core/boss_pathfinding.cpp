#include "core/boss_pathfinding.h"
#include "level/level.h"
#include <algorithm>

// Simple BFS for shortest path on grid
std::vector<std::pair<int,int>> bossFindPath(const Level& lvl, float startX, float startZ, float goalX, float goalZ) {
    const auto& data = lvl.map.data();
    int H = lvl.map.getHeight();
    int W = lvl.map.getWidth();
    float tile = lvl.metrics.tile;
    float offX = lvl.metrics.offsetX;
    float offZ = lvl.metrics.offsetZ;

    auto toIdx = [&](float x, float z) {
        int tx = (int)((x - offX) / tile);
        int tz = (int)((z - offZ) / tile);
        return std::make_pair(tx, tz);
    };
    auto toWorld = [&](int tx, int tz) {
        float wx = offX + tx * tile + tile * 0.5f;
        float wz = offZ + tz * tile + tile * 0.5f;
        return std::make_pair(wx, wz);
    };

    auto [sx, sz] = toIdx(startX, startZ);
    auto [gx, gz] = toIdx(goalX, goalZ);
    if (sx < 0 || sz < 0 || sx >= W || sz >= H || gx < 0 || gz < 0 || gx >= W || gz >= H)
        return {};

    std::vector<std::vector<bool>> visited(H, std::vector<bool>(W, false));
    std::vector<std::vector<std::pair<int,int>>> parent(H, std::vector<std::pair<int,int>>(W, {-1,-1}));
    std::queue<std::pair<int,int>> q;
    q.push({sx, sz});
    visited[sz][sx] = true;

    int dxs[4] = {1, -1, 0, 0};
    int dzs[4] = {0, 0, 1, -1};

    while (!q.empty()) {
        auto [x, z] = q.front(); q.pop();
        if (x == gx && z == gz) break;
        for (int d = 0; d < 4; ++d) {
            int nx = x + dxs[d];
            int nz = z + dzs[d];
            if (nx < 0 || nz < 0 || nx >= W || nz >= H) continue;
            char c = data[nz][nx];
            if (c == '1' || c == '2') continue;
            if (!visited[nz][nx]) {
                visited[nz][nx] = true;
                parent[nz][nx] = {x, z};
                q.push({nx, nz});
            }
        }
    }
    // reconstruct path
    std::vector<std::pair<int,int>> path;
    int x = gx, z = gz;
    if (!visited[z][x]) return {};
    while (x != sx || z != sz) {
        path.push_back({x, z});
        auto [px, pz] = parent[z][x];
        x = px; z = pz;
    }
    std::reverse(path.begin(), path.end());
    return path;
}
