#pragma once

#include "Engine_Defines.h"

BEGIN(Engine)

typedef struct tagVoxel
{
	_byte		bID = { -1 };
	_byte		bState = { -1 };
	_byte		bOpenessScore = { -1 };
	_int		iNeighborFlag = {};		//	��Ʈ �÷��׸� �̿��Ͽ� �̿����� 26�� 26��Ʈ Ȱ��
} VOXEL;

END