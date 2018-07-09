#include "HeroSkillBook.h"
#include "utility.h"
#include <string>
#include <iostream>
#include <stdint.h>
#include "ItemInst.h"
#include "ItemTmpl.h"
#include "HeroInst.h"
#include "SkillTmpl.h"
#include <random>
#include <algorithm>
#include <iterator>
#include <numeric>

using namespace std;
static std::list<uint32_t> m_newBooks;
bool CHeroSkillBookMgr::Init()
{
	return true;
}

void CHeroSkillBookMgr::CalNewBook(std::vector<CItemInst*> & books, CUser *user, uint32_t heroId) const
{
	m_newBooks.clear();

	std::set<uint32_t> bookStar;
	std::set<uint32_t> heroSkillStar;
	std::unordered_map<uint32_t,CItemInst*> m_EmptyOri; //emptybook
	std::unordered_map<uint32_t, CSkillInst*> skillTemp;
	uint32_t compensationVal = 0;//compensation Probability (val = 40)
	
	int index = 0;
	for (auto i = books.begin(); i != books.end(); i++)
	{
		uint32_t star = (*i)->GetItemTmpl()->GetStar();
		bookStar.insert(star);
		CItemInst* oriItem = (*i);
		m_EmptyOri.insert(std::make_pair(index, oriItem));
		index++;
	}

	CHeroInst *hero = user->GetHero(heroId);



	int skillIndex = 0;
	for (auto i = hero->GetSkills().begin(); i != hero->GetSkills().end(); i++)
	{
		CSkillInst * sk = &(*i);
		if(false == sk->GetSkillTmpl()->GetIsExclusive())
		{
			uint32_t skillStar = sk->GetSkillTmpl()->GetStar();
			heroSkillStar.insert(skillStar);
			skillTemp.insert(std::make_pair(skillIndex, sk));
			skillIndex++;
		}
	}


	while(m_EmptyOri.size() > 0)
	{
		UpdateBookMaxStar(m_EmptyOri, bookStar);   //reload the emptyBook of max star
		uint32_t maxStar = GetMaxStar(bookStar);

		int selectEmptyBook = -1;
		for (auto i = m_EmptyOri.begin(); i != m_EmptyOri.end(); i++)
		{
			auto item = i->second;
			uint32_t itemStar = item->GetItemTmpl()->GetStar();
			if (maxStar == itemStar)
			{
				selectEmptyBook = i->first;
				break;
			}
		}

		UpdateHeroSkillMaxStar(skillTemp, heroSkillStar);

		bool isFound = false;
		std::vector<uint32_t> sameStar;
		for(auto i = skillTemp.begin();i != skillTemp.end(); i++)
		{
			auto sk = i->second;
			uint32_t skillStar = sk->GetSkillTmpl()->GetStar();
			if (maxStar == skillStar)
			{
				sameStar.push_back(i->first);
			}
		}

		if (sameStar.size() > 0)
		{
			isFound = true;
			auto isGet = GetSkillBookOfProbability(1, 1, compensationVal);
			if (!isGet)
			{
				m_EmptyOri.erase(selectEmptyBook);
				break;
			}
			FindSkillAndDeleteEmptyBook(m_EmptyOri, skillTemp, selectEmptyBook, sameStar);
			break;
		}

		if (!isFound)
		{
			//get the low star than emptybook
			for (auto i = skillTemp.begin(); i != skillTemp.end(); i++)
			{
				CSkillInst * skTemp = i->second;
				if (skTemp->GetSkillTmpl()->GetStar() > maxStar)
				{
					heroSkillStar.erase(skTemp->GetSkillTmpl()->GetStar());
				}
			}
			// no skill could be found
			if(heroSkillStar.size() <= 0)
			{
				m_EmptyOri.erase(selectEmptyBook);
				continue;//out this time
			}

			//get the more big star in low skills
			set<uint32_t>::iterator iter;
			iter = heroSkillStar.end();
			iter--;
			int lowStar = *iter;
			std::vector<uint32_t> samelowStar;
			for (auto i = skillTemp.begin(); i != skillTemp.end(); i++)
			{
				auto sk = i->second;
				if (sk->GetSkillTmpl()->GetStar() == lowStar)
				{
					samelowStar.push_back(i->first);
				}
			}
			if(samelowStar.size() > 0)
			{
				FindSkillAndDeleteEmptyBook(m_EmptyOri, skillTemp, selectEmptyBook, samelowStar);
				compensationVal = 0;
				break;
			}
		}
	}
}

void CHeroSkillBookMgr::FindSkillAndDeleteEmptyBook(std::unordered_map<uint32_t, CItemInst*> & m_EmptyOri,
	std::unordered_map<uint32_t, CSkillInst*> & skillTemp, uint32_t selectBook, std::vector<uint32_t> & sameStar)const
{
	uint32_t randomSelect = CUtility::GetRandomOneFromVector(sameStar);
	auto skItem = skillTemp.find(randomSelect);
	auto sk = skItem->second;
	DelSkillAndEmptyBook(sk, m_EmptyOri, skillTemp, selectBook, skItem->first);
}

void CHeroSkillBookMgr::DelSkillAndEmptyBook(CSkillInst * sk, std::unordered_map<uint32_t, CItemInst*> & m_EmptyOri,
	std::unordered_map<uint32_t, CSkillInst*> & skillTemp,uint32_t selectBook, uint32_t selectSkill)const
{
	auto skillLevel = sk->GetLevel();
	uint32_t skillBookItemId = CItemMgr::get_const_instance().GetItemBySkillIdAndLevel(sk->GetSkillId(), skillLevel);
	m_newBooks.push_back(skillBookItemId);
	skillTemp.erase(selectSkill);
	m_EmptyOri.erase(selectBook);
}

void CHeroSkillBookMgr::GetNewBooks(NetMsg::WriteSkillBookAck *ack)const
{
	for(auto i = m_newBooks.begin();i != m_newBooks.end();i++)
	{
		uint32_t itemId = *i;
		NetMsg::ItemInfo* items = ack->add_items();
		items->set_id(itemId);
		items->set_num(1);
	}
}

void CHeroSkillBookMgr::AddNewSkillBook(CUser* user)const
{
	for (auto i = m_newBooks.begin();i != m_newBooks.end();i++)
	{
		uint32_t itemId = *i;
		CUtility::AddUserItem(user, itemId,1);
	}
}

void CHeroSkillBookMgr::UpdateBookMaxStar(std::unordered_map<uint32_t, CItemInst*> & ori, std::set<uint32_t> & maxStar)const
{
	maxStar.clear();
	for(auto i = ori.begin();i != ori.end();i++)
	{
		CItemInst* itemInst = i->second;
		CItemTmpl *itemTmpl = itemInst->GetItemTmpl();
		uint32_t star = itemTmpl->GetStar();
		maxStar.insert(star);
	}
}

void CHeroSkillBookMgr::UpdateHeroSkillMaxStar(std::unordered_map<uint32_t, CSkillInst*> & skillInsts, std::set<uint32_t> & maxStar)const
{
	maxStar.clear();
	for (auto i = skillInsts.begin();i != skillInsts.end();i++)
	{
		CSkillInst* sk = i->second;
		uint32_t skillStar = sk->GetSkillTmpl()->GetStar();
		maxStar.insert(skillStar);
	}
}

bool CHeroSkillBookMgr::GetSkillBookOfProbability(uint32_t emptyBookStar, uint32_t heroSkillStar, uint32_t & compensation)const
{
	if(emptyBookStar > heroSkillStar)
		return true;
	if(emptyBookStar == heroSkillStar)
	{
		auto val = CUtility::RandomInt(0,100);
		if( val <= (60 + compensation))
		{
			compensation = 0;
			return true;
		}
		else
		{
			compensation = 40;//²¹³¥¸ÅÂÊ40
			return false;
		}
			
	}
	if(emptyBookStar < heroSkillStar)
		return false;
	
	return false;
}

uint32_t CHeroSkillBookMgr::GetMaxStar(std::set<uint32_t> & val)const
{
	std::set<uint32_t>::iterator iter;
	iter = val.end();
	iter--;
	uint32_t star = *iter;
	return star;
}

void CHeroSkillBookMgr::DelMaxStar(std::set<uint32_t> & val)const
{
	std::set<uint32_t>::iterator iter;
	iter = val.end();
	val.erase(--iter);
}
