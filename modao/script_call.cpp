#include "net_msg.h"
#include "socket_server.h"
#include "singleton.h"
#include "protocol.h"
#include "online_user.h"
#include "user.h"
#include "utility.h"
#include "script_call.h"
#include "database.h"
#include "call_script.h"
#include "huo_dong.h"
#include <lua.hpp>
#include <boost/thread.hpp>
#include <boost/format.hpp>

//世界大战开始时间（小时）
const int WW_BEGIN_TIME = 21;
//世界大战结束时间
const int WW_END_TIME = 22;

#ifdef NEW_QU
const int WW_DAY = -1;//4;
#else
const int WW_DAY = 6;//4;
#endif

const int sMaxMoney[] = {500000,700000,1000000,2000000,5000000};

int DialogT(CUser *pUser,const char *name,const char *text)
{
    CUser *p1 = GetTeamMember1(pUser);
    CUser *p2 = GetTeamMember2(pUser);
    Dialog(pUser,name,text);
    Dialog(p1,name,text);
    Dialog(p2,name,text);
    return 1;
}

int Dialog(CUser *pUser,const char *name,const char *text)
{
    if(pUser == NULL)
        return 1;
    CSocketServer &sock = SingletonSocket::instance();
    CNetMessage msg;
    msg.SetType(PRO_INTERACT);
    
    msg<<(uint8)1<<name<<text;
    sock.SendMsg(pUser->GetSock(),msg);
    return 1;
}

int Option(CUser *pUser,const char *name,const char *text,const char *opt)
{
    if(pUser == NULL)
        return 1;
        
    char *split[20];
    string str = opt;
    int num = SplitLine(split,20,(char*)str.c_str());
    if(num % 2 != 0)
    {
        //cout<<opt<<endl;
        cout<<"opt error"<<endl;
        return 0;
    }
    num /= 2;
    /********************
    TYPE=2 弹出选项 CONT格式为:
    +-----+-------+-----+-------+-----+-----+-----+
    | LEN | TITLE | NUM | OPTID | LEN | OPT | ... |
    +-----+-------+-----+-------+-----+-----+-----+
    |  2  |  Var  |  1  |   1   |  2  | Var | ... | 
    +-----+-------+-----+-------+-----+-----+-----+
    *********************/
    CSocketServer &sock = SingletonSocket::instance();
    CNetMessage msg;
    msg.SetType(PRO_INTERACT);
    
    msg<<(uint8)2<<name<<text<<(uint8)num;
    for(int i = 0; i < num; i++)
    {
        msg<<atoi(split[2*i])<<split[2*i+1];
    }
    sock.SendMsg(pUser->GetSock(),msg);
    return 1;
}

SItemTemplate *GetItem(int itemId)
{
    return SingletonItemManager::instance().GetItem(itemId);
}

void AddItemTmpl(SItemTemplate *pItem)
{
    SItemTemplate *pTemp = new SItemTemplate;
    *pTemp = *pItem;

    SingletonItemManager::instance().AddItem(pTemp);
}

void SysInfo(CUser *pUser,const char *info)
{
#ifdef DEBUG    
    cout<<"SysInfo:"<<endl
        <<info<<endl;
#endif
    SendSysInfo(pUser,info);
}

void SysInfo1(CUser *pUser,const char *info)
{
    SendSysInfo1(pUser,info);
}

void SMessage(CUser *pUser,const char *pMsg)
{
    CSocketServer &sock = SingletonSocket::instance();
    CNetMessage msg;
    msg.SetType(PRO_INTERACT);
    if(pUser == NULL)
        return;

    msg<<(uint8)5<<pMsg;
    sock.SendMsg(pUser->GetSock(),msg);
}

//物品选择
void SelectItem(CUser *pUser,int i,int j)
{
    CSocketServer &sock = SingletonSocket::instance();
    CNetMessage msg;
    msg.SetType(PRO_INTERACT);
    if(pUser == NULL)
        return;

    msg<<(uint8)3<<(uint8)i<<(uint8)j;
    sock.SendMsg(pUser->GetSock(),msg);
}

//选择宠物
void SelectPet(CUser *pUser,int petId,const char *name,const char *pMsg)
{
    if((pUser == NULL) || (pMsg == NULL) || (name == NULL))
        return;
    CSocketServer &sock = SingletonSocket::instance();
    CNetMessage msg;
    msg.SetType(PRO_INTERACT);
    
    msg<<(uint8)6<<name<<pMsg<<(uint16)petId;
    sock.SendMsg(pUser->GetSock(),msg);
}

void OpenPackage(CUser *pUser,int type)
{
    if(pUser == NULL)
        return;
    CSocketServer &sock = SingletonSocket::instance();
    CNetMessage msg;
    msg.SetType(PRO_INTERACT);
    
    msg<<(uint8)7<<(uint8)type;
    sock.SendMsg(pUser->GetSock(),msg);
}

/**************
用户学习技能金钱和潜能消耗：
***************/
static int GetMoney(uint16 skillId,uint8 skillLevel)
{
    uint8 skillType = GetSkillType(skillId);
    uint8 jie = GetSkillJie(skillId);
    int money = 0;
    if((skillId == 23) || (skillId == 24))
    {
        jie = 3;
        skillType = ESTDamageHp;
    }
    if(skillId == 21)
    {
        money = (int)(skillLevel * skillLevel * 4.5);
    }
    else if(skillType == ESTDamageHp)
    {//咒术
        /**********
        咒术：
        1阶：等级*等级*4
        2阶：等级*等级*5
        3阶：等级*等级*7
        4阶：等级*等级*8
        *************/
        switch(jie)
        {
        case 1:
            money = skillLevel * skillLevel * 4;
            break;
        case 2:
            money = skillLevel * skillLevel * 5;
            break;
        case 3:
            money = skillLevel * skillLevel * 7;
            break;
        case 4:
            money = skillLevel * skillLevel * 8;
            break;
        }
    }
    else if(skillType <= ESTFrost)
    {//负面
        /**********
        1阶：等级*等级*8.4
        2阶：等级*等级*10.5
        3阶：等级*等级*14.7
        4阶：等级*等级*16.8
        **********/
        switch(jie)
        {
        case 1:
            money = (int)(skillLevel * skillLevel * 8.4);
            break;
        case 2:
            money = (int)(skillLevel * skillLevel * 10.5);
            break;
        case 3:
            money = (int)(skillLevel * skillLevel * 14.7);
            break;
        case 4:
            money = (int)(skillLevel * skillLevel * 16.8);
            break;
        }       
    }
    else
    {/***********
        辅助系：
        1阶：等级*等级*4.8
        2阶：等级*等级*6
        3阶：等级*等级*8.4
        4阶：等级*等级*9.6
        ***************/
        switch(jie)
        {
        case 1:
            money = (int)(skillLevel * skillLevel * 4.8);
            break;
        case 2:
            money = (int)(skillLevel * skillLevel * 6);
            break;
        case 3:
            money = (int)(skillLevel * skillLevel * 8.4);
            break;
        case 4:
            money = (int)(skillLevel * skillLevel * 9.6);
            break;
        }       
    }
    return money;
}

//10级可学攻击技能，20级可学负面技能，40级可学辅助技能。10级可学1阶技能，
//20级可学2阶技能，40级可学3阶技能，60级可学4阶技能。
//1阶技能学到20级可学2阶技能，1阶和2阶技能学到40级后可学3阶技能，1阶，2阶，
//3阶技能都学到60级后可学4级技能。
static bool CanLearnSkill(CUser *pUser,uint16 id,uint8 skillLevel)
{
    if(skillLevel > pUser->GetLevel())
        return false;
    if(id == 22)
        return true;
    if(id == 121)
    {
        if(pUser->GetLevel() >= 20)
            return true;
        else 
            return false;
    }
    if(id == 23)
    {
        if((pUser->GetSrcSkillLevel(6) < 40)
            || (pUser->GetSrcSkillLevel(21) < 40)
            || (pUser->GetLevel() < 40))
        {
            return false;
        }
        return true;
    }
    else if(id == 24)
    {
        if((pUser->GetSrcSkillLevel(18) < 40)
            || (pUser->GetSrcSkillLevel(22) < 40)
            || (pUser->GetLevel() < 40))
        {
            return false;
        }
        return true;
    }
    
    uint8 skillType = GetSkillType(id);
    
    if(skillType == ESTDamageHp)
    {//咒术
        if(pUser->GetLevel() < 10)
            return false;
    }
    else if(skillType <= ESTFrost)
    {//负面     
        if(pUser->GetLevel() < 40)
            return false;
    }
    else
    {//辅助系：
        if(pUser->GetLevel() < 20)
            return false;
    }
    
    uint8 jie = GetSkillJie(id);
    
    switch(jie)
    {
    case 1:
        if(pUser->GetLevel() < 10)
            return false;
        break;
    case 2:
        if(id == 6) 
        {
            if(pUser->GetSrcSkillLevel(21) < 20)
                return false;
            else 
                return true;
        }
        else if(id == 18)
        {
            if(pUser->GetSrcSkillLevel(22) < 20)
                return false;
            else 
                return true;
        }
        else if(id == 110)
        {
            if(pUser->GetSrcSkillLevel(121) < 20)
                return false;
            else 
                return true;
        }
        if(pUser->GetLevel() < 20)
            return false;
        if(pUser->GetSrcSkillLevel(id-1) < 20)
            return false;
        break;
    case 3:
        if(id == 111)
        {
            if((pUser->GetSrcSkillLevel(121) < 40) || (pUser->GetSrcSkillLevel(110) < 40))
                return false;
            else 
                return true;
        }
        if(pUser->GetLevel() < 40)
            return false;
        if(pUser->GetSrcSkillLevel(id-1) < 40)
            return false;
        if(pUser->GetSrcSkillLevel(id-2) < 40)
            return false;
        break;
    case 4:
        if(pUser->GetLevel() < 60)
            return false;
        if(pUser->GetSrcSkillLevel(id-1) < 60)
            return false;
        if(pUser->GetSrcSkillLevel(id-2) < 60)
            return false;
        if(pUser->GetSrcSkillLevel(id-3) < 60)
            return false;
        break;
    }
    return true;
}

bool LearnSkill(CUser *pUser,int id)
{
    if(pUser == NULL)
        return false;
    uint16 skillId = id;
    uint8 skillLevel = pUser->GetSrcSkillLevel(skillId)+1;
    
    if(!CanLearnSkill(pUser,skillId,skillLevel))
        return false;
        
    int qianNeng = GetMoney(skillId,skillLevel);
    int money = qianNeng/3;
    if((pUser->GetMoney() < money) || (pUser->GetQianNeng() < (int)qianNeng))
    {
        return false;
    }
    pUser->AddMoney(-money);
    pUser->AddQianNeng(-qianNeng);
    pUser->SetSkill(skillId,skillLevel);
    return true;
}

void ListStudySkill(CUser *pUser,const char *skills)
{
    if((pUser == NULL) || (skills == NULL))
        return;
    char *p[50];
    string skill = skills;
    uint8 num = SplitLine(p,50,(char*)skill.c_str());
    CSocketServer &sock = SingletonSocket::instance();
    CNetMessage msg;
    msg.SetType(MSG_SERVER_LIST_SKILL);
    
    msg<<num;
    uint16 skillId = 0;
    for(uint8 i = 0; i < num; i++)
    {
        skillId = atoi(p[i]);
        msg<<skillId;
    }
    sock.SendMsg(pUser->GetSock(),msg);
}

void StudySkill(CUser *pUser,const char *skills)
{
    if((pUser == NULL) || (skills == NULL))
        return;
    char *p[50];
    string skill = skills;
    uint8 num = SplitLine(p,50,(char*)skill.c_str());
    CSocketServer &sock = SingletonSocket::instance();
    CNetMessage msg;
    msg.SetType(PRO_INTERACT);
    
    msg<<(uint8)8;
    msg<<num;
    uint16 skillId = 0;
    uint16 skillLevel = 0;
    int money = 0;
    int qianNeng = 0;
    for(uint8 i = 0; i < num; i++)
    {
        skillId = atoi(p[i]);
        skillLevel = pUser->GetSrcSkillLevel(skillId);//+1;
        qianNeng = GetMoney(skillId,skillLevel+1);
        money = qianNeng/3;
        msg<<skillId<<skillLevel<<qianNeng<<money;
        //cout<<"技能id:"<<(int)skillId
            //<<"技能等级:"<<(int)skillLevel<<endl;
    }
    sock.SendMsg(pUser->GetSock(),msg);
}

void SellItem(CUser *pUser,int type,const char *items)//items "1|2|3……"
{
    if((pUser == NULL) || (items == NULL))
        return;
        
    CSocketServer &sock = SingletonSocket::instance();
    
    char *split[100];
    string item = items;
    uint8 num = SplitLine(split,20,(char*)item.c_str());
    
    CNetMessage msg;
    msg.SetType(PRO_INTERACT);
    msg<<(uint8)4<<(uint8)type;
    uint16 pos = msg.GetDataLen();
    msg<<num;
    CItemTemplateManager &itemMgr = SingletonItemManager::instance();
    uint8 count = 0;
    for(uint8 i = 0; i < num; i++)
    {
        SItemTemplate *pItem = itemMgr.GetItem(atoi(split[i]));
        if(pItem != NULL)
        {
            //msg<<pItem->id;//<<(int)pItem->jiage;
            SItemInstance item = {0};
            item.tmplId = pItem->id;
            MakeItemInfo(&item,msg);
            msg<<(int)pItem->jiage;
            count++;
        }
    }
    msg.WriteData(pos,&count,sizeof(count));
    sock.SendMsg(pUser->GetSock(),msg);
}

void CloseInteract(CUser *pUser)
{
    if(pUser == NULL)
        return;
    CNetMessage msg;
    msg.SetType(PRO_CLOSE_INTERACT);
    SingletonSocket::instance().SendMsg(pUser->GetSock(),msg);
    pUser->CallScriptLevelUp();
}

void TransportUser(CUser *pUser,int sceneId,uint8 x,uint8 y,uint8 face)
{
    if(pUser == NULL)
        return;
    CSocketServer &sock = SingletonSocket::instance();
    CSceneManager &scene = SingletonSceneManager::instance();
    
    CScene *pScene = NULL;
    CScene *pOld = pUser->GetScene();
    if(pOld == NULL)
        return;
    if((sceneId == BANG_PAI_SCENE_ID) || (sceneId == 305))
    {
        if(pUser->GetBangPai() == 0)
            return;
        pScene = scene.GetBangPaiScene(sceneId,pUser->GetBangPai());
    }
    else if(pOld->GetGroupId() != 0)
    {
        pScene = scene.FindScene(sceneId,pOld->GetGroupId());
    }
        
    if(pScene == NULL)
    {
        pScene = scene.FindScene(sceneId);
    }
    if(pScene == NULL)
        return;
    
    if((pOld->GetMapId() >= 260) && (pOld->GetMapId() <= 266)
        && ((pScene->GetMapId() < 260) || (pScene->GetMapId() > 266)))
    {
        pUser->DelPackageById(1544,-1);
        pUser->DelPackageById(1543,-1);
        pUser->DelPackageById(1542,-1);
        pUser->SetXZBangId(0);
    }
    
    
    CNetMessage msg;
    msg.SetType(PRO_JUMP_SCENE);
    msg<<pScene->GetMapId()<<x<<y<<face;
    sock.SendMsg(pUser->GetSock(),msg);
    pUser->SetPos(x,y);
    pUser->SetFace(face);
    if(pUser->GetHp() <= 0)
        pUser->AddHp(1);
    if(pUser->GetSceneId() != sceneId)
        pUser->EnterScene(pScene);
}

void UserJumpTo(CUser *pUser,uint16 sceneId,uint8 x,uint8 y,uint8 face)
{
    if(pUser == NULL)
        return;
    pUser->SetFace(face);
    pUser->SetPos(x,y);
    SendUserPos(pUser);
}

bool EnterBangPaiScene(CUser *pUser,int bId)//14,14,8
//bool EnterBangPaiScene(CUser *pUser,uint8 x,uint8 y,uint8 face)
{
    if((pUser == NULL) || (pUser->GetBangPai() == 0))
        return false;
    CSocketServer &sock = SingletonSocket::instance();
    CSceneManager &scene = SingletonSceneManager::instance();
    
    CScene *pScene;
    if(bId == 0)
        pScene = scene.GetBangPaiScene(BANG_PAI_SCENE_ID,pUser->GetBangPai());
    else 
        pScene = scene.GetBangPaiScene(BANG_PAI_SCENE_ID,bId);
    if(pScene == NULL)
        return false;
    CNetMessage msg;
    msg.SetType(PRO_JUMP_SCENE);
    uint8 x = 14;
    uint8 y = 14;
    uint8 face = 8;
    msg<<BANG_PAI_SCENE_ID<<x<<y<<face;
    sock.SendMsg(pUser->GetSock(),msg);
    pUser->SetPos(x,y);
    pUser->SetFace(face);
    pUser->EnterScene(pScene);
    return true;
}

const char *GetMonsterName(int id)
{
    SMonsterTmpl *pMonster = SingletonMonsterManager::instance().GetTmpl(id);
    if(pMonster == NULL)    
        return NULL;
    return pMonster->name.c_str();
}

int AddDefaultNpc(CUser *pUser,int npcId,int scenseId,int x,int y,int timeOut)
{
    return AddNpc(pUser,npcId,(const char*)NULL,scenseId,x,y,timeOut);
}

static bool GetHumanData(int xiang,HumanData &human,string &name)
{
    CGetDbConnect getDb;
    CDatabaseSql *pDb = getDb.GetDbConnect();
    char sql[512];
    sprintf(sql,"SELECT role_id FROM `human_npc` where xiang=%d ORDER BY rand() LIMIT 1",xiang);
    if ((pDb == NULL) || (!pDb->Query(sql)))
    {
        return false;
    }
    char **row = pDb->GetRow();
    if(row == NULL)
        return false;
    //uint32 roldId = atoi(row[0]);
    human.roleId = atoi(row[0]);
    sprintf(sql,"select equipment,sex,name from role_info where id=%s",row[0]);
    if(pDb->Query(sql))
    {
        row = pDb->GetRow();
        if(row != NULL)
        {
            SItemInstance *pEquip = new SItemInstance[EETShouZhuo2+1];
            StrToHex(row[0],(uint8*)pEquip,sizeof(SItemInstance)*(EETShouZhuo2+1));
            human.weapon = pEquip[EETWuQi].tmplId;
            human.helmet = pEquip[EETMaoZi].tmplId;
            human.armor = pEquip[EETKuiJia].tmplId;
            human.level = pEquip[EETWuQi].level;
            human.sex = atoi(row[1]);
            human.helmetClass = pEquip[EETMaoZi].quality;
            human.armorClass = pEquip[EETKuiJia].quality;
            name = row[2];
            delete pEquip;
            return true;
        }
    }
    return false;
}

const char *GetUserNpcName(CUser *pUser,int npcId)
{
    if(pUser != NULL)
        return pUser->GetNpcName(npcId);
    return NULL;
}

int AddHumanNpc(CUser *pUser,int xiang,int npcId,int sceneId,int x,int y,int timeOut)
{
    /*
   +-----+--------+--------+--------+-------+-------+-------+
   | SEX | WEAPON | ULEVEL | HELMET | CLASS | ARMOR | CLASS |
   +-----+--------+--------+--------+-------+-------+-------+
   |  1  |    2   |    1   |   2    |    1  |   2   |   1   |
   +-----+--------+--------+--------+-------+-------+-------+
    */
    if(pUser == NULL)
        return 0;
    SNpcInstance npc;
    CNpcManager &npcManager = SingletonNpcManager::instance();
    SNpcTemplate *pNpc = npcManager.GetNpcTemplate(npcId);
    if(pNpc == NULL)
        return 0;
    
    HumanData human;
    string name;
    if(!GetHumanData(xiang,human,name))
        return 0;
    npc.pNpc = new SNpcTemplate;
    npc.pHumanData = new HumanData;
    *(npc.pHumanData) = human;
    *(npc.pNpc) = *pNpc;
    npc.pNpc->name = name;
    if(npc.pNpc == NULL)
        return 0;
        
    npc.id = npcId;
    npc.type = 1;
    npc.templateId  = npcId;
    npc.x = x;
    npc.y = y;
    if(timeOut != 0)
        npc.timeOut = timeOut*60+GetSysTime();

    npc.sceneId = sceneId;
    pUser->AddNpc(sceneId,npc);
    if(sceneId == pUser->GetScene()->GetId())
    {
        CNetMessage msg;
        msg.SetType(PRO_ADD_NPC);
        //msg<<npc.id<<npc.pNpc->name<<npc.x<<npc.y<<npc.pNpc->pic;
        npc.MakeNpcInfo(msg);
        SingletonSocket::instance().SendMsg(pUser->GetSock(),msg);
    }
    return npc.id;
}

int AddNpc(CUser *pUser,int npcId,const char *name,int sceneId,int x,int y,int timeOut)
{
    if(pUser == NULL)
        return 0;
    SNpcInstance npc;
    CNpcManager &npcManager = SingletonNpcManager::instance();
    SNpcTemplate *pNpc = npcManager.GetNpcTemplate(npcId);
    if(pNpc == NULL)
        return 0;
    npc.pNpc = new SNpcTemplate;
    *(npc.pNpc) = *pNpc;
    
    if(name != NULL)
        npc.pNpc->name = name;
    if(npc.pNpc == NULL)
        return 0;
        
    npc.id = npcId;
    npc.templateId  = npcId;
    npc.x = x;
    npc.y = y;
    if(timeOut != 0)
        npc.timeOut = timeOut*60+GetSysTime();

    npc.sceneId = sceneId;
    pUser->AddNpc(sceneId,npc);
    if(sceneId == pUser->GetScene()->GetMapId())//GetId())
    {
        CNetMessage msg;
        msg.SetType(PRO_ADD_NPC);
        npc.MakeNpcInfo(msg);
        SingletonSocket::instance().SendMsg(pUser->GetSock(),msg);
    }
    return npc.id;
}

int GetNpcSceneId(int npcId)
{
    CNpcManager &npcManager = SingletonNpcManager::instance();
    SNpcInstance *pNpc = npcManager.GetNpcInstance(npcId);
    if(pNpc != NULL)
        return pNpc->sceneId;
    return 0;
}

SNpcPos GetNpcScenePos(int npcId)
{
    SNpcPos pos = {0};
    CNpcManager &npcManager = SingletonNpcManager::instance();
    SNpcInstance *pNpc = npcManager.GetNpcInstance(npcId);
    if(pNpc == NULL)
        return pos;
    pos.sceneId = pNpc->sceneId;
    pos.x = pNpc->x;
    pos.y = pNpc->y;
    return pos;
}

//得到砙爸锌晒�
SNpcPos GetCanWalkPos(int sceneId)
{
    SNpcPos pos = {0};
    CSceneManager &scene = SingletonSceneManager::instance();
    CScene *pScene = scene.FindScene(sceneId);
    if(pScene == NULL)
        return pos;
    uint8 x = 0,y = 0;
    if(pScene->GetCanWalkPos(x,y))
    {
        pos.sceneId = sceneId;
        pos.x = x;
        pos.y = y;
    }
    return pos;
}

const char *GetNpcName(int npcId)
{
    CNpcManager &npcManager = SingletonNpcManager::instance();
    SNpcInstance *pNpc = npcManager.GetNpcInstance(npcId);
    if(pNpc != NULL)
        return pNpc->pNpc->name.c_str();
    return NULL;
}

void DelNpc(CUser *pUser,int npcId)
{
    if(pUser == NULL)
        return;
    
    SNpcInstance npc;
    if(pUser->DelNpc(npcId,npc) == pUser->GetScene()->GetMapId())//GetId())
    {
        CNetMessage msg;
        msg.SetType(PRO_DEL_NPC);
        msg<<npc.id;
        delete npc.pNpc;
        delete npc.pHumanData;
        SingletonSocket::instance().SendMsg(pUser->GetSock(),msg);
        //cout<<"del npc:"<<npcId<<endl;
    }
}

void BeginFight(CUser *pUser,int monsterId,int level)
{
    if((pUser != NULL) && (pUser->GetScene() != NULL))
    {
        pUser->GetScene()->EnterFight(pUser,monsterId,level);
    }
}

void ZhaoTao1(CUser *pUser)
{
    if((pUser != NULL) && (pUser->GetScene() != NULL))
    {
        pUser->GetScene()->ZhaoTao1(pUser);
    }
}
void ZhaoTao2(CUser *pUser)
{
    if((pUser != NULL) && (pUser->GetScene() != NULL))
    {
        pUser->GetScene()->ZhaoTao2(pUser);
    }
}
void ZhaoTao3(CUser *pUser)
{
    if((pUser != NULL) && (pUser->GetScene() != NULL))
    {
        pUser->GetScene()->ZhaoTao3(pUser);
    }
}
void HumanNpcFight(CUser *pUser,int npcId)
{
    if((pUser != NULL) && (pUser->GetScene() != NULL))
    {
        pUser->GetScene()->HumanNpcFight(pUser,npcId);
    }
}

void ShiMenFight(CUser *pUser)
{
    if((pUser != NULL) && (pUser->GetScene() != NULL))
    {
        pUser->GetScene()->ShiMenFight(pUser);
    }
}

void EpisodeBattle1(CUser *pUser)
{
    if((pUser != NULL) && (pUser->GetScene() != NULL))
    {
        pUser->GetScene()->EpisodeBattle1(pUser);
    }
}
void EpisodeBattle2(CUser *pUser)
{
    if((pUser != NULL) && (pUser->GetScene() != NULL))
    {
        pUser->GetScene()->EpisodeBattle2(pUser);
    }
}
void EpisodeBattle3(CUser *pUser)
{
    if((pUser != NULL) && (pUser->GetScene() != NULL))
    {
        pUser->GetScene()->EpisodeBattle3(pUser);
    }
}
void EpisodeBattle4(CUser *pUser)
{
    if((pUser != NULL) && (pUser->GetScene() != NULL))
    {
        pUser->GetScene()->EpisodeBattle4(pUser);
    }
}
void EpisodeBattle5(CUser *pUser)
{
    if((pUser != NULL) && (pUser->GetScene() != NULL))
    {
        pUser->GetScene()->EpisodeBattle5(pUser);
    }
}
void EpisodeBattle6(CUser *pUser)
{
    if((pUser != NULL) && (pUser->GetScene() != NULL))
    {
        pUser->GetScene()->EpisodeBattle6(pUser);
    }
}
void EpisodeBattle7(CUser *pUser)
{
    if((pUser != NULL) && (pUser->GetScene() != NULL))
    {
        pUser->GetScene()->EpisodeBattle7(pUser);
    }
}
void EpisodeBattle8(CUser *pUser)
{
    if((pUser != NULL) && (pUser->GetScene() != NULL))
    {
        pUser->GetScene()->EpisodeBattle8(pUser);
    }
}
void EpisodeBattle9(CUser *pUser)
{
    if((pUser != NULL) && (pUser->GetScene() != NULL))
    {
        pUser->GetScene()->EpisodeBattle9(pUser);
    }
}
void EpisodeBattle10(CUser *pUser)
{
    if((pUser != NULL) && (pUser->GetScene() != NULL))
    {
        pUser->GetScene()->EpisodeBattle10(pUser);
    }
}
void EpisodeBattle11(CUser *pUser)
{
    if((pUser != NULL) && (pUser->GetScene() != NULL))
    {
        pUser->GetScene()->EpisodeBattle11(pUser);
    }
}
void EpisodeBattle12(CUser *pUser)
{
    if((pUser != NULL) && (pUser->GetScene() != NULL))
    {
        pUser->GetScene()->EpisodeBattle12(pUser);
    }
}
void EpisodeBattle13(CUser *pUser)
{
    if((pUser != NULL) && (pUser->GetScene() != NULL))
    {
        pUser->GetScene()->EpisodeBattle13(pUser);
    }
}
void EpisodeBattle14(CUser *pUser)
{
    if((pUser != NULL) && (pUser->GetScene() != NULL))
    {
        pUser->GetScene()->EpisodeBattle14(pUser);
    }
}
void EpisodeBattle15(CUser *pUser)
{
    if((pUser != NULL) && (pUser->GetScene() != NULL))
    {
        pUser->GetScene()->EpisodeBattle15(pUser);
    }
}
void EpisodeBattle16(CUser *pUser)
{
    if((pUser != NULL) && (pUser->GetScene() != NULL))
    {
        pUser->GetScene()->EpisodeBattle16(pUser);
    }
}
void EpisodeBattle17(CUser *pUser)
{
    if((pUser != NULL) && (pUser->GetScene() != NULL))
    {
        pUser->GetScene()->EpisodeBattle17(pUser);
    }
}

void EpisodeBattle18(CUser *pUser)
{
    if((pUser != NULL) && (pUser->GetScene() != NULL))
    {
        pUser->GetScene()->EpisodeBattle18(pUser);
    }
}

void EpisodeBattle19(CUser *pUser)
{
    if((pUser != NULL) && (pUser->GetScene() != NULL))
    {
        pUser->GetScene()->EpisodeBattle19(pUser);
    }
}

void EpisodeBattle20(CUser *pUser)
{
    if((pUser != NULL) && (pUser->GetScene() != NULL))
    {
        pUser->GetScene()->EpisodeBattle20(pUser);
    }
}
void EpisodeBattle21(CUser *pUser)
{
    if((pUser != NULL) && (pUser->GetScene() != NULL))
    {
        pUser->GetScene()->EpisodeBattle21(pUser);
    }
}

void WenTianBattle(CUser *pUser)
{
    if((pUser != NULL) && (pUser->GetScene() != NULL))
    {
        pUser->GetScene()->WenTianBattle(pUser);
    }
}

void LanRuoBattle(CUser *pUser)
{
    if((pUser != NULL) && (pUser->GetScene() != NULL))
    {
        pUser->GetScene()->LanRuoBattle(pUser);
    }
}

//三阶段
void LanRuoBattle3(CUser *pUser)
{
    if((pUser != NULL) && (pUser->GetScene() != NULL))
    {
        pUser->GetScene()->LanRuoBattle3(pUser);
    }
}

//四阶段
void LanRuoBattle4(CUser *pUser)
{
    if((pUser != NULL) && (pUser->GetScene() != NULL))
    {
        pUser->GetScene()->LanRuoBattle4(pUser);
    }
}

//五阶段
void LanRuoBattle5(CUser *pUser)
{
    if((pUser != NULL) && (pUser->GetScene() != NULL))
    {
        pUser->GetScene()->LanRuoBattle5(pUser);
    }
}

void OpenPackBattle1(CUser *pUser)
{
    if((pUser != NULL) && (pUser->GetScene() != NULL))
    {
        pUser->GetScene()->OpenPackBattle1(pUser);
    }
}

void OpenPackBattle2(CUser *pUser)
{
    if((pUser != NULL) && (pUser->GetScene() != NULL))
    {
        pUser->GetScene()->OpenPackBattle2(pUser);
    }
}

void LouLuoFight(CUser *pUser)
{
    if((pUser != NULL) && (pUser->GetScene() != NULL))
    {
        pUser->GetScene()->LouLuoFight(pUser);
    }
}
void WuNianFight(CUser *pUser)
{
    if((pUser != NULL) && (pUser->GetScene() != NULL))
    {
        pUser->GetScene()->WuNianFight(pUser);
    }
}
void WuXinFight(CUser *pUser)
{
    if((pUser != NULL) && (pUser->GetScene() != NULL))
    {
        pUser->GetScene()->WuXinFight(pUser);
    }
}
void DengLongGuiFight(CUser *pUser)
{
    if((pUser != NULL) && (pUser->GetScene() != NULL))
    {
        pUser->GetScene()->DengLongGuiFight(pUser);
    }
}
void BaiYueFight(CUser *pUser)
{
    if((pUser != NULL) && (pUser->GetScene() != NULL))
    {
        pUser->GetScene()->BaiYueFight(pUser);
    }
}
void ShuiGuiFight(CUser *pUser)
{
    if((pUser != NULL) && (pUser->GetScene() != NULL))
    {
        pUser->GetScene()->ShuiGuiFight(pUser);
    }
}

void ChuShiFight(CUser *pUser)
{
    if((pUser != NULL) && (pUser->GetScene() != NULL))
    {
        pUser->GetScene()->ChuShiFight(pUser);
    }
}

void QiLinFightXian(CUser *pUser)
{
    if((pUser != NULL) && (pUser->GetScene() != NULL))
    {
        pUser->GetScene()->QiLinFightXian(pUser);
    }
}

void QiLinFightYao(CUser *pUser)
{
    if((pUser != NULL) && (pUser->GetScene() != NULL))
    {
        pUser->GetScene()->QiLinFightYao(pUser);
    }
}

int GetTeamMemNum(CUser *pUser)
{
    if((pUser == NULL) || (pUser->GetTeam() == 0))
        return 0;
    CScene *pScene = pUser->GetScene();
    if(pScene != NULL)
        return pScene->GetTeamMemNum(pUser->GetTeam());
    return 0;
}

CUser *GetTeamMember1(CUser *pUser)
{
    if(pUser == NULL)
        return NULL;
    CScene *pScene = pUser->GetScene();
    if(pScene != NULL)
        return pScene->GetTeamMember1(pUser->GetRoleId());
    return NULL;
}

CUser *GetTeamLeader(CUser *pUser)
{
    if(pUser == NULL)
        return NULL;
    if(pUser->GetTeam() == 0)
        return NULL;
    if(pUser->GetTeam() == pUser->GetRoleId())
        return pUser;
    ShareUserPtr ptr = SingletonOnlineUser::instance().GetUserByRoleId(pUser->GetTeam());
    return ptr.get();
}

CUser *GetTeamMember2(CUser *pUser)
{
    if(pUser == NULL)
        return NULL;
    CScene *pScene = pUser->GetScene();
    if(pScene != NULL)
        return pScene->GetTeamMember2(pUser->GetRoleId());
    return NULL;
}

const char *GetSceneName(int id)
{
    CSceneManager &scene = SingletonSceneManager::instance();
    CScene *pScene = scene.FindScene(id);
    if(pScene == NULL)
        return NULL;

    return pScene->GetName();
}

void UserRest(CUser *pUser)
{
    if(pUser == NULL)
        return;
    int addHp = pUser->GetMaxHp()-pUser->GetHp();
    if(addHp > 0)
        pUser->AddHp(addHp);
    int addMp = pUser->GetMaxMp() - pUser->GetMp();
    if(addMp > 0)
        pUser->AddMp(addMp);
}

const uint16 YAO_GUAI_LOU_LUO_ID    = 314;
const uint16 WU_XIN_ID  = 316;
const uint16 WU_NIAN_ID = 315;

void AddLouLuo(CUser *pUser)
{
    if(pUser == NULL)
        return;
    CScene *pScene = pUser->GetScene();
    if(pScene == NULL)
        return;
    pScene->AddNpc(YAO_GUAI_LOU_LUO_ID,true);
}

//添加无念
void AddWuNian(CUser *pUser)
{
    if(pUser == NULL)
        return;
    CScene *pScene = pUser->GetScene();
    if(pScene == NULL)
        return;
    pScene->AddNpc(WU_NIAN_ID,true);
}

//添加无心
void AddWuXin(CUser *pUser)
{
    if(pUser == NULL)
        return;
    CScene *pScene = pUser->GetScene();
    if(pScene == NULL)
        return;
    pScene->AddNpc(WU_XIN_ID,true);
}

void EnterLanRuoDiGong(CUser *pUser)
{
    if(pUser == NULL)
        return;
    
    CSceneManager &scene = SingletonSceneManager::instance();
    CScene *pScene = scene.GetLanRuoDiGong();
    if(pScene == NULL)
        return;
    if(pScene != NULL)
    {
        TransportUser(pUser,pScene->GetId(),22,8,8);
    }
}

void DelFaceNpc(CUser *pUser)
{
    if(pUser == NULL)
        return;
    CScene *pScene = pUser->GetScene();
    if(pScene == NULL)
        return;
    uint8 x,y;
    pUser->GetFacePos(x,y);
    pScene->DelNpc(x,y);
}

void UpdateNpcState(CUser *pUser,int npcId,int state)
{
    if(pUser == NULL)
        return;
        
    CSocketServer &sock = SingletonSocket::instance();
    CNetMessage msg;
    msg.SetType(PRO_UPDATE_NPC);
    msg<<(uint16)npcId<<(uint8)state;
    sock.SendMsg(pUser->GetSock(),msg);
}

bool CreateBangPai(CUser *pUser,const char *name,int type)
{
    if((type < 0) || (type > 1))
        return false;
    if(strlen(name) <= 2)
        return false;
    if(pUser == NULL)
        return false;
#ifdef QU_4
    const int costMoney = 1000000;
#else
    const int costMoney = 100000;
#endif

    if(type == 0)
    {
        if(pUser->GetMoney() < costMoney)
            return false;
    }
    
    if((type == 1) && !pUser->HaveItem(1816))
        return false;
    
    CBangPaiManager &bangPaiMgr = SingletonCBangPaiManager::instance();
    CBangPai *pBangPai = NULL;
    pBangPai = bangPaiMgr.CreateBangPai(pUser,name);
    if(pBangPai != NULL)
    {
        pUser->SetBangPai(pBangPai->GetId());
    }
    else
    {
        return false;
    }
    if(type == 0)
        pUser->AddMoney(-costMoney);
    else if(type == 1)
        pUser->DelPackageById(1816,1);
    pUser->UpdateBangPai();
    return true;
}

void InputStr(CUser *pUser,const char *pMsg)
{
    if((pUser == NULL) && (pMsg == NULL))
        return;
    CSocketServer &sock = SingletonSocket::instance();
    CNetMessage msg;
    msg.SetType(PRO_INTERACT);
    
    msg<<(uint8)9<<pMsg;
    sock.SendMsg(pUser->GetSock(),msg);
}

/*
TYPE=18 选择属性
+-----+
| IND |
+-----+
|  1  |
+-----+
物品在背包中的索引
*/
void SelectAttr(CUser *pUser,uint8 pos)
{
    if(pUser == NULL)
        return;
    CSocketServer &sock = SingletonSocket::instance();
    CNetMessage msg;
    msg.SetType(PRO_INTERACT);
    
    msg<<(uint8)18<<pos;
    sock.SendMsg(pUser->GetSock(),msg);
}

void BankMoney(CUser *pUser,int type)
{
    if(pUser == NULL)
        return;
    CSocketServer &sock = SingletonSocket::instance();
    CNetMessage msg;
    msg.SetType(PRO_INTERACT);
    
    msg<<(uint8)10<<(uint8)type<<pUser->GetBankMoney();
    sock.SendMsg(pUser->GetSock(),msg);
}

void ShiPinJianDing(CUser *pUser,int cost)
{
    if(pUser == NULL)
        return;
    CSocketServer &sock = SingletonSocket::instance();
    CNetMessage msg;
    msg.SetType(PRO_INTERACT);
    
    msg<<(uint8)12<<(uint16)cost;
    sock.SendMsg(pUser->GetSock(),msg);
}

//钱庄数： 用户等级*用户等级*1000
//钱庄加、减钱，函数内会把钱转移到玩家身上，money 为负既减钱
int AddBankMoney(CUser *pUser,int money)
{
    if(pUser == NULL)
        return 0;
    
    if(money > 0)
    {
        int maxMoney = pUser->GetLevel()*pUser->GetLevel()*1000;
        if(pUser->GetBankMoney() >= maxMoney)
            return 0;
        if(money > maxMoney - pUser->GetBankMoney())
            money = maxMoney - pUser->GetBankMoney();
            
        if(pUser->GetMoney() < money)
            return 0;
        pUser->SetBankMoney(pUser->GetBankMoney() + money);
        pUser->AddMoney(-money);
    }
    else
    {
        money = -money;
        if(pUser->GetBankMoney() < money)
            return 0;
        pUser->SetBankMoney(pUser->GetBankMoney() - money);
        pUser->AddMoney(money);
    }
    return money;
}

//发送钱庄物品
void SendBankItem(CUser *pUser)
{
    if(pUser == NULL)
        return;
    CSocketServer &sock = SingletonSocket::instance();
    CNetMessage msg;
    msg.SetType(PRO_INTERACT);
    
    msg<<(uint8)11;
    pUser->MakeBankItemList(msg);
    sock.SendMsg(pUser->GetSock(),msg);    
}

//添加钱庄物品，函数内会从包裹中删除
bool AddbankItem(CUser *pUser,uint8 pos,uint8 num)
{
    if(pUser == NULL)
        return false;
    return pUser->MoveItemToBank(pos,num);
}

//获取物品，函数内会加入包裹
bool GetBankItem(CUser *pUser,uint8 pos,uint8 num)
{
    if(pUser == NULL)
        return false;
    return pUser->MoveItemToPack(pos,num);
}

SNpcPos GetNpcPos(CUser *pUser)
{
    SNpcPos pos = {0};
    if(pUser == NULL)
        return pos;
                           //id,等级,三个坐标(需要除16)
    const uint16 NPC_POS[] = {
        3,10,288,64,400,80,320,192
        ,54,10,192,144,368,176,432,208
        ,41,15,144,240,320,240,48,288
        ,45,15,80,80,272,112,48,176
        ,51,15,384,48,240,112,144,224
        ,88,15,80,176,352,256,128,352
        ,94,15,80,80,400,96,384,432
        ,124,15,320,112,368,224,176,384
        ,55,20,192,96,336,224,208,368
        ,72,20,32,144,64,336,384,336
        ,87,20,144,96,352,176,192,352
        ,90,20,160,128,384,64,112,336
        ,123,20,336,64,96,352,400,400
        ,125,20,160,80,80,368,336,384
        ,56,25,192,64,336,112,80,416
        ,71,25,208,256,416,240,32,384
        ,86,25,64,80,320,224,256,384
        ,91,25,288,80,64,208,224,400
        ,113,25,336,80,48,144,400,384
        ,126,25,352,96,128,320,288,384
        ,57,30,176,96,304,304,96,384
        ,69,30,128,112,352,160,96,336
        ,85,30,160,144,320,208,368,400
        ,89,30,352,96,240,144,144,336
        ,92,30,208,144,400,192,272,368
        ,112,30,416,144,144,224,304,416
        ,58,35,320,160,96,208,400,352
        ,59,35,224,48,80,256,320,336
        ,67,35,48,102,272,256,320,400
        ,82,35,96,80,192,208,352,320
        ,84,35,96,176,48,320,400,208
        ,111,35,80,112,336,192,208,320
        ,157,35,80,80,288,256,224,384
        ,49,40,96,64,320,128,160,256
        ,61,40,96,208,384,272,176,336
        ,68,40,256,208,112,256,336,432
        ,83,40,128,80,384,208,192,304
        ,96,40,128,112,384,176,208,304
        ,114,40,80,96,368,128,96,320
        ,46,45,208,176,384,80,320,288
        ,52,45,96,80,400,224,192,368
        ,81,45,128,144,400,144,48,272
        ,93,45,160,112,400,160,144,384
        ,95,45,160,128,384,304,192,320
        ,122,45,80,96,352,160,240,384
        ,151,45,48,112,352,96,400,368
        ,50,50,224,96,160,256,288,320
        ,53,50,96,128,288,160,416,96
        ,97,50,224,128,320,208,160,368
        ,121,50,144,112,272,192,96,400
        ,152,50,64,80,288,192,384,400
        ,42,55,144,80,336,288,64,256
        ,43,55,128,64,240,256,80,400
        ,98,55,208,80,368,256,240,304
        ,115,55,160,112,96,240,224,352
        ,153,55,352,176,64,256,400,416
        ,44,60,96,96,432,80,384,224
        ,48,60,144,112,304,160,224,268
        ,99,60,80,144,288,176,192,336
        ,116,60,128,128,176,384,384,320
        ,154,60,64,176,256,240,64,416
        ,60,65,80,96,384,144,176,256
        ,63,65,128,112,288,240,64,320
        ,70,65,64,224,400,96,240,368
        ,100,65,304,112,96,336,288,384
        ,117,65,112,112,192,288,352,352
        ,155,65,208,80,272,240,96,384
        ,65,70,128,128,368,256,128,416
        ,66,70,144,112,352,192,256,416
        ,101,70,144,96,176,368,384,352
        ,118,70,144,80,336,192,176,416
        ,156,70,224,224,96,400,416,432
        ,47,75,288,96,160,176,304,336
        ,64,75,176,96,208,288,384,400
        ,102,75,64,224,325,160,256,304
        ,119,75,192,128,64,304,352,352
        ,62,80,240,160,64,320,208,416
        ,103,80,32,112,240,144,272,400
        ,120,80,176,64,304,224,160,416
        ,127,80,352,80,288,288,48,336
        ,131,85,96,128,304,144,128,320
        ,132,85,96,112,336,80,48,384
        ,133,85,48,144,288,224,176,352
        ,134,85,80,112,416,192,320,400
        ,135,90,48,128,128,352,320,384
        ,136,90,48,112,272,240,80,288
        ,137,90,32,176,288,304,160,432
        ,138,95,144,160,288,320,160,352
        ,139,95,224,96,272,256,192,384
        ,140,95,96,160,432,240,192,352
        ,141,95,160,208,80,272,384,400
        ,142,100,64,144,400,144,224,304
    };
    int num = sizeof(NPC_POS)/sizeof(uint16)/8;
    uint8 level = pUser->GetLevel();
    level = ((level/5)+1)*5;
    int maxPos = 0;
    for(int i = num-1; i >= 0; i--)
    {
        if(NPC_POS[8*i+1] == level)
        {
            maxPos = i;
            break;
        }
    }
    int randPos = Random(0,maxPos);
    pos.sceneId = NPC_POS[8*randPos];
    int xyPos = Random(0,2);
    pos.x = NPC_POS[8*randPos+2+2*xyPos]/16;
    pos.y = NPC_POS[8*randPos+2+2*xyPos+1]/16;
    return pos;
}


SMonsterTmpl *GetMonster(int id)
{
    return SingletonMonsterManager::instance().GetTmpl(id);
}

void ShiBaiChengFa(CUser *pUser)
{
    if(pUser == NULL)
        return;
        
    stringstream info;
	pUser->DecreaseAllNaiJiu();
    info<<"所有装备耐久减少10%";
    if(pUser->GetHp() <= 0)
    {
        pUser->AddHp(pUser->GetMaxHp());
    }
    int addExp = GetLevelUpExp(pUser->GetLevel());
    addExp = (int)(addExp*5/100);
    pUser->AddExp(-addExp);
    info<<"|经验减少:"<<addExp;
    if(Random(1,10) == 1)
    {
        string name;
        pUser->DropItem(name);
        if(name.size() > 0)
        {
            info<<"|掉落装备:"<<name;
        }
    }
    
    CScene *pScene = pUser->GetScene();
    if(pScene != NULL)
        pScene->LeaveTeam(pUser);
    TransportUser(pUser,1,13,13,8);
	SendPopMsg(pUser,info.str().c_str());
}

void AddBattleRes(CUser *pUser,int scriptId,int monsterId)
{
    if(pUser == NULL)
        return;
    pUser->AddMonsterScript(monsterId,scriptId);
}

void RemoveBattleRes(CUser *pUser,int scriptId,int monsterId)
{
    if(pUser == NULL)
        return;
    pUser->DelMonsterScript(monsterId,scriptId);
}

bool ExecMonsterScript(CUser *pUser,int monsterId,int num)
{
    list<int> scriptList;
    pUser->FindMonsterScript(monsterId,&scriptList);
    if(!scriptList.empty())
    {
        for(list<int>::iterator i = scriptList.begin(); i != scriptList.end(); i++)
        {
            CCallScript *pMonsterScript = FindScript(*i);//(buf);
            if(pMonsterScript != NULL)
            {
                pUser->SetCallScript(*i);
                int result = 1;
                pMonsterScript->Call("NotifyRes","uii>i",pUser,monsterId,num,&result);
                if(result != 0)
                    return true;
            }
        }
        return true;
    }
    return false;
}

void CompoundOrnament(CUser *pUser)
{
    if(pUser == NULL)
        return;
    CSocketServer &sock = SingletonSocket::instance();
    CNetMessage msg;
    msg.SetType(PRO_INTERACT);
    msg<<(uint8)13;
    sock.SendMsg(pUser->GetSock(),msg);
}

void DoItem(CUser *pUser,int stype)
{
    if(pUser == NULL)
        return;
    CSocketServer &sock = SingletonSocket::instance();
    CNetMessage msg;
    msg.SetType(PRO_INTERACT);
    msg<<(uint8)14<<(uint8)stype;
    sock.SendMsg(pUser->GetSock(),msg);
}

void CompoundItem(CUser *pUser,const char *str)
{
    if((pUser == NULL) || (str == NULL))
        return;
    CSocketServer &sock = SingletonSocket::instance();
    CNetMessage msg;
    msg.SetType(PRO_INTERACT);
    msg<<(uint8)15;
    char *p[20];
    string s = str;
    uint8 num = SplitLine(p,20,(char*)s.c_str());
    msg<<num;
    for(uint8 i = 0; i < num; i++)
    {
        msg<<(uint16)atoi(p[i]);
    }
    sock.SendMsg(pUser->GetSock(),msg);
}

void CompoundMaterial(CUser *pUser,int id)
{
    if(pUser == NULL)
        return;
    uint16 heChengItem[] = {601,602,603,401,
                            604,605,606,451,
                            607,608,609,501};
    for(uint8 i = 0; i < sizeof(heChengItem)/sizeof(uint16)/4; i++)
    {
        if(id == heChengItem[4*i+3])
        {
            CSocketServer &sock = SingletonSocket::instance();
            CNetMessage msg;
            msg.SetType(PRO_INTERACT);
            msg<<(uint8)16<<(uint16)id;
            msg<<(uint8)3
                <<heChengItem[4*i]<<(uint8)0<<(uint8)1
                <<heChengItem[4*i+1]<<(uint8)0<<(uint8)1
                <<heChengItem[4*i+2]<<(uint8)0<<(uint8)1<<0;
            sock.SendMsg(pUser->GetSock(),msg);
            return;
        }
    }
    if(((id >= 402) && (id <= 412))
        || ((id >= 452) && (id <= 462))
        || ((id >= 502) && (id <= 512)))
    {
        CSocketServer &sock = SingletonSocket::instance();
        CNetMessage msg;
        msg.SetType(PRO_INTERACT);
        msg<<(uint8)16<<(uint16)id<<(uint8)0;
        msg<<(uint8)1<<(uint16)(id-1)<<(uint8)3<<0;
        sock.SendMsg(pUser->GetSock(),msg);
        return;
    }
    const uint16 minKaiJIaId = 800;
    const uint16 maxKaiJiaId = 810;
    
    if((id >= minKaiJIaId) && (id <= maxKaiJiaId))
    {
        uint16 items[] = {640,641,642};
        
        int money[] = {2000,5000,15000,20000,30000,50000,80000,120000,200000,300000};
        uint8 num = id - minKaiJIaId + 1;
        CSocketServer &sock = SingletonSocket::instance();
        CNetMessage msg;
        msg.SetType(PRO_INTERACT);
        msg<<(uint8)16<<(uint16)id;
        if(id == minKaiJIaId)
        {
            msg<<(uint8)3
                <<items[0]<<num<<(uint8)3
                <<items[1]<<num<<(uint8)3
                <<items[2]<<num<<(uint8)3<<money[id-minKaiJIaId];
        }
        else
        {
            msg<<(uint8)4
                <<(uint16)(id-1)<<(uint8)0<<(uint8)1
                <<items[0]<<num<<(uint8)3
                <<items[1]<<num<<(uint8)3
                <<items[2]<<num<<(uint8)3<<money[id-minKaiJIaId];
        }
        sock.SendMsg(pUser->GetSock(),msg);
        return;
    }
}

static bool sHuoDong = false;
static int sHuoDongBeiLv = 1;

void SetHuoDong(bool flag)
{
    sHuoDong = flag;
}
void SetHuoDongBeiLv(int beilv)
{
    if(beilv > 0)
        sHuoDongBeiLv = beilv;
    else
        sHuoDongBeiLv = 1;
}
bool InHuoDong()
{
    return sHuoDong;
}
int GetHuoDongBeiLv()
{
    //cout<<sHuoDongBeiLv<<endl;
    return sHuoDongBeiLv;
}
void RankLevel(CUser *pUser,int type,char *tag)
{
    if(pUser == NULL)
        return;
    CSocketServer &sock = SingletonSocket::instance();
    CNetMessage msg;
    msg.SetType(PRO_INTERACT);
    
    msg<<(uint8)17;
    
    CGetDbConnect getDb;
    CDatabaseSql *pDb = getDb.GetDbConnect();
                       //0      1       2      
    char sql[256];
    snprintf(sql,256,"select role_id,level,role_name from level_rank where type=%d "\
        "order by rank",type);
    uint8 num = 0;
    msg<<(uint8)type<<tag;
    uint16 pos = msg.GetDataLen();
    msg<<num;
    if ((pDb != NULL)
        && (pDb->Query(sql)))
    {
        char **row;
        while ((row = pDb->GetRow()) != NULL)
        {
            num++;
            msg<<(int)atoi(row[0])<<row[2]<<atoi(row[1]);
        }
    }
    msg.WriteData(pos,&num,1);
    sock.SendMsg(pUser->GetSock(),msg);
}
void RankLevel(CUser *pUser,int type)
{
    if(pUser == NULL)
        return;
    CSocketServer &sock = SingletonSocket::instance();
    CNetMessage msg;
    msg.SetType(PRO_INTERACT);
    
    msg<<(uint8)17;
    
    CGetDbConnect getDb;
    CDatabaseSql *pDb = getDb.GetDbConnect();
                       //0      1       2      
    char sql[256];
    snprintf(sql,256,"select role_id,level,role_name from level_rank where type=%d "\
        "order by rank",type);
    uint8 num = 0;
    msg<<(uint8)type;
    uint16 pos = msg.GetDataLen();
    msg<<num;
    if ((pDb != NULL)
        && (pDb->Query(sql)))
    {
        char **row;
        while ((row = pDb->GetRow()) != NULL)
        {
            num++;
            msg<<(int)atoi(row[0])<<row[2]<<atoi(row[1]);
        }
    }
    msg.WriteData(pos,&num,1);
    sock.SendMsg(pUser->GetSock(),msg);
}

void DuiHuanBG(CUser *pUser,char *info)
{
    if((pUser == NULL) || (info == NULL))
        return;
    string str = info;
    char *split[90];
    int num = SplitLine(split,90,(char*)str.c_str());
    if(num % 3 != 0)
        return;
    CSocketServer &sock = SingletonSocket::instance();
    CNetMessage msg;
    msg.SetType(PRO_INTERACT);
    msg<<(uint8)21;
    msg<<pUser->GetData32(5)<<(uint8)(num/3);
    for(int i = 0; i < num; i += 3)
    {
        msg<<(uint8)atoi(split[i])<<(uint16)atoi(split[i+1])<<atoi(split[i+2]);
    }
    sock.SendMsg(pUser->GetSock(),msg);
}

void GetBangHistory(CUser *pUser)
{
    if(pUser == NULL)
        return;
    CBangPaiManager &bPMgr = SingletonCBangPaiManager::instance();
    CBangPai *pBangPai = bPMgr.FindBangPai(pUser->GetBangPai());
    if(pBangPai == NULL)
        return;
    
    CSocketServer &sock = SingletonSocket::instance();
    CNetMessage msg;
    msg.SetType(PRO_INTERACT);
    msg<<(uint8)23;
    pBangPai->MakeHistory(msg);
    sock.SendMsg(pUser->GetSock(),msg);
}

void DonateBang(CUser *pUser)
{
    if(pUser == NULL)
        return;
    CBangPaiManager &bPMgr = SingletonCBangPaiManager::instance();
    CBangPai *pBangPai = bPMgr.FindBangPai(pUser->GetBangPai());
    if(pBangPai == NULL)
        return;
    
    CSocketServer &sock = SingletonSocket::instance();
    CNetMessage msg;
    msg.SetType(PRO_INTERACT);
    msg<<(uint8)22;
    
    int money = sMaxMoney[4];
    if((pBangPai->GetLevel() >= 1) && (pBangPai->GetLevel() <= 5))
        money = sMaxMoney[pBangPai->GetLevel()-1];
    msg<<pBangPai->GetMoney()<<money;
    sock.SendMsg(pUser->GetSock(),msg);
}

//返回0 未领过
//返回1 已经领过奖 中奖
//返回2 已经领过 未中奖
//返回-1 不在领奖名单中
int GetAward2(CUser *pUser)
{
    if(pUser == NULL)
        return -1;
    CGetDbConnect getDb;
    CDatabaseSql *pDb = getDb.GetDbConnect();
    char sql[128];
    snprintf(sql,127,"select state from user_award2 where role_id=%d",pUser->GetRoleId());
    
    char **row;
    if ((pDb != NULL)
        && (pDb->Query(sql))
        && ((row = pDb->GetRow()) != NULL))
    {
        return atoi(row[0]);
    }
    return -1;
}

void SetAword2(CUser *pUser,int state)
{
    if(pUser == NULL)
        return;
    CGetDbConnect getDb;
    CDatabaseSql *pDb = getDb.GetDbConnect();
    char sql[128];
    snprintf(sql,127,"update user_award2 set state=%d,time=%lu where role_id=%d",
        state,GetSysTime(),pUser->GetRoleId());
    
    if (pDb != NULL)
        pDb->Query(sql);
}

// <0没有重奖，0重奖已领过，>0重奖物品
SUserAward GetAward(CUser *pUser)
{
    SUserAward aw;
    aw.id = -1;
    aw.num = 0;
    if(pUser == NULL)
    {
        return aw;
    }
    CGetDbConnect getDb;
    CDatabaseSql *pDb = getDb.GetDbConnect();
    char sql[128];
    snprintf(sql,127,"select item_id,state,num from user_award where role_id=%d",
                pUser->GetRoleId());
    
    char **row;
    if ((pDb != NULL) && (pDb->Query(sql)))
    {
        while((row = pDb->GetRow()) != NULL)
        {
            if(atoi(row[1]) == 0)
            {
                aw.id = atoi(row[0]);
                aw.num = atoi(row[2]);
                return aw;
            }
            else
            {
                aw.id = 0;
            }
        }
    }
    return aw;
}

//设置玩家已经领取奖励
void SetGetAword(CUser *pUser)
{
    if(pUser == NULL)
        return;
    CGetDbConnect getDb;
    CDatabaseSql *pDb = getDb.GetDbConnect();
    char sql[128];
    snprintf(sql,127,"update user_award set get_time=FROM_UNIXTIME(%lu),state=1 where role_id=%d and state!=1 limit 1",
                GetSysTime(),pUser->GetRoleId());
    
    if (pDb != NULL)
    {
        pDb->Query(sql);
    }
}

int GetHour()
{
    time_t t = GetSysTime();
    struct tm *pTm = localtime(&t);
    if(pTm == NULL)
        return 0;
    return pTm->tm_hour;
}

void SaveDate(CUser *pUser,int type,int data)
{
    if(pUser == NULL)
        return;
    CGetDbConnect getDb;
    CDatabaseSql *pDb = getDb.GetDbConnect();
    char sql[128];
    snprintf(sql,127,"INSERT INTO script_save (user_id,type,data,time) "\
                "VALUES (%d,%d,%d,FROM_UNIXTIME(%lu))",
                pUser->GetRoleId(),type,data,GetSysTime());
    if (pDb != NULL)
    {
        pDb->Query(sql);
    }
}

//提升师父等级
void upgrade_master(CUser *pUser)
{
    if(pUser == NULL)
        return;
    pUser->SetData8(0,pUser->GetData8(0)+1);
    if(pUser->GetData8(0) == 3)
    {
        pUser->AddTitle(EUTShiFu);
    }
}

//获得师傅名字
const char *get_master(CUser *pUser)
{
    static char buf[96];
    
    if(pUser == NULL)
        return NULL;
    CGetDbConnect getDb;
    CDatabaseSql *pDb = getDb.GetDbConnect();
    if(pDb == NULL)
        return NULL;
        
    char sql[128];
    snprintf(sql,127,"select master from master_prentice where p_id=%d limit 1",pUser->GetRoleId());
    
    if(!pDb->Query(sql))
        return NULL;
    char **row = pDb->GetRow();
    if(row != NULL)
    {
        snprintf(buf,96,"%s",row[0]);
        return buf;
    }
    return NULL;
}

//获得徒弟列表
const char *get_disciple(CUser *pUser)
{
    static char buf[96];
    
    if(pUser == NULL)
        return NULL;
    CGetDbConnect getDb;
    CDatabaseSql *pDb = getDb.GetDbConnect();
    if(pDb == NULL)
        return NULL;
        
    char sql[128];
    snprintf(sql,127,"select prentice from master_prentice where m_id=%d",pUser->GetRoleId());
    
    if(!pDb->Query(sql))
        return NULL;
    char **row;
    int len = 0;
    buf[0] = 0;
    while((row = pDb->GetRow()) != NULL)
    {
        if(len > 0)
        {
            strcat(buf,"|");
            len++;
        }
        if(len < 96)
            snprintf(buf+len,96-len,"%s",row[0]);
        len = strlen(buf);
    }
    if(buf[0] == 0)
        return NULL;
    return buf;
}

//是否有师父
bool have_master(CUser *pUser)
{
    if(pUser == NULL)
        return false;
    CGetDbConnect getDb;
    CDatabaseSql *pDb = getDb.GetDbConnect();
    if(pDb == NULL)
        return false;
    char sql[128];
    snprintf(sql,127,"select prentice from master_prentice where p_id=%d limit 1",pUser->GetRoleId());
    
    if(!pDb->Query(sql))
        return false;
    char **row = pDb->GetRow();
    if(row == NULL)
        return false;
    return true;
}

uint32 GetMaster(CUser *pUser)
{
    if(pUser == NULL)
        return 0;
    CGetDbConnect getDb;
    CDatabaseSql *pDb = getDb.GetDbConnect();
    if(pDb == NULL)
        return 0;
    char sql[128];
    snprintf(sql,127,"select m_id from master_prentice where p_id=%d limit 1",pUser->GetRoleId());
    
    if(!pDb->Query(sql))
        return 0;
    char **row = pDb->GetRow();
    if(row != NULL)
        return atoi(row[0]);
    return 0;
}

//是否有徒弟
bool have_disciple(CUser *pUser)
{
    if(pUser == NULL)
        return false;
    CGetDbConnect getDb;
    CDatabaseSql *pDb = getDb.GetDbConnect();
    if(pDb == NULL)
        return false;
    char sql[128];
    snprintf(sql,127,"select prentice from master_prentice where m_id=%d limit 1",pUser->GetRoleId());
    
    if(!pDb->Query(sql))
        return false;
    char **row = pDb->GetRow();
    if(row == NULL)
        return false;
    return true;
}

void ChuShi(CUser *pUser)
{
    if(pUser == NULL)
        return;
    
    CGetDbConnect getDb;
    CDatabaseSql *pDb = getDb.GetDbConnect();
    if(pDb == NULL)
        return;
    char sql[256];
    
    snprintf(sql,127,"select p_level,m_id from master_prentice where p_id=%d",pUser->GetRoleId());
    if(!pDb->Query(sql))
        return;
    char **row = pDb->GetRow();
    if(row == NULL)
        return;
    uint8 level = atoi(row[0]);
    uint32 masterId = atoi(row[1]);
    snprintf(sql,127,"delete from master_prentice where p_id=%d",pUser->GetRoleId());

    pDb->Query(sql);
    
    uint8 type = 3;
    snprintf(sql,256,"INSERT INTO mp_award (role_id,role_level,type,award_type) VALUES (%d,%d,1,%d)",
        pUser->GetRoleId(),level,type);
    pDb->Query(sql);
    
    CUser *p = GetTeamMember1(pUser);
    if(p != NULL)
    {
        snprintf(sql,256,"INSERT INTO mp_award (role_id,role_level,type,award_type) VALUES (%d,%d,0,%d)",
            p->GetRoleId(),level,type);
        pDb->Query(sql);
        p->SetData8(3,p->GetData8(3)+1);
    }
    else
    {
        char buf[64];
        snprintf(buf,64,"您的徒弟%s已经独自出师。",pUser->GetName());
        SendSysMail(masterId,buf);
    }
}

//解散师徒关系(徒弟调用)
void cancel_master(CUser *pUser)
{
    if(pUser == NULL)
        return;
    CGetDbConnect getDb;
    CDatabaseSql *pDb = getDb.GetDbConnect();
    if(pDb == NULL)
        return;
    char sql[128];
    snprintf(sql,127,"delete from master_prentice where p_id=%d",pUser->GetRoleId());
    pDb->Query(sql);
    pUser->SetData32(0,GetSysTime());
}

//解散师徒关系(师父调用)
void cancel_disciple(CUser *pUser,int id)
{
    if(pUser == NULL)
        return ;
    CGetDbConnect getDb;
    CDatabaseSql *pDb = getDb.GetDbConnect();
    if(pDb == NULL)
        return;
    char sql[128];
    snprintf(sql,127,"select prentice from master_prentice where m_id=%d limit %d,1",
        pUser->GetRoleId(),id);
    
    if(!pDb->Query(sql))
        return;
    char **row = pDb->GetRow();
    if(row == NULL)
        return;
    
    snprintf(sql,127,"delete from master_prentice where prentice='%s'",row[0]);
    pDb->Query(sql);
    pUser->SetData32(0,GetSysTime());
}

int master_level(CUser *pUser)
{
    if(pUser == NULL)
        return 0;
    return pUser->GetData8(0);
}
//是否可以做师父
bool is_master(CUser *pUser)
{
    if(pUser == NULL)
        return false;
    CGetDbConnect getDb;
    CDatabaseSql *pDb = getDb.GetDbConnect();
    if(pDb == NULL)
        return false;
    if(pUser->GetData8(0) == 0)
        return false;
    char sql[128];
    snprintf(sql,127,"select count(prentice) from master_prentice where m_id=%d",pUser->GetRoleId());
    
    if(!pDb->Query(sql))
        return false;
    char **row = pDb->GetRow();
    if(row == NULL)
        return true;
    
    if(atoi(row[0]) >= 3)
        return false;
    return true;
}

//收徒
int do_master(CUser *pUser)
{
    if(pUser == NULL)
        return 1;
    CUser *p = GetTeamMember1(pUser);
    if(p == NULL)
        return 1;
    if(pUser->GetData32(0) + 3*24*3600 > (uint32)GetSysTime())
        return 1;
    
    if(p->GetData32(0) + 3*24*3600 > (uint32)GetSysTime())
        return 2;
    
    char sql[128];
    CGetDbConnect getDb;
    CDatabaseSql *pDb = getDb.GetDbConnect();
    if(pDb == NULL)
        return 1;
        
    snprintf(sql,127,"select count(p_id) from master_prentice where m_id=%d",pUser->GetRoleId());
    
    if(pDb->Query(sql))
    {
        char **row = pDb->GetRow();
        if(row != NULL)
        {
            if(atoi(row[0]) >= pUser->GetData8(0)+3)
                return 3;
        }
    }

    snprintf(sql,127,"INSERT INTO master_prentice (master,m_id,prentice,p_id,p_level,val) VALUES ("\
            "'%s',%d,'%s',%d,%d,%d)",
            pUser->GetName(),pUser->GetRoleId(),
            p->GetName(),p->GetRoleId(),p->GetLevel(),0);
    pDb->Query(sql);
    return 0;
}

bool GetDiscipleAward(CUser *pUser,int type)
{
    if(pUser == NULL)
        return false;
    CGetDbConnect getDb;
    CDatabaseSql *pDb = getDb.GetDbConnect();
    if(pDb == NULL)
        return false;
                
    char sql[256];
    snprintf(sql,256,
        "select role_id from mp_award where role_id=%d and type=1 and award_type=%d",
        pUser->GetRoleId(),type);
    if(pDb->Query(sql))
    {
        char **row = pDb->GetRow();
        if(row != NULL)
            return true;
    }
    return false;
}

void DelDiscipleAward(CUser *pUser,int type)
{
    if(pUser == NULL)
        return ;
    CGetDbConnect getDb;
    CDatabaseSql *pDb = getDb.GetDbConnect();
    if(pDb == NULL)
        return;
                
    char sql[256];
    snprintf(sql,256,
        "UPDATE mp_award SET award_type=%d where role_id=%d and type=1 and award_type=%d limit 1",
        //"delete from mp_award where role_id=%d and type=1 and award_type=%d",
        type+100,pUser->GetRoleId(),type);
    pDb->Query(sql);
}

int GetMasterAward(CUser *pUser,int type)
{
    if(pUser == NULL)
        return false;
    CGetDbConnect getDb;
    CDatabaseSql *pDb = getDb.GetDbConnect();
    if(pDb == NULL)
        return false;
                
    char sql[256];
    snprintf(sql,256,
        "select role_id,role_level from mp_award where role_id=%d and type=0 and award_type=%d",
        pUser->GetRoleId(),type);
    if(pDb->Query(sql))
    {
        char **row = pDb->GetRow();
        if(row != NULL)
            return atoi(row[1]);
    }
    return 0;
}

void DelMasterAward(CUser *pUser,int type)
{
    if(pUser == NULL)
        return ;
    CGetDbConnect getDb;
    CDatabaseSql *pDb = getDb.GetDbConnect();
    if(pDb == NULL)
        return;

    char sql[256];
    snprintf(sql,256,
        "UPDATE mp_award SET award_type=%d where role_id=%d and type=0 and award_type=%d limit 1",
        type+100,pUser->GetRoleId(),type);
    pDb->Query(sql);
}

void EnterFuBenWJ(CUser *pUser)
{
    if(pUser == NULL)
        return;
    //CScene *pScene = pUser->GetScene();
    CSceneManager &scene = SingletonSceneManager::instance();
    CScene *pScene = scene.GetFuBen(307);
    if(pScene != NULL)
    {
        TransportUser(pUser,pScene->GetId(),15,15,8);
    }
}

void EnterFuBenShiYao(CUser *pUser,int level)
{
    if(pUser == NULL)
        return;
    //CScene *pScene = pUser->GetScene();
    CSceneManager &scene = SingletonSceneManager::instance();
    CScene *pScene = scene.GetFuBen(306);
    if(pScene != NULL)
    {
        pScene->SetShiYaoLevel(level);
        TransportUser(pUser,pScene->GetId(),15,15,8);
    }
}

void EnterGuiYu(CUser *pUser,int level)
{
    if(pUser == NULL)
        return;
    //CScene *pScene = pUser->GetScene();
    CSceneManager &scene = SingletonSceneManager::instance();
    CScene *pScene = scene.GetGuiYu(level);
    if(pScene != NULL)
    {
        TransportUser(pUser,pScene->GetId(),10,10,8);
    }
}

//找出妖孽
void FindYaoNieBattle(CUser *pUser)
{
    if((pUser != NULL) && (pUser->GetScene() != NULL))
    {
        pUser->GetScene()->FindYaoNieBattle(pUser);
    }
}

//击败狂兽
void JiBaiKSBattle(CUser *pUser)
{
    if((pUser != NULL) && (pUser->GetScene() != NULL))
    {
        pUser->GetScene()->JiBaiKSBattle(pUser);
    }
}

//寻宝战斗
void XunBaoFight(CUser *pUser)
{
    if((pUser != NULL) && (pUser->GetScene() != NULL))
    {
        pUser->GetScene()->XunBaoFight(pUser);
    }
}

//拆分
int ChaiFen(CUser *pUser,uint8 weaponPos,uint8 attrPos,uint8 stonePos)
{
    if(pUser == NULL)
        return 2;
    return pUser->ChaiFen(weaponPos,attrPos,stonePos);
}

//融合
int RongHe(CUser *pUser,uint8 weaponPos,uint8 stonePos)
{
    if(pUser == NULL)
        return 2;
    return pUser->RongHe(weaponPos,stonePos);
}

char *GetPaiMing(int level)
{
    if(level == 40)
    {
        CSceneManager &scene = SingletonSceneManager::instance();
        CScene *pScene = scene.FindScene(320);
        if(pScene == NULL)
            return NULL;
        return pScene->GetMatchPaiMing();
    }
    else if(level == 50)
    {
        CSceneManager &scene = SingletonSceneManager::instance();
        CScene *pScene = scene.FindScene(321);
        if(pScene == NULL)
            return NULL;
        return pScene->GetMatchPaiMing();
    }
    else if(level == 60)
    {
        CSceneManager &scene = SingletonSceneManager::instance();
        CScene *pScene = scene.FindScene(322);
        if(pScene == NULL)
            return NULL;
        return pScene->GetMatchPaiMing();
    }
    else if(level == 60)
    {
        CSceneManager &scene = SingletonSceneManager::instance();
        CScene *pScene = scene.FindScene(323);
        if(pScene == NULL)
            return NULL;
        return pScene->GetMatchPaiMing();
    }
    return NULL;
}

int GetWeekDay()
{
    time_t t = GetSysTime();
    tm *pTm = localtime(&t);
    if(pTm == NULL)
        return 0;
    return pTm->tm_wday;
}

//列出指定页帮派列表
void ListBang(CUser *pUser,int page)
{
    if(pUser == NULL)
        return;
    CBangPaiManager &bPMgr = SingletonCBangPaiManager::instance();
    CSocketServer &sock = SingletonSocket::instance();
    CNetMessage msg;
    msg.SetType(PRO_INTERACT);
    msg<<(uint8)19<<(uint8)page;
    bPMgr.MakeBangPaiList(page,msg,pUser->GetBangPai(),false);
    sock.SendMsg(pUser->GetSock(),msg);
}

//对某一帮派宣战
//0宣战成功,1 不是指定时间,2 位阶不够
//3 已宣战过,4 对繁荣度小于0的帮派宣战
int DeclareWar(CUser *pUser,int bid)
{
    if(pUser == NULL)
        return 5;
    CBangPaiManager &bPMgr = SingletonCBangPaiManager::instance();
    CBangPai *pBangPai1 = bPMgr.FindBangPai(pUser->GetBangPai());
    CBangPai *pBangPai2 = bPMgr.FindBangPai(bid);
    if((pBangPai1 == NULL) || (pBangPai2 == NULL))
        return 5;
    if((pBangPai1->GetXuanZhanBang() == (int)pBangPai2->GetId())
        || (pBangPai2->GetXuanZhanBang() == (int)pBangPai1->GetId()))
    {
        char info[128];
        snprintf(info,128,"%s 对 %s 宣战",pBangPai1->GetName().c_str(),pBangPai2->GetName().c_str());
        SysInfoToAllUser(info);
        return 0;
    }
    if(pBangPai1->JieMeng(pBangPai2))
        return 6;
    //if(pBangPai2->GetFanRong() < 0)
    if(pBangPai1->GetFanRong() - pBangPai2->GetFanRong() > 100)
        return 4;
    if(pBangPai1->GetMemberRank(pUser->GetRoleId()) > EBRFuBangZhu)
        return 2;
    if(pBangPai1->GetXuanZhanBang() != 0)
        return 3;
    pBangPai1->SetXuanZhanBang(bid,GetSysTime());
    pBangPai2->SetBeiXuanZhan(GetSysTime());
    char info[128];
    snprintf(info,128,"%s 对 %s 宣战",pBangPai1->GetName().c_str(),pBangPai2->GetName().c_str());
    SysInfoToAllUser(info);
    return 0;
}

//根据帮派id,得到帮派繁荣度
int GetBangPros(CUser *pUser,int bid)
{
    if(pUser == NULL)
        return 0;
    CBangPaiManager &bPMgr = SingletonCBangPaiManager::instance();
    CBangPai *pBangPai = bPMgr.FindBangPai(bid);
    if(pBangPai != NULL)
    {
        return pBangPai->GetFanRong();
    }
    return 0;
}

//进入帮战砙�
bool EnterBangZhan(CUser *pUser)
{
    if(pUser == NULL)
        return false;
    if(pUser->GetBangPai() == 0)
        return false;
    
    CGetDbConnect getDb;
    CDatabaseSql *pDb = getDb.GetDbConnect();
    char sql[128];
    sprintf(sql,"SELECT bangpai1,bangpai2 FROM xz_bang where bangpai1=%d or bangpai2=%d",
                pUser->GetBangPai(),pUser->GetBangPai());
    if ((pDb == NULL) || (!pDb->Query(sql)))
    {
        return false;
    }
    char **row = pDb->GetRow();
    if(row == NULL)
        return false;
    
    int bang1 = atoi(row[0]);
    int bang2 = atoi(row[1]);
    int bzSceneId = (bang2<<16)|bang1;
    
    CSceneManager &scene = SingletonSceneManager::instance();
    CScene *pScene = NULL;
    CScene *pBZScene = scene.GetBangZhanScene(bzSceneId,&pScene);
    if((pBZScene == NULL) || (pScene == NULL))
        return false;
    if(pBZScene->GetJiFen(bang1) == 0)
        pBZScene->SetJiFen(bang1,0);
    if(pBZScene->GetJiFen(bang2) == 0)
        pBZScene->SetJiFen(bang2,0);
        
    if(bang2 == (int)pUser->GetBangPai())
    {
        TransportUser(pUser,pBZScene->GetId(),17,15,8);
        pUser->SetXZBangId(bang1);
        return true;
    }
    else
    {
        TransportUser(pUser,pScene->GetId(),7,17,8);
        pUser->SetXZBangId(bang2);
        return true;
    }
}

//进入帮战砙�
bool EnterBangZhan(CUser *pUser,int bId)
{
    if(pUser == NULL)
        return false;
    if(pUser->GetBangPai() == 0)
        return false;
    
    CBangPaiManager &bPMgr = SingletonCBangPaiManager::instance();
    CBangPai *pBangPai1 = bPMgr.FindBangPai(pUser->GetBangPai());
    CBangPai *pBangPai2 = bPMgr.FindBangPai(bId);
    if((pBangPai1 == NULL) || (pBangPai2 == NULL))
        return false;
    int bzId;
    if(pBangPai1->GetXuanZhanBang() == bId)
    {
        bzId = (pUser->GetBangPai()<<16)|bId;
    }
    else if(pBangPai2->GetXuanZhanBang() == (int)pUser->GetBangPai())
    {
        bzId = (bId<<16)|pUser->GetBangPai();
    }
    else
    {
        return false;
    }
    CSceneManager &scene = SingletonSceneManager::instance();
    CScene *pScene = NULL;
    CScene *pBZScene = scene.GetBangZhanScene(bzId,&pScene);
    if(pBZScene != NULL)
    {
        if(pBZScene->GetJiFen(pUser->GetBangPai()) == 0)
            pBZScene->SetJiFen(pUser->GetBangPai(),0);
        if(pBZScene->GetJiFen(bId) == 0)
            pBZScene->SetJiFen(bId,0);
    }
    if(pBangPai1->GetXuanZhanBang() == bId)
    {
        if(pBZScene != NULL)
        {
            TransportUser(pUser,pBZScene->GetId(),7,17,8);
            return true;
        }
    }
    else
    {
        if(pScene != NULL)
        {
            TransportUser(pUser,pScene->GetId(),17,15,8);
            return true;
        }
    }
    return false;
}

//帮战雕像触发战斗
//战斗胜利后直接占领该雕像
int FightDiaoXiang(CUser *pUser,int id)
{
    if(pUser == NULL)
        return 0;
    CSceneManager &scene = SingletonSceneManager::instance();
    CScene *pScene = pUser->GetScene();
    if(pScene == NULL)
        return 0;
    CScene *pGroup = scene.FindScene(pScene->GetGroupId());
    if(pGroup != NULL)
    {
        if(pGroup->GetDiaoXiangBang(id) == (int)pUser->GetBangPai())
            return 1;
        if(pGroup->GetDiaoXiangTime(id) + 120 > GetSysTime())
            return 1;
    }
    pScene->DiaoXiangFight(pUser,id);
    return 0;
}

//查看本帮积分
int GetMeScore(CUser *pUser)
{
    if(pUser == NULL)
        return 0;
    CSceneManager &scene = SingletonSceneManager::instance();
    CScene *pScene = pUser->GetScene();
    if(pScene == NULL)
        return 0;
    CScene *pGroup = scene.FindScene(pScene->GetGroupId());
    if(pGroup != NULL)
        return pGroup->GetJiFen(pUser->GetBangPai());
    return 0;
}

//查看敌帮积分
int GetEnemyScore(CUser *pUser)
{
    if(pUser == NULL)
        return 0;
    CSceneManager &scene = SingletonSceneManager::instance();
    CScene *pScene = pUser->GetScene();
    if(pScene == NULL)
        return 0;
    CScene *pGroup = scene.FindScene(pScene->GetGroupId());
    if(pGroup != NULL)
        return pGroup->GetOtherJiFen(pUser->GetBangPai());
    return 0;
}

//查看本人贡献积分
int GetOwnScore(CUser *pUser)
{
    if(pUser == NULL)
        return 0;
    return pUser->GetBangZhanScene();
}

//上交矿产
int ShangjiaoKuang(CUser *pUser)
{
    if(pUser == NULL)
        return 0;
    pUser->SetBangZhanScore(pUser->GetBangZhanScene()+2);
    
    CSceneManager &scene = SingletonSceneManager::instance();
    CScene *pScene = pUser->GetScene();
    if(pScene == NULL)
        return 0;
    CScene *pGroup = scene.FindScene(pScene->GetGroupId());
    if(pGroup != NULL)
    {
        int jifen = pGroup->GetJiFen(pUser->GetBangPai());
        int kuang = pGroup->GetKuang(pUser->GetBangPai());
        pGroup->SetJiFen(pUser->GetBangPai(),jifen+2);
        pGroup->SetKuang(pUser->GetBangPai(),kuang+2);
    }
    pUser->DelPackageById(1543,1);
    pUser->DelPackageById(1544,1);
    UpdateUserInfo(pUser);
    
    CBangPaiManager &bPMgr = SingletonCBangPaiManager::instance();
    CBangPai *pBangPai = bPMgr.FindBangPai(pUser->GetBangPai());
    if(pBangPai != NULL)
    {
        char buf[64];
        snprintf(buf,64,"%s帮成功收集1块矿石",pBangPai->GetName().c_str());
        SendSysInfoToGroup(pGroup->GetGroupId(),buf);
    }
    return 0;
}

//查看矿产
int GetBangKuang(CUser *pUser)
{
    if(pUser == NULL)
        return 0;
    CSceneManager &scene = SingletonSceneManager::instance();
    CScene *pScene = pUser->GetScene();
    if(pScene == NULL)
        return 0;
    CScene *pGroup = scene.FindScene(pScene->GetGroupId());
    if(pGroup != NULL)
        return pGroup->GetKuang(pUser->GetBangPai());
    return 0;
}

//下次可以出动魅影战士剩余时间
int GetMeiYingTime(CUser *pUser)
{
    if(pUser == NULL)
        return 0;
    CSceneManager &scene = SingletonSceneManager::instance();
    CScene *pScene = pUser->GetScene();
    if(pScene == NULL)
        return 0;
    CScene *pGroup = scene.FindScene(pScene->GetGroupId());
    if(pGroup != NULL)
        return pGroup->GetMeYingLeftTime(pUser->GetBangPai());
    return 0;
}

//出动魅影战士
int StartMeiYing(CUser *pUser)
{
    const int KUANG_NUM  = 150;
    if(pUser == NULL)
        return 2;
    CSceneManager &scene = SingletonSceneManager::instance();
    CScene *pScene = pUser->GetScene();
    if(pScene == NULL)
        return 2;
    CScene *pGroup = scene.FindScene(pScene->GetGroupId());
    if(pGroup == NULL)
        return 2;
    int kuang = pGroup->GetKuang(pUser->GetBangPai());
    if(kuang < KUANG_NUM)
        return 1;
    pGroup->SetKuang(pUser->GetBangPai(),kuang-KUANG_NUM);
    
    CBangPaiManager &bPMgr = SingletonCBangPaiManager::instance();
    CBangPai *pBangPai = bPMgr.FindBangPai(pUser->GetBangPai());
    if((pBangPai == NULL) || (pBangPai->GetMemberRank(pUser->GetRoleId()) > EBRZhangLao))
        return 2;
    
    pGroup->StartMeiYing(pUser->GetBangPai(),30,600);
    
    if(pBangPai != NULL)
    {
        char buf[64];
        snprintf(buf,64,"%s帮发动魅影战士突袭。",pBangPai->GetName().c_str());
        SendSysInfoToGroup(pGroup->GetGroupId(),buf);
    }
    return 0;
}

int GetBZPosition(CUser *pUser)
{
    if(pUser == NULL)
        return 0;
    CScene *pScene = pUser->GetScene();
    CSceneManager &scene = SingletonSceneManager::instance();
    if(pScene == NULL)
        return 0;
    CScene *pGroup = scene.FindScene(pScene->GetGroupId());
    if(pGroup == NULL)
        return 0;
    if((0xffff & pGroup->GetId()) == (int)pUser->GetBangPai())
        return 1;
    else    
        return 0;
}

//得到占领雕像的帮派
// (0=友帮,1=敌帮)  2
int GetDiaoXiangState(CUser *pUser,int id)
{
    if(pUser == NULL)
        return 0;
    CSceneManager &scene = SingletonSceneManager::instance();
    CScene *pScene = pUser->GetScene();
    if(pScene == NULL)
        return 0;
    CScene *pGroup = scene.FindScene(pScene->GetGroupId());
    if(pGroup != NULL)
    {
        if(pGroup->GetDiaoXiangBang(id) == (int)pUser->GetBangPai())
            return 0;
        else if(pGroup->GetDiaoXiangBang(id) == 0)
            return 2;
        else 
            return 1;
    }
    return 0;
}

//得到雕像剩余保护时间
int GetDiaoXiangTime(CUser *pUser,int id)
{
    if(pUser == NULL)
        return 0;
    CSceneManager &scene = SingletonSceneManager::instance();
    CScene *pScene = pUser->GetScene();
    if(pScene == NULL)
        return 0;
    CScene *pGroup = scene.FindScene(pScene->GetGroupId());
    if(pGroup != NULL)
    {
        if(pGroup->GetDiaoXiangTime(id) + 120 > GetSysTime())
            return 120 - (GetSysTime() - pGroup->GetDiaoXiangTime(id));
        else 
            return 0;
    }
    return 0;
}

static time_t sWaZhongTime;

bool InWaZhongHuoDong()
{
    time_t t = GetSysTime();
    struct tm *pTm = localtime(&t);
    if(pTm == NULL)
        return false;
    if(pTm->tm_mday < 13)
        return false;
    return pTm->tm_hour == 21;
}

static SPoint sPoint[40];

char *GetHuoDongDi()
{
    static char buf[256];
    buf[0] = 0;
    for(uint8 i = 0; i < sizeof(sPoint)/sizeof(SPoint); i++)
    {
        if((sPoint[i].x != 0xff) && (sPoint[i].y != 0xff))
        {
            if(i != 0)
                strcat(buf,"|");
            sprintf(buf+strlen(buf),"%d|%d",sPoint[i].x,sPoint[i].y);
        }
    }
    if(buf[0] == 0)
        return NULL;
    return buf;
}
static int sWaKuangScendId;

int GetWaZhongScene()
{
    return sWaKuangScendId;
}

void RandSelWaZhongPoint()
{
    const uint16 scendIds[] = 
    {
        51,55,72,45,56,71,94,87,88,90,124,123,125,113,86
        //11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,31,32,33,34,35,36,37,38,39
        //41,42,43,44,45 ,46,48,49,50,51 ,52,53,54,55,56,57,58,59,60,61,67,68,69,71,72,81,82,83,84,85,86,87,88,89,90,91,92,93,94,95,96,97,98,99,111,112,113,114,115,116,121,122,123,124,125,126,151,152,153,154,157
    };
    CSceneManager &scene = SingletonSceneManager::instance();
    uint16 sceneId = Random(0,sizeof(scendIds)/sizeof(uint16)-1);
    sceneId = scendIds[sceneId];
    sWaKuangScendId = sceneId;
    
    CScene *pScene = scene.FindScene(sceneId);
    if(pScene == NULL)
        return;
    
    char buf[128];
    snprintf(buf,128,"21-22点天降元宵活动正在%s进行，大家快去挖宝，系统会掉各种元宵券，奖励丰厚哦！"//"端午节大放粽活动正在%s进行，大家快去挖各种粽子，获得节日奖励！"
                ,pScene->GetName());
    SysInfoToAllUser(buf);

    for(uint8 i = 0; i < sizeof(sPoint)/sizeof(SPoint); i++)
    {
        if(!pScene->GetCanWalkPos(sPoint[i].x,sPoint[i].y))
        {
            sPoint[i].x = 10;
            sPoint[i].y = 10;
        }
    }
}

time_t GetWaZhongTime()
{
    return sWaZhongTime;
}

void SetWaZhongTime(time_t t)
{
    sWaZhongTime = t;
}

void DelHuoDongDi(int x,int y)
{
    for(uint8 i = 0; i < sizeof(sPoint)/sizeof(SPoint); i++)
    {
        if((sPoint[i].x == x) && (sPoint[i].y == y))
        {
            sPoint[i].x = 0xff;
            sPoint[i].y = 0xff;
            return;
        }
    }
}

void ListWarBang(CUser *pUser)
{
    if(pUser == NULL)
        return;
    CBangPaiManager &bPMgr = SingletonCBangPaiManager::instance();
    CSocketServer &sock = SingletonSocket::instance();
    CNetMessage msg;
    msg.SetType(PRO_INTERACT);
    msg<<(uint8)19<<(uint8)0;
    bPMgr.MakeXZBang(msg,pUser->GetBangPai());
    sock.SendMsg(pUser->GetSock(),msg);
}

const char *GetBangName(int id)
{
    CBangPaiManager &bPMgr = SingletonCBangPaiManager::instance();
    CBangPai *pBangPai = bPMgr.FindBangPai(id);
    if(pBangPai == NULL)
        return NULL;
    return pBangPai->GetName().c_str();
}

void UpdateUserInfo(CUser *pUser)
{
    if(pUser == NULL)
        return;
    CScene *pScene = pUser->GetScene();
    if(pScene != NULL)
        pScene->UpdateUserInfo(pUser);
}

bool PlantResource(CUser *pUser,int ind)
{
    if(pUser == NULL)
        return false;
    CBangPaiManager &bPMgr = SingletonCBangPaiManager::instance();
    CBangPai *pBangPai = bPMgr.FindBangPai(pUser->GetBangPai());
    if(pBangPai == NULL)
        return false;
    SItemInstance *pItem = pUser->GetItem(ind);
    
    SZhongZhi zzItem;
    if((pItem != NULL) && (pBangPai->ZhongZhi(zzItem,pUser,pItem->tmplId)))
    {
        pUser->DelPackage(ind,1);
        CNetMessage msg;
        msg.SetType(MSG_SERVER_ADD_OBJECT);
        msg<<(uint8)1<<zzItem.itemId<<zzItem.state<<zzItem.x<<zzItem.y;
        CSocketServer &sock = SingletonSocket::instance();
        CScene *pScene = pUser->GetScene();
        if(pScene == NULL)
            return false;
        list<uint32> userList;
        pScene->GetUserList(userList);
        COnlineUser &onlineUser = SingletonOnlineUser::instance();
        for(list<uint32>::iterator i = userList.begin(); i != userList.end(); i++)
        {
            ShareUserPtr ptr = onlineUser.GetUserByRoleId(*i);
            if(ptr.get() == NULL) 
                continue;
            sock.SendMsg(ptr->GetSock(),msg);
        }
        return true;
    }
    return false;
}

int GainResource(CUser *pUser,int ind)
{
    if(pUser == NULL)
        return 0;
    CBangPaiManager &bPMgr = SingletonCBangPaiManager::instance();
    CBangPai *pBangPai = bPMgr.FindBangPai(pUser->GetBangPai());
    if(pBangPai == NULL)
        return 0;
    return pBangPai->GainResource(pUser,ind);
}

void ListResource(CUser *pUser)
{
    if(pUser == NULL)
        return;
    CNetMessage msg;
    msg.SetType(PRO_INTERACT);
    
    msg<<(uint8)20;
    CBangPaiManager &bPMgr = SingletonCBangPaiManager::instance();
    CBangPai *pBangPai = bPMgr.FindBangPai(pUser->GetBangPai());
    if(pBangPai == NULL)
        return;
    CSocketServer &sock = SingletonSocket::instance();
    pBangPai->MakeUserRes(pUser->GetRoleId(),msg);
    sock.SendMsg(pUser->GetSock(),msg);
}

//op=1 加速,op=2 破坏,op=3 修复
void DoFarm(CUser *pUser,int op,int ind)
{
    if(pUser == NULL)
        return;
    CBangPaiManager &bPMgr = SingletonCBangPaiManager::instance();
    CBangPai *pBangPai = bPMgr.FindBangPai(pUser->GetBangPai());
    if(pBangPai == NULL)
        return;
    SZhongZhi zzItem;
    if(pBangPai->DoFarm(op,ind,zzItem))
    {
        CNetMessage msg;
        msg.SetType(MSG_SERVER_UPDATE_OBJECT);
        msg<<zzItem.itemId<<zzItem.state<<zzItem.x<<zzItem.y;
        CSocketServer &sock = SingletonSocket::instance();
        CScene *pScene = pUser->GetScene();
        if(pScene == NULL)
            return;
        list<uint32> userList;
        pScene->GetUserList(userList);
        COnlineUser &onlineUser = SingletonOnlineUser::instance();
        for(list<uint32>::iterator i = userList.begin(); i != userList.end(); i++)
        {
            ShareUserPtr ptr = onlineUser.GetUserByRoleId(*i);
            if(ptr.get() == NULL) 
                continue;
            sock.SendMsg(ptr->GetRoleId(),msg);
        }
    }
}

SResInfo GetResourceInfo(CUser *pUser,int ind)
{
    SResInfo info = {0};
    if(pUser == NULL)
        return info;
    CBangPaiManager &bPMgr = SingletonCBangPaiManager::instance();
    CBangPai *pBangPai = bPMgr.FindBangPai(pUser->GetBangPai());
    if(pBangPai == NULL)
        return info;
    SZhongZhi zzItem;
    if(pBangPai->GetZhongZhiInfo(ind,zzItem))
    {
        info.itemId = zzItem.itemId;
        info.state = zzItem.state;//状态0正常，1成熟，2加速，3破坏
        info.stateTime = 2*3600 + zzItem.time - GetSysTime();//状态剩余时间
        if(info.stateTime < 0)
            info.stateTime = 0;
        CItemTemplateManager &itemMgr = SingletonItemManager::instance();
        SItemTemplate *pItem = itemMgr.GetItem(info.itemId);
        if(pItem != NULL)
        {
            info.leftTime = pItem->addSudu*3600 - zzItem.growTime;//成熟剩余时间
            if(info.leftTime < 0)
                info.leftTime = 0;
        }
    }
    if(info.state == EZZSJiaSu)
    {
        info.leftTime = (int)(info.leftTime/1.2);
    }
    else if((info.state & (EZZSShuiTuLiuShi|EZZSPoHuai)) != 0)
    {
        double beiLv = 1;
        if((info.state & EZZSPoHuai) != 0)
            beiLv -= 0.25;
        if((info.state & EZZSShuiTuLiuShi) != 0)
            beiLv -= 0.25;
        info.leftTime = (int)(info.leftTime/beiLv);
    }
    return info;
}

int GetBangResource(CUser *pUser,int id)
{
    if(pUser == NULL)
        return 0;
    CBangPaiManager &bPMgr = SingletonCBangPaiManager::instance();
    CBangPai *pBangPai = bPMgr.FindBangPai(pUser->GetBangPai());
    if(pBangPai == NULL)
        return 0;
    switch(id)
    {
        case 1:
            return pBangPai->GetRes1();
        case 2:
            return pBangPai->GetRes2();
        case 3:
            return pBangPai->GetRes3();
        case 4:
            return pBangPai->GetRes4();
    }
    return 0;       
}

//提升帮派等级
//id=0 升级帮派 id=1 兵器阁 id=2 守御阁 id=3 百草堂 id=4 试炼堂
//0成功
int UpgradeBang(CUser *pUser,int id)
{
    if(pUser == NULL)
        return 1;
    CBangPaiManager &bPMgr = SingletonCBangPaiManager::instance();
    CBangPai *pBangPai = bPMgr.FindBangPai(pUser->GetBangPai());
    if(pBangPai == NULL)
        return 1;
    
    switch(id)
    {
    case 0:
        {
            if((pBangPai->GetLevel() == 1)
                 && (pBangPai->m_bqLevel == 2) 
                 && (pBangPai->m_syLevel == 2)
                 && (pBangPai->m_bcLevel == 2)
                 && (pBangPai->m_slLevel == 2))
            {
                if((pBangPai->GetMoney() >= 80000)
                    && (pBangPai->GetRes1() >= 100000)
                    && (pBangPai->GetRes2() >= 100000)
                    && (pBangPai->GetRes3() >= 80000)
                    && (pBangPai->GetRes4() >= 30000))
                {
                    pBangPai->SetMoney(pBangPai->GetMoney() - 80000);
                    pBangPai->SetRes1(pBangPai->GetRes1() - 100000);
                    pBangPai->SetRes2(pBangPai->GetRes2() - 100000);
                    pBangPai->SetRes3(pBangPai->GetRes3() - 80000);
                    pBangPai->SetRes4(pBangPai->GetRes4() - 30000);
                    pBangPai->SetLevel(2);
                    return 0;
                }
            }
            else if((pBangPai->GetLevel() == 2)
                 && (pBangPai->m_bqLevel == 3) 
                 && (pBangPai->m_syLevel == 3)
                 && (pBangPai->m_bcLevel == 3)
                 && (pBangPai->m_slLevel == 3))
            {
                if((pBangPai->GetMoney() >= 310000)
                    && (pBangPai->GetRes1() >= 210000)
                    && (pBangPai->GetRes2() >= 210000)
                    && (pBangPai->GetRes3() >= 150000)
                    && (pBangPai->GetRes4() >= 100000))
                {
                    pBangPai->SetMoney(pBangPai->GetMoney() - 310000);
                    pBangPai->SetRes1(pBangPai->GetRes1() - 210000);
                    pBangPai->SetRes2(pBangPai->GetRes2() - 210000);
                    pBangPai->SetRes3(pBangPai->GetRes3() - 150000);
                    pBangPai->SetRes4(pBangPai->GetRes4() - 100000);
                    return 0;
                }
            }
            else if((pBangPai->GetLevel() == 3)
                 && (pBangPai->m_bqLevel == 4) 
                 && (pBangPai->m_syLevel == 4)
                 && (pBangPai->m_bcLevel == 4)
                 && (pBangPai->m_slLevel == 4))
            {
                if((pBangPai->GetMoney() >= 800000)
                    && (pBangPai->GetRes1() >= 300000)
                    && (pBangPai->GetRes2() >= 500000)
                    && (pBangPai->GetRes3() >= 300000)
                    && (pBangPai->GetRes4() >= 700000))
                {
                    pBangPai->SetMoney(pBangPai->GetMoney() - 800000);
                    pBangPai->SetRes1(pBangPai->GetRes1() - 300000);
                    pBangPai->SetRes2(pBangPai->GetRes2() - 500000);
                    pBangPai->SetRes3(pBangPai->GetRes3() - 300000);
                    pBangPai->SetRes4(pBangPai->GetRes4() - 700000);
                    pBangPai->SetLevel(4);
                    return 0;
                }
            }
            else if((pBangPai->GetLevel() == 4)
                 && (pBangPai->m_bqLevel == 5) 
                 && (pBangPai->m_syLevel == 5)
                 && (pBangPai->m_bcLevel == 5)
                 && (pBangPai->m_slLevel == 5))
            {
                if((pBangPai->GetMoney() >= 1500000)
                    && (pBangPai->GetRes1() >= 1000000)
                    && (pBangPai->GetRes2() >= 800000)
                    && (pBangPai->GetRes3() >= 700000)
                    && (pBangPai->GetRes4() >= 1000000))
                {
                    pBangPai->SetMoney(pBangPai->GetMoney() - 1500000);
                    pBangPai->SetRes1(pBangPai->GetRes1() - 1000000);
                    pBangPai->SetRes2(pBangPai->GetRes2() - 800000);
                    pBangPai->SetRes3(pBangPai->GetRes3() - 700000);
                    pBangPai->SetRes4(pBangPai->GetRes4() - 1000000);
                    pBangPai->SetLevel(5);
                    return 0;
                }
            }
        }
        break;
    case 1:
        {
            int val[] = {
                10,10,8,3,7,
                19,31,15,10,21,
                80,90,40,60,40,
                125,175,95,105,125
            };
            if((pBangPai->m_bqLevel >= 2) && (pBangPai->GetLevel() < pBangPai->m_bqLevel))
                return 1;
            if((pBangPai->m_bqLevel < 5) && (pBangPai->m_bqLevel >= 1))
            {
                int ind = (pBangPai->m_bqLevel-1)*5;
                const int beilv = 10000; 
                if((pBangPai->GetMoney() > val[ind]*beilv)
                    && (pBangPai->GetRes1() >= val[ind+1]*beilv)
                    && (pBangPai->GetRes2() >= val[ind+2]*beilv)
                    && (pBangPai->GetRes3() >= val[ind+3]*beilv)
                    && (pBangPai->GetRes4() >= val[ind+4]*beilv))
                {
                    pBangPai->SetMoney(pBangPai->GetMoney() - val[ind]*beilv);
                    pBangPai->SetRes1(pBangPai->GetRes1() - val[ind+1]*beilv);
                    pBangPai->SetRes2(pBangPai->GetRes2() - val[ind+2]*beilv);
                    pBangPai->SetRes3(pBangPai->GetRes3() - val[ind+3]*beilv);
                    pBangPai->SetRes4(pBangPai->GetRes4() - val[ind+4]*beilv);
                    pBangPai->m_bqLevel++;
                    return 0;
                }
            }
        }
        break;
    case 2:
        {
            int val[] = {
                10,7,10,7,8,
                10,15,31,19,19,
                50,50,80,30,50,
                100,100,150,100,70
            };
            if((pBangPai->m_syLevel >= 2) && (pBangPai->GetLevel() < pBangPai->m_syLevel))
                return 1;
            if((pBangPai->m_syLevel < 5) && (pBangPai->m_syLevel >= 1))
            {
                int ind = (pBangPai->m_syLevel-1)*5;
                const int beilv = 10000; 
                if((pBangPai->GetMoney() > val[ind]*beilv)
                    && (pBangPai->GetRes1() >= val[ind+1]*beilv)
                    && (pBangPai->GetRes2() >= val[ind+2]*beilv)
                    && (pBangPai->GetRes3() >= val[ind+3]*beilv)
                    && (pBangPai->GetRes4() >= val[ind+4]*beilv))
                {
                    pBangPai->SetMoney(pBangPai->GetMoney() - val[ind]*beilv);
                    pBangPai->SetRes1(pBangPai->GetRes1() - val[ind+1]*beilv);
                    pBangPai->SetRes2(pBangPai->GetRes2() - val[ind+2]*beilv);
                    pBangPai->SetRes3(pBangPai->GetRes3() - val[ind+3]*beilv);
                    pBangPai->SetRes4(pBangPai->GetRes4() - val[ind+4]*beilv);
                    pBangPai->m_syLevel++;
                    return 0;
                }
            }
        }
        break;
    case 3:
        {
            int val[] = {
                3,8,3,10,10,
                15,10,19,31,15,
                40,40,80,90,40,
                95,95,125,175,105
            };
            if((pBangPai->m_bcLevel >= 2) && (pBangPai->GetLevel() < pBangPai->m_bcLevel))
                return 1;
            if((pBangPai->m_bcLevel < 5) && (pBangPai->m_bcLevel >= 1))
            {
                int ind = (pBangPai->m_bcLevel-1)*5;
                const int beilv = 10000; 
                if((pBangPai->GetMoney() > val[ind]*beilv)
                    && (pBangPai->GetRes1() >= val[ind+1]*beilv)
                    && (pBangPai->GetRes2() >= val[ind+2]*beilv)
                    && (pBangPai->GetRes3() >= val[ind+3]*beilv)
                    && (pBangPai->GetRes4() >= val[ind+4]*beilv))
                {
                    pBangPai->SetMoney(pBangPai->GetMoney() - val[ind]*beilv);
                    pBangPai->SetRes1(pBangPai->GetRes1() - val[ind+1]*beilv);
                    pBangPai->SetRes2(pBangPai->GetRes2() - val[ind+2]*beilv);
                    pBangPai->SetRes3(pBangPai->GetRes3() - val[ind+3]*beilv);
                    pBangPai->SetRes4(pBangPai->GetRes4() - val[ind+4]*beilv);
                    pBangPai->m_bcLevel++;
                    return 0;
                }
            }
        }
        break;
    case 4:
        {
            int val[] = {
                7,3,7,10,10,
                21,19,10,21,31,
                40,80,40,80,90,
                105,105,125,105,175
            };
            if((pBangPai->m_slLevel >= 2) && (pBangPai->GetLevel() < pBangPai->m_slLevel))
                return 1;
            if((pBangPai->m_slLevel < 5) && (pBangPai->m_slLevel >= 1))
            {
                int ind = (pBangPai->m_slLevel-1)*5;
                const int beilv = 10000; 
                if((pBangPai->GetMoney() > val[ind]*beilv)
                    && (pBangPai->GetRes1() >= val[ind+1]*beilv)
                    && (pBangPai->GetRes2() >= val[ind+2]*beilv)
                    && (pBangPai->GetRes3() >= val[ind+3]*beilv)
                    && (pBangPai->GetRes4() >= val[ind+4]*beilv))
                {
                    pBangPai->SetMoney(pBangPai->GetMoney() - val[ind]*beilv);
                    pBangPai->SetRes1(pBangPai->GetRes1() - val[ind+1]*beilv);
                    pBangPai->SetRes2(pBangPai->GetRes2() - val[ind+2]*beilv);
                    pBangPai->SetRes3(pBangPai->GetRes3() - val[ind+3]*beilv);
                    pBangPai->SetRes4(pBangPai->GetRes4() - val[ind+4]*beilv);
                    pBangPai->m_slLevel++;
                    return 0;
                }
            }
        }
        break;
    case 5:
        {
            int val[] = {
                10,6,6,3,5,
                30,13,11,17,15,
                50,25,30,21,28,
                100,40,33,35,42
            };
            if((pBangPai->m_jmLevel >= 2) && (pBangPai->GetLevel() < pBangPai->m_jmLevel))
                return 1;
            if((pBangPai->m_jmLevel < 5) && (pBangPai->m_jmLevel >= 1))
            {
                int ind = (pBangPai->m_jmLevel-1)*5;
                const int beilv = 10000; 
                if((pBangPai->GetMoney() > val[ind]*beilv)
                    && (pBangPai->GetRes1() >= val[ind+1]*beilv)
                    && (pBangPai->GetRes2() >= val[ind+2]*beilv)
                    && (pBangPai->GetRes3() >= val[ind+3]*beilv)
                    && (pBangPai->GetRes4() >= val[ind+4]*beilv))
                {
                    pBangPai->SetMoney(pBangPai->GetMoney() - val[ind]*beilv);
                    pBangPai->SetRes1(pBangPai->GetRes1() - val[ind+1]*beilv);
                    pBangPai->SetRes2(pBangPai->GetRes2() - val[ind+2]*beilv);
                    pBangPai->SetRes3(pBangPai->GetRes3() - val[ind+3]*beilv);
                    pBangPai->SetRes4(pBangPai->GetRes4() - val[ind+4]*beilv);
                    pBangPai->m_jmLevel++;
                    return 0;
                }
            }
        }
        break;
    case 6:
        {
            int val[] = {
                10,1,1,1,1,
                20,7,7,7,7,
                50,15,15,15,15,
                80,25,25,25,25,
                100,40,40,40,40
            };
            if((pBangPai->m_zcLevel >= 0) && (pBangPai->m_jmLevel < pBangPai->m_zcLevel))
                return 1;
            if((pBangPai->m_zcLevel < 5) && (pBangPai->m_zcLevel >= 0))
            {
                int ind = (pBangPai->m_zcLevel)*5;
                const int beilv = 10000; 
                if((pBangPai->GetMoney() > val[ind]*beilv)
                    && (pBangPai->GetRes1() >= val[ind+1]*beilv)
                    && (pBangPai->GetRes2() >= val[ind+2]*beilv)
                    && (pBangPai->GetRes3() >= val[ind+3]*beilv)
                    && (pBangPai->GetRes4() >= val[ind+4]*beilv))
                {
                    pBangPai->SetMoney(pBangPai->GetMoney() - val[ind]*beilv);
                    pBangPai->SetRes1(pBangPai->GetRes1() - val[ind+1]*beilv);
                    pBangPai->SetRes2(pBangPai->GetRes2() - val[ind+2]*beilv);
                    pBangPai->SetRes3(pBangPai->GetRes3() - val[ind+3]*beilv);
                    pBangPai->SetRes4(pBangPai->GetRes4() - val[ind+4]*beilv);
                    pBangPai->m_zcLevel++;
                    return 0;
                }
            }
        }
        break;
    case 7:
        {
            int val[] = {
                15,2,2,2,2,
                30,10,10,10,10,
                50,20,20,20,20,
                80,40,40,40,40,
                100,60,60,60,60
            };
            if((pBangPai->m_fzcLevel >= 0) && (pBangPai->m_jmLevel < pBangPai->m_fzcLevel))
                return 1;
            if((pBangPai->m_fzcLevel < 5) && (pBangPai->m_fzcLevel >= 0))
            {
                int ind = (pBangPai->m_fzcLevel)*5;
                const int beilv = 10000; 
                if((pBangPai->GetMoney() > val[ind]*beilv)
                    && (pBangPai->GetRes1() >= val[ind+1]*beilv)
                    && (pBangPai->GetRes2() >= val[ind+2]*beilv)
                    && (pBangPai->GetRes3() >= val[ind+3]*beilv)
                    && (pBangPai->GetRes4() >= val[ind+4]*beilv))
                {
                    pBangPai->SetMoney(pBangPai->GetMoney() - val[ind]*beilv);
                    pBangPai->SetRes1(pBangPai->GetRes1() - val[ind+1]*beilv);
                    pBangPai->SetRes2(pBangPai->GetRes2() - val[ind+2]*beilv);
                    pBangPai->SetRes3(pBangPai->GetRes3() - val[ind+3]*beilv);
                    pBangPai->SetRes4(pBangPai->GetRes4() - val[ind+4]*beilv);
                    pBangPai->m_fzcLevel++;
                    return 0;
                }
            }
        }
        break;
    }
    return 1;
}

//获得所在砙鞍锱蒳d
int GetSceneBang(CUser *pUser)
{
    if(pUser == NULL)
        return 0;
    CScene *pScene = pUser->GetScene();
    if(pScene == NULL)
        return 0;
    return pScene->GetId()>>16;
}

//获得帮派等级
//id=0 帮派 id=1 兵器阁 id=2 守御阁 id=3 百草堂 id=4 试炼堂
int GetBangLevel(CUser *pUser,int id)
{
    CBangPaiManager &bPMgr = SingletonCBangPaiManager::instance();
    CBangPai *pBangPai = bPMgr.FindBangPai(pUser->GetBangPai());
    if(pBangPai == NULL)
        return 0;
    switch(id)
    {
    case 0:
        return pBangPai->GetLevel();
    case 1:
        return pBangPai->m_bqLevel;
    case 2:
        return pBangPai->m_syLevel;
    case 3:
        return pBangPai->m_bcLevel;
    case 4:
        return pBangPai->m_slLevel;
    case 5:
        return pBangPai->m_jmLevel;
    case 6:
        return pBangPai->m_zcLevel;
    case 7:
        return pBangPai->m_fzcLevel;
    }
    return 0;
}

int GetBangMoney(CUser *pUser)
{
    CBangPaiManager &bPMgr = SingletonCBangPaiManager::instance();
    CBangPai *pBangPai = bPMgr.FindBangPai(pUser->GetBangPai());
    if(pBangPai == NULL)
        return 0;
    return pBangPai->GetMoney();
}
void AddBangMoney(CUser *pUser,int money)
{
    CBangPaiManager &bPMgr = SingletonCBangPaiManager::instance();
    CBangPai *pBangPai = bPMgr.FindBangPai(pUser->GetBangPai());
    if(pBangPai == NULL)
        return;
    pBangPai->SetMoney(pBangPai->GetMoney() + money);
    if((pBangPai->GetLevel() >= 1) && (pBangPai->GetLevel() <= 5)
        && (pBangPai->GetMoney() > sMaxMoney[pBangPai->GetLevel()-1]))
    {
        pBangPai->SetMoney(sMaxMoney[pBangPai->GetLevel()-1]);
    }
}

bool AddPet(CUser *pUser,int monsterId)
{
    if(pUser == NULL)
        return false;
    CMonsterManager &monsterMgr = SingletonMonsterManager::instance();
    ShareMonsterPtr monster = monsterMgr.CreateMonster(monsterId,EMTBaoBao);
    if(monster.get() == NULL)
        return false;
    SharePetPtr pet = monsterMgr.CreatePet(monster.get(),false); 
    return pUser->AddPet(pet);
}

const char *GetQuestion()
{
    static vector<string> questionList;
    if(questionList.size() == 0)
    {
        CGetDbConnect getDb;
        CDatabaseSql *pDb = getDb.GetDbConnect();
        if(pDb != NULL)
        {
            if(pDb->Query("select question,answer1,answer2,answer3,answer4 from question"))
            {
                char **row;
                char buf[256];
                while((row = pDb->GetRow()) != NULL)
                {
                    snprintf(buf,256,"%s|%s|%s|%s|%s",row[0],row[1],row[2],row[3],row[4]);
                    questionList.push_back(buf);
                }
            }
        }
    }
    if(questionList.size() > 0)
    {
        int r = Random(0,questionList.size()-1);
        return questionList[r].c_str();
    }
    return NULL;
}

const char *GetQuestion1()
{
    static vector<string> questionList;
    if(questionList.size() == 0)
    {
        CGetDbConnect getDb;
        CDatabaseSql *pDb = getDb.GetDbConnect();
        if(pDb != NULL)
        {
            if(pDb->Query("select question,answer1,answer2,answer3,answer4 from question1"))
            {
                char **row;
                char buf[256];
                while((row = pDb->GetRow()) != NULL)
                {
                    snprintf(buf,256,"%s|%s|%s|%s|%s",row[0],row[1],row[2],row[3],row[4]);
                    questionList.push_back(buf);
                }
            }
        }
    }
    if(questionList.size() > 0)
    {
        int r = Random(0,questionList.size()-1);
        return questionList[r].c_str();
    }
    return NULL;
}

const char *GetQuestion2()
{
    static vector<string> questionList;
    if(questionList.size() == 0)
    {
        CGetDbConnect getDb;
        CDatabaseSql *pDb = getDb.GetDbConnect();
        if(pDb != NULL)
        {
            if(pDb->Query("select question,answer1,answer2,answer3,answer4 from question2"))
            {
                char **row;
                char buf[256];
                while((row = pDb->GetRow()) != NULL)
                {
                    snprintf(buf,256,"%s|%s|%s|%s|%s",row[0],row[1],row[2],row[3],row[4]);
                    questionList.push_back(buf);
                }
            }
        }
    }
    if(questionList.size() > 0)
    {
        int r = Random(0,questionList.size()-1);
        return questionList[r].c_str();
    }
    return NULL;
}
//返回"鉴定书1|鉴定书2|鉴定书3"
char *IdentifyBook(CUser *pUser,uint8 pos)
{
    if(pUser == NULL)
        return NULL;
    SItemInstance *pItem = pUser->GetItem(pos);
    if((pItem == NULL) || (pItem->tmplId != 1817))
        return NULL;
    uint8 bangDing = pItem->bangDing;
    pUser->DelPackage(pos);
    static uint16 jianDing[] = {540,4,541,2,542,2,543,2,544,4,545,2,546,1,547,1,548,3,549,2,550,2,551,4,552,1,553,4,554,2,555,1,556,1,557,4,558,2,559,1,560,1,561,3,562,1,563,1,564,4,565,1,566,1,567,3,568,1,569,3,570,3,571,3,572,1,573,2,574,3,575,3,576,3,577,3,578,2,579,3,580,2,581,1,582,1,583,1,584,1,585,1,586,1,587,1,588,1};
	if(jianDing[3] == 2)
	{
		for(uint16 i = 3; i < sizeof(jianDing)/sizeof(uint16); i += 2)
		{
			jianDing[i] += jianDing[i-2];
		}
    }
    
    //出1本书的概率为40%，2本书概率为30%，3本书为30%
    uint16 r = Random(0,99);
    uint8 num;
    if(r < 40)
        num = 1;
    else if(r < 70)
        num = 2;
    else
        num = 3;
    
    CItemTemplateManager &itemMgr = SingletonItemManager::instance();
    static char buf[32];
    buf[0] = 0;
    for(int i = 0; i < num; i++)
    {
        r = Random(0,100);
        for(uint16 j = 1; j < sizeof(jianDing)/sizeof(uint16); j += 2)
		{
			if(r <= jianDing[j])
			{
			    SItemInstance item = {0};
			    item.tmplId = jianDing[j-1];
			    item.bangDing = bangDing;
			    if(pUser->AddPackage(item))
			    {
			        SItemTemplate *pItem = itemMgr.GetItem(jianDing[j-1]);
			        if(pItem != NULL)
			        {
			            if(i != 0)
			                strcat(buf,"|");
			            strcat(buf,pItem->name.c_str());
			        }
			    }
			    break;
			}
		}
    }
    return buf;
}

//答题活动是否开始
static bool sDaTiBegin = false;
static time_t sStartTime = 0;
static int sDaTiType = 0;
struct SUserScore
{
    uint32 roleId;
    int score;
};
static vector<SUserScore> sDaTiUsers;

struct SSortUserScore
{
    bool operator()(const SUserScore &s1,const SUserScore &s2)
    {
        return s1.score < s2.score;
    }
};

void HuoDongTimer()
{
    time_t t = GetSysTime();
    struct tm *pTm = localtime(&t);
    if(pTm == NULL)
        return;
    int hour = pTm->tm_hour;
    int min = pTm->tm_min;
    if(pTm->tm_mon != 2)
        return;
        
    if(((pTm->tm_wday == 3) || (pTm->tm_wday == 5) || (pTm->tm_wday == 0))// || (pTm->tm_wday == 1))
        && (hour == 10))//10
    {
        if((min == 30) && (pTm->tm_sec <= 0))
        {
            SysInfoToAllUser("距离才【科考送通宝】开始还有30分钟");
        }
        else if((min == 40) && (pTm->tm_sec <= 0))
        {
            SysInfoToAllUser("距离才【科考送通宝】开始还有20分钟");
        }
        else if((min == 50) && (pTm->tm_sec <= 0))
        {
            SysInfoToAllUser("距离【科考送通宝】开始还有10分钟");
        }
    }
    if(hour == 11)//11
    {
        if(!sDaTiBegin)
        {
            sDaTiBegin = true;
            sStartTime = GetSysTime();
            sDaTiType = 0;
            sDaTiUsers.clear();
        }
        else if((min % 5 == 0) && (pTm->tm_sec <= 0))
        {
            if(pTm->tm_wday == 3)
                SysInfoToAllUser("11:00-12:00【科考送通宝】正在越亭郡北乡试官处进行！");
            else if(pTm->tm_wday == 5)
                SysInfoToAllUser("11:00-12:00【科考送通宝】正在丹阳城东南会试官处进行！");
            else if(pTm->tm_wday == 0)
                SysInfoToAllUser("11:00-12:00【科考送通宝】正在皇宫殿试官处进行！");
        }
    }
    if((sDaTiBegin) && (GetSysTime() - sStartTime > 60*60))
    {
        sDaTiBegin = false;
        SSortUserScore sortScore;
        std::sort(sDaTiUsers.begin(),sDaTiUsers.end(),sortScore);
        CGetDbConnect getDb;
        CDatabaseSql *pDb = getDb.GetDbConnect();
        if(pDb == NULL)
            return;
        char sql[4096];
        COnlineUser &onlineUser = SingletonOnlineUser::instance();
            
        if(sDaTiType == 1)
        {//前200名给予参加会试资格位变量98
            int end = sDaTiUsers.size();
            pDb->Query("truncate table dati_paiming1");
            CUser *pUser = new CUser;
            for(int i = 0; i < end; i++)
            {
                snprintf(sql,512,"INSERT INTO dati_paiming1(role_id,score) VALUES (%d,%d)",
                    sDaTiUsers[i].roleId,sDaTiUsers[i].score);
                pDb->Query(sql);
                if(i >= 200)
                    continue;
                    
                ShareUserPtr ptr = onlineUser.GetUserByRoleId(sDaTiUsers[i].roleId);
                if(ptr.get() != NULL)
                {
                    ptr->SetBitSet(98);
                }
                else
                {
                    snprintf(sql,512,"select bitset from role_info where id=%u",sDaTiUsers[i].roleId);
                    if(!pDb->Query(sql))
                        continue;
                    char **row = pDb->GetRow();
                    if(row == NULL)
                        continue;
                    pUser->SetBitSet(row[0]);
                    pUser->SetBitSet(98);
                    string str;
                    pUser->GetBitSet(str);
                    snprintf(sql,4096,"update role_info set bitset='%s' where id=%u",str.c_str(),sDaTiUsers[i].roleId);
                    pDb->Query(sql);
                }
            }
            delete pUser;
        }
        else if(sDaTiType == 2)
        {//50名给予参加殿试资格位变量99 给予举人称号
            int end = sDaTiUsers.size();
            pDb->Query("truncate table dati_paiming2");
            CUser *pUser = new CUser;
            for(int i = 0; i < end; i++)
            {
                snprintf(sql,512,"INSERT INTO dati_paiming2(role_id,score) VALUES (%d,%d)",
                    sDaTiUsers[i].roleId,sDaTiUsers[i].score);
                pDb->Query(sql);
                if(i >= 50)
                    continue;
                ShareUserPtr ptr = onlineUser.GetUserByRoleId(sDaTiUsers[i].roleId);
                if(ptr.get() != NULL)
                {
                    ptr->SetBitSet(99);
                    ptr->AddTitle(EUT8);
                }
                else
                {
                    snprintf(sql,512,"select bitset,save_data from role_info where id=%u",sDaTiUsers[i].roleId);
                    if(!pDb->Query(sql))
                        continue;
                    char **row = pDb->GetRow();
                    if(row == NULL)
                        continue;
                    pUser->SetBitSet(row[0]);
                    pUser->ReadSaveData(row[1]);  
                    pUser->SetBitSet(99);
                    pUser->AddTitle(EUT8);
                    string str;
                    string str1;
                    pUser->WriteSaveData(str1);
                    pUser->GetBitSet(str);
                    snprintf(sql,4096,"update role_info set bitset='%s',save_data='%s',title=%d where id=%u",
                        str.c_str(),str1.c_str(),pUser->GetTitle(),sDaTiUsers[i].roleId);
                    pDb->Query(sql);
                }
            }
            delete pUser;
        }
        else if(sDaTiType == 3)
        {//第一名给予状元称号,第二名榜眼称号，第三名获得“探花
            int end = sDaTiUsers.size();
            pDb->Query("truncate table dati_paiming3");
            CUser *pUser = new CUser;
            for(int i = 0; i < end; i++)
            {
                snprintf(sql,512,"INSERT INTO dati_paiming3(role_id,score) VALUES (%d,%d)",
                    sDaTiUsers[i].roleId,sDaTiUsers[i].score);
                pDb->Query(sql);
                if(i >= 3)
                    continue;
                    
                ShareUserPtr ptr = onlineUser.GetUserByRoleId(sDaTiUsers[i].roleId);
                if(ptr.get() != NULL)
                {
                    ptr->AddTitle(EUT5+i);
                    char msg[256] = {0};
                    snprintf(msg,256,"恭喜%s获得",pUser->GetName());
                    if(i == 0)
                        strcat(msg,"状元");
                    else if(i == 1)
                        strcat(msg,"榜眼");
                    else if(i == 2)
                        strcat(msg,"探花");
                    SysInfoToAllUser(msg);
                }
                else
                {
                    snprintf(sql,512,"select save_data,name from role_info where id=%u",sDaTiUsers[i].roleId);
                    if(!pDb->Query(sql))
                        continue;
                    char **row = pDb->GetRow();
                    if(row == NULL)
                        continue;
                    char msg[256] = {0};
                    snprintf(msg,256,"恭喜%s获得",row[1]);
                    if(i == 0)
                        strcat(msg,"状元");
                    else if(i == 1)
                        strcat(msg,"榜眼");
                    else if(i == 2)
                        strcat(msg,"探花");
                    SysInfoToAllUser(msg);
                    pUser->ReadSaveData(row[0]);  
                    string str;
                    pUser->AddTitle(EUT5+i);
                    pUser->WriteSaveData(str);
                    snprintf(sql,4096,"update role_info set save_data='%s',title=%d where id=%u",
                        str.c_str(),pUser->GetTitle(),sDaTiUsers[i].roleId);
                    pDb->Query(sql);
                }
            }
            delete pUser;
        }
        sDaTiUsers.clear();
    }
}

int GetDaTiPaiMing(CUser *pUser,int type)
{
    if(pUser == NULL)
        return 0;
    CGetDbConnect getDb;
    CDatabaseSql *pDb = getDb.GetDbConnect();
    if(pDb == NULL)
        return 0;
    char sql[256];
    snprintf(sql,256,"select id from dati_paiming%d where role_id=%u",type,pUser->GetRoleId());
    if(!pDb->Query(sql))
        return 0;
    char **row = pDb->GetRow();
    if(row != NULL)
        return atoi(row[0]);
    return 0;
}
//提交成绩
//乡试 1
//会试 2
//殿试 3
void SubmitScore(CUser *pUser,int type,int score)
{
    if(pUser == NULL)
        return;
        
    sDaTiType = type;
    int size = sDaTiUsers.size();
    for(int i = 0; i < size; i++)
    {
        if(sDaTiUsers[i].roleId == pUser->GetRoleId())
        {
            sDaTiUsers[i].score = score;
            return;
        }
    }
    SUserScore userScore;
    userScore.roleId = pUser->GetRoleId();
    userScore.score = score;
    sDaTiUsers.push_back(userScore);
}

void QiChongBattle1(CUser *pUser)
{
    if((pUser != NULL) && (pUser->GetScene() != NULL))
    {
        pUser->GetScene()->QiChongBattle1(pUser);
    }
}
void QiChongBattle2(CUser *pUser)
{
    if((pUser != NULL) && (pUser->GetScene() != NULL))
    {
        pUser->GetScene()->QiChongBattle2(pUser);
    }
}
void QiChongBattle3(CUser *pUser)
{
    if((pUser != NULL) && (pUser->GetScene() != NULL))
    {
        pUser->GetScene()->QiChongBattle3(pUser);
    }
}
void QiChongBattle4(CUser *pUser)
{
    if((pUser != NULL) && (pUser->GetScene() != NULL))
    {
        pUser->GetScene()->QiChongBattle4(pUser);
    }
}
void QiChongBattle5(CUser *pUser)
{
    if((pUser != NULL) && (pUser->GetScene() != NULL))
    {
        pUser->GetScene()->QiChongBattle5(pUser);
    }
}
void QiChongBattle6(CUser *pUser)
{
    if((pUser != NULL) && (pUser->GetScene() != NULL))
    {
        pUser->GetScene()->QiChongBattle6(pUser);
    }
}

void QiChongBattle7(CUser *pUser)
{
    if((pUser != NULL) && (pUser->GetScene() != NULL))
    {
        pUser->GetScene()->QiChongBattle7(pUser);
    }
}
void QiChongBattle8(CUser *pUser)
{
    if((pUser != NULL) && (pUser->GetScene() != NULL))
    {
        pUser->GetScene()->QiChongBattle8(pUser);
    }
}
void QiChongBattle9(CUser *pUser)
{
    if((pUser != NULL) && (pUser->GetScene() != NULL))
    {
        pUser->GetScene()->QiChongBattle9(pUser);
    }
}
void QiChongBattle10(CUser *pUser)
{
    if((pUser != NULL) && (pUser->GetScene() != NULL))
    {
        pUser->GetScene()->QiChongBattle10(pUser);
    }
}
void QiChongBattle11(CUser *pUser)
{
    if((pUser != NULL) && (pUser->GetScene() != NULL))
    {
        pUser->GetScene()->QiChongBattle11(pUser);
    }
}
void QiChongBattle12(CUser *pUser)
{
    if((pUser != NULL) && (pUser->GetScene() != NULL))
    {
        pUser->GetScene()->QiChongBattle12(pUser);
    }
}
void QiChongBattle13(CUser *pUser)
{
    if((pUser != NULL) && (pUser->GetScene() != NULL))
    {
        pUser->GetScene()->QiChongBattle13(pUser);
    }
}
void QiChongBattle14(CUser *pUser)
{
    if((pUser != NULL) && (pUser->GetScene() != NULL))
    {
        pUser->GetScene()->QiChongBattle14(pUser);
    }
}

void LangBattle1(CUser *pUser)
{
    if((pUser != NULL) && (pUser->GetScene() != NULL))
    {
        pUser->GetScene()->LangBattle1(pUser);
    }
}
void LangBattle2(CUser *pUser)
{
    if((pUser != NULL) && (pUser->GetScene() != NULL))
    {
        pUser->GetScene()->LangBattle2(pUser);
    }
}
void LangBattle3(CUser *pUser)
{
    if((pUser != NULL) && (pUser->GetScene() != NULL))
    {
        pUser->GetScene()->LangBattle3(pUser);
    }
}
void LangBattle4(CUser *pUser)
{
    if((pUser != NULL) && (pUser->GetScene() != NULL))
    {
        pUser->GetScene()->LangBattle4(pUser);
    }
}
void LangBattle5(CUser *pUser)
{
    if((pUser != NULL) && (pUser->GetScene() != NULL))
    {
        pUser->GetScene()->LangBattle5(pUser);
    }
}
void LangBattle6(CUser *pUser)
{
    if((pUser != NULL) && (pUser->GetScene() != NULL))
    {
        pUser->GetScene()->LangBattle6(pUser);
    }
}
void LangBattle7(CUser *pUser)
{
    if((pUser != NULL) && (pUser->GetScene() != NULL))
    {
        pUser->GetScene()->LangBattle7(pUser);
    }
}
void LangBattle8(CUser *pUser)
{
    if((pUser != NULL) && (pUser->GetScene() != NULL))
    {
        pUser->GetScene()->LangBattle8(pUser);
    }
}
void LangBattle9(CUser *pUser)
{
    if((pUser != NULL) && (pUser->GetScene() != NULL))
    {
        pUser->GetScene()->LangBattle9(pUser);
    }
}
void LangBattle10(CUser *pUser)
{
    if((pUser != NULL) && (pUser->GetScene() != NULL))
    {
        pUser->GetScene()->LangBattle10(pUser);
    }
}
void LangBattle12(CUser *pUser)
{
    if((pUser != NULL) && (pUser->GetScene() != NULL))
    {
        pUser->GetScene()->LangBattle12(pUser);
    }
}

void ShiYaoYW1(CUser *pUser)
{
    if((pUser != NULL) && (pUser->GetScene() != NULL))
    {
        pUser->GetScene()->ShiYaoYW1(pUser);
    }
}
void ShiYaoYW2(CUser *pUser)
{
    if((pUser != NULL) && (pUser->GetScene() != NULL))
    {
        pUser->GetScene()->ShiYaoYW2(pUser);
    }
}
void ShiYaoYW3(CUser *pUser)
{
    if((pUser != NULL) && (pUser->GetScene() != NULL))
    {
        pUser->GetScene()->ShiYaoYW3(pUser);
    }
}
void ShiYaoYW4(CUser *pUser)
{
    if((pUser != NULL) && (pUser->GetScene() != NULL))
    {
        pUser->GetScene()->ShiYaoYW4(pUser);
    }
}
void ShiYaoYW5(CUser *pUser)
{
    if((pUser != NULL) && (pUser->GetScene() != NULL))
    {
        pUser->GetScene()->ShiYaoYW5(pUser);
    }
}
void ShiYaoYW6(CUser *pUser)
{
    if((pUser != NULL) && (pUser->GetScene() != NULL))
    {
        pUser->GetScene()->ShiYaoYW6(pUser);
    }
}
void ShiYaoYW7(CUser *pUser)
{
    if((pUser != NULL) && (pUser->GetScene() != NULL))
    {
        pUser->GetScene()->ShiYaoYW7(pUser);
    }
}
void ShiYaoYW8(CUser *pUser)
{
    if((pUser != NULL) && (pUser->GetScene() != NULL))
    {
        pUser->GetScene()->ShiYaoYW8(pUser);
    }
}
void ShiYaoYW9(CUser *pUser)
{
    if((pUser != NULL) && (pUser->GetScene() != NULL))
    {
        pUser->GetScene()->ShiYaoYW9(pUser);
    }
}
void ShiYaoYW10(CUser *pUser)
{
    if((pUser != NULL) && (pUser->GetScene() != NULL))
    {
        pUser->GetScene()->ShiYaoYW10(pUser);
    }
}

#define DEFINE_FIGHT(ScriptFight)\
void ScriptFight(CUser *pUser)\
{\
    if((pUser != NULL) && (pUser->GetScene() != NULL))\
    {\
        pUser->GetScene()->ScriptFight(pUser);\
    }\
}

DEFINE_FIGHT(ShiYaoYX1)
DEFINE_FIGHT(ShiYaoYX2)
DEFINE_FIGHT(ShiYaoYX3)
DEFINE_FIGHT(ShiYaoYX4)
DEFINE_FIGHT(ShiYaoYX5)
DEFINE_FIGHT(ShiYaoYX6)
DEFINE_FIGHT(ShiYaoYX7)
DEFINE_FIGHT(ShiYaoYX8)
DEFINE_FIGHT(ShiYaoYX9)
DEFINE_FIGHT(ShiYaoYX10)

DEFINE_FIGHT(ShiYaoPT1)
DEFINE_FIGHT(ShiYaoPT2)
DEFINE_FIGHT(ShiYaoPT3)
DEFINE_FIGHT(ShiYaoPT4)
DEFINE_FIGHT(ShiYaoPT5)
DEFINE_FIGHT(ShiYaoPT6)
DEFINE_FIGHT(ShiYaoPT7)
DEFINE_FIGHT(ShiYaoPT8)
DEFINE_FIGHT(ShiYaoPT9)
DEFINE_FIGHT(ShiYaoPT10)

DEFINE_FIGHT(NuYanQiLing)
DEFINE_FIGHT(XianBing1)
DEFINE_FIGHT(XianBing2)
DEFINE_FIGHT(YaoBing1)
DEFINE_FIGHT(YaoBing2)
DEFINE_FIGHT(MenPaiChuanSong)
DEFINE_FIGHT(QingLongBaoBao)
DEFINE_FIGHT(KuLouDaWang)

DEFINE_FIGHT(JuQingXianRen)
DEFINE_FIGHT(JuQingQiLing)
DEFINE_FIGHT(JuQingMingYao)

DEFINE_FIGHT(DiaoYuFight)

DEFINE_FIGHT(ThreeKuLou)

int GetChongZhi(CUser *pUser)
{
    if(pUser == NULL)
        return 0;
    CGetDbConnect getDb;
    CDatabaseSql *pDb = getDb.GetDbConnect();
    char sql[512];
    sprintf(sql,"SELECT tongbao FROM role_cz where role_id=%u",pUser->GetRoleId());
    if ((pDb == NULL) || (!pDb->Query(sql)))
    {
        return 0;
    }
    char **row = pDb->GetRow();
    if(row != NULL)
        return atoi(row[0]);
    return 0;
}
/*
1	人数不符。弹出提示：只有男女双方二人组队才能申请结婚。	 
2	等级不符。弹出提示：只有双方等级均在40级以上才可结婚。	 
3	亲密度不符。弹出提示：双方友好度必须达到1000以上才可结婚。	 
4	身份不符。弹出提示：结婚前请先解散师徒关系。	 
5	已经结婚。弹出提示：您的队伍中已经有人结婚了，请离婚后再来。	 
6	时间不符。弹出提示：您的队伍中有成员刚刚离婚，
3天内不能再次结婚。	 
*/
int CheckMarry(CUser *pUser) 
{
    if(pUser == NULL)
        return 1;
        
    if(GetTeamMemNum(pUser) != 2)
        return 1;
    CUser *p = GetTeamMember1(pUser);
    if(p == NULL)
        return 1;
    if((pUser->GetLevel() < 40) || (p->GetLevel() < 40))
        return 2;
    if(pUser->GetSex() == p->GetSex())
        return 1;
        
    uint16 qinmi = 0;
    pUser->GetHotVal(p->GetRoleId(),qinmi);
    if(qinmi < 1000)
        return 3;
    
    qinmi = 0;
    p->GetHotVal(pUser->GetRoleId(),qinmi);
    if(qinmi < 1000)
        return 3;
        
    CGetDbConnect getDb;
    CDatabaseSql *pDb = getDb.GetDbConnect();
    if(pDb == NULL)
        return 4;
    char sql[128];
    snprintf(sql,127,"select m_id from master_prentice where (m_id=%u and p_id=%u) "\
        "or (p_id=%u and m_id=%u)",pUser->GetRoleId(),p->GetRoleId(),pUser->GetRoleId(),p->GetRoleId());
    
    if(!pDb->Query(sql))
        return 4;
    char **row = pDb->GetRow();
    if(row != NULL)
        return 4;
    
    if((pUser->GetData32(6) != 0) || (p->GetData32(6) != 0))
        return 5;
    if((GetSysTime() - pUser->GetData32(7) < 3*3600*24) 
        || (GetSysTime() - p->GetData32(7) < 3*3600*24))
        return 6;
    
    return 0;
}

const int MARRY_LIST_SIZE = 20;
const int MARRY_INFO_MAX_LEN = 64;
struct SMarryInfo
{
    char info[MARRY_INFO_MAX_LEN];
    char name1[MAX_NAME_LEN];
    char name2[MAX_NAME_LEN];
    uint32 id;
    uint32 id1;
    time_t beginTime;
    int tongbao;
};

static SMarryInfo sMarryList[MARRY_LIST_SIZE];

int DoDivorce(CUser *pUser)
{
    if(pUser == NULL)
        return 1;
    uint32 roleId = pUser->GetData32(6);
    if(roleId == 0)
        return 1;
    
    for(uint8 i = 0; i < MARRY_LIST_SIZE; i++)
    {
        if((sMarryList[i].id == pUser->GetRoleId()) 
            || (sMarryList[i].id == pUser->GetData32(6)))
        {
            sMarryList[i].id = 0;
            sMarryList[i].tongbao = 0;
            break;
        }
    }
    pUser->SetHotVal(roleId,0);
    pUser->SetData32(6,0);
    pUser->SetData32(7,GetSysTime());
    pUser->SetData8(6,0);
    
    pUser->SendUpdateInfo(44,0);
    
    UpdateUserInfo(pUser);
    
    char info[64];
    ShareUserPtr ptr = SingletonOnlineUser::instance().GetUserByRoleId(roleId);
    if(ptr.get() == NULL)
    {
        char sql[1024];
        snprintf(sql,1024,"select save_data,name from role_info where id=%u",roleId);
        CGetDbConnect getDb;
        CDatabaseSql *pDb = getDb.GetDbConnect();
        if(pDb == NULL)
            return 0;
        if(!pDb->Query(sql))
            return 0;
        char **row = pDb->GetRow();
        if(row == NULL)
            return 0;
        CUser *p = new CUser;
        p->ReadSaveData(row[0]);  
        p->SetData32(6,0);
        p->SetData8(6,0);
        p->SetData32(7,GetSysTime());
        string str;
        p->WriteSaveData(str);
        snprintf(sql,4096,"update role_info set save_data='%s' where id=%u",
            str.c_str(),roleId);
        pDb->Query(sql);
        delete p;
        
        snprintf(info,64,"%s与您解除了夫妻关系",row[1]);
        SendSysMail(pUser,info);
        
        snprintf(info,64,"%s与您解除了夫妻关系",pUser->GetName());
        sprintf(sql,"INSERT INTO role_mail (from_id,to_id,from_name,time,msg) VALUES ("\
                    "%d,%d,'%s',%lu,'%s')",
                    0,roleId,"【系统】",GetSysTime(),info);
        pDb->Query(sql);
    }
    else
    {
        ptr->SetData8(6,0);
        ptr->SetData32(6,0);
        ptr->SetData32(7,GetSysTime());
        ptr->SendUpdateInfo(44,0);
        UpdateUserInfo(ptr.get());
        
        snprintf(info,64,"%s与您解除了夫妻关系",ptr->GetName());
        SendSysMail(pUser,info);
        
        snprintf(info,64,"%s与您解除了夫妻关系",pUser->GetName());
        SendSysMail(ptr.get(),info);
    }
    return 0;
}

void SaveMarryList()
{
    CGetDbConnect getDb;
    CDatabaseSql *pDb = getDb.GetDbConnect();
    if(pDb == NULL)
        return;
    pDb->Query("truncate marry_hall");
    char buf[512];
    for(uint8 i = 0; i < MARRY_LIST_SIZE; i++)
    {
        if(GetSysTime() - sMarryList[i].beginTime > 2*3600)
            sMarryList[i].id = 0;
        if(sMarryList[i].id != 0)
        {
            snprintf(buf,512,"INSERT INTO marry_hall (id,id1,name1,name2,info,start_time,tongbao) "\
                "VALUES (%d,%d,'%s','%s','%s',%lu,%d)",
                sMarryList[i].id,
                sMarryList[i].id1,
                sMarryList[i].name1,
                sMarryList[i].name2,
                sMarryList[i].info,sMarryList[i].beginTime,sMarryList[i].tongbao);
            pDb->Query(buf);
        }
    }
}

void ReadMarryList()
{
    CGetDbConnect getDb;
    CDatabaseSql *pDb = getDb.GetDbConnect();
    if(pDb == NULL)
        return;
    
    if(!pDb->Query("select id,name1,name2,info,start_time,tongbao,id1 from marry_hall order by start_time"))
        return;
    
    char **row;
    
    while((row = pDb->GetRow()) != NULL)
    {
        for(uint8 i = 0; i < MARRY_LIST_SIZE; i++)
        {
            if(sMarryList[i].id == 0)
            {
                sMarryList[i].id = atoi(row[0]);
                sMarryList[i].id1 = atoi(row[6]);
                strcat(sMarryList[i].name1,row[1]);
                strcat(sMarryList[i].name2,row[2]);
                strcat(sMarryList[i].info,row[3]);
                sMarryList[i].beginTime = atoi(row[4]);
                sMarryList[i].tongbao = atoi(row[5]);
                break;
            }
        }
    }
}

/*
TYPE=24 当前婚礼列表        
+-----+----+-----+------+        
| NUM | ID | LEN | CONT |        
+-----+----+-----+------+        
|  1  |  4 |  2  |  Var |        
+-----+----+-----+------+ 
*/
void GetWedding(CUser *pUser)
{
    if(pUser == NULL)
        return;
    CSocketServer &sock = SingletonSocket::instance();
    CNetMessage msg;
    msg.SetType(PRO_INTERACT);
    
    uint8 num = 0;
    msg<<(uint8)24;
    uint16 pos = msg.GetDataLen();
    msg<<num;
    for(uint8 i = 0; i < MARRY_LIST_SIZE; i++)
    {
        if(GetSysTime() - sMarryList[i].beginTime > 2*3600)
            sMarryList[i].id = 0;
        if(sMarryList[i].id != 0)
        {
            msg<<sMarryList[i].id<<sMarryList[i].info;
            num++;
        }
    }
    msg.WriteData(pos,&num,1);
    sock.SendMsg(pUser->GetSock(),msg);
}

int GetMarryLeftTime(CUser *pUser)
{
    if(pUser == NULL)
        return 0;
    CScene *pScene = pUser->GetScene();
    uint32 id = 0;
    if(pScene != NULL)
        id = pScene->GetId();
        
    for(uint8 i = 0; i < MARRY_LIST_SIZE; i++)
    {
        if(sMarryList[i].id == (uint32)id)
        {
            int t = 2*3600 - (GetSysTime() - sMarryList[i].beginTime);
            if(t < 0)
                t = 0;
            return t;
        }
    }
    return 0;
}

bool HaveMarryHall(uint32 id)
{
    for(uint8 i = 0; i < MARRY_LIST_SIZE; i++)
    {
        if(GetSysTime() - sMarryList[i].beginTime > 2*3600)
            sMarryList[i].id = 0;
        if(sMarryList[i].id == id)
        {
            return true;
        }
    }
    return false;
}

//婚礼殿堂(副本)250 ret=1 组队状态无法进入 
//ret=2 没有相应喜贴 新郎新娘双方在婚礼有效时间内，可自由出入
int EnterWedding(CUser *pUser,int id) 
{
    if(pUser == NULL)
        return 3;
        
    bool haveHall = false;
    uint8 i = 0;
    for(; i < MARRY_LIST_SIZE; i++)
    {
        if(((int)sMarryList[i].id == id) 
            && (GetSysTime() - sMarryList[i].beginTime < 2*3600))
        {
            haveHall = true;
            break;
        }
    }
    if(!haveHall)
        return 3;
    if(pUser->GetTeam() != 0)
        return 1;
    if(pUser->AdminLevel() <= 0)
    {
        if((pUser->GetData32(6) != (uint32)id) && (pUser->GetRoleId() != (uint32)id))
        {
            //if(!pUser->HaveItem(1843))
            if(!pUser->HaveNameItem(1843,sMarryList[i].name1) 
                && !pUser->HaveNameItem(1843,sMarryList[i].name2))
                return 2;
        }
    }
    
    CSceneManager &scene = SingletonSceneManager::instance();
    CScene *pScene = scene.GetMarryHall(id);
    if(pScene == NULL)
        return 0;
    CNetMessage msg;
    msg.SetType(PRO_JUMP_SCENE);
    uint8 x = 15;
    uint8 y = 25;
    uint8 face = 8;
    msg<<(uint16)pScene->GetMapId()<<x<<y<<face;
    
    CSocketServer &sock = SingletonSocket::instance();
    sock.SendMsg(pUser->GetSock(),msg);
    pUser->SetPos(x,y);
    pUser->SetFace(face);
    pUser->EnterScene(pScene);
    return 0;
}

//type=1 普通婚礼 type=2 豪华婚礼,婚礼两小时结束
//普通婚礼：给予银色恋情戒指(463) 皇宫放礼花 系统公告“恭喜XXX与XXX喜结良缘，大家一起来祝福他们吧。” 
//豪华婚礼: 给予永恒之恋戒指(464)，结婚礼服状态（持续2小时),世界所有地图放礼花，
//系统公告“XXX与XXX的爱情感动上天，六界齐贺，特此送上最隆重的祝福。”
int DoMarry(CUser *pUser,int type)
{
    if(pUser == NULL)
        return 1;
        
    if(GetTeamMemNum(pUser) != 2)
        return 1;
    CUser *p = GetTeamMember1(pUser);
    if(p == NULL)
        return 1;
    
    //pUser->SetHotVal(p->GetRoleId(),0);
    //p->SetHotVal(pUser->GetRoleId(),0);
    if(CheckMarry(pUser) == 0)
    {
        pUser->SetData32(6,p->GetRoleId());
        p->SetData32(6,pUser->GetRoleId());
        pUser->SetData32(8,GetSysTime());
        p->SetData32(8,GetSysTime());
        
        CNetMessage msg;
        msg.SetType(MSG_SERVER_VISUAL_EFFECT);
        msg<<(uint8)2;
        char buf[256];
        if(type == 1)
        {
            SendSceneMsg(msg,20);
            pUser->AddBangDingPackage(463,1,p->GetName());
            p->AddBangDingPackage(463,1,pUser->GetName());
            snprintf(buf,256,"恭喜[%s]与[%s]喜结良缘，大家一起来祝福他们吧。",
                pUser->GetName(),p->GetName());
            SysInfoToAllUser(buf);
        }
        else if(type == 2)
        {
            pUser->AddBangDingPackage(464,1,p->GetName());
            p->AddBangDingPackage(464,1,pUser->GetName());
            pUser->SetData8(6,1);
            p->SetData8(6,1);
            
            pUser->SetData32(10,2000);
            p->SetData32(10,2000);
            
            SendMsgToAllUser(msg);
            snprintf(buf,256,"[%s]与[%s]的爱情感动上天，六界齐贺，特此送上最隆重的祝福。",
                pUser->GetName(),p->GetName());
            SysInfoToAllUser(buf);
            UpdateUserInfo(pUser);
            pUser->SendUpdateInfo(44,1);
            p->SendUpdateInfo(44,1);
            UpdateUserInfo(p);
        }
    }
    for(uint8 i = 0; i < MARRY_LIST_SIZE; i++)
    {
        if(GetSysTime() - sMarryList[i].beginTime > 2*3600)
            sMarryList[i].id = 0;
        if(sMarryList[i].id == 0)
        {
            sMarryList[i].id = pUser->GetRoleId();
            sMarryList[i].id1 = p->GetRoleId();
            sMarryList[i].beginTime = GetSysTime();
            sMarryList[i].tongbao = 0;
            snprintf(sMarryList[i].info,sizeof(sMarryList[i].info),"[%s]与[%s]婚礼殿堂",
                pUser->GetName(),p->GetName());
            snprintf(sMarryList[i].name1,sizeof(sMarryList[i].name1),"%s",pUser->GetName());
            snprintf(sMarryList[i].name2,sizeof(sMarryList[i].name2),"%s",p->GetName());
            break;
        }
    }
    return 0;
}

int GetMarriedId(CUser *pUser)
{
    if(pUser == NULL)
        return 0;
    return pUser->GetData32(6);
}

//已经结婚 返回对方角色名，否则返回nil
const char *GetMarried(CUser *pUser)
{
    if(pUser == NULL)
        return NULL;
    uint32 roleId = pUser->GetData32(6);
    if(roleId == 0)
        return NULL;
    ShareUserPtr ptr = SingletonOnlineUser::instance().GetUserByRoleId(roleId);
    if(ptr.get() != NULL)
        return ptr->GetName();
    CGetDbConnect getDb;
    CDatabaseSql *pDb = getDb.GetDbConnect();
    if(pDb == NULL)
        return NULL;
    char sql[128];
    snprintf(sql,127,"select name from role_info where id=%u",roleId);
    if(!pDb->Query(sql))
        return NULL;
    char **row = pDb->GetRow();
    if(row == NULL)
        return NULL;
    return row[0];
}

/*ret=1 通宝不足 ret=2 时间未到 ret=0 成功，3
玩家得到物品，系统公告”XXX（使用者名称）在婚礼上摆下XX（数量）个XXXX（食物名称），大宴宾客。
山珍海味（1000通宝一份）
美味佳肴（100通宝一份）
特色小吃（10通宝一份）*/
int YanQing(CUser *pUser,int id,int num)
{
    if(pUser == NULL)
        return 0;
    if(num <= 0)
        return 0;
    if((id != 1840) && (id != 1841) && (id != 1842))
        return 0;
    CScene *pScene = pUser->GetScene();
    if(pScene == NULL)
        return 0;
    if(pScene->GetMapId() != 250)
        return 3;
    
    if(((uint32)pScene->GetId() != (pUser->GetData32(6))) 
        && (pUser->GetRoleId() != (uint32)pScene->GetId()))
        return 3;
    
    int money = 0;
    if(id == 1840)
        money = 1000;
    else if(id == 1841)
        money = 100;
    else 
        money = 10;
    if(pUser->GetTongBao() < money * num)
        return 1;
    pUser->AddTongBao(-money*num);
    char buf[512];
    if(id == 1840)
        snprintf(buf,512,"%s在婚礼上摆下%d个山珍海味，大宴宾客。",pUser->GetName(),num);
    else if(id == 1841)
        snprintf(buf,512,"%s在婚礼上摆下%d个美味佳肴，大宴宾客。",pUser->GetName(),num);
    else 
        snprintf(buf,512,"%s在婚礼上摆下%d个特色小吃，大宴宾客。",pUser->GetName(),num);
    SysInfoToAllUser(buf);
    
    CNetMessage msg;
    msg.SetType(MSG_SERVER_VISUAL_EFFECT);
    msg<<(uint8)3;
    SendSceneMsg(msg,pUser->GetScene());
    
    list<uint32> userList;
    pScene->GetUserList(userList);
    userList.remove(pUser->GetRoleId());
    userList.remove(pUser->GetData32(6));
    
    int tolUser = userList.size();
    if(tolUser <= 0)
        return 0;

    COnlineUser &onlineUser = SingletonOnlineUser::instance();
    for(int i = 0; i < num; i++)
    {
        list<uint32>::iterator iter = userList.begin();
        int r = Random(0,tolUser-1);
        for(int j = 0; j < r; j++)
        {
            iter++;
        }
        ShareUserPtr p = onlineUser.GetUserByRoleId(*iter);
        if(p.get() != NULL)
        {
            p->AddBangDingPackage(id);
        }
    }
    
    return 0;
}

bool IsBrideGroom(CUser *pUser)
{
    if(pUser == NULL)
        return false;
    CScene *pScene = pUser->GetScene();
    if(pScene == NULL)
        return false;
    uint8 i = 0;
    for(; i < MARRY_LIST_SIZE; i++)
    {
        if((int)sMarryList[i].id == pScene->GetId())
        {
            break;
        }
    }
    if(i >= MARRY_LIST_SIZE)
        return false;
    if((pUser->GetData32(6) == (uint32)sMarryList[i].id) 
        || (pUser->GetRoleId() == (uint32)sMarryList[i].id))
        return true;
    return false;
}

//婚礼结束后，把礼官所得通宝分发给新人
int GiveHongBao(CUser *pUser,int ind)
{
    //1837 100
    //1838 1000
    if(pUser == NULL)
        return 0;
    CScene *pScene = pUser->GetScene();
    if(pScene == NULL)
        return 0;
    int tongbao = 0;
    SItemInstance *pItem = pUser->GetItem(ind);
    if(pItem == NULL)
        return 0;
    //经典祝福红包：XXX（给予者昵称）为新人献上祝福。 
    //福星高照红包：XXXXXX（给予者昵称）祝福新人XXX与XXX（夫妻名字永结同心，百年好合。
    uint8 i = 0;
    for(; i < MARRY_LIST_SIZE; i++)
    {
        if((int)sMarryList[i].id == pScene->GetId())
        {
            break;
        }
    }
    if(i >= MARRY_LIST_SIZE)
        return 0;
    char buf[128];
    if(pItem->tmplId == 1837)
    {
        CNetMessage msg;
        msg.SetType(PRO_MSG_CHAT);
        snprintf(buf,sizeof(buf),"[%s]为新人献上祝福，赠送一百通宝红包一个",pUser->GetName());
        msg<<(uint8)0<<0<<"[系统]"<<buf;
        SendSceneMsg(msg,pUser->GetScene());
        tongbao = 100;
        AddTongBao(sMarryList[i].id,50);
        AddTongBao(sMarryList[i].id1,50);
    }
    else if(pItem->tmplId == 1838)
    {
        CNetMessage msg;
        msg.SetType(PRO_MSG_CHAT);
        snprintf(buf,sizeof(buf),"[%s]祝福新人[%s]与[%s]永结同心，百年好合，赠送一千通宝红包一个",
            pUser->GetName(),sMarryList[i].name1,sMarryList[i].name2);
        msg<<(uint8)0<<0<<"[系统]"<<buf;
        SendMsgToAllUser(msg);
        tongbao = 1000;
        AddTongBao(sMarryList[i].id,500);
        AddTongBao(sMarryList[i].id1,500);
    }
    else
        return 0;
    pUser->DelPackage(ind);
    sMarryList[i].tongbao += tongbao;
    return 0;
}

//ret兑换的通宝数目 
int DuiHuanTB(CUser *pUser)
{
    if(pUser == NULL)
        return 0;
    int tongbao = pUser->DuiHuanTB();
    if(tongbao > 0)
        pUser->AddTongBao(tongbao);
    return tongbao;
}

/*
TYPE=25 输入数量   
+-----+   
| ID  |   
+-----+   
|  2  |   
+-----+
*/
void InputNumber(CUser *pUser,int id)
{
    if(pUser == NULL)
        return;
    CSocketServer &sock = SingletonSocket::instance();
    CNetMessage msg;
    msg.SetType(PRO_INTERACT);
    
    msg<<(uint8)25<<(uint16)id;
    sock.SendMsg(pUser->GetSock(),msg);
}

//eind-宠铠位置 aind-属性位置
//拆分,0成功，1 没有相应石头 3 属性值为空 4 包裹满
int ChaiFenSX(CUser *pUser,uint8 eind,uint8 aind)
{
    if(pUser == NULL)
        return 1;
    return pUser->ChaiFenSX(eind,aind);
}

void AddPetXiuWei(CUser *pUser,uint8 ind,int xw)
{
    if(pUser == NULL)
        return;
    SharePetPtr pet = pUser->GetPet(ind);
    SPet *pPet = pet.get();
    if(pPet != NULL)
    {
        pPet->xiuWei += xw;
        pUser->UpdatePetInfo(ind,23,pPet->xiuWei);
    }
}

int ChangeCharName(CUser *pUser,char *name)
{
    if((pUser == NULL) || (name == NULL))
        return 1;
    string n = name;
    int nameLen = strlen(name);
    if((nameLen < 2) || (nameLen > 16))
        return 1;
    if(IllegalStr(n))
        return 1;
        
    CGetDbConnect getDb;
    CDatabaseSql *pDb = getDb.GetDbConnect();
    if(pDb == NULL)
        return 1;
    char buf[512];
    sprintf(buf,"update role_info set name='%s' where id=%d",name,pUser->GetRoleId());
    if(pDb->Query(buf))
    {
        pUser->SetName(name);
        return 0;
    }
    return 1;
}

bool CanChangeName(CUser *pUser)
{
    if(pUser == NULL)
        return false;
    return strstr(pUser->GetName(),"@六区") != NULL;
}

static bool sMLSBegin = false;
static vector<SUserScore> sMLSUser;

void MLSTimer()
{
    time_t t = GetSysTime();
    struct tm *pTm = localtime(&t);
    if(pTm == NULL)
        return;
    int hour = pTm->tm_hour;
    int min = pTm->tm_min;
    //if((pTm->tm_wday != 2) && (pTm->tm_wday != 4) && (pTm->tm_wday != 6))// && (pTm->tm_wday != 1))
        //return;
        
    if(hour == MLS_TIME-1)
    {
        if((min == 30) && (pTm->tm_sec <= 0))
        {
            SysInfoToAllUser("【魔道暴走赛】活动即将开始");
        }
        else if((min == 45) && (pTm->tm_sec <= 0))
        {
            SysInfoToAllUser("【魔道暴走赛】活动即将开始");
        }
    }
    if((hour == MLS_TIME) && (min <= 30))
    {
        if((min % 15 == 0) && (pTm->tm_sec <= 0))
        {
            SysInfoToAllUser("【魔道暴走赛】活动开始了，请玩家到星寿村找宠物仙子领取任务");
        }
    }
    if(!sMLSBegin && (hour == MLS_TIME))// && (hour <= 10))
    {
        sMLSBegin = true;
        CGetDbConnect getDb;
        CDatabaseSql *pDb = getDb.GetDbConnect();
        if(pDb == NULL)
            return;
        pDb->Query("truncate table mls_match");
        sMLSUser.clear();
    }
    if(sMLSBegin && (hour == MLS_TIME+1))
    {
        sMLSBegin = false;
        SSortUserScore sortScore;
        std::sort(sMLSUser.begin(),sMLSUser.end(),sortScore);
        
        CGetDbConnect getDb;
        CDatabaseSql *pDb = getDb.GetDbConnect();
        if(pDb == NULL)
            return;
        pDb->Query("truncate table mls_match");
        
        char sql[1024];
        for(uint32 i = 0; i < sMLSUser.size(); i++)
        {
            sprintf(sql,"INSERT INTO mls_match (id,role_id,score) VALUES (%d,%u,%d)",
                i+1,sMLSUser[i].roleId,sMLSUser[i].score);
            pDb->Query(sql);
        }
        
        int zjNum = 0;
        uint32 num = 0;
        if(sMLSUser.size() > 3)
            num = 3;
        else
            return;
        for(; num < sMLSUser.size(); num++)
        {
            zjNum++;
            sprintf(sql,"update mls_match set zhongjiang=1 where role_id=%u",sMLSUser[Random(num,sMLSUser.size()-1)].roleId);
            pDb->Query(sql);
            if(zjNum >= 10)
                break;
        }
    }
}

//提交马拉松成绩
void MLSChengJi(CUser *pUser,int val)
{
    if(pUser == NULL)
        return;
    SUserScore s;
    s.roleId = pUser->GetRoleId();
    s.score = val;
    sMLSUser.push_back(s);
}

//查询马拉松排名，0没参加
int MLSGetPaiMing(CUser *pUser)
{
    if(pUser == NULL)
        return -1;
    CGetDbConnect getDb;
    CDatabaseSql *pDb = getDb.GetDbConnect();
    if(pDb == NULL)
        return -1;
    char sql[1024];
    sprintf(sql,"select id from mls_match where role_id=%u",pUser->GetRoleId());
    if(!pDb->Query(sql))
        return 0;
    char **row = pDb->GetRow();
    if(row == NULL)
        return 0;
    return atoi(row[0]);
}

//0为幸运玩家,-1为没有参加,-2领取过奖励,>0为没重奖
int MLSGetJiangLi(CUser *pUser)
{
    if(pUser == NULL)
        return -1;
    CGetDbConnect getDb;
    CDatabaseSql *pDb = getDb.GetDbConnect();
    if(pDb == NULL)
        return 1;
    char sql[1024];
    sprintf(sql,"select zhongjiang from mls_match where role_id=%u",pUser->GetRoleId());
    if(!pDb->Query(sql))
        return -1;
    char **row = pDb->GetRow();
    if(row == NULL)
        return -1;
    if(atoi(row[0]) == 2)
        return -2;
    if(atoi(row[0]) == 1)
        return 0;
    return 1;
}

void MLSLingJiang(CUser *pUser)
{
    if(pUser == NULL)
        return;
    CGetDbConnect getDb;
    CDatabaseSql *pDb = getDb.GetDbConnect();
    char sql[1024];
    sprintf(sql,"update mls_match set zhongjiang=2 where role_id=%u",pUser->GetRoleId());
    pDb->Query(sql);
}

static int sCurTongJiId = 10000;
const int MAX_TONGJI_NUM = 100;
static STongJiInfo sTongJiList[MAX_TONGJI_NUM];

bool GetTongJiInfo(int id,STongJiInfo *pInfo)
{
    uint8 i = 0;
    for(; i < MAX_TONGJI_NUM; i++)
    {
        if(sTongJiList[i].id == id)
        {
            *pInfo = sTongJiList[i];
            return true;
        }
    }
    return false;
}

void DelTongJi(int id,uint32 finishedUser = 0)
{
    uint8 i = 0;
    for(; i < MAX_TONGJI_NUM; i++)
    {
        if(sTongJiList[i].id == id)
        {
            char buf[256];
            if(finishedUser == 0)
                snprintf(buf,256,"%s通缉已过期，您的任务失败。",sTongJiList[i].name);
            else
                snprintf(buf,256,"%s通缉已有人完成，您的任务失败。",sTongJiList[i].name);
                
            for(list<uint32>::iterator iter = sTongJiList[i].userList.begin(); iter != sTongJiList[i].userList.end(); iter++)
            {
                DelMission(*iter,id);
                if(*iter != finishedUser)
                    SendSysMail(*iter,buf);
            }
            sTongJiList[i].id = 0;
            sTongJiList[i].roleId = 0;
            sTongJiList[i].owner = 0;
            sTongJiList[i].money = 0;
            break;
        }
    }
}

//显示发布通缉界面 
void IPostTongji(CUser *pUser)
{
    if(pUser == NULL)
        return;
    CSocketServer &sock = SingletonSocket::instance();
    CNetMessage msg;
    msg.SetType(PRO_INTERACT);
    
    msg<<(uint8)28;
    sock.SendMsg(pUser->GetSock(),msg);
}

void FinishPkMiss(CUser *pUser,int missId)
{
    for(uint8 i = 0; i < MAX_TONGJI_NUM; i++)
    {
        if(sTongJiList[i].id == missId)
        {
            //if(pUser->GetSex() == 0)
            pUser->SetData32(12,pUser->GetData32(12)+9);
            pUser->AddMoney(sTongJiList[i].money);
            SendPopMsg(pUser,"您已经成功击杀目标。");
            char buf[128];
            snprintf(buf,128,"您发布的%s通缉任务已有人成功完成，替你铲除目标。",sTongJiList[i].name);
            SendSysMail(sTongJiList[i].owner,buf);
            DelTongJi(sTongJiList[i].id,pUser->GetRoleId());
            break;
        }
    }
}
//显示通缉榜 
void ITongjiBang(CUser *pUser)
{
    if(pUser == NULL)
        return;
    CSocketServer &sock = SingletonSocket::instance();
    CNetMessage msg;
    msg.SetType(PRO_INTERACT);
    
    msg<<(uint8)26;
    uint16 pos = msg.GetDataLen();
    msg<<(uint8)0;
    uint8 num = 0;
    COnlineUser &onlineUser = SingletonOnlineUser::instance();
    for(uint8 i = 0; i < MAX_TONGJI_NUM; i++)
    {
        if(sTongJiList[i].id != 0)
        {
            if(sTongJiList[i].t < GetSysTime())
            {
                AddMoney(sTongJiList[i].owner,sTongJiList[i].money);
                SendSysMail(sTongJiList[i].owner,"您发布的通缉任务已经过期，悬赏金返还至你的背包中。");
                DelTongJi(sTongJiList[i].id);
                continue;
            }
            msg<<sTongJiList[i].id<<sTongJiList[i].name<<sTongJiList[i].level;
            if(onlineUser.GetUserByRoleId(sTongJiList[i].roleId).get() != NULL)
                msg<<(uint8)1;
            else
                msg<<(uint8)0;
            if(sTongJiList[i].userList.empty())
                sTongJiList[i].state = 0;
            else 
                sTongJiList[i].state = 1;
            msg<<sTongJiList[i].money<<(uint32)(sTongJiList[i].t-GetSysTime())<<sTongJiList[i].state;
            num++;
        }
    }
    msg.WriteData(pos,&num,1);
    sock.SendMsg(pUser->GetSock(),msg);
}

//显示我发布的任务 
void IMyTongji(CUser *pUser)
{
    if(pUser == NULL)
        return;
    CSocketServer &sock = SingletonSocket::instance();
    CNetMessage msg;
    msg.SetType(PRO_INTERACT);
    
    msg<<(uint8)27;
    uint16 pos = msg.GetDataLen();
    msg<<(uint8)0;
    uint8 num = 0;
    COnlineUser &onlineUser = SingletonOnlineUser::instance();
    for(uint8 i = 0; i < MAX_TONGJI_NUM; i++)
    {
        if((sTongJiList[i].id != 0) && (sTongJiList[i].t < GetSysTime()))
        {
            AddMoney(sTongJiList[i].owner,sTongJiList[i].money);
            SendSysMail(sTongJiList[i].owner,"您发布的通缉任务已经过期，悬赏金返还至你的背包中。");
            DelTongJi(sTongJiList[i].id);
            continue;
        }
        if(sTongJiList[i].owner == pUser->GetRoleId())
        {
            msg<<sTongJiList[i].id<<sTongJiList[i].name<<sTongJiList[i].level;
            if(onlineUser.GetUserByRoleId(sTongJiList[i].roleId).get() != NULL)
                msg<<(uint8)1;
            else
                msg<<(uint8)0;
            if(sTongJiList[i].userList.empty())
                sTongJiList[i].state = 0;
            else 
                sTongJiList[i].state = 1;
            msg<<sTongJiList[i].money<<sTongJiList[i].t-GetSysTime()<<sTongJiList[i].state;
            num++;
        }
    }
    msg.WriteData(pos,&num,1);
    sock.SendMsg(pUser->GetSock(),msg);
}

const int MAX_ACCEPT_TONGJI_NUM = 100;

//type=0 通缉令(1846) type=1 3万金币 
//4.发布的任务已经达到10个，再发布弹出提示：发布失败，您最多只能发布10个任务。 
//5.对同一目标多次发布。弹出提示：您已经发布过该任务。
int PostTongji(CUser *pUser,int id,const char *name,int type,int money)
{
    if((pUser == NULL) || (money < 10000))
        return 3;
    if(pUser->GetRoleId() == (uint32)id)
        return 1;
        
    uint8 i = MAX_TONGJI_NUM;
    uint8 num = 0;
    uint8 j = 0;
    for(; j < MAX_TONGJI_NUM; j++)
    {
        if((sTongJiList[j].id == 0) && (i == MAX_TONGJI_NUM))
        {
            i = j;
        }
        if(sTongJiList[j].owner == pUser->GetRoleId())
        {
            if(sTongJiList[j].roleId == (uint32)id)
                return 5;
            if(++num >= 10)
                return 4;
        }
    }
    if(i >= MAX_TONGJI_NUM)
        return 1;
    
    if((id == 0) && (name != NULL))
    {
        id = GetRoleId(name,sTongJiList[i].level);
        if(id == 0)
            return 1;
        strcpy(sTongJiList[i].name,name);
    }
    else if((sTongJiList[i].level = GetRoleName(id,sTongJiList[i].name)) == 0)
    {
        return 1;
    }
    if(type == 0)
    {
        if(!pUser->DelPackageById(1846,1))
            return 2;
        if(pUser->GetMoney() < money)
            return 3;
        pUser->AddMoney(-money);
    }
    else if(type == 1)
    {
        const int costMoney = 30000;
        if(pUser->GetMoney() < costMoney + money)
            return 3;
        pUser->AddMoney(-(costMoney+money));
    }
    else 
        return 2;
        
    sTongJiList[i].id = ++sCurTongJiId;
    sTongJiList[i].owner = pUser->GetRoleId();
    sTongJiList[i].roleId = id;
    sTongJiList[i].money = money;
    sTongJiList[i].t = GetSysTime()+24*3600*7;
    sTongJiList[i].state = 0;
    sTongJiList[i].userList.clear();
    return 0;
}

int ChangeTongji(CUser *pUser,int id,int money)
{
    if((pUser == NULL) || (money < 10000))
        return 1;
    
    uint8 i = 0;
    for(; i < MAX_TONGJI_NUM; i++)
    {
        if((sTongJiList[i].id == id) && (sTongJiList[i].owner == pUser->GetRoleId()))
        {
            if(sTongJiList[i].userList.empty())
                sTongJiList[i].state = 0;
            else 
                sTongJiList[i].state = 1;
            if(sTongJiList[i].state == 0)
            {
                sTongJiList[i].money = money;
                return 0;
            }
            return 1;
        }
    }
    return 1;
}

int CancelTongji(CUser *pUser,int id)
{
    if(pUser == NULL)
        return 1;
    
    uint8 i = 0;
    for(; i < MAX_TONGJI_NUM; i++)
    {
        if((sTongJiList[i].id == id) && (sTongJiList[i].owner == pUser->GetRoleId()))
        {
            if(sTongJiList[i].userList.empty())
                sTongJiList[i].state = 0;
            else 
                sTongJiList[i].state = 1;
            if(sTongJiList[i].state == 0)
            {
                sTongJiList[i].id = 0;
                sTongJiList[i].roleId = 0;
                sTongJiList[i].owner = 0;
                pUser->AddMoney(sTongJiList[i].money);
                return 0;
            }
            return 1;
        }
    }
    return 1;
}

//TakeTongji(pUser,id) 3 不能接取自己发布的任务 4 不能接取击杀自己的任务

int TakeTongji(CUser *pUser,int id)
{
    if(pUser == NULL)
        return 2;
    
    uint8 i = 0;
    for(; i < MAX_TONGJI_NUM; i++)
    {
        if(sTongJiList[i].id == id)
            break;
    }
    if(i >= MAX_TONGJI_NUM)
        return 2;
    if(sTongJiList[i].t < GetSysTime())
        return 2;
        
    if(sTongJiList[i].owner == pUser->GetRoleId())
        return 3;
    if(sTongJiList[i].roleId == pUser->GetRoleId())
        return 4;

    for(list<uint32>::iterator iter = sTongJiList[i].userList.begin(); iter != sTongJiList[i].userList.end(); iter++)
    {
        if(pUser->GetRoleId() == *iter)
            return 1;
    }
    
    char buf[64];
    //400 info="tid|目标id"
    snprintf(buf,64,"%d|%u",sTongJiList[i].id,sTongJiList[i].roleId);
    
    if(pUser->AddMission(sTongJiList[i].id,buf))
    {
        sTongJiList[i].userList.push_back(pUser->GetRoleId());
        sTongJiList[i].state = 1;
        return 0;
    }
    else
    {
        return 5;
    }
}

void ReadTongJi()
{
    CGetDbConnect getDb;
    CDatabaseSql *pDb = getDb.GetDbConnect();
    
    if(pDb == NULL)
    {
        return;
    }
    
    if(!pDb->Query("select id,owner,role_id,name,level,end_time,money,state,accept_user from tong_ji"))
        return;
    char **row;
    int i = 0;
    while((row = pDb->GetRow()) != NULL)
    {
        sTongJiList[i].id = atoi(row[0]);
        if(sCurTongJiId < sTongJiList[i].id)
            sCurTongJiId = sTongJiList[i].id;
            
        sTongJiList[i].owner = atoi(row[1]);
        sTongJiList[i].roleId = atoi(row[2]);
        strcpy(sTongJiList[i].name,row[3]);
        sTongJiList[i].level = atoi(row[4]);
        sTongJiList[i].t = atoi(row[5]);
        sTongJiList[i].money = atoi(row[6]);
        sTongJiList[i].state = atoi(row[7]);
        char *p[MAX_ACCEPT_TONGJI_NUM];
        uint8 num = SplitLine(p,MAX_ACCEPT_TONGJI_NUM,row[8]);
        for(int j = 0; j < num; j++)
        {
            sTongJiList[i].userList.push_back(atoi(p[j]));
        }
        i++;
        if(i >= MAX_TONGJI_NUM)
            break;
    }
}

void SaveTongJi()
{
    CGetDbConnect getDb;
    CDatabaseSql *pDb = getDb.GetDbConnect();
    
    if(pDb == NULL)
    {
        return;
    }
    
    char sql[2048];
    char user[1024];
    pDb->Query("truncate table tong_ji");
    for(int i = 0; i < MAX_TONGJI_NUM; i++)
    {
        if(sTongJiList[i].id != 0)
        {
            user[0] = 0;
            int len;
            for(list<uint32>::iterator iter = sTongJiList[i].userList.begin(); iter != sTongJiList[i].userList.end(); iter++)
            {
                len = strlen(user);
                if(len >= 1010)
                    break;
                if(user[0] != 0)
                    strcat(user,"|");
                snprintf(user+strlen(user),1024-strlen(user),"%u",*iter);
            }
            snprintf(sql,2048,"INSERT INTO tong_ji (id,owner,role_id,name,level,end_time,money,state,accept_user) "\
                "VALUES (%d,%u,%u,'%s',%d,%lu,%d,%d,'%s')",
                sTongJiList[i].id,
                sTongJiList[i].owner,
                sTongJiList[i].roleId,
                sTongJiList[i].name,
                sTongJiList[i].level,
                sTongJiList[i].t,
                sTongJiList[i].money,
                sTongJiList[i].state,
                user);
            pDb->Query(sql);
        }
    }
}

void DelTongJiMiss(CUser *pUser,int id)
{
    if(pUser == NULL)
        return;
    STongJiInfo info;
    uint8 i = 0;
    for(; i < MAX_TONGJI_NUM; i++)
    {
        if(sTongJiList[i].id == id)
        {
            sTongJiList[i].userList.remove(pUser->GetRoleId());
            break;
        }
    }
    pUser->DelMission(id);
}

//得到结盟帮,"id1|id2"，没有返回NULL
char *GetAllyBang(CUser *pUser)
{
    if(pUser == NULL)
        return NULL;
    uint32 bId = pUser->GetBangPai();
    if(bId == 0)
        return NULL;

    CBangPaiManager &bangPaiMgr = SingletonCBangPaiManager::instance();
    CBangPai *pBangPai = bangPaiMgr.FindBangPai(bId);
    if(pBangPai == NULL)
        return NULL;
    
    static char buf[128];
    list<uint32> jiemeng;
    pBangPai->GetJieMeng(jiemeng);
    if(jiemeng.size() > 0)
    {
        buf[0] = 0;
        for(list<uint32>::iterator i = jiemeng.begin(); i != jiemeng.end(); i++)
        {
            if(i != jiemeng.begin())
                strcat(buf,"|");
            sprintf(buf+strlen(buf),"%u",*i);
        }
        return buf;
    }
    return NULL;    
}

//帮派结盟
void AllyBang(CUser *pUser,int bId)
{
    if(pUser == NULL)
        return;
    uint32 userBId = pUser->GetBangPai();
    if(bId == 0)
        return;

    CBangPaiManager &bangPaiMgr = SingletonCBangPaiManager::instance();
    CBangPai *pBangPai = bangPaiMgr.FindBangPai(userBId);
    CBangPai *pBangPai1 = bangPaiMgr.FindBangPai(bId);
    if((pBangPai == NULL) || (pBangPai1 == NULL))
        return;
    if(pBangPai->JieMeng(pBangPai1))
        return;
        
    char buf[256];
    snprintf(buf,256,"%s帮派与%s帮派结盟，双方携手共创霸业。",pBangPai->GetName().c_str(),pBangPai1->GetName().c_str());
    SysInfoToAllUser(buf);
    pBangPai->AddJieMeng(bId);
    pBangPai1->AddJieMeng(userBId);
}


//解散结盟
void UnallyBang(CUser *pUser,int bId)
{
    if(pUser == NULL)
        return;
    uint32 userBId = pUser->GetBangPai();
    if(bId == 0)
        return;

    CBangPaiManager &bangPaiMgr = SingletonCBangPaiManager::instance();
    CBangPai *pBangPai = bangPaiMgr.FindBangPai(userBId);
    CBangPai *pBangPai1 = bangPaiMgr.FindBangPai(bId);
    if((pBangPai == NULL) || (pBangPai1 == NULL))
        return;
    
    pBangPai->DelJieMeng(bId);
    pBangPai1->DelJieMeng(userBId);
}

//是否能结盟
int CheckAlly(CUser *pUser,int bId)
{
    if(pUser == NULL)
        return 4;
    uint32 userBId = pUser->GetBangPai();
    if(bId == 0)
        return 4;

    CBangPaiManager &bangPaiMgr = SingletonCBangPaiManager::instance();
    CBangPai *pBangPai = bangPaiMgr.FindBangPai(userBId);
    CBangPai *pBangPai1 = bangPaiMgr.FindBangPai(bId);
    if((pBangPai == NULL) || (pBangPai1 == NULL))
        return 4;
    if((pBangPai->GetXuanZhanBang() == (int)pBangPai1->GetId())
        || (pBangPai1->GetXuanZhanBang() == (int)pBangPai->GetId()))
        return 5;
        
    list<uint32> jiemeng;
    pBangPai->GetJieMeng(jiemeng);
    if(jiemeng.size() >= CBangPai::JIE_MENG_MAX_NUM)
        return 1;
    if(GetSysTime() - pBangPai->GetDelJMTime() < 7*24*3600)
        return 2;
    if(GetSysTime() - pBangPai1->GetDelJMTime() < 7*24*3600)
        return 3;
    return 0;
}
//200经验 任一晶石 50道行  任一晶石 300潜能 任一晶石
                     //type,id,jin,mu,shui,huo,tu
const int DUI_HUANG[] = {
1,200,0,0,0,0,0,
1,1000,2,1,0,0,0,
1,2000,0,0,2,1,1,
1,4500,2,0,2,0,2,
1,7000,1,3,1,1,2,
2,50,0,0,0,0,0,
2,200,0,0,2,0,1,
2,400,1,2,0,1,0,
2,900,1,1,1,3,0,
2,1400,1,2,1,3,1,
3,300,0,0,0,0,0,
3,1500,0,2,0,1,0,
3,3000,1,0,1,0,2,
3,7000,0,0,2,2,2,
3,10000,3,1,1,1,2,
4,1801,2,2,2,2,2,
4,1802,2,3,4,2,4,
4,610,5,0,5,0,5,
4,451,5,5,5,0,0,
4,501,0,9,2,9,0,
4,401,0,0,5,5,5,
4,612,5,0,5,1,1,
4,622,2,7,5,1,0,
4,630,3,1,1,5,5,
4,616,7,2,2,1,0,
4,631,0,7,3,3,7,
4,1804,2,2,0,8,0,
4,1805,2,2,8,0,0,
4,1810,0,2,0,2,8,
4,1811,5,0,0,0,7,
4,1815,3,4,0,3,2,
4,1817,5,2,0,5,0};

// 显示兑换表
void IWWDuihuan(CUser *pUser)
{
    if(pUser == NULL)
        return;
    
    uint8 num = sizeof(DUI_HUANG)/sizeof(int)/7;
    CSocketServer &sock = SingletonSocket::instance();
    CNetMessage msg;
    msg.SetType(PRO_INTERACT);
    msg<<(uint8)29<<num;
    
    for(uint8 i = 0; i < num; i++)
    {
        msg<<(uint8)DUI_HUANG[7*i]<<DUI_HUANG[7*i+1]
            <<(uint8)DUI_HUANG[7*i+2]
            <<(uint8)DUI_HUANG[7*i+3]
            <<(uint8)DUI_HUANG[7*i+4]
            <<(uint8)DUI_HUANG[7*i+5]
            <<(uint8)DUI_HUANG[7*i+6];
    }
    msg<<pUser->GetData8(10)
            <<pUser->GetData8(11)
            <<pUser->GetData8(12)
            <<pUser->GetData8(13)
            <<pUser->GetData8(14);
    sock.SendMsg(pUser->GetSock(),msg);
}

static int sTolKuang = 0;
static int sLostKuang = 0;
struct SBangKuang
{
    int bId;
    int kuang;
};
const int WW_MAX_BANG_NUM   = 100;
static SBangKuang sBangKuangs[WW_MAX_BANG_NUM];
static int sWinBang = 0;
static int sTolJiFen = 0;
static int sXieZhuBang = 0;

//计算世界大战胜利者
static bool sJiSuanWWWin = false;

int InviteAlly(CUser *pUser,int bId)
{
    if(sXieZhuBang == bId)
        return 3;
    if(pUser == NULL)
        return 1;
    uint32 userBId = pUser->GetBangPai();
    if(bId == 0)
        return 1;

    CBangPaiManager &bangPaiMgr = SingletonCBangPaiManager::instance();
    CBangPai *pBangPai = bangPaiMgr.FindBangPai(userBId);
    CBangPai *pBangPai1 = bangPaiMgr.FindBangPai(bId);
    if((pBangPai == NULL) || (pBangPai1 == NULL))
        return 1;
    
    if(!pBangPai->JieMeng(pBangPai1))
        return 1;
    
    ShareUserPtr ptr = SingletonOnlineUser::instance().GetUserByRoleId(pBangPai1->GetBangZhu());
    CUser *pU = ptr.get();
    if(pU == NULL)
        return 2;
    
    CSocketServer &sock = SingletonSocket::instance();
    CNetMessage msg;
    msg.SetType(MSG_SERVER_INVITE_ALLY);    
    msg<<userBId<<pBangPai->GetName();
    sock.SendMsg(pU->GetSock(),msg);
    return 0;
}

void AddBangKuang(int bId)
{
    if(bId == 0)
        return;
    for(int i = 0; i < WW_MAX_BANG_NUM; i++)
    {
        if(sBangKuangs[i].bId == bId)
        {
            sBangKuangs[i].kuang++;
            return;
        }
    }
    for(int i = 0; i < WW_MAX_BANG_NUM; i++)
    {
        if(sBangKuangs[i].bId == 0)
        {
            sBangKuangs[i].bId = bId;
            sBangKuangs[i].kuang = 1;
            return;
        }
    }
}

//提交晶石 
void WWTijiaoKuang(CUser *pUser,int id)
{
    if((id < 0) || (id > 4))
        return;
    if(pUser == NULL)
        return;
    if(!InWorldWar())
    {
        pUser->DelPackageById(1558+id,1);
        return;
    }
    uint8 kuang = pUser->GetData8(10+id);
    if(kuang >= 0xff)
        return;
    pUser->SetData8(10+id,kuang+1);
    sLostKuang++;
    AddBangKuang(pUser->GetBangPai());
    pUser->DelPackageById(1558+id,1);

    if(sLostKuang == (int)(sTolKuang*0.1))
    {
        SysInfoToAllUser("仙境之城已经丢失10%的资源，被兑换成各种 奖励。");
        SysInfoToAllUser("仙境之城已经丢失10%的资源，被兑换成各种 奖励。");
    }
    else if(sLostKuang == (int)(sTolKuang*0.3))
    {
        SysInfoToAllUser("仙境之城已经丢失30%的资源，被兑换成各种 奖励。");
        SysInfoToAllUser("仙境之城已经丢失30%的资源，被兑换成各种 奖励。");
    }
    else if(sLostKuang == (int)(sTolKuang*0.4))
    {
        SysInfoToAllUser("仙境之城已经丢失40%的资源，被兑换成各种 奖励。");
        SysInfoToAllUser("仙境之城已经丢失40%的资源，被兑换成各种 奖励。");
    }
    if(sLostKuang >= sTolKuang)
    {
        sJiSuanWWWin = true;
        int bId = 0;
        int kuang = 0;
        for(int i = 0; i < WW_MAX_BANG_NUM; i++)
        {
            if(sBangKuangs[i].bId != 0)
            {
                if(kuang < sBangKuangs[i].kuang)
                {
                    kuang = sBangKuangs[i].kuang;
                    bId = sBangKuangs[i].bId;
                }
            }
        }
        CBangPaiManager &bPMgr = SingletonCBangPaiManager::instance();
        CBangPai *pBangPai = bPMgr.FindBangPai(sWinBang);
        if(pBangPai != NULL)
        {
            ShareUserPtr ptr = SingletonOnlineUser::instance().GetUserByRoleId(pBangPai->GetBangZhu());
            if(ptr.get() != NULL)
                ptr->SendUpdateInfo(44,0);
        }
        sWinBang = bId;
        char buf[128];
        pBangPai = bPMgr.FindBangPai(sWinBang);
        if(pBangPai == NULL)
        {
            SysInfoToAllUser("本次大战攻方获胜，所有攻方得到额外奖励。");
            SysInfoToAllUser("本次大战攻方获胜，所有攻方得到额外奖励。");
            SysInfoToAllUser("本次大战攻方获胜，所有攻方得到额外奖励。");
            return;
        }
        ShareUserPtr ptr = SingletonOnlineUser::instance().GetUserByRoleId(pBangPai->GetBangZhu());
        if(ptr.get() != NULL)
            ptr->SendUpdateInfo(44,2);
        sXieZhuBang = 0;
        sprintf(buf,"本次大战攻方获胜，海外仙境被%s帮派占领,所有攻方得到额外奖励。",pBangPai->GetName().c_str());
        SysInfoToAllUser(buf);
        SysInfoToAllUser(buf);
        SysInfoToAllUser(buf);
    }
}

//得到晶石数量 
int WWGetKuang(CUser *pUser,int id)
{
    if((id < 0) || (id > 4))
        return 0;
    if(pUser == NULL)
        return 0;
    return pUser->GetData8(10+id);
}

//设置晶石数量
void WWSetKuang(CUser *pUser,int id,int num)
{
    if((id < 0) || (id > 4))
        return;
    if(pUser == NULL)
        return;
    pUser->SetData8(10+id,num);
}

void FightCoupleTrial(CUser *pUser)
{
    if((pUser != NULL) && (pUser->GetScene() != NULL))
    {
        pUser->GetScene()->FightCoupleTrial(pUser);
    }
}

//得到当前仙境大战胜利帮派 
int WWGetWinBang()
{
    return sWinBang;
}

//得到总晶石 
int WWGetTotalKuang()
{
    return sTolKuang;
}

//得到抢走的矿 
int WWGetLostKuang()
{
    return sLostKuang;
}

void ReadWWInfo()
{
    CGetDbConnect getDb;
    CDatabaseSql *pDb = getDb.GetDbConnect();
    if(pDb == NULL)
        return;
    if(!pDb->Query("select win_bang,tol_kuang,lost_kuang,xie_zhu from ww_info"))
        return;
    char **row = pDb->GetRow();
    if(row == NULL)
        return;
    sWinBang = atoi(row[0]);
    sTolKuang = atoi(row[1]);
    sLostKuang = atoi(row[2]);
    sXieZhuBang = atoi(row[3]);
}

void SaveWWInfo()
{
    CGetDbConnect getDb;
    CDatabaseSql *pDb = getDb.GetDbConnect();
    if(pDb == NULL)
        return;
    pDb->Query("truncate table ww_info");
    char sql[256];
    snprintf(sql,256,"INSERT INTO ww_info (win_bang,xie_zhu,tol_kuang,lost_kuang ) VALUES (%d,%d,%d,%d)",
        sWinBang,sXieZhuBang,sTolKuang,sLostKuang);
    pDb->Query(sql);
}

extern int GetOnlineUserNum();

void WWTimer()
{
    /*
    活动开始前10分钟，滚动一次：仙境之战即将开始，请双方做好准备。
    活动开始后10分钟内，滚动三次：仙境之战正式开始。
    活动结束或胜败以分，5分钟内滚动三次：
    攻方胜利：本次大战攻方获胜，仙境之战被XXX（攻方挖矿最多的帮派）帮派占领。
    守方胜利：本次大战，XXX帮派团结一心，成功防守，获得胜利。
    */
    time_t t = GetSysTime();
    struct tm *pTm = localtime(&t);
    if(pTm == NULL)
        return;
    if(pTm->tm_wday != WW_DAY)
        return;
        
    int hour = pTm->tm_hour;
    int min = pTm->tm_min;
    int sec = pTm->tm_sec;
    
    if(hour < WW_BEGIN_TIME)
    {
        if((hour == WW_BEGIN_TIME - 1) && (min >= 30) && (min % 5 == 0) && (sec <= 0))
        {
            SysInfoToAllUser("仙境之战即将开始，请双方做好准备。");
        }
        for(int i = 0; i < WW_MAX_BANG_NUM; i++)
        {
            if(sBangKuangs[i].bId != 0)
            {
                sBangKuangs[i].kuang = 0;
                sBangKuangs[i].bId = 0;
            }
        }
        sTolKuang = 9*GetOnlineUserNum();
        sLostKuang = 0;
        sTolJiFen = 0;
        sJiSuanWWWin = false;
    }
    else if(hour == WW_BEGIN_TIME)
    {
        if((min <= 10) && (min % 3 == 0) && (sec <= 0))
        {
            SysInfoToAllUser("仙境之战的战火再次燃起，2级炼化石，圣灵之翼各种珍宝爆发群仙争夺！");
        }
    }
    else if((hour >= WW_END_TIME) && !sJiSuanWWWin)
    {
        sJiSuanWWWin = true;
        if(sLostKuang > sTolKuang/2)
        {
            int bId = 0;
            int kuang = 0;
            for(int i = 0; i < WW_MAX_BANG_NUM; i++)
            {
                if(sBangKuangs[i].bId != 0)
                {
                    if(kuang < sBangKuangs[i].kuang)
                    {
                        kuang = sBangKuangs[i].kuang;
                        bId = sBangKuangs[i].bId;
                    }
                }
            }
            CBangPaiManager &bPMgr = SingletonCBangPaiManager::instance();
            CBangPai *pBangPai = bPMgr.FindBangPai(sWinBang);
            if(pBangPai != NULL)
            {
                ShareUserPtr ptr = SingletonOnlineUser::instance().GetUserByRoleId(pBangPai->GetBangZhu());
                if(ptr.get() != NULL)
                    ptr->SendUpdateInfo(44,0);
            }
            sWinBang = bId;
            char buf[128];
            pBangPai = bPMgr.FindBangPai(sWinBang);
            if(pBangPai == NULL)
            {
                SysInfoToAllUser("本次大战攻方获胜，所有攻方得到额外奖励。");
                SysInfoToAllUser("本次大战攻方获胜，所有攻方得到额外奖励。");
                SysInfoToAllUser("本次大战攻方获胜，所有攻方得到额外奖励。");
                return;
            }
            ShareUserPtr ptr = SingletonOnlineUser::instance().GetUserByRoleId(pBangPai->GetBangZhu());
            if(ptr.get() != NULL)
                ptr->SendUpdateInfo(44,2);
            sXieZhuBang = 0;
            sprintf(buf,"本次大战攻方获胜，海外仙境被%s帮派占领,所有攻方得到额外奖励。",pBangPai->GetName().c_str());
            SysInfoToAllUser(buf);
            SysInfoToAllUser(buf);
            SysInfoToAllUser(buf);
        }
        else
        {
            CBangPaiManager &bPMgr = SingletonCBangPaiManager::instance();
            char buf[128];
            CBangPai *pBangPai = bPMgr.FindBangPai(sWinBang);
            if(pBangPai == NULL)
                return;
            sprintf(buf,"本次大战，%s帮派团结一心，成功防守，获得胜利。",pBangPai->GetName().c_str());
            sXieZhuBang = 0;
            SysInfoToAllUser(buf);
            SysInfoToAllUser(buf);
            SysInfoToAllUser(buf);
        }
    }
}

bool InWorldWar()
{
    time_t t = GetSysTime();
    struct tm *pTm = localtime(&t);
    if(pTm == NULL)
        return false;
    if(pTm->tm_wday != WW_DAY)
        return false;
    int hour = GetHour();
    if((hour < WW_BEGIN_TIME) || (hour >= WW_END_TIME) || sJiSuanWWWin)
    {
        return false;
    }
    return true;
}

bool CanWaKuang(CUser *pUser)
{
    if(pUser == NULL)
        return false;
    if(pUser->GetBangPai() == 0)
        return true;
    return !IsFangShou(pUser->GetBangPai());
}

//得到积分 
int WWGetJifen(CUser *pUser)
{
    if(pUser == NULL)
        return 0;
    
    return pUser->GetData16(3);
}

void AddTolJiFen(int add)
{
    sTolJiFen += add;
}

//得到帮派总积分 
int WWGetTotalJifen()
{
    return sTolJiFen;
}

//积分兑换通宝 
//总通宝数(剩余矿石×2)/总积分数*个人积分数=个人通宝数
int WWDuihuanJifen(CUser *pUser)
{
    if(sTolJiFen == 0)
        return 0;
        
    if(pUser == NULL)
        return 0;
    if(pUser->GetData16(3) == 0)
        return 0;

    /*if((int)pUser->GetBangPai() != sWinBang)
    {
        tongBao = 2*pUser->GetData16(3)*(sTolKuang-sLostKuang)/sTolJiFen;
        
        CBangPaiManager &bangPaiMgr = SingletonCBangPaiManager::instance();
        CBangPai *pBangPai = bangPaiMgr.FindBangPai(sWinBang);
        if((pBangPai != NULL) && (pBangPai->GetBangZhu() == pUser->GetRoleId()))
        {
            tongBao = 2*(sTolKuang-sLostKuang);
        }
        if(tongBao <= 0)
            tongBao = 1;
        pUser->AddTongBao(tongBao);
    }*/
    //pUser->SetData16(3,0);
    //1%的晶石资源=200通宝。按照战斗结束，剩余资源的百分比，守方获得相应的总通宝数，最高为10000通宝（50%晶石），超过的部分不计。
    int tolTongBao = 20000*(sTolKuang-sLostKuang)/sTolKuang;
    if(tolTongBao > 10000)
        tolTongBao = 10000;
    tolTongBao /= 2;
    int tongBao = 0;
    if((int)pUser->GetBangPai() == sWinBang)
    {
        CBangPaiManager &bangPaiMgr = SingletonCBangPaiManager::instance();
        CBangPai *pBangPai = bangPaiMgr.FindBangPai(sWinBang);
        if((pBangPai != NULL) && (pBangPai->GetBangZhu() == pUser->GetRoleId()))
        {
            tongBao = tolTongBao;
        }
    }
    if(tongBao == 0)
    {
        tongBao = tolTongBao*pUser->GetData16(3)/sTolJiFen;
    }
    //战斗积分超过100，额外奖励200通宝
    //战斗积分超过300，额外奖励500通宝
    if(pUser->GetData16(3) >= 300)
        tongBao += 500;
    else if(pUser->GetData16(3) >= 100)
        tongBao += 200;
        
    pUser->AddTongBao(tongBao);
    return tongBao;
}

//使用物品 
int UseItemToNpc(CUser *pUser,int id,int item)
{
    if(pUser == NULL)
        return 1;
    pUser->DelPackageById(item,1);
    if(item == 643)
        UseShiTou(id);
    else if(item == 644)
        UseYuGan(id);
    return 0;
}

//下注 
int Xiazhu(CUser *pUser,int first,int second,int num)
{
    if(pUser == NULL)
        return 1;
    if(InSaiPao())
        return 1;
    int res = UserTouZhu(pUser->GetRoleId(),first,second,num);
    if(res == 0)
    {
        pUser->DelPackageById(645,num);    
        return res;
    }
    return res;
}

int AgreeWWFangShou(CUser *pUser,uint8 agree,int bId)
{
    if(sXieZhuBang == bId)
        return 4;
    if(InWorldWar())
        return 3;
    if(pUser == NULL)
        return 1;
    uint32 userBId = pUser->GetBangPai();
    if(bId == 0)
        return 1;
    if(sWinBang != bId)
        return 1;
        
    CBangPaiManager &bangPaiMgr = SingletonCBangPaiManager::instance();
    CBangPai *pBangPai = bangPaiMgr.FindBangPai(userBId);
    CBangPai *pBangPai1 = bangPaiMgr.FindBangPai(bId);
    if((pBangPai == NULL) || (pBangPai1 == NULL))
        return 1;
        
    if(!pBangPai->JieMeng(pBangPai1))
        return 1;
    
    ShareUserPtr ptr = SingletonOnlineUser::instance().GetUserByRoleId(pBangPai1->GetBangZhu());
    CUser *pU = ptr.get();
    if(pU == NULL)
        return 2;
    
    //取消（对方弹出提示：对方帮主拒绝协助） 
    //同意（双方弹出提示：XXXX将协助XX帮进行本周海外仙境之战） 
    if(agree == 1)
    {
        char buf[256];
        snprintf(buf,256,"%s将协助%s帮进行本周海外仙境之战",pBangPai->GetName().c_str(),pBangPai1->GetName().c_str());
        SendPopMsg(pUser,buf);
        SendPopMsg(pU,buf);
        snprintf(buf,256,"%s帮派邀请%s帮派共同防守本次仙境之战。",pBangPai1->GetName().c_str(),pBangPai->GetName().c_str());
        SysInfoToAllUser(buf);
        SysInfoToAllUser(buf);
        SysInfoToAllUser(buf);
        sXieZhuBang = userBId;
    }
    else
    {
        SendPopMsg(pU,"对方帮主拒绝协助");
    }
    return 0;
}

bool IsFangShou(int bId)
{
    if(bId == 0)
        return false;
    if((bId == sWinBang) || (bId == sXieZhuBang))
        return true;
    return false;
}

bool FuBenKaiJia(CUser *pUser,uint8 pos)
{
    if(pUser == NULL)
        return false;
    SharePetPtr pet = pUser->GetPet(pos);
    SPet *pPet = pet.get();
    if(pPet == NULL)
        return false;
    if(pPet->kaiJia.tmplId != 0)
        return false;
    SItemInstance &item = pPet->kaiJia;
    item.name[0] = SItemInstance::CAN_NOT_XIE_XIA;
    
    item.tmplId = 807;
    CItemTemplateManager &itemMgr = SingletonItemManager::instance();
    SItemTemplate *pItem = itemMgr.GetItem(item.tmplId);
    if(pItem != NULL)
        item.naijiu = pItem->naijiu;
        
    item.addAttrType[0] = EAAqixue;
    item.addAttrVal[0]  = 3000;
    
    item.addAttrType[1] = EAAfangyu;
    item.addAttrVal[1]  = 3000;
    
    item.addAttrType[2] = EAAshanghai;
    item.addAttrVal[2]  = 3000  ;
    
    item.addAttrType[3] = EAAquanjineng ;
    item.addAttrVal[3]  = 100    ;
    
    item.addAttrType[4] = EAAquanshuxing ;
    item.addAttrVal[4]  = 100     ;
    
    item.addAttrType[5] = EAAbishalv;
    item.addAttrVal[5]  = 100  ;
    
    item.addAttrType[6] = EAAshui  ;
    item.addAttrVal[6]  = 100 ;
    
    item.addAttrType[7] = EAAhuo  ;
    item.addAttrVal[7]  = 100;
    
    item.addAttrType[8] = EAAmu    ;
    item.addAttrVal[8]  = 100 ;
    pUser->UpdatePet(pos);
    return true;
}

void TeXiao(int type,int scene)
{
    CNetMessage msg;
    msg.SetType(MSG_SERVER_VISUAL_EFFECT);
    msg<<(uint8)type;
    if(scene == 0)
    {
        SendMsgToAllUser(msg);
    }
    else
    {
        SendSceneMsg(msg,scene);
    }
}

static void SendMsg(CSocketServer *pSock,CNetMessage *pMsg,CUser *pUser)
{
    pSock->SendMsg(pUser->GetSock(),*pMsg);
}

void SendSysChannelMsg(const char *info)
{
    CSocketServer &sock = SingletonSocket::instance();
    CNetMessage msg;
    msg.SetType(PRO_MSG_CHAT);
    msg<<(uint8)0<<0<<"【系统】"<<info;
    SingletonOnlineUser::instance().ForEachUser(boost::bind(SendMsg,&sock,&msg,_1));
}

//拦路虎战斗
void SaiPaoFight(CUser *pUser)
{
    if((pUser != NULL) && (pUser->GetScene() != NULL))
    {
        pUser->GetScene()->LanLuoHu(pUser,6);
    }
}

//0正确，1领取，2无
int FindJiHuoMa(char *str)
{
    CGetDbConnect getDb;
    CDatabaseSql *pDb = getDb.GetDbConnect();
    char sql[512];
    sprintf(sql,"SELECT state FROM jihuoma where str='%s'",str);
    if ((pDb == NULL) || (!pDb->Query(sql)))
    {
        return 2;
    }
    char **row = pDb->GetRow();
    if(row == NULL)
        return 2;
    if(atoi(row[0]) != 0)
        return 1;
    sprintf(sql,"update jihuoma set state=1 where str='%s'",str);
    if(pDb->Query(sql))
    {
        return 0;
    }
    return 2;
}

void OpenXinShi(CUser *pUser)
{
    if(pUser == NULL)
        return;
    CSocketServer &sock = SingletonSocket::instance();
    CNetMessage msg;
    msg.SetType(MSG_SERVER_XINSHI);
    
    msg<<(uint8)0;
    sock.SendMsg(pUser->GetSock(),msg);
}

/*
+----+-----+-----+-----+-----+------+----+------+----+------+-------+-----+-----+
| OP | NUM | MID | CID | LEN | NAME | ID | IDEF | ID | IDEF | MONEY | LEN | MSG | 
+----+-----+-----+-----+-----+------+----+------+----+------+-------+-----+-----+
|  1 |  1  |  4  |  4  |  2  |  Var |  2 |  Var |  2 |  Var |   4   |  2  | Var |
+----+-----+-----+-----+-----+------+----+------+----+------+-------+-----+-----+
OP=2 列出收到的信*/
void ListXinShi(CUser *pUser)
{
    if(pUser == NULL)
        return;
    CSocketServer &sock = SingletonSocket::instance();
    
    CGetDbConnect getDb;
    CDatabaseSql *pDb = getDb.GetDbConnect();
    if(pDb == NULL)
        return;
        
    char sql[512];
    
    sprintf(sql,"delete from xin_shi where UNIX_TIMESTAMP(time) < %lu",GetSysTime() - 7*24*3600);
    pDb->Query(sql);
    
    sprintf(sql,"SELECT id,from_id,from_name,item,money,msg,UNIX_TIMESTAMP(time) FROM xin_shi where to_id=%u",pUser->GetRoleId());
    
    if(!pDb->Query(sql))
        return;
    
    CNetMessage msg;
    msg.SetType(MSG_SERVER_XINSHI);
    msg<<(uint8)2<<(uint8)pDb->GetRowNum();
    char **row;
    while((row = pDb->GetRow()) != NULL)
    {
        msg<<atoi(row[0])<<atoi(row[1])<<row[2];
        if((row[3] == NULL) || (row[3][0] == 0))
        {
            msg<<(uint8)0<<(uint16)0<<(uint8)0<<(uint16)0;
        }
        else
        {
            SItemInstance item[2] = {{0}};
            uint32 len = sizeof(item);
            UnCompress(row[3],(uint8*)&item,len);
            msg<<item[0].num;
            MakeItemInfo(item,msg);
            msg<<item[1].num;
            MakeItemInfo(&(item[1]),msg);
        }
        msg<<atoi(row[4])<<row[5]<<(uint32)(7*24*3600-(GetSysTime()-atoi(row[6])));
    }
    sock.SendMsg(pUser->GetSock(),msg);
}

int GetLeiTaiJiFen(CUser *pUser,int mapId)
{
    if(pUser == NULL)
        return 0;
    
    CSceneManager &scene = SingletonSceneManager::instance();
    
    CScene *pScene = scene.FindScene(mapId);
    if(pScene == NULL)
        return 0;
    return pScene->GetUserJiFen(pUser->GetRoleId());
}

void TiJiaoYiJian(CUser *pUser,char *info)
{
    if(info == NULL)
        return;
    if(pUser == NULL)
        return;
    CGetDbConnect getDb;
    CDatabaseSql *pDb = getDb.GetDbConnect();
    char sql[512];
    snprintf(sql,sizeof(sql),"INSERT INTO yi_jian (role_id,info) VALUES (%u,'%s')",pUser->GetRoleId(),info);
    if (pDb != NULL)
    {
        pDb->Query(sql);
    }
}

/*消息类型13,type 30
内容(byte)宠物数量 (byte)位置(short)宠物id （char*)宠物名字 （tyte）宠物类型 （tyep）向性
（type）等级（int）经验（int）当前血量（int）最大血量（int）当前蓝（int）最大蓝
（int）物理伤害（int）技能伤害(short)速度（int）防御
（int）武学（int）修为（short）寿命（byte）忠诚（int）亲密
（short）体质（short）力量（short）敏捷（short）灵性（short）耐力（short）属性点
（short）气血成长（short）蓝成长（short）速度成长（short）物攻成长（short）法攻成长
(type)技能数量(short)技能id（byte）技能等级
*/
/*
得到宠物信息列表后
取和存物品发送ncp交互信息，内容为：（int）type（int）pos，type为0表示存宠物，pos为身上宠物的位置；type为1表示取宠物，pos为宠物在仓库中的位置
*/
void SendBankPet(CUser *pUser)
{
    if(pUser == NULL)
        return;
    CSocketServer &sock = SingletonSocket::instance();
    CNetMessage msg;
    msg.SetType(PRO_INTERACT);
    
    msg<<(uint8)30;
    pUser->MakeBankPet(msg);
    sock.SendMsg(pUser->GetSock(),msg);    
}

//pos宠物位置
bool SaveBankPet(CUser *pUser,uint8 pos)
{
    if(pUser == NULL)
        return false;
    return pUser->SaveBankPet(pos);
}

//pos在宠物商店中的位置
bool GetBankPet(CUser *pUser,uint8 pos)
{
    if(pUser == NULL)
        return false;
    return pUser->GetBankPet(pos);
}

void SendMissionInfo(CUser *pUser,uint16 missionId,const char *info)
{
    if(info == NULL)
        return;
    CNetMessage msg;
    msg.SetType(PRO_TASK_INFO_EXT);
    msg<<missionId;
    CSocketServer &sock = SingletonSocket::instance();
    msg<<info;
    sock.SendMsg(pUser->GetSock(),msg);
}

//命令13 类型31师徒榜
//内容：(char*)榜单名字，(char*)信息名，(int)ext(扩展使用） num玩家数量（最大100）（char*)name (int)level
//师徒榜玩家信息
//客户端发送内容(byte)0师傅榜，1徒弟榜
//服务器返回(char*)info,文字中|做为换行符
//type 0 师傅榜，1徒弟榜
/*void MasterPrenticeRank(CUser *pUser,int type,const char *pRandName,const char *pInfo)
{
    if(pUser == NULL)
        return;
    CSocketServer &sock = SingletonSocket::instance();
    CNetMessage msg;
    msg.SetType(PRO_INTERACT);
    
    msg<<(uint8)31;
    
    CGetDbConnect getDb;
    CDatabaseSql *pDb = getDb.GetDbConnect();
                       //0      1       2      
    char sql[256];
    //id role_Id type time
    snprintf(sql,256,"select role_id,level,role_name from level_rank where type=%d "\
        "order by rank",type);
    uint8 num = 0;
    int function = 0;
    msg<<tag<<function;
    uint16 pos = msg.GetDataLen();
    msg<<num;
    if ((pDb != NULL)
        && (pDb->Query(sql)))
    {
        char **row;
        while ((row = pDb->GetRow()) != NULL)
        {
            num++;
            msg<<(int)atoi(row[0])<<row[2]<<atoi(row[1]);
        }
    }
    msg.WriteData(pos,&num,1);
    sock.SendMsg(pUser->GetSock(),msg);
}*/

int CanJieBai(CUser *pUser)
{
    if((pUser == NULL) || (pUser->GetLevel() < 60))
        return 13;
    if(GetSysTime() - pUser->GetData32(13) < 3600*24)
        return 14;
        
    uint8 teamMemNum = GetTeamMemNum(pUser);
    
    CGetDbConnect getDb;
    CDatabaseSql *pDb = getDb.GetDbConnect();
    if(pDb == NULL)
        return 5;
    char sql[512];
    
    CUser *p1 = GetTeamMember1(pUser);
    CUser *p2 = GetTeamMember2(pUser);
    if(teamMemNum == 2)
    {
        if(p1 == NULL)
            return 3;
        snprintf(sql,sizeof(sql),"select m_id from master_prentice where (m_id=%u and p_id=%u) "\
            "or (p_id=%u and m_id=%u)",pUser->GetRoleId(),p1->GetRoleId(),pUser->GetRoleId(),p1->GetRoleId());
        if((pDb->Query(sql)) && (pDb->GetRow() != NULL))
            return 7;
        if(pUser->GetData32(6) == p1->GetRoleId())
            return 8;
        uint16 hotVal = 0;
        pUser->GetHotVal(p1->GetRoleId(),hotVal);
        if(hotVal < 1000)
            return 9;
    }
    else if(teamMemNum == 3)
    {
        if((p1 == NULL) || (p2 == NULL))
            return 3;
        snprintf(sql,sizeof(sql),"select m_id from master_prentice where (m_id=%u and p_id=%u) "\
            "or (p_id=%u and m_id=%u)",pUser->GetRoleId(),p1->GetRoleId(),pUser->GetRoleId(),p1->GetRoleId());
        if((pDb->Query(sql)) && (pDb->GetRow() != NULL))
            return 7;
        if(pUser->GetData32(6) == p1->GetRoleId())
            return 8;
        uint16 hotVal = 0;
        pUser->GetHotVal(p1->GetRoleId(),hotVal);
        if(hotVal < 1000)
            return 9;
        p1->GetHotVal(pUser->GetRoleId(),hotVal);
        if(hotVal < 1000)
            return 9;
        snprintf(sql,sizeof(sql),"select m_id from master_prentice where (m_id=%u and p_id=%u) "\
            "or (p_id=%u and m_id=%u)",pUser->GetRoleId(),p2->GetRoleId(),pUser->GetRoleId(),p2->GetRoleId());
        if((pDb->Query(sql)) && (pDb->GetRow() != NULL))
            return 7;
        if(pUser->GetData32(6) == p2->GetRoleId())
            return 8;
        hotVal = 0;
        pUser->GetHotVal(p2->GetRoleId(),hotVal);
        if(hotVal < 1000)
            return 9;
        p2->GetHotVal(pUser->GetRoleId(),hotVal);
        if(hotVal < 1000)
            return 9;
        snprintf(sql,sizeof(sql),"select m_id from master_prentice where (m_id=%u and p_id=%u) "\
            "or (p_id=%u and m_id=%u)",p1->GetRoleId(),p2->GetRoleId(),p2->GetRoleId(),p1->GetRoleId());
        if((pDb->Query(sql)) && (pDb->GetRow() != NULL))
            return 7;
        if(p1->GetData32(6) == p2->GetRoleId())
            return 8;
        hotVal = 0;
        p1->GetHotVal(p2->GetRoleId(),hotVal);
        if(hotVal < 1000)
            return 9;
        p2->GetHotVal(p1->GetRoleId(),hotVal);
        if(hotVal < 1000)
            return 9;
    }
    else
    {
        return 3;
    }
    
    snprintf(sql,sizeof(sql),"select id from jie_bai where role_id=%u",pUser->GetRoleId());
    if(!pDb->Query(sql))
        return 5;
    
    char **row = pDb->GetRow();
    if(row != NULL)
        return 5;
    
    
    if(p1 != NULL)
    {
        if(GetSysTime() - p1->GetData32(13) < 3600*24)
            return 14;
        if(p1->GetLevel() < 60)
            return 13;
        snprintf(sql,sizeof(sql),"select id from jie_bai where role_id=%u",p1->GetRoleId());
        if(!pDb->Query(sql))
            return 6;
        row = pDb->GetRow();
        if(row != NULL)
            return 6;
    }
    
    if(p2 != NULL)
    {
        if(GetSysTime() - p2->GetData32(13) < 3600*24)
            return 14;
        if(p2->GetLevel() < 60)
            return 13;
        snprintf(sql,sizeof(sql),"select id from jie_bai where role_id=%u",p2->GetRoleId());
        if(!pDb->Query(sql))
            return 6;
        row = pDb->GetRow();
        if(row != NULL)
            return 6;
    }
    return 0;
}

//结拜,type=1金钱结拜，2 结伴石结拜
//0成功，1金钱不足，2没有结义石,3没有队伍，4结拜称号不是四个汉字,5已有结拜,6队伍中有结拜
//7师徒不能结拜，8夫妻不能结拜，9亲密度低于1000,10不是双，11不是三,12有非法字符,13等级不够
int JieBai(CUser *pUser,int type,const char *title)
{
    if((pUser == NULL) || (pUser->GetLevel() < 60))
        return 13;
    uint8 teamMemNum = GetTeamMemNum(pUser);
    
    if((title == NULL) || (strlen(title) != 8))
        return 4;
    if(IsIllegalMsg(title))
        return 12;
    CGetDbConnect getDb;
    CDatabaseSql *pDb = getDb.GetDbConnect();
    if(pDb == NULL)
        return 5;
    char sql[512];
    
    CUser *p1 = GetTeamMember1(pUser);
    CUser *p2 = GetTeamMember2(pUser);
    if(teamMemNum == 2)
    {
        if(strncmp(title+4,"双",2) != 0)
            return 10;
        if(p1 == NULL)
            return 3;
        snprintf(sql,sizeof(sql),"select m_id from master_prentice where (m_id=%u and p_id=%u) "\
            "or (p_id=%u and m_id=%u)",pUser->GetRoleId(),p1->GetRoleId(),pUser->GetRoleId(),p1->GetRoleId());
        if((pDb->Query(sql)) && (pDb->GetRow() != NULL))
            return 7;
        if(pUser->GetData32(6) == p1->GetRoleId())
            return 8;
        uint16 hotVal = 0;
        pUser->GetHotVal(p1->GetRoleId(),hotVal);
        if(hotVal < 1000)
            return 9;
    }
    else if(teamMemNum == 3)
    {
        if(strncmp(title+4,"三",2) != 0)
            return 11;
        if((p1 == NULL) || (p2 == NULL))
            return 3;
        snprintf(sql,sizeof(sql),"select m_id from master_prentice where (m_id=%u and p_id=%u) "\
            "or (p_id=%u and m_id=%u)",pUser->GetRoleId(),p1->GetRoleId(),pUser->GetRoleId(),p1->GetRoleId());
        if((pDb->Query(sql)) && (pDb->GetRow() != NULL))
            return 7;
        if(pUser->GetData32(6) == p1->GetRoleId())
            return 8;
        uint16 hotVal = 0;
        pUser->GetHotVal(p1->GetRoleId(),hotVal);
        if(hotVal < 1000)
            return 9;
        snprintf(sql,sizeof(sql),"select m_id from master_prentice where (m_id=%u and p_id=%u) "\
            "or (p_id=%u and m_id=%u)",pUser->GetRoleId(),p2->GetRoleId(),pUser->GetRoleId(),p2->GetRoleId());
        if((pDb->Query(sql)) && (pDb->GetRow() != NULL))
            return 7;
        if(pUser->GetData32(6) == p2->GetRoleId())
            return 8;
        hotVal = 0;
        pUser->GetHotVal(p2->GetRoleId(),hotVal);
        if(hotVal < 1000)
            return 9;
        snprintf(sql,sizeof(sql),"select m_id from master_prentice where (m_id=%u and p_id=%u) "\
            "or (p_id=%u and m_id=%u)",p1->GetRoleId(),p2->GetRoleId(),p2->GetRoleId(),p1->GetRoleId());
        if((pDb->Query(sql)) && (pDb->GetRow() != NULL))
            return 7;
        if(p1->GetData32(6) == p2->GetRoleId())
            return 8;
        hotVal = 0;
        p1->GetHotVal(p2->GetRoleId(),hotVal);
        if(hotVal < 1000)
            return 9;
    }
    else
    {
        return 3;
    }
    
    snprintf(sql,sizeof(sql),"select id from jie_bai where role_id=%u",pUser->GetRoleId());
    if(!pDb->Query(sql))
        return 5;
    
    char **row = pDb->GetRow();
    if(row != NULL)
        return 5;
    
    
    if(p1 != NULL)
    {
        if(p1->GetLevel() < 60)
            return 13;
        snprintf(sql,sizeof(sql),"select id from jie_bai where role_id=%u",p1->GetRoleId());
        if(!pDb->Query(sql))
            return 6;
        row = pDb->GetRow();
        if(row != NULL)
            return 6;
    }
    
    if(p2 != NULL)
    {
        if(p2->GetLevel() < 60)
            return 13;
        snprintf(sql,sizeof(sql),"select id from jie_bai where role_id=%u",p2->GetRoleId());
        if(!pDb->Query(sql))
            return 6;
        row = pDb->GetRow();
        if(row != NULL)
            return 6;
    }
    const int jieBaiMoney = 400000;
    const uint16 itemId = 1847;
    if(type == 1)
    {
        if(pUser->GetMoney() < jieBaiMoney)
            return 1;
        pUser->AddMoney(-jieBaiMoney);
    }
    else if(type == 2)
    {
        if(!pUser->HaveItem(itemId))
            return 2;
        pUser->DelPackageById(itemId,1);
    }
    else
    {
        return 1;
    }
    uint32 jbId = 0;
    snprintf(sql,sizeof(sql),"INSERT INTO jie_bai (jiebai_id, role_id,title) VALUES (%u,%u,'%s')",
        jbId,pUser->GetRoleId(),title);
    pDb->Query(sql);
    jbId = pDb->InsertId();
    
    snprintf(sql,sizeof(sql),"update jie_bai set jiebai_id=%u where id=%u",jbId,jbId);
    pDb->Query(sql);
    
    if(p1 != NULL)
    {
        snprintf(sql,sizeof(sql),"INSERT INTO jie_bai (jiebai_id, role_id,title) VALUES (%u,%u,'%s')",
            jbId,p1->GetRoleId(),title);
        pDb->Query(sql);
        p1->AddTextTitle(title);
    }
    if(p2 != NULL)
    {
        snprintf(sql,sizeof(sql),"INSERT INTO jie_bai (jiebai_id, role_id,title) VALUES (%u,%u,'%s')",
            jbId,p2->GetRoleId(),title);
        pDb->Query(sql);
        p2->AddTextTitle(title);
    }
    pUser->AddTextTitle(title);
    return 0;
}
//0成功
int JieSanJieBai(CUser *pUser)
{
    if(pUser == NULL)
        return 1;
    CGetDbConnect getDb;
    CDatabaseSql *pDb = getDb.GetDbConnect();
    if(pDb == NULL)
        return 1;
    char sql[512];
    snprintf(sql,sizeof(sql),"select jiebai_id,title from jie_bai where role_id=%u",pUser->GetRoleId());
    if(!pDb->Query(sql))
        return 1;
    char **row = pDb->GetRow();
    if(row == NULL)
        return 1;
    uint32 jbId = atoi(row[0]);
    string title = row[1];
    snprintf(sql,sizeof(sql),"select role_id from jie_bai where jiebai_id=%u",jbId);
    if(!pDb->Query(sql))
        return 1;
    
    pUser->SetData32(13,GetSysTime());
    pUser->DelTextTitle(title.c_str());
    CGetDbConnect getDb1;
    CDatabaseSql *pDb1 = getDb1.GetDbConnect();
    char info[128];
    snprintf(info,sizeof(info),"%s与您解散了结拜关系",pUser->GetName());
    while((row = pDb->GetRow()) != NULL)
    {
        uint32 roleId = atoi(row[0]);
        if(pUser->GetRoleId() != roleId)
        {
            SendSysMail(roleId,info);
            pUser->SetHotVal(roleId,0);
        }
        ShareUserPtr ptr = SingletonOnlineUser::instance().GetUserByRoleId(roleId);
        CUser *p = ptr.get();
        if(p != NULL)
        {
            p->DelTextTitle(title.c_str());
        }
        else
        {
            CUser *pU = new CUser;
            auto_ptr<CUser> user(pU);
            snprintf(sql,sizeof(sql),"select bank_item from role_info where id=%u",roleId);
            if(pDb1->Query(sql) && ((row = pDb1->GetRow()) != NULL))
            {
                pU->SetBankItem(row[0]);
                pU->DelTextTitle(title.c_str());
                string str;
                pU->GetBankItem(str);
                char *buf = new char[10240];
                auto_ptr<char> freeChar;
                snprintf(buf,10240,"update role_info set bank_item='%s' where id=%u",str.c_str(),roleId);
                pDb1->Query(buf);
            }
        }
    }
    snprintf(sql,sizeof(sql),"delete from jie_bai where jiebai_id=%u",jbId);
    pDb->Query(sql);
    return 0;
}

const char *GetJieBaiUser(CUser *pUser)
{
    if(pUser == NULL)
        return NULL;
    CGetDbConnect getDb;
    CDatabaseSql *pDb = getDb.GetDbConnect();
    if(pDb == NULL)
        return NULL;
    char sql[128];
    snprintf(sql,sizeof(sql),"select jiebai_id from jie_bai where role_id=%u",pUser->GetRoleId());
    if(!pDb->Query(sql))
        return NULL;
    char **row = pDb->GetRow();
    if(row == NULL)
        return NULL;
    uint32 jbId = atoi(row[0]);
    string title = row[1];
    snprintf(sql,sizeof(sql),"select u.name from jie_bai as j,role_info as u where jiebai_id=%u and j.role_id=u.id and j.role_id!=%u"
        ,jbId,pUser->GetRoleId());
    if(!pDb->Query(sql))
        return NULL;
    static char buf[64];
    buf[0] = 0;
    int len = 0;
    while((row = pDb->GetRow()) != NULL)
    {
        if(len == 0)
            strcpy(buf,row[0]);
        else
            snprintf(buf+len,sizeof(buf)-len,",%s",row[0]);
        len = strlen(buf);
    }
    return buf;
}

void DelRole(CUser *pUser,int timeOut)
{
    if(pUser == NULL)
        return;
    uint32 roleId = pUser->GetRoleId();
    uint32 userId = pUser->GetUserId();
    
    CGetDbConnect getDb;
    CDatabaseSql *pDb = getDb.GetDbConnect();
    if(pDb == NULL)
        return;
        
    boost::format fmt("select role0,role1,role2,role3,role4,role5,"\
        "del_time0,del_time1,del_time2,del_time3,del_time4,del_time5 from user_info where id=%1%");
    fmt%userId;
    string sql = fmt.str();
    
    if(!pDb->Query(sql.c_str()))
    {
        return;
    }
    char **row = pDb->GetRow();
    if(row == NULL)
        return;
    uint8 i = 0;
    for(; i < MAX_ROLE_NUM; i++)
    {
        if(atoi(row[i]) == (int)roleId)
            break;
    }
    if(i == MAX_ROLE_NUM)
        return;
    
    sql = (boost::format("update user_info set del_time%1%=%2% where id=%3%")%(int)i%(GetSysTime()-7*24*3600+timeOut)%userId).str();
    pDb->Query(sql.c_str());
}
