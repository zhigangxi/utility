#include "VsBattle.h"
#include "utility.h"
#include "HeroInst.h"
#include "Evolve.h"
#include "session.h"
#include "ReadWriteUser.h"
#include "user.h"

bool CVsBattle::Init(uint64_t battleId, VSType type, uint32_t userId, CHeroInst *userHero, uint32_t heroId)
{
	if (userHero == nullptr)
		return false;
	auto hero = userHero->GetHeroTmpl();
	if (hero == nullptr)
		return false;

	auto heroClass = CHeroClassMgr::get_const_instance().GetHeroClass(heroId);
	if (heroClass == nullptr)
		return false;

	m_userId = userId;
	m_heroStar = heroClass->GetStar().lv;
	m_userHeroId = hero->GetId();
	m_heroId = heroId;
	m_battleId = battleId;
	m_type = type;
	m_curTurn = 0;
	m_userHeroHp = hero->GetVsHealth();
	
	m_heroHp = heroClass->GetVsHealth();
	m_userHeroNuqi = 1 + CEvolve::get_const_instance().GetAwakenLevel(hero->GetId());
	m_heroNuqi = 1;
	m_createTime = boost::posix_time::microsec_clock::local_time();
	m_readyTime = m_createTime;
	m_curOptTime;

	m_userHeroStrength = userHero->GetTolStrength();
	m_heroStrength = heroClass->GetStrength().lv;
	return true;
}

void CVsBattle::GetEnemy(NetMsg::VSBattleEnemy & enemy)
{
	enemy.set_id(m_heroId);
	enemy.set_star(m_heroStar);
	enemy.set_nuqi(m_heroNuqi);
}

uint32_t CVsBattle::SetOptType(uint32_t turn, uint32_t optType)
{
	if (turn == 0)
		return 0;
	
	uint32_t pos = turn - 1;
	if (m_curTurn > turn && m_userInputs.size() > turn)
		return m_userInputs[pos];
	else if (turn <= m_userInputs.size())
	{
		m_userInputs[pos] = optType;
		return optType;
	}
	else
	{
		m_userInputs.push_back(optType);
		return optType;
	}
	return 0;
}

void CVsBattle::SetReadTime()
{
	m_readyTime = boost::posix_time::microsec_clock::local_time();
	m_curOptTime = m_readyTime;
}

bool CVsBattle::Update(boost::posix_time::ptime  now)
{
	auto &battleMgr = CVsBattleMgr::get_const_instance();
	boost::posix_time::time_duration msdiff;
	if (m_readyTime == m_createTime)
	{
		msdiff = now - m_readyTime;
		if (msdiff.total_milliseconds() == battleMgr.GetReadyTime())
		{
			m_readyTime = now;
			m_curOptTime = now;
		}
		return false;
	}
	msdiff = now - m_curOptTime;
	if (msdiff.total_milliseconds() >= battleMgr.GetTurnTime(m_curTurn))
	{
		m_curOptTime = now;
		AddTurn();
		if (m_curTurn > m_userInputs.size())
		{
			uint32_t opt = AutoSelOpt(true);
			opt = SetOptType(m_curTurn, opt);
			//m_userInputs.push_back(opt);
		}
		auto user = CUtility::GetOnlineUser(m_userId);
		if (user == nullptr)
			return true;
		return DoBattleTurn(user);
	}
	return false;
}

void CVsBattle::BattleEnd(CUser *user)
{
	NetMsg::VSBattleEnd *ack = new NetMsg::VSBattleEnd;
	ack->set_battleid(m_battleId);

	auto award = CVsBattleMgr::get_const_instance().GetAward(m_type);
	uint32_t awardId = 0;
	if (m_userHeroHp >= m_heroHp)
	{
		ack->set_victory(1);
		if (award != nullptr)
			awardId = award->WinAwardId;
	}
	else if (m_userHeroHp < m_heroHp)
	{
		ack->set_victory(2);
		if (award != nullptr)
			awardId = award->LoseAwardId;
	}
	else
	{
		ack->set_victory(3);
		if (award != nullptr)
			awardId = award->DrawAwardId;
	}
	SharedMsg ackMsg(ack);
	CUtility::SendMsg(user, ackMsg);

	if (awardId != 0)
	{
		std::vector<uint32_t> awardIds = { awardId };
		CUtility::AddAwards(user, awardIds);
	}
}

void CVsBattle::CalculationBattle(uint32_t opt1, uint32_t opt2, int strength1, int strength2, int &hp1, int &hp2, int &nuqi1, int &nuqi2)
{
	auto role = CVsBattleMgr::get_const_instance().GetRule(opt1,opt2);
	if (role == nullptr)
		return;
	//破防率 = 武力差 * 3 % , 即差别如果 = 33即100%破防了。
	//伤害 = (进攻方武力 / 5 + （攻方武力 - 防御方武力）)*(1 + 破防效果)
	float poFang = (strength1 - strength2)*0.03f;
	if (poFang > 1.0f)
		poFang = 0.5f;
	else
		poFang = 0;
	int delStr = (strength1*0.2f + (strength1 - strength2)*(1 + poFang));
	if (delStr < 10)
		delStr = 10;

	hp1 -= role->Player1LostHealth * delStr;
	if (hp1 < 0)
		hp1 = 0;

	poFang = (strength2 - strength1)*0.03f;
	if (poFang > 1.0f)
		poFang = 0.5f;
	else
		poFang = 0;
	delStr = (strength2*0.2f + (strength2 - strength1)*(1 + poFang));
	if (delStr < 10)
		delStr = 10;
	hp2 -= role->Player2LostHealth * delStr;
	if (hp2 < 0)
		hp2 = 0;

	nuqi1 += role->Player1DeltaQi;
	nuqi2 += role->Player2DeltaQi;
}

bool CVsBattle::DoBattleTurn(CUser *user)
{
	if (m_userInputs.size() < m_curTurn)
		return true;

	uint32_t heroOpt = AutoSelOpt(false);
	uint32_t userOpt = m_userInputs[m_curTurn - 1];
	if (user->GetNewGuide() <= 22)
	{
		/*VSInputType_ATTACK,
		VSInputType_DEFENCE,
		VSInputType_ULTRA,*/
		if (userOpt == VSInputType_ATTACK)
			heroOpt = VSInputType_DEFENCE;
		else if (userOpt == VSInputType_DEFENCE)
			heroOpt = VSInputType_DEFENCE;
		else
			heroOpt = VSInputType_ATTACK;
	}
	CalculationBattle(userOpt, heroOpt, m_heroStrength, m_userHeroStrength, m_userHeroHp, m_heroHp, m_userHeroNuqi, m_heroNuqi);
	
	NetMsg::VSBattleTurn *ackMsg = new NetMsg::VSBattleTurn;
	auto meState = ackMsg->mutable_mestate();
	auto otherState = ackMsg->mutable_otherstate();
	meState->set_battleid(m_battleId);
	meState->set_hp(m_userHeroHp);
	meState->set_nuqi(m_userHeroNuqi);
	meState->set_opttype(userOpt);
	otherState->set_battleid(m_battleId);
	otherState->set_hp(m_heroHp);
	otherState->set_nuqi(m_heroNuqi);
	otherState->set_opttype(heroOpt);
	ackMsg->set_battleid(m_battleId);

	SharedMsg ack(ackMsg);
	CUtility::SendMsg(user, ack);

	if (m_userHeroHp <= 0 || m_heroHp <= 0 || m_curTurn >= 5)
	{
		BattleEnd(user);
		return true;
	}

	return false;
}

void CVsBattle::AddTurn()
{
	if (m_curTurn > 0)
	{
		m_userHeroNuqi++;
		m_heroNuqi++;
	}
	m_curTurn++;
}

uint32_t CVsBattle::AutoSelOpt(bool user)
{
	uint32_t nuqi;
	if (user)
		nuqi = m_userHeroNuqi;
	else
		nuqi = m_heroNuqi;

	int opt;
	if (nuqi >= 3)
		opt = VSInputType_ULTRA;
	else
	{
		int r = CUtility::RandomInt(1, 100);
		if (r <= 50)
			opt = VSInputType_ATTACK;
		else
			opt = VSInputType_DEFENCE;
	}
	return opt;
}

bool CVsBattleMgr::Init()
{
	m_turnTime = { 5600,9533,6707,5607,5607};

	std::list<CReadData> datas;
	std::vector<const char *> fieldNames;

	fieldNames = { "player1_input","player2_input","player1_lost_health","player1_delta_qi","player2_lost_health","player2_delta_qi" };
	if (!CReadWriteUser::ReadData(datas, fieldNames, "vs_rule"))
		return false;
	for (auto i = datas.begin(); i != datas.end(); i++)
	{
		VsRule role;
		role.Player1Input = (VSInputType)i->GetVal<uint32_t>(0);
		role.Player2Input = (VSInputType)i->GetVal<uint32_t>(1);
		uint32_t mask = GetMask(role.Player1Input,role.Player2Input);
		role.Player1LostHealth = i->GetVal<uint32_t>(2);
		role.Player1DeltaQi = i->GetVal<uint32_t>(3);
		role.Player2LostHealth = i->GetVal<uint32_t>(4);
		role.Player2DeltaQi = i->GetVal<uint32_t>(5);
		m_vsRules.insert(std::make_pair(mask,role));
	}

	datas.clear();
	fieldNames = {"type","win_award","draw_award","lose_award"};
	if (!CReadWriteUser::ReadData(datas, fieldNames, "vs_vs"))
		return false;
	for (auto i = datas.begin(); i != datas.end(); i++)
	{
		VsAward award;
		award.type = (VSType)i->GetVal<uint32_t>(0);
		award.WinAwardId = i->GetVal<uint32_t>(1);
		award.DrawAwardId = i->GetVal<uint32_t>(2);
		award.LoseAwardId = i->GetVal<uint32_t>(3);
	}
	return true;
}

const VsAward * CVsBattleMgr::GetAward(VSType type) const
{
	auto iter = m_awards.find(type);
	if (iter == m_awards.end())
		return nullptr;
	return &(iter->second);
}

const VsRule * CVsBattleMgr::GetRule(uint32_t player1Input, uint32_t player2Input) const
{
	uint32_t mask = GetMask(player1Input, player2Input);
	auto iter = m_vsRules.find(mask);
	if(iter == m_vsRules.end())
		return nullptr;
	return &(iter->second);
}

void CVsBattleMgr::Update(boost::posix_time::ptime now)
{
	auto iter = m_battles.begin();

	while (iter != m_battles.end())
	{
		if (iter->second->Update(now))
		{
			auto i = iter;
			i++;
			delete iter->second;
			m_battles.erase(iter);
			iter = i;
		}
		else
		{
			iter++;
		}
	}
}

CVsBattle * CVsBattleMgr::CreateBattle(VSType type, uint32_t userId, CHeroInst * userHero, uint32_t heroId)
{
	CVsBattle *battle = new CVsBattle;
	m_curBattleId++;
	if (!battle->Init(m_curBattleId, type, userId, userHero, heroId))
	{
		delete battle;
		return nullptr;
	}
	m_battles.insert(std::make_pair(m_curBattleId, battle));
	return battle;
}

CVsBattle * CVsBattleMgr::GetBattle(uint64_t battleId)
{
	auto iter = m_battles.find(battleId);
	if(iter == m_battles.end())
		return nullptr;
	return iter->second;
}
