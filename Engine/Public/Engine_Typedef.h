#pragma once

/* ���� ����ϴ� Ÿ�Ե��� �̸� ���� �̸����� ������ �س��´�. */
#include <stdint.h>

namespace Engine
{
	typedef unsigned char				_ubyte;
	typedef signed char					_byte;
	typedef char						_char;
	typedef wchar_t						_tchar;

	typedef unsigned short				_ushort;
	typedef signed short				_short;

	typedef unsigned int				_uint;
	typedef signed int					_int;

	typedef unsigned long				_ulong;
	typedef signed long					_long;

	typedef unsigned long				_ullong;
	typedef signed long					_llong;

	typedef float						_float;
	typedef double						_double;

	typedef bool						_bool;

	typedef XMFLOAT2					_float2;
	typedef XMFLOAT3					_float3;
	typedef XMFLOAT4					_float4;
	typedef XMVECTOR					_vector;
	typedef FXMVECTOR					_fvector;
	typedef GXMVECTOR					_gvector;
	typedef HXMVECTOR					_hvector;
	typedef CXMVECTOR					_cvector;

	typedef XMFLOAT4X4					_float4x4;
	typedef XMMATRIX					_matrix;
	typedef FXMMATRIX					_fmatrix;

	typedef XMINT3						_int3;
	typedef XMUINT3						_uint3;

	typedef XMINT4						_int4;
	typedef XMUINT4						_uint4;
	
}