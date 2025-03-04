#pragma once

#include "Engine_Defines.h"

BEGIN(Engine)

#pragma region Default Setup Constant
const _uint			g_iWorldLength = { 1024 };
const _uint			g_iSectorLength = { 64 };


#ifdef _DEBUG
const _uint			g_iTerrainMaxHeight = { 128 };
const _float		g_fDefaultVoxelLength = { 2.0f };
#else
const _uint			g_iTerrainMaxHeight = { 128 };
const _float		g_fDefaultVoxelLength = { 0.5f };
#endif
const _float		g_fVoxelSizeMinLimit = { 0.25f };
#pragma endregion

//	For.CullDist Voxel
const _float		g_fDefaultVoxelCullDist = { 100.f };

const _uint			g_iNumVoxelInstancePool = { 1 };



////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////




////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

//	For. Update ( 추후 수정사항 )
//	For. U Int System 
const _uint			g_iShift_X = { 0 };
const _uint			g_iShift_Y = { 10 };
const _uint			g_iShift_Z = { 20 };

const _uint			g_iVoxelPosLimit = { (1 << 10) - 1 };
const _uint			g_iVoxelIndexLimit = { (1 << 30) - 1 };

const _uint			g_iConvertFlagX = { 0x03ff };		//	하위 10개 비트
const _uint			g_iConvertFlagY = { g_iConvertFlagX << 10 };
const _uint			g_iConvertFlagZ = { g_iConvertFlagY << 10 };

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

//	For. Update ( 추후 수정사항 )
//	For. U long long System 
//const _ullong		g_llShift_X = { 0 };
//const _ullong		g_llShift_Y = { 20 };
//const _ullong		g_llShift_Z = { 40 };
//						  
//const _ullong		g_llVoxelPosLimit = { (1 << 20) - 1 };
//const _ullong		g_llVoxelIndexLimit = { (1 << 60) - 1 };
//						  
//const _ullong		g_llConvertFlagX = { 0xfffff };		//	하위 20개 비트
//const _ullong		g_llConvertFlagY = { g_llConvertFlagX << 20 };
//const _ullong		g_llConvertFlagZ = { g_llConvertFlagY << 20 };


/////////// 섹터 비트 시프트 /////////////////////////////////////////////
/////////// 섹터 비트 시프트 /////////////////////////////////////////////
/////////// 섹터 비트 시프트 /////////////////////////////////////////////
/////////// 섹터 비트 시프트 /////////////////////////////////////////////
/////////// 섹터 비트 시프트 /////////////////////////////////////////////
//	미구현



//	이웃 플래그

//	XYZ 순서 
//	X: Left Center Right
//	Y: Top Center Bottom
//	Z: Front Center Back

const _uint			g_iLBB = { 1 };
const _uint			g_iCBB = { 1 << 1 };
const _uint			g_iRBB = { 1 << 2 };
const _uint			g_iLCB = { 1 << 3 };
const _uint			g_iCCB = { 1 << 4 };
const _uint			g_iRCB = { 1 << 5 };
const _uint			g_iLTB = { 1 << 6 };
const _uint			g_iCTB = { 1 << 7 };
const _uint			g_iRTB = { 1 << 8 };

const _uint			g_iLBC = { 1 << 9 };
const _uint			g_iCBC = { 1 << 10 };
const _uint			g_iRBC = { 1 << 11 };
const _uint			g_iLCC = { 1 << 12 };
const _uint			g_iCCC = { 1 << 13 };
const _uint			g_iRCC = { 1 << 14 };
const _uint			g_iLTC = { 1 << 15 };
const _uint			g_iCTC = { 1 << 16 };
const _uint			g_iRTC = { 1 << 17 };

const _uint			g_iLBF = { 1 << 18 };
const _uint			g_iCBF = { 1 << 19 };
const _uint			g_iRBF = { 1 << 20 };
const _uint			g_iLCF = { 1 << 21 };
const _uint			g_iCCF = { 1 << 22 };
const _uint			g_iRCF = { 1 << 23 };
const _uint			g_iLTF = { 1 << 24 };
const _uint			g_iCTF = { 1 << 25 };
const _uint			g_iRTF = { 1 << 26 };

END