#ifndef _MEN_PAI_H_
#define _MEN_PAI_H_
#include "hash_table.h"
#include "self_typedef.h"
#include <boost/thread.hpp>
#include <list>
#include <string>
using namespace std;

class CUser;
class CNetMessage;
class CDatabaseSql;
/*
1-帮主
2-副帮主
3-长老
4-护法
5-堂主
6-帮众
*/
enum EBangPaiRank
{
    EBRBangZhu = 1,
    EBRFuBangZhu,
    EBRZhangLao,
    EBRHuFa,
    EBRTangZhu,
    EBRBangZhong
};

struct SBangPaiMember
{
    uint32 roleId;
    uint8 rank;
};

enum SZZState
{
    EZZSNormal = 0,
    EZZChengShu = 1,
    EZZSJiaSu = 2,
    EZZSPoHuai = 4,
    EZZSShuiTuLiuShi = 8,
};

struct SZhongZhi
{
    SZhongZhi()
    {
        itemId = 0;
        state = 0;//状态0正常，1成熟，2加速，4破坏,8水土流失
        x = 0;//x坐标
        y = 0;//y坐标
        roleId = 0;//种植角色id
        time = 0;//种植时间
        growTime = 0;
        gain = 0;
    }
    uint16 itemId;//种植作物类型(物品id)
    uint8 state;//状态0正常，1成熟，2加速，3破坏
    uint8 x;//x坐标
    uint8 y;//y坐标
    uint32 roleId;//种植角色id
    string roleName;
    time_t time;//
    int growTime;
    int gain;
};

class CBangPai
{
public:
    CBangPai():
    m_bqLevel(1),
	m_syLevel(1),
	m_bcLevel(1),
	m_slLevel(1),
	m_jmLevel(1),
	m_zcLevel(0), 
	m_fzcLevel(0),
    m_level(1),
    m_fanrong(0),
	m_money(500000),
	m_title(0),
	m_huodongTime(0),
	m_paiMing(0),
	m_huoyue(0),
	m_beiXZTime(0),
	m_XZTime(0),
	m_XZBangId(0),
	m_jisuanZZ(0),
	m_zaiHaiTime(0),
	m_gonggaoTime(0),
	m_res1(0),
	m_res2(0),
	m_res3(0),
	m_res4(0),
	m_ziJinDay(0),
	m_delJMTime(0)
    {
    }
    
    uint8 GetState()
    {
        if(IsBeiXuanZhan() || (GetXuanZhanBang() != 0))
            return 1;
        return 0;
    }
    void DismissBang(uint32 roleId);// 解散帮派
    void UndismissBang(uint32 roleId);
    
    void SetId(uint32 id)
    {
        m_id = id;
    }
    uint32 GetId()
    {
        return m_id;
    }
    void DelAskForJoin(uint32 id)
    {
        boost::mutex::scoped_lock lk(m_mutex);
        m_askJoinUser.remove(id);
    }
    void GetAskForJoin(list<uint32> &userList)
    {
        boost::mutex::scoped_lock lk(m_mutex);
        userList = m_askJoinUser;
    }
    /*list<uint32> *GetAdmin()
    {
        return &m_adminUser;
    }*/
    void GetMember(list<uint32> &userList)
    {
        boost::mutex::scoped_lock lk(m_mutex);
        userList = m_userList;
    }
    int GetRankMemNum(uint8 rank);
    bool AddMember(uint32 id,uint8 rank,bool noLimit = false);
    bool IsAskJoin(uint32 id);
    bool IsAdmin(uint32 id);
    bool AddAskJoin(uint32 id);
    void DelMember(uint32 id);
    uint8 GetMemberRank(uint32 id);
    void SetMemberRank(uint32 id,uint8 rank);
    
    void Read(char *);
    void SetName(const char *name)
    {
        boost::mutex::scoped_lock lk(m_mutex);
        if(name != NULL)
            m_name = name;
    }
    string GetName()
    {
        boost::mutex::scoped_lock lk(m_mutex);
        return m_name;
    }
    uint8 GetLevel()
    {
        return m_level;
    }
	int GetFanRong()
	{
	    return m_fanrong;
	}
	int GetMoney()
	{
	    return m_money;
	}
	uint8 GetHuoDongTime()
	{
	    return m_huodongTime;
	}
	string GetKouHao()
	{
	    boost::mutex::scoped_lock lk(m_mutex);
	    return m_kouhao.c_str();
	}
	
	void SetLevel(uint8 level)
    {
        m_level = level;
    }
	void SetFanRong(int fanrong)
	{
	    m_fanrong = fanrong;
	    
	}
	void SetMoney(int money)
	{
	    m_money = money;
	}
	void SetHuoDongTime(uint8 t)
	{
	    m_huodongTime = t;
	}
	void SetKouHao(const char *kouhao)
	{
	    boost::mutex::scoped_lock lk(m_mutex);
	    if(kouhao != NULL)
	    {
	        m_kouhao = kouhao;
	        
	    }
	}
	uint32 GetBangZhu()
	{
	    boost::mutex::scoped_lock lk(m_mutex);
	    if(m_allMember.size() > 0)
	        return (m_allMember.begin())->roleId;
        return 0;
	}
	uint32 GetBangCreater()
	{
	    boost::mutex::scoped_lock lk(m_mutex);
	    if(m_allMember.size() > 0)
	        return (m_allMember.begin())->roleId;
        return 0;
	}
	uint16 GetUserNum()
	{
	    boost::mutex::scoped_lock lk(m_mutex);
	    return m_allMember.size();
	}
	string GetGongGao()
	{
	    boost::mutex::scoped_lock lk(m_mutex);
	    return m_gonggao;
	}
	void SetGongGao(const char *gonggao)
	{
	    boost::mutex::scoped_lock lk(m_mutex);
	    if(gonggao != NULL)
	    {
	        m_gonggao = gonggao;
	        
	    }
	}
	void SetPaiMing(int paiming)
	{
	    m_paiMing = paiming;
	}
	uint16 GetPaiMing()
	{
	    return m_paiMing;
	}
	void SetHuoYue(int huoYue)
	{
	    m_huoyue = huoYue;
	}
	int GetHuoYue()
	{
	    return m_huoyue;
	}
	void SetTitle(int title)
	{
	    m_title = title;
	}
	int GetTitle()
	{
	    return m_title;
	}
	int GetRes1()
	{
	    return m_res1;
	}
	int GetRes2()
	{
	    return m_res2;
	}
	int GetRes3()
	{
	    return m_res3;
	}
	int GetRes4()
	{
	    return m_res4;
	}
	void SetRes1(int val)
	{
	    m_res1 = val;
	}
	void SetRes2(int val)
	{
	    m_res2 = val;
	}
	void SetRes3(int val)
	{
	    m_res3 = val;
	}
	void SetRes4(int val)
	{
	    m_res4 = val;
	}
	
	void SetBeiXuanZhan(time_t t);//设置被宣战
	bool IsBeiXuanZhan();//是否被宣战
	void SetXuanZhanBang(uint32 bId,time_t t);//设置宣战帮派id
	int GetXuanZhanBang();//得到宣战帮派id
	bool ZhongZhi(SZhongZhi &zhongZhi,CUser*,uint16 itemId);
	void MakeZZMsg(CNetMessage &msg);//
	void MakeUserRes(uint32 roleId,CNetMessage &msg);
	int GainResource(CUser *pUser,int id);
	void SaveZhongZhi(CDatabaseSql *pDb);
	void InitZhongZhi(bool query = true);
	//op=1 加速,op=2 破坏,op=3 修复
	bool DoFarm(int op,uint32 ind,SZhongZhi &zz);
	bool GetZhongZhiInfo(uint32 ind,SZhongZhi &zz);
	void Timer();
	void Save();
	bool ThieveResource(CUser *pUser,CBangPai *pBangPai,CNetMessage &msg);
	bool PoHuai(CUser *pUser,CBangPai *pBangPai,CNetMessage &msg);
	void MakeHistory(CNetMessage &msg);
	
	void SetState(uint8 s)
	{
	    m_state = s;
	}
	uint8 GetDimisss()
	{
	    return m_state;
	}
	void SetJieMing(char *row);
	void GetJieMeng(list<uint32> &jimengList);
	bool AddJieMeng(uint32 bId);
	void DelJieMeng(uint32 bId);
	void SetDelJMTime(time_t t)
	{
	    m_delJMTime = t;
	}
	time_t GetDelJMTime()
	{
	    return m_delJMTime;
	}
	bool JieMeng(CBangPai *pBangPai);
	//兵器阁 id=2 守御阁 id=3 百草堂 id=4 试炼堂
	//5 机密 6 侦查，7 反侦查
	int m_bqLevel;
	int m_syLevel;
	int m_bcLevel;
	int m_slLevel;
	int m_jmLevel;
	int m_zcLevel;
	int m_fzcLevel;
	static const uint32 JIE_MENG_MAX_NUM = 2;
private:
    boost::mutex    m_mutex;
    uint32 m_id;
    string m_name;
    list<uint32> m_adminUser;
    list<uint32> m_userList;
    list<uint32> m_askJoinUser;
    
    list<SBangPaiMember> m_allMember;
    //map<uint16,SZhongZhi> m_
    vector<SZhongZhi>   m_zhongZhi;//帮派种植
    vector<list<uint32> > m_thiefList;//偷窃列表
    list<string> m_histroy;//破坏、偷盗、腐烂记录
    
    uint8 m_level;
    int m_fanrong;
	int m_money;
	int m_title;
	uint8 m_huodongTime;
	uint16 m_paiMing;
	int m_huoyue;
	time_t m_beiXZTime;//被宣战时间
	time_t m_XZTime;//宣战时间
	uint32 m_XZBangId;//宣战帮派id
	time_t m_jisuanZZ;//计算种植时间
	time_t m_zaiHaiTime;//自然灾害计算时间
	time_t m_gonggaoTime;//发送帮派公告时间
	int m_res1;
	int m_res2;
	int m_res3;
	int m_res4;
	int m_ziJinDay;//减少帮派money日期
	time_t m_delJMTime;
	
	list<uint32> m_jieMengList;
	uint8 m_state;
	string m_kouhao;
	string m_gonggao;
};

class CNetMessage;

class CBangPaiManager
{
public:
    CBangPaiManager():m_bangPaiList(100)
    {
    }
    CBangPai *FindBangPai(uint32 id);
    CBangPai *CreateBangPai(CUser *pUser,const char *name);
    void MakeBangPaiList(uint8 page,CNetMessage &msg,int bId,bool haveMeBang = true);
    void MakeXZBang(CNetMessage &msg,int bId);
    
    void DelBangPai(uint32 id);
    void Erase(uint32 id);
    bool Init();
    void SaveZhongZhi();
    void Timer();
private:
    CHashTable<uint16,CBangPai*> m_bangPaiList;
    boost::mutex    m_mutex;
    int m_curId;
};

#endif
