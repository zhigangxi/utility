#include "PlaneConf.h"
#include "nlohmann/json.hpp"
#include <fstream>
#include <iostream>

using json = nlohmann::json;
using namespace std;

bool PlaneConf::Init(nlohmann::json& j)
{
	try {
		id = j["id"];
		name = j["name"];
		mod = j["mod"];
		textureNum = j["texture"];
		textureType = j["texture_type"];
	}
	catch (json::type_error e) {
		cout << "PlaneConf::Init error" << endl;
		cout << e.what() << endl;
		return false;
	}
	char name[128];
	char modName[128];
	strncpy(modName, this->mod.c_str(), sizeof(modName));
	for (size_t i = 0; i < strlen(modName); i++)
	{
		if (modName[i] == '.')
		{
			modName[i] = 0;
			break;
		}
	}
	for (int i = 0; i < textureNum; i++)
	{
		if (textureType == 1)
			snprintf(name, sizeof(name), "%s_%d.jpg", modName, i + 1);
		else
			snprintf(name, sizeof(name), "%s_%d.png", modName, i + 1);
		textureNames.push_back(name);
	}
	return true;
}

bool PlaneConf::InitEditInfo(nlohmann::json& j)
{
	try {
		rotateX = j["rotate_x"];
		rotateY = j["rotate_y"];
		rotateZ = j["rotate_z"];
		scale = j["scale"];
		auto firePos = j["muzzles"];
		int num = firePos.size();
		if (num == MUZZLES_NUM * 2)
		{
			for (auto i = 0; i < num / 2; i++)
			{
				Position p;
				p.x = firePos[2 * i];
				p.y = firePos[2 * i + 1];
				this->muzzles[i] = p;
			}
		}
		auto polygon = j["polygon"];
		num = polygon.size();
		if (num % 2 == 0 && num > 0)
		{
			for (auto i = 0; i < num / 2; i++)
			{
				Position p;
				p.x = polygon[2 * i];
				p.y = polygon[2 * i + 1];
				this->polygon.push_back(p);
			}
		}
		auto pos = j["pos"];
		num = pos.size();
		if (num == 2)
		{
			this->pos.x = pos[0];
			this->pos.y = pos[1];
		}
	}
	catch (json::type_error e) {
		cout << "PlaneConf::InitEditInfo error" << endl;
		cout << e.what() << endl;
		return false;
	}
	return true;
}

void PlaneConf::GetJson(nlohmann::json& j)
{
	j["id"] = id;
	j["name"] = name;
	j["mod"] = mod;
	j["texture"] = textureNum;
	j["rotate_x"] = rotateX;
	j["rotate_y"] = rotateY;
	j["rotate_z"] = rotateZ;
	j["scale"] = scale;
	std::vector<int> intArr;
	for (int i = 0; i < MUZZLES_NUM; i++)
	{
		intArr.push_back(muzzles[i].x);
		intArr.push_back(muzzles[i].y);
	}
	j["muzzles"] = intArr;
	intArr.clear();
	for (auto i = this->polygon.begin(); i != this->polygon.end(); i++)
	{
		intArr.push_back(i->x);
		intArr.push_back(i->y);
	}
	j["polygon"] = intArr;
	intArr.clear();
	intArr.push_back(this->pos.x);
	intArr.push_back(this->pos.y);
	j["pos"] = intArr;
}

bool PlaneConfMgr::ReadPlanes(std::string confFile)
{
	std::ifstream jsFile(confFile);
	if (!jsFile.is_open()) {
		cout << "can't open " << confFile << endl;
		return false;
	}
	json j;
	try {
		jsFile >> j;
	}
	catch (json::type_error e) 
	{
		cout << e.what() << endl;
		return false;
	}
	if (j.size() <= 0) {
		cout << "json file:" << confFile << " error" << endl;
		return false;
	}

	for (auto i = _planeConfs.begin(); i != _planeConfs.end(); i++)
	{
		delete i->second;
	}
	_planeConfs.clear();
	for (size_t i = 0; i < j.size(); i++)
	{
		PlaneConf* plane = new PlaneConf;
		if (!plane->Init(j[i])) {
			return false;
		}
		_planeConfs.insert(make_pair(plane->id, plane));
	}
	return true;
}

void PlaneConfMgr::SavePlanes(std::string dataFile)
{
	json j;
	for (auto i = _planeConfs.begin(); i != _planeConfs.end(); i++)
	{
		json conf;
		i->second->GetJson(conf);
		j.push_back(conf);
	}
	auto msgpack = json::to_msgpack(j);
	std::ofstream msgPackJs(dataFile,ios::binary);
	msgPackJs.write((const char*)(&msgpack[0]),msgpack.size());
	msgPackJs.flush();
}

bool PlaneConfMgr::LoadPlanes(std::string dataFile)
{
	std::ifstream jsFile(dataFile, ios::binary);
	if (!jsFile.is_open()) {
		cout << "can't open " << dataFile << endl;
		return false;
	}
	jsFile.seekg(0, jsFile.end);
	auto num = jsFile.tellg();
	jsFile.seekg(0, jsFile.beg);
	vector<uint8_t> buf;
	buf.resize(num);
	jsFile.read((char*)&(buf[0]), num);

	json j = json::from_msgpack(buf);

	for (size_t i = 0; i < j.size(); i++)
	{
		PlaneConf* plane;
		auto iter = _planeConfs.find(j[i]["id"]);
		if (iter == _planeConfs.end()) {
			plane = new PlaneConf;
			if (!plane->Init(j[i])) {
				return false;
			}
			_planeConfs.insert(make_pair(plane->id, plane));
		}else {
			plane = iter->second;
			if (!plane->InitEditInfo(j[i])) {
				return false;
			}
		}
	}
	return true;
}

