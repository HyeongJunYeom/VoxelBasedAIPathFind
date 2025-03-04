#pragma once

namespace Engine
{
	/* For.API_Input */
	enum EKeyState { NONE, DOWN, PRESSING, UP };

	enum class VTX_CUBE_PASS { _TEX, _WIRE_FLOOR, _WIRE_WALL, _SOL_FLOOR, _SOL_WALL, _END};

	enum class SHADER_PASS_DEFERRED
	{
		PASS_DEBUG,
		PASS_LIGHT_DIRECTIONAL,
		PASS_LIGHT_POINT,
		PASS_FINAL,
		PASS_SSAO,
		PASS_SSAO_BLUR,
		PASS_END
	};
}