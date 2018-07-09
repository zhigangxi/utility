#pragma once
#include "scene_manager.h"
#include "../common/include/singleton.h"

#include <stdio.h>
#include <map>
#include <vector>
#include <string>
#include <boost/shared_ptr.hpp>
#include <tchar.h>
#include "LogicEngine.h"
#include "logicdef.h"
#include <boost/multi_array.hpp>
#ifdef _DEBUG
#define LOGINENGINE_DLL	_T("LogicEngineND.dll")
#else
#define LOGINENGINE_DLL	_T("LogicEngineN.dll")
#endif
#define TERRAIN_FLAG *(DWORD*)"TERN"
struct TerrainHeader
{
	DWORD dwFlag;
	DWORD dwVersion;					//Currently 800
};

// 地形基本信息
struct TerrainInfo
{
	short m_Width;		// 水平格子数
	short m_Height;		// 垂直格子数
	float m_TileSize;	// 格子大小
	float m_LevelSize;	// 每个高度阶的实际数值
};

typedef boost::multi_array<bool,2> ObsArray;
struct SCEINFO
{
	int scene_id_;
	int scene_type_;
	string scene_name_;
	string map_name_;
	int col_,row_;
	ObsArray obstruct_info_;//障碍信息
	SCEINFO():col_(0),row_(0){}
	SCEINFO& operator = (const SCEINFO& info)
	{
		scene_id_ = info.scene_id_;
		scene_type_ = info.scene_type_;
		scene_name_ = info.scene_name_;
		map_name_ = info.map_name_;
		col_ = info.col_;
		row_ = info.row_;
		obstruct_info_.resize(boost::extents[info.col_][info.row_]);
		obstruct_info_ = info.obstruct_info_;
		return *this;
	}
};


class server_data:public singleton<server_data>
{
public:
	server_data()
	{
		m_hLogicEngine = LoadLibrary(LOGINENGINE_DLL);
		if(!m_hLogicEngine)
		{
			std::cout<<"无法装载动态连接库"<<std::endl;
		}
		LPCSTR TheWorld=("_GetTheWorld@0");
		FnGetTheWorld pFnGetTheWorld = (FnGetTheWorld)GetProcAddress( m_hLogicEngine, TheWorld);
		if(!pFnGetTheWorld)
		{
			std::cout<<"无法取得入口函数 "<<std::endl;
		}
		m_pTheWorld = (*pFnGetTheWorld)();
		m_pTheWorld->LoadFromFile(_T("..\\..\\..\\release\\JLinData\\Data\\极岛战纪.xml"),NULL);
	};
	~server_data() {};
public:
	void LoadMapMonsterData(std::string map_name_);
	void LoadServerData();
	void LoadMapTypeData();
	void LoadObstructInfo( SCEINFO& info );
	bool IsObstruct( string mapname,int x, int z );
	void Start();
	int  CreateID();
	bool LoadTerrain(const SCEINFO& info);//加载地形信息
	bool GetTerrainInfo(string mapname,TerrainInfo& info);
	bool MatchMap( string mapname, monster_map** pMap );
public:


	//std::string map_name_;
	monster_map monster_map_b0003;
	monster_map monster_map_b0004;
	cWorld*	m_pTheWorld;
	cBaseNode*  m_pWorldRoot;
	cLeafNode * pLeafNode;
	HMODULE m_hLogicEngine;
	SCEINFO sceneinfo;
	std::map<int,SCEINFO> sceinfo_map;
	std::map<int,Logic::UpgradeData_t> UpgradeInfo_map;
	std::map<int,Logic::ItemData_t> UseItemData_map;
	std::map<int,Logic::MonsterAtt_t> MonsterTypeData_map;
	std::map<int,Logic::ItemEData_t> ItemEData_map;
	Logic::UpgradeData_t* pUpgradeData;
	Logic::ItemData_t* pUseItemData;
	Logic::MonsterAtt_t* pMonsterAttData;
	Logic::ItemEData_t* pItemEData;
	std::map<string,TerrainInfo> m_mapTerrain;
};