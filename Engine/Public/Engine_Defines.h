#pragma once

#pragma warning (disable : 4251)

#include <d3d11.h>
#include <DirectXMath.h>
#include <d3dcompiler.h>
#include <DirectXCollision.h>

#define DIRECTINPUT_VERSION 0x0800

#define		VK_MAX			0xff

#define		EPSILON			0.00001f
#define		EPSILON_MIN		0.05f


#include "Effects11/d3dx11effect.h"
#include "DirectXTK/DDSTextureLoader.h"
#include "DirectXTK/WICTextureLoader.h"
#include "DirectXTK/ScreenGrab.h"
#include "DirectXTK/PrimitiveBatch.h"
#include "DirectXTK/VertexTypes.h"
#include "DirectXTK/Effects.h"

#include "DirectXTK/SpriteBatch.h"
#include "DirectXTK/SpriteFont.h"

#include "assimp/scene.h"
#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"

#include <random>
#include <chrono>
#include <thread>

using namespace DirectX;

#include <iostream>

#include <algorithm>
#include <string>
#include <vector>
#include <array>
#include <list>
#include <map>
#include <set>
#include <queue>
#include <stack>
#include <unordered_map>
#include <unordered_set>
#include <memory.h>
#include <utility>
#include <thread>

/* FileSystem */
#include <filesystem>
#include <iostream>
#include <cstdlib>

#include <fstream>

/* ImGui */
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"

using namespace std;

namespace Engine
{
	const wstring		g_strTransformTag = { TEXT("Com_Transform") };
	const wstring		g_strObstacleTag = { TEXT("Com_Obstacle") };
	const wstring		g_strModelTag = { TEXT("Com_Model") };
	const wstring		g_strAnimModelTag = { TEXT("Com_AnimModel") };
	const wstring		g_strShaderModelTag = { TEXT("Com_Shader_Model") };
	const wstring		g_strShaderAnimModelTag = { TEXT("Com_Shader_AnimModel") };
	const wstring		g_strNavigationTag = { TEXT("Com_Navigaiton") };
}

#include "Engine_Macro.h"
#include "Engine_Function.h"
#include "Engine_Typedef.h"
#include "Engine_Struct.h"
#include "Engine_Enums.h"

#include "VoxelIncludes.h"
#include "PathIncludes.h"

using namespace Engine;

#ifdef _DEBUG

#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#ifndef DBG_NEW 

#define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ ) 
#define new DBG_NEW 

#endif

#endif // _DEBUG