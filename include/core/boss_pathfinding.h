#pragma once
#include <vector>
#include <queue>
#include <utility>
#include <cmath>
struct Level; // declaração adiantada para evitar include circular
//Retorna o caminho mais curto entre start e goal usando BFS, ignorando inimigos e considerando '1' e '2' como paredes
std::vector<std::pair<int,int>> bossFindPath(const Level& lvl, float startX, float startZ, float goalX, float goalZ);
