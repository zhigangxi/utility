#pragma once

#include "macros.h"
#include <stdint.h>
#include <string>
#include <list>
#include <boost/serialization/singleton.hpp>
#include <vector>
#include "user.h"
#include <unordered_map>
#include "SkillInst.h"

class CHeroSkillBookMgr:public boost::serialization::singleton<CHeroSkillBookMgr>
{
	public:
		bool Init();
		void CalNewBook(std::vector<CItemInst*> & books, CUser *user, uint32_t heroId)const;
		uint32_t GetMaxStar(std::set<uint32_t> & val)const;
		void DelMaxStar(std::set<uint32_t> & val)const;
		void UpdateBookMaxStar(std::unordered_map<uint32_t, CItemInst*> & ori, std::set<uint32_t> & maxStar)const;
		void UpdateHeroSkillMaxStar(std::unordered_map<uint32_t, CSkillInst*> & skillInsts, std::set<uint32_t> & maxStar)const;
		bool GetSkillBookOfProbability(uint32_t emptyBookStar, uint32_t heroSkillStar, uint32_t & compensation)const;
		void GetNewBooks(NetMsg::WriteSkillBookAck *ack)const;
		void AddNewSkillBook(CUser* user)const;
		void DelSkillAndEmptyBook(CSkillInst * sk, std::unordered_map<uint32_t, CItemInst*> & m_EmptyOri,
			std::unordered_map<uint32_t, CSkillInst*> & skillTemp, uint32_t selectBook, uint32_t selectSkill)const;

		void FindSkillAndDeleteEmptyBook(std::unordered_map<uint32_t, CItemInst*> & m_EmptyOri,
			std::unordered_map<uint32_t, CSkillInst*> & skillTemp, uint32_t selectBook, std::vector<uint32_t> & sameStar)const;
	private:
	
		
		
};