#include "CampMgr.h"
#include "ReadWriteUser.h"
#include "NetMsg.pb.h"

using namespace std;

bool CCampMgr::Init()
{
	list<CReadData> datas;
	std::vector<const char *> fieldNames;

	fieldNames = { "id","clear_cd_cost" };
	if (!CReadWriteUser::ReadData(datas, fieldNames, "camp_camp"))
		return false;
	for (auto i = datas.begin(); i != datas.end(); i++)
	{
		m_clearCDCosts.insert(make_pair(i->GetVal<uint32_t>(0), i->GetVal<uint32_t>(1)));
	}

	datas.clear();
	fieldNames = { "level","level_up_time","level_up_cost","require_level" };

	if (!CReadWriteUser::ReadData(datas, fieldNames, "camp_base", "order by level"))
		return false;

	vector<CampData> bases;
	auto copyBaseCamps = [&](CReadData &data) {
		CampData camp;
		camp.campType = NetMsg::BaseCamp;
		camp.Level = data.GetVal<uint32_t>(0);
		camp.LevelUpTime = data.GetVal<uint32_t>(1);
		camp.LevelUpCost = data.GetVal<uint32_t>(2);
		camp.RequireLevel = data.GetVal<uint32_t>(3);
		bases.push_back(camp);
	};
	for_each(datas.begin(), datas.end(), copyBaseCamps);
	m_otherCamps.insert(make_pair(NetMsg::BaseCamp, bases));

	datas.clear();
	fieldNames = { "level","level_up_time","level_up_cost"};
	if (!CReadWriteUser::ReadData(datas, fieldNames, "camp_armory", "order by level"))
		return false;
	vector<CampData> armorys;
	auto copyArmory= [&](CReadData &data) {
		CampData camp;
		camp.campType = NetMsg::ArmoryCamp;
		camp.Level = data.GetVal<uint32_t>(0);
		camp.LevelUpTime = data.GetVal<uint32_t>(1);
		camp.LevelUpCost = data.GetVal<uint32_t>(2);
		armorys.push_back(camp);
	};
	for_each(datas.begin(), datas.end(), copyArmory);
	m_otherCamps.insert(make_pair(NetMsg::ArmoryCamp, armorys));

	datas.clear();
	fieldNames = { "level","level_up_time","level_up_cost" };
	if (!CReadWriteUser::ReadData(datas, fieldNames, "camp_barrack", "order by level"))
		return false;
	vector<CampData> barracks;
	auto copybarrack = [&](CReadData &data) {
		CampData camp;
		camp.campType = NetMsg::BarrackCamp;
		camp.Level = data.GetVal<uint32_t>(0);
		camp.LevelUpTime = data.GetVal<uint32_t>(1);
		camp.LevelUpCost = data.GetVal<uint32_t>(2);
		barracks.push_back(camp);
	};
	for_each(datas.begin(), datas.end(), copybarrack);
	m_otherCamps.insert(make_pair(NetMsg::BarrackCamp, barracks));

	datas.clear();
	fieldNames = { "level","level_up_time","level_up_cost","output_per_hour","output_limit" };
	if (!CReadWriteUser::ReadData(datas, fieldNames, "camp_farmland", "order by level"))
		return false;
	vector<CampData> farmlands;
	auto copyFarmland = [&](CReadData &data) {
		CampData camp;
		camp.campType = NetMsg::FarmlandCamp;
		camp.Level = data.GetVal<uint32_t>(0);
		camp.LevelUpTime = data.GetVal<uint32_t>(1);
		camp.LevelUpCost = data.GetVal<uint32_t>(2);
		camp.OutputPerHour = data.GetVal<float>(3);
		camp.OutputHourLimit = data.GetVal<float>(4);
		farmlands.push_back(camp);
	};
	for_each(datas.begin(), datas.end(), copyFarmland);
	m_otherCamps.insert(make_pair(NetMsg::FarmlandCamp, farmlands));

	datas.clear();
	fieldNames = { "level","level_up_time","level_up_cost","output_per_hour","output_limit" };
	if (!CReadWriteUser::ReadData(datas, fieldNames, "camp_market", "order by level"))
		return false;
	vector<CampData> markets;
	auto copyMarket = [&](CReadData &data) {
		CampData camp;
		camp.campType = NetMsg::MarketCamp;
		camp.Level = data.GetVal<uint32_t>(0);
		camp.LevelUpTime = data.GetVal<uint32_t>(1);
		camp.LevelUpCost = data.GetVal<uint32_t>(2);
		camp.OutputPerHour = data.GetVal<float>(3);
		camp.OutputHourLimit = data.GetVal<float>(4);
		markets.push_back(camp);
	};
	for_each(datas.begin(), datas.end(), copyMarket);
	m_otherCamps.insert(make_pair(NetMsg::MarketCamp, markets));

	return true;
}

bool CCampMgr::InitUserCamp(ServerPB::UserCamp *camp, uint32_t type, uint32_t lv) const
{
	auto data = GetCamp(type, lv);
	if (data == nullptr)
		return false;

	/*switch (type)
	{
	case NetMsg::BaseCamp:
		break;
	case NetMsg::MarketCamp:
		break;
	case NetMsg::FarmlandCamp:
		break;
	case NetMsg::BarrackCamp:
		break;
	case NetMsg::ArmoryCamp:
		break;
	default:
		return false;
	}*/
	camp->set_level(lv);
	camp->set_lefttime(0);
	camp->set_leveluptime(0);
	//camp->set_outputdata(0);
	camp->set_outputtime(time(nullptr));
	camp->set_type(type);
	return true;
}

const CampData *CCampMgr::GetCamp(uint32_t type, uint32_t lv) const
{
	auto iter = m_otherCamps.find(type);
	if (iter == m_otherCamps.end())
		return nullptr;
	if (lv > iter->second.size() || lv < 1)
		return nullptr;
	return &(iter->second[lv-1]);
}
uint32_t CCampMgr::GetClearCDCost(uint32_t type) const
{
	auto iter = m_clearCDCosts.find(type);
	if (iter != m_clearCDCosts.end())
		return iter->second;
	return 0;
}