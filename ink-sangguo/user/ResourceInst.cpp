#include "ResourceInst.h"
#include "ResourceTmpl.h"
#include "Expressions.h"
#include <time.h>

uint64_t CResourceInst::m_curResId;

bool CResourceInst::Init(uint32_t resourceFieldId, uint32_t level,uint32_t force,std::unordered_map<uint32_t, uint32_t> & hasPos)
{
	CResourceTmpl *tmpl = CResourceMgr::get_const_instance().GetResourceInfoByIndex(resourceFieldId);
	if (tmpl == nullptr)
		return false;

	SetResourceTmpl(tmpl);
	SetResourceID(tmpl->GetID());
	uint32_t posId = 0;
	bool getPos = CResourceMgr::get_const_instance().GetResourcePosId(force, posId, hasPos);
	if(getPos)
		SetPosID(posId);
	else
		return false;

	ResourceNumberData data;
	data.Level = level;
	int resNum = (int)DesignData::Expressions::sGetInstance().sResourcePlayerNumber(0, &data);
	SetResourceNum(resNum);
	SetLeftResourceNum(resNum);

	SetResCD(tmpl->GetResCDTime());

	time_t now = time(nullptr);
	SetCreateTime(now);
	if (GetId() == 0)
	{
		SetId(GenerateUID());
	}
	return true;
}




