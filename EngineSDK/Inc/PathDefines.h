#pragma once

#include "Engine_Defines.h"

BEGIN(Engine)

typedef array<_float, static_cast<_uint>(HEURISTIC_TYPE::_END)>			HEURISTIC_WEIGHTS;
typedef unordered_map<_uint, NODE_INFO>									NODE_INFOS;

END