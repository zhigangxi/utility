#pragma once
#include <stdint.h>
#include "macros.h"
#include <unordered_map>
#include "ServerPb.pb.h"
class CResourceTmpl;
class CResourceInst
{
public:

	CResourceInst() : m_id(0),
		m_createTime(0),
		m_resourceID(0),
		m_resourceNum(0),
		m_resCD(0),
		m_posId(0),
		m_bClose(false),
		m_resourceTmpl(nullptr),
		m_updateToDb(false)
	{

	}

	bool Init(uint32_t resourceFieldId, uint32_t level,uint32_t force, std::unordered_map<uint32_t, uint32_t> & hasPos);

	DEFINE_PROPERTY(uint64_t, m_id, Id);
	DEFINE_PROPERTY(time_t, m_createTime, CreateTime);
	DEFINE_PROPERTY(uint32_t, m_resourceID, ResourceID);
	DEFINE_PROPERTY(uint32_t, m_resourceNum, ResourceNum);
	DEFINE_PROPERTY(uint32_t, m_leftResourceNum, LeftResourceNum);
	DEFINE_PROPERTY(uint32_t, m_resCD, ResCD);
	DEFINE_PROPERTY(uint32_t, m_posId, PosID);//
	DEFINE_PROPERTY(uint32_t, m_bClose, ResourceClose);//
	DEFINE_PROPERTY(CResourceTmpl*, m_resourceTmpl, ResourceTmpl);

	DEFINE_PROPERTY(bool, m_updateToDb, UpdateToDb);//
	DEFINE_STATIC_UID(uint64_t, m_curResId);


	private:
	// std::unordered_map<uint32_t, ServerPB::MiningInfo> m_miningInfos;

};