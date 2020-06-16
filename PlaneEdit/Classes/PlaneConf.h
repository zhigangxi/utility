#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include "nlohmann/json.hpp"

class Position
{
public:
	int x;
	int y;
};

struct PlaneConf
{
	bool Init(nlohmann::json& j);
	bool InitEditInfo(nlohmann::json& j);

	void GetJson(nlohmann::json& j);

	int id;
	std::string name;
	std::string mod;
	int textureNum = 1;
	int textureType = 1;//1:jpg,2:png
	int rotateX = 0;//0-360
	int rotateY = 0;
	int rotateZ = 0;
	int scale = 100;//%
	const static int MUZZLES_NUM = 3;
	const static int MAX_POLYGON_SIZE = 8;
	Position muzzles[MUZZLES_NUM];
	std::vector<Position> polygon;
	Position pos;

	std::vector<std::string> textureNames;
};

class PlaneConfMgr
{
public:
	//read from json file(excel export)
	bool ReadPlanes(std::string confFile);

	void SavePlanes(std::string dataFile);

	//load from binary(edit export)
	bool LoadPlanes(std::string dataFile);

	PlaneConf* GetConf(int id)
	{
		auto iter = _planeConfs.find(id);
		if (iter == _planeConfs.end())
			return nullptr;
		return iter->second;
	}
	void GetAllPlane(std::vector< PlaneConf*>& allPlane)
	{
		for (auto i = _planeConfs.begin(); i != _planeConfs.end(); i++)
		{
			allPlane.push_back(i->second);
		}
	}
private:
	std::unordered_map<int, PlaneConf*> _planeConfs;
};