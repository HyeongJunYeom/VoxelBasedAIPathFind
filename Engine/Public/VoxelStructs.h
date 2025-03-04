#pragma once

#include "Engine_Defines.h"

BEGIN(Engine)

typedef struct tagVoxel
{
	_byte		bID = { -1 };
	_byte		bState = { -1 };
	_byte		bOpenessScore = { -1 };
	_int		iNeighborFlag = {};		//	비트 플래그를 이용하여 이웃정보 26방 26비트 활용
} VOXEL;

END