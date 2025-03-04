#pragma once


#include "GameInstance.h"

#ifndef _DEBUG
#define USE_MAP_DEFAULT
#endif
namespace Client
{
	enum LEVEL { LEVEL_STATIC, LEVEL_LOADING, LEVEL_LOGO, LEVEL_GAMEPLAY, LEVEL_END };

	const unsigned int	g_iWinSizeX = 1920;
	const unsigned int	g_iWinSizeY = 1080;
}


extern HWND g_hWnd;
extern HINSTANCE g_hInst;

using namespace std;
using namespace Client;

#ifdef UNICODE
#pragma comment(linker, "/entry:wWinMainCRTStartup /subsystem:console")
#else
#pragma comment(linker, "/entry:WinMainCRTStartup /subsystem:console")
#endif