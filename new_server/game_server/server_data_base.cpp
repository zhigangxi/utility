#pragma once
#include "stdafx.h"
#include "server_data_base.h"
#include "monster.h"
#include "game_map.h"
#include "logic_data_loader.hpp"
#include "xml_data_loader.hpp"
#include "net_msg.h"
#include <ctime>
#include "pugixml.hpp"


void server_data::LoadMapMonsterData(std::string map_name_)
{
	std::string maplogic;
	maplogic="..\\..\\..\\release\\JLinData\\Stage\\"+map_name_+_T(".logic");
	logic_data_loader_ptr item_ptr(new logic_data_loader(maplogic.c_str()));
	CNetMessage pack;
	item_ptr->fetch_data(pack);
	int count = 0;
	pack>>count;
	if (count != 0)
	{
		monster_map* pMap = NULL;
		if ( MatchMap(map_name_,&pMap) )
		{
			for (int i = 0;i<count;i++)
			{
				monster_ptr mp( new monster() );
				mp->fill_data(pack);
				pMap->insert(std::make_pair(mp->monster_id(),mp));
			}
		}
	}
}
void server_data::LoadMapTypeData()
{
	cLeafNode * pLeafNode = server_data::instance().m_pTheWorld->GetUnitTemplate(Logic::Unit_MapCorreTable);
	UINT nRow = pLeafNode->GetRows();
	for(UINT i=0;i<nRow;i++)
	{
		const Logic::MapCorreTable_t * pMapScene=(const Logic::MapCorreTable_t *)pLeafNode->GetRow(i)->GetBuffer();
		sceneinfo.scene_id_=pMapScene->m_nMapID;
		sceneinfo.scene_name_=pMapScene->m_szMapName;
		sceneinfo.map_name_=pMapScene->m_szMapFile;
		if(!strcmp(pMapScene->m_szMapFile,"home001"))
			sceneinfo.scene_type_=0;
		else if(*pMapScene->m_szMapFile=='B'||*pMapScene->m_szMapFile=='b')
			sceneinfo.scene_type_=2;
		else  if(*pMapScene->m_szMapFile=='C'||*pMapScene->m_szMapFile=='c')
			sceneinfo.scene_type_=1;
		else  if(*pMapScene->m_szMapFile=='D'||*pMapScene->m_szMapFile=='d')
			sceneinfo.scene_type_=3;

		//scene_map_.insert(std::make_pair(tp->scene_id(),tp));
		if (sceneinfo.scene_type_==1||sceneinfo.scene_type_==3)
		{
			scene_ptr sp(new CScene(sceneinfo.scene_id_,sceneinfo.scene_type_,sceneinfo.map_name_,sceneinfo.scene_name_));
			//	sp->AddCommandDeal();
			scene_manager::instance().scene_map_.insert(make_pair(sp->scene_id(),sp));
		}
		LoadObstructInfo(sceneinfo);
		LoadTerrain(sceneinfo);
		sceinfo_map.insert(make_pair(sceneinfo.scene_id_,sceneinfo));
		LoadMapMonsterData(sceneinfo.map_name_);
	}
}

void server_data::LoadServerData()
{
	data_table_loader<Logic::MonsterAtt_t> *item_ptr1 = new data_table_loader<Logic::MonsterAtt_t>;
	int i = 1;
	while ((pMonsterAttData = item_ptr1->fetch_data(i,Logic::Unit_MonsterAtt))!=NULL)
	{
		MonsterTypeData_map[pMonsterAttData->m_nMonsterID] =  *pMonsterAttData;
		i++;
	}

	data_table_loader<Logic::UpgradeData_t> *item_ptr2 = new data_table_loader<Logic::UpgradeData_t>;
		i = 1;
	while ((pUpgradeData = item_ptr2->fetch_data(i,Logic::Unit_UpgradeData))!=NULL)
	{
		UpgradeInfo_map[pUpgradeData->m_nLevel] = *pUpgradeData;
		i++;
	}

	data_table_loader<Logic::ItemData_t> *item_ptr3 = new data_table_loader<Logic::ItemData_t>;
		i = 1;
	while ((pUseItemData = item_ptr3->fetch_data(i,Logic::Unit_ItemData))!=NULL)
	{
		UseItemData_map[pUseItemData->m_nItem] = *pUseItemData;
		i++;
	}

	data_table_loader<Logic::ItemEData_t> *item_ptr4 = new data_table_loader<Logic::ItemEData_t>;
		i = 1;
	while ((pItemEData = item_ptr4->fetch_data(i,Logic::Unit_ItemEData))!=NULL)
	{
		ItemEData_map[pItemEData->m_nItem] = *pItemEData;
		i++;
	}

	delete item_ptr1;
	delete item_ptr2;
	delete item_ptr3;
	delete item_ptr4;
}

void server_data::Start()
{
	LoadServerData();
	LoadMapTypeData();
	srand((unsigned)time(NULL)); 
}

int server_data::CreateID()
{
	static long id_ = 1;
	return InterlockedIncrement(&id_);
}

void server_data::LoadObstructInfo( SCEINFO& info )
{
	string name = info.map_name_;
	pugi::xml_document pathdoc;
	size_t pos = 0;
	pos = name.find_first_of('.');
	std::string filename=name.substr(0,pos);
	filename+=(_T(".Logic"));
	filename = _T("..\\..\\..\\release\\JLinData\\Stage\\") + filename;
	FILE* file = fopen(filename.c_str(),"r");
	bool isFileExist = bool(file != NULL);
	if(file) fclose(file);
	if ( isFileExist )
	{
		pathdoc.load_file(filename.c_str());
		pugi::xml_node logic = pathdoc.child("Logic");
		pugi::xml_node pathdef = logic.child("pathDef");
		int rows,cols;
		FILE *fileRead;
		string pathname = filename.substr(0,filename.size()-string(".logic").size()) + ".path";
		fileRead = fopen(pathname.c_str(),"rb");
		if( fileRead == NULL )
			return;
		fread(&cols, sizeof(int), 1, fileRead);
		fread(&rows, sizeof(int), 1, fileRead);
		info.col_ = cols;
		info.row_ = rows;
		int row,col,boxId,isObstruct;
		sceneinfo.obstruct_info_.resize(boost::extents[cols][rows]);
		for ( int i = 0; i < cols*rows; ++i )
		{
			fread(&row, sizeof(int), 1, fileRead);
			fread(&col, sizeof(int), 1, fileRead);
			fread(&boxId, sizeof(int), 1, fileRead);
			fread(&isObstruct, sizeof(int), 1, fileRead);
			info.obstruct_info_[col][row] = static_cast<bool>(isObstruct < 0);
		}
		fclose(fileRead);
	}
}

bool server_data::IsObstruct( string mapname,int x, int z )
{
	std::map<string,TerrainInfo>::iterator it = m_mapTerrain.find( mapname );
	if( it == m_mapTerrain.end() )
		return false;
	std::map<int,SCEINFO>::iterator sce_it = sceinfo_map.begin();
	for ( ; sce_it != sceinfo_map.end(); sce_it++ )
	{
		if( sce_it->second.map_name_ == mapname ) 
			break;
	}
	if( sce_it == sceinfo_map.end() )
		return false;
	int titleSize = (int)it->second.m_TileSize;
	int m,n;
	if ( x <= 0 || z <= 0 ) 
		return false;
	m = (int)((int)(x/titleSize)%it->second.m_Width);
	n = (int)((int)(z/titleSize)%it->second.m_Height);
	if ( m< 0 || n < 0 )
		return false;
	if ( m < sce_it->second.col_ && n < sce_it->second.row_ && sce_it->second.obstruct_info_[m][n] )
		return true;
	return false;
}

bool server_data::LoadTerrain(const SCEINFO& sInfo)
{
	string mapname = sInfo.map_name_;
	string filename = _T("..\\..\\..\\release\\JLinData\\Stage\\Terrain\\") + mapname + _T(".ter");
	FILE* pFile = fopen(filename.c_str(),"rb");
	if ( pFile )
	{
		int filesize = sizeof(TerrainHeader) + sizeof(TerrainInfo);
		char* pBuf = new char[filesize+1];
		char* p = pBuf;
		if ( fread( pBuf,filesize,1,pFile ) == 0 )
		{
			cout << "load file:" << mapname + _T(".ter") << " failed." << endl;
			delete[] p;
			fclose(pFile);
			return false;
		}
		// ÎÄ¼þÍ·
		TerrainHeader * header = (TerrainHeader*)pBuf;
		if( !header || header->dwFlag != TERRAIN_FLAG )
		{
			delete[] p;
			return false;
		}
		pBuf += sizeof(TerrainHeader);
		TerrainInfo info = *(TerrainInfo*)pBuf;
		m_mapTerrain[sInfo.map_name_] = info;
		delete[] p;
		fclose(pFile);
		return true;
	}
	else
		return false;
}

bool server_data::GetTerrainInfo(string mapname,TerrainInfo& info)
{
	std::map<string,TerrainInfo>::iterator it = m_mapTerrain.find(mapname);
	if ( it != m_mapTerrain.end() )
	{
		info = it->second;
		return true;
	}
	return false;
}

bool server_data::MatchMap(string mapname, monster_map** pMap )
{
	transform(mapname.begin(),mapname.end(),mapname.begin(),tolower);
	if( mapname == "b0003" )
	{
		*pMap = &monster_map_b0003;
		return true;
	}
	else if ( mapname == "b0004" )
	{
		*pMap = &monster_map_b0004;
		return true;
	}
	return false;
}