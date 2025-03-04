#pragma once

BEGIN(Engine)

typedef struct tagNodeInfo
{
	_uint		iParentIndex = {};
	_float		fG_Cost = { FLT_MAX };
} NODE_INFO;


typedef struct tagCommandInfo
{
	PATH_MODE			ePathMode = { PATH_MODE::_END };
	_uint				iGoalIndex = {};
	_uint				iStartIndex = {};
	list<_uint>			BestPathIndices;
}COMMAND_INFO;

typedef struct tagCommandInfoFORMATION : public COMMAND_INFO
{
	_uint				iFormationIndex = {};
} COMMAND_INFO_FORMATION;

typedef struct tagCommandInfoSAPF : public COMMAND_INFO
{
} COMMAND_INFO_SAPF;

typedef struct tagCommandInfoMAPF : public COMMAND_INFO
{
	_uint				iFlowIndex = {};
} COMMAND_INFO_MAPF;

END