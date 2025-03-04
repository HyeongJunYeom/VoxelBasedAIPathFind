#pragma once

#include "Engine_Defines.h"

BEGIN(Engine)

enum class ALGORITHM { _A_STAR, _GBFS, _DAIJKSTRA, _BFS, _END };
enum class MAIN_HEURISTIC { _EUCLIDEAN, _MANHATTAN, _END };
enum class HEURISTIC_TYPE { _DISTANCE, _HEIGHT, _OPENNESS, _END };
enum class TEST_MODE { _DEFAULT, _INFITINTE, _END };

enum class PATH_MODE { _SINGLE_AGENT, _MULTI_AGENT, _FORMATION, _END };

END