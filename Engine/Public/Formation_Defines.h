#pragma once

BEGIN(Engine)

const _int Formation_TwoLine_Offset_Width[] = {
	-1, 1,
	-1, 1,
	-1, 1,
	-1, 1,
	-1, 1,
	-1, 1,
	-1, 1,
	-1, 1,
	-1, 1,
	-1, 1,
	-1, 1,
	-1, 1,
	-1, 1,
	-1, 1,
	-1, 1,
	-1, 1
};

const _int Formation_TwoLine_Offset_Depth[] = {
	0, -1,
	-2, -3,
	-4, -5,
	-6, -7,
	-8, -9,
	-10, -11,
	-12, -13,
	-14, -15,
	-16, -17,
	-18, -19,
	-20, -21,
	-22, -23,
	-24, -25,
	-26, -27,
	-28, -29,
	-30, -31
};


const _int Formation_FourLine_Offset_Width[] = {
	-3, -1, 1, 3,
	-3, -1, 1, 3,
	-3, -1, 1, 3,
	-3, -1, 1, 3,
	-3, -1, 1, 3,
	-3, -1, 1, 3,
	-3, -1, 1, 3,
	-3, -1, 1, 3
};

const _int Formation_FourLine_Offset_Depth[] = {
	0, 0, 0, 0,
	-2, -2, -2, -2,
	-4, -4, -4, -4,
	-6, -6, -6, -6,
	-8, -8, -8, -8,
	-10, -10, -10, -10,
	-12, -12, -12, -12,
	-14, -14, -14, -14
};

const _int Formation_EightLine_Offset_Width[] = {
	-7, -5, -3, -1, 1, 3, 5, 7,
	-7, -5, -3, -1, 1, 3, 5, 7,
	-7, -5, -3, -1, 1, 3, 5, 7,
	-7, -5, -3, -1, 1, 3, 5, 7
};

const _int Formation_EightLine_Offset_Depth[] = {
	0, 0, 0, 0, 0, 0, 0, 0,
	-2, -2, -2, -2, -2, -2, -2, -2,
	-4, -4, -4, -4, -4, -4, -4, -4,
	-6, -6, -6, -6, -6, -6, -6, -6
};


const _int Formation_Circle_Offset_Width[] = {
	-1, 1,
	-2, 2,
	-3, 3,
	-5, 5,
	-5, 5,
	-3, 3,
	-2, 2,
	-1, 1
};

const _int Formation_Circle_Offset_Depth[] = {
	0, 0,
	-2, -2,
	-2, -2,
	-4, -4,
	-6, -6,
	-8, -8,
	-8, -8,
	-10, -10
};

END