#include "utility.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <boost/format.hpp>
#include <boost/scoped_ptr.hpp>
#include <zlib.h>
#include "singleton.h"
#include "net_msg.h"
#include "database.h"
#include "md5.h"
#include "call_script.h"
#include "ini_file.h"
#include "script_call.h"

int StrToHex(const char *str,uint8 *pHex,int hexLen)
{
    if(hexLen < (int)strlen(str)/2)
        return 0;
        
    int i = 0;
    for (; i < (int)strlen(str)/2; i++)
    {
        int temp = 0;
        sscanf(str+2*i,"%02x",&temp);
        pHex[i] = temp;
    }
    return i;
}

void HexToStr(uint8 *pHex,int hexLen,string &str)
{
    char buf[4];
    for (int i = 0; i < hexLen; i++)
    {
        sprintf(buf,"%02x",pHex[i]);
        str.append(buf);
    }
}

void SaveTrade(uint32 user1,int money1,string &item1,string &pet1,
               uint32 user2,int money2,string &item2,string pet2)
{
    CGetDbConnect getDb;
    CDatabaseSql *pDb = getDb.GetDbConnect();
    
    if(pDb == NULL)
        return;
        
    char buf[1024];
    snprintf(buf,1024,"INSERT INTO trade_log (time,user1,money1,item1,pet1,user2,money2,item2,pet2) "\
            "VALUES (%lu,%d,%d,'%s','%s',%d,%d,'%s','%s')",
            GetSysTime(),
            user1,money1,item1.c_str(),pet1.c_str(),
            user2,money2,item2.c_str(),pet2.c_str());   
    pDb->Query(buf);
}

void SaveUseItem(uint32 userId,SItemInstance &item,const char *reason,
                uint8 num,string before,string end)
{
    if(item.tmplId == 0)
        return;
        
    CGetDbConnect getDb;
    CDatabaseSql *pDb = getDb.GetDbConnect();
    
    if(pDb == NULL)
        return;
        
    char buf[2048];
    string strItem;
    HexToStr(item,strItem);
    snprintf(buf,2048,"INSERT INTO use_item_log (role_id,item,num,reason,before_use,end_use,time) "\
             "VALUES (%d,'%s',%d,'%s','%s','%s',%lu)",userId,strItem.c_str(),num,reason,
             before.c_str(),end.c_str(),GetSysTime());
    pDb->Query(buf);
}

void SaveJieBangPet(uint32 roleId,SPet &pet)
{
    CGetDbConnect getDb;
    CDatabaseSql *pDb = getDb.GetDbConnect();
    
    if(pDb == NULL)
        return;
        
    char buf[1024];
    string strPet;
    HexToStr(pet,strPet);
    snprintf(buf,1024,"INSERT INTO jiebang_pet (role_id,pet,time) VALUES (%d, '%s', %lu)"
        ,roleId,strPet.c_str(),GetSysTime());
    pDb->Query(buf);
}

void SaveDelPet(uint32 userId,SPet *pPet)
{
    if(pPet == NULL)
        return;
    CGetDbConnect getDb;
    CDatabaseSql *pDb = getDb.GetDbConnect();
    
    if(pDb == NULL)
        return;
        
    char buf[1024];
    string strPet;
    HexToStr(*pPet,strPet);
    snprintf(buf,1024,"INSERT INTO del_pet (user_id,time,pet) VALUES (%d,%lu,'%s')"
        ,userId,GetSysTime(),strPet.c_str());
    pDb->Query(buf);
}

void SaveCatchPet(uint32 roleId,SPet *pPet)
{
    if(pPet == NULL)
        return;
    CGetDbConnect getDb;
    CDatabaseSql *pDb = getDb.GetDbConnect();
    
    if(pDb == NULL)
        return;
        
    char buf[1024];
    string strPet;
    HexToStr(*pPet,strPet);
    snprintf(buf,1024,"INSERT INTO catch_pet (role_id,pet_id,hp,mp,speed,attack,skill_attack) "\
            "VALUES (%d,%d,%d,%d,%d,%d,%d)",roleId,pPet->tmplId,pPet->hpCZ,pPet->mpCZ,pPet->speedCZ,
            pPet->attackCZ,pPet->skillAttackCZ);
    pDb->Query(buf);
}

void SaveUserShopItem(uint32 buyer,uint32 seller,int money,string &item)
{
    CGetDbConnect getDb;
    CDatabaseSql *pDb = getDb.GetDbConnect();
    
    if(pDb == NULL)
        return;
        
    char buf[1024];
    snprintf(buf,1024,"INSERT INTO user_shop_item (time,buyer,seller,money,item)"\
            "VALUES (%lu,%d,%d,%d,'%s')",
            GetSysTime(),buyer,seller,money,item.c_str());
    
    pDb->Query(buf);
}

void SaveUserShopPet(uint32 buyer,uint32 seller,int money,string &pet)
{
    CGetDbConnect getDb;
    CDatabaseSql *pDb = getDb.GetDbConnect();
    
    if(pDb == NULL)
        return;
        
    char buf[1024];
    snprintf(buf,1024,"INSERT INTO user_shop_pet (time,buyer,seller,money,pet)"\
            "VALUES (%lu,%d,%d,%d,'%s')",
            GetSysTime(),buyer,seller,money,pet.c_str());
    pDb->Query(buf);
}

void SaveBuyShopItem(uint32 userId,int itemId,int num,int itemLevel,int useTongbao,int leftTongbao)
{
    CGetDbConnect getDb;
    CDatabaseSql *pDb = getDb.GetDbConnect();
    
    if(pDb == NULL)
        return;
        
    char buf[1024];
    snprintf(buf,1024,"INSERT INTO buy_shop_item (item_id,num,item_level,role_id,use_tongbao,left_tongbao,time)"\
            "VALUES (%d,%d,%d,%d,%d,%d,%lu)",
            itemId,num,itemLevel,userId,useTongbao,leftTongbao,GetSysTime());
    pDb->Query(buf);
}

int SplitLine(char **templa, int templatecount, char *pkt)
{
  int i = 0;
  while(*pkt == '|')
    ++pkt;
  while(*pkt != 0)
  {
    if((*pkt == '\r') || (*pkt == '\n') || (*pkt == '\t'))
    {
      memmove(pkt,pkt+1,strlen(pkt+1)+1);
    }
    else if(i == 0)
    {
      templa[i] = pkt;
      ++i;
    }
    else if((*pkt == '|') && (i < templatecount))
    {
      *pkt = 0;
      ++pkt;
      while(*pkt == '|')
        ++pkt;
      templa[i] = pkt;
      ++i;
    }
    else 
    {
      ++pkt;
    }
  }
  return i;
}

int split_line (char **tem,int temcount, char *pkt)
{
  int i = 0;
  
  if (!pkt)
    return -1;
  while (ISSPACE (*pkt))
    pkt++;
  while (*pkt && i < temcount)
  {
    if (*pkt == '"')
    {
      /* quoted string */
      pkt++;
      tem[i++] = pkt;
      pkt = strchr (pkt, '"');
      if (!pkt)
      {
        /* bogus line */
        return -1;
      }
      *pkt++ = 0;
      if (!*pkt)
        break;
      pkt++;		/* skip the space */
    }
    else
    {
      tem[i++] = pkt;
      pkt = strpbrk (pkt, " \t\r\n");
      if (!pkt)
        break;
      *pkt++ = 0;
    }
    while (ISSPACE (*pkt))
      pkt++;
  }
  return i;
}

int Random(int min,int max)
{
    if(min >= max)
        return min;
    if(max - min + 1 == 0)
        return 0;
    int r = rand();
    r %= (max - min + 1);
    return r + min;
}

void SendSysInfo(CUser *pUser,const char *info)
{
    if(pUser == NULL)
        return;
        
    CSocketServer &sock = SingletonSocket::instance();
    CNetMessage msg;
    msg.SetType(PRO_SYS_INFO);
    msg<<info;
    sock.SendMsg(pUser->GetSock(),msg);
}

void SendSysInfo1(CUser *pUser,const char *info)
{
    if(pUser == NULL)
        return;
        
    CSocketServer &sock = SingletonSocket::instance();
    CNetMessage msg;
    msg.SetType(PRO_SYS_INFO1);
    msg<<info;
    sock.SendMsg(pUser->GetSock(),msg);
}

void SendPopMsg(CUser *pUser,const char *info)
{
    if((pUser == NULL) || (info == NULL))
        return;
    CSocketServer &sock = SingletonSocket::instance();
    CNetMessage msg;
    msg.SetType(PRO_SYS_POP_MSG);
    msg<<info;
    sock.SendMsg(pUser->GetSock(),msg);
}

void SendSysChannelMsg(CUser *pUser,const char *info)
{
    if((pUser == NULL) || (info == NULL))
        return;
    CSocketServer &sock = SingletonSocket::instance();
    CNetMessage msg;
    msg.SetType(PRO_MSG_CHAT);
    msg<<(uint8)0<<0<<"【系统】"<<info;
    sock.SendMsg(pUser->GetSock(),msg);
}

void SendUserPos(CUser *pUser)
{
    if(pUser == NULL)
        return;
        
    CSocketServer &sock = SingletonSocket::instance();
    CNetMessage msg;
    msg.SetType(PRO_SYNC_POS);
    msg<<pUser->GetRoleId()<<pUser->GetX()<<pUser->GetY()<<pUser->GetFace();
    sock.SendMsg(pUser->GetSock(),msg);
}

void GetTime(time_t t,uint8 &month,uint8 &day,uint8 &hour,uint8 &min)
{
    struct tm *pTm = localtime(&t);
    if(pTm == NULL)
        return;
    
    month = pTm->tm_mon+1;
    day = pTm->tm_mday;
    hour = pTm->tm_hour;
    min = pTm->tm_min;
}

uint32 GetLevelUpExp(uint8 level)
{
    const uint32 levelExp[] = {40,108,133,188,285,436,653,948,1333,1820,2421,3148,4013,5028,6205,7556,9093,10828,12773,14940,17341,19988,22893,26068,29525,33276,37333,41708,46413,51460,56861,62628,68773,75308,82245,89596,97373,105588,114253,123380,132981,143068,153653,164748,176366,197697,220398,244509,270068,297115,325689,355830,387575,420965,456039,492835,531393,571753,613952,658031,704029,751984,801936,853925,908002,1113822,1331073,1560006,1800874,2053927,2319418,2597597,2888717,3193030,3510786,3842237,4187635,4547231,4921278,5310026,5713727,6600000,7700000,8900000,10000000,12000000,14000000,16000000,19000000,22000000,26000000,30000000,35000000,41000000,48000000,56000000,65000000,75000000,87000000,100000000,120000000,140000000,160000000,190000000,220000000,260000000,300000000,350000000,410000000,480000000,560000000,650000000,750000000,870000000,1000000000,1200000000,1400000000,1600000000,1900000000,2200000000u};
    uint8 maxLevel = sizeof(levelExp)/sizeof(int);
    if(level >= maxLevel)
        return 0xffffffff;

    return levelExp[level-1];
}


void ReadHuoDongDrop(CDatabaseSql *pDb)
{
    if(!pDb->Query("select id,`drop` from huodong_drop"))
    {
        return;
    }
    char **row;
    CMonsterManager &mgr = SingletonMonsterManager::instance();
    const int maxDrowNum = 20;
    while((row = pDb->GetRow()) != NULL)
    {
        SMonsterTmpl *pMonster = mgr.GetTmpl(atoi(row[0]));
        if(pMonster == NULL)
            continue;
        if(row[1] == NULL)
            continue;
        char *p[100];
        uint8 num = SplitLine(p,100,row[1]);
        
        if(num > 2)
        {
            if(num > 2*maxDrowNum)
                num = 2*maxDrowNum;
            pMonster->huoDongDropNum = num/2;
            if(pMonster->pHuoDongDrop == NULL)
                pMonster->pHuoDongDrop = new SDropItem[maxDrowNum];
            int region = 0;
            for(int i = 0; i < num/2; i++)
            {
                pMonster->pHuoDongDrop[i].begin = region;
                region += atoi(p[2*i]) - 1;
                pMonster->pHuoDongDrop[i].end = region;
                region++;
                pMonster->pHuoDongDrop[i].itemId = atoi(p[2*i+1]);
            }
        }
    }
}

void ReadMonster()
{
    CGetDbConnect getDb;
    CDatabaseSql *pDb = getDb.GetDbConnect();
                     //0    1   2           3       4   5       6       7   8       9 
    string sql("select id,name,min_level,max_level,exp,min_hp,max_hp,min_mp,max_mp,min_speed,"\
        //10        11          12          13              14               15     16          17
        "max_speed,min_attack,max_attatck,min_skill_attack,max_skill_attack,xiang,drop_item,t_drop_item,"\
        //18
        "skill_id from monster");
    if ((pDb != NULL)
        && (pDb->Query(sql.c_str())))
    {
        char **row;
        int num = 0;
        char *p[100];
        while ((row = pDb->GetRow()) != NULL)
        {
            SMonsterTmpl *pMonster = new SMonsterTmpl;
            pMonster->id = atoi(row[0]);
            pMonster->name = row[1];
            pMonster->minLevel = atoi(row[2]);
            pMonster->maxLevel = atoi(row[3]);
            pMonster->exp = atoi(row[4]);
            pMonster->minHp = atoi(row[5]);
            pMonster->maxHp = atoi(row[6]);
            pMonster->minMp = atoi(row[7]);
            pMonster->maxMp = atoi(row[8]);
            pMonster->minSpeed = atoi(row[9]);
            pMonster->maxSpeed = atoi(row[10]);
            pMonster->minAttack = atoi(row[11]);
            pMonster->maxAttack = atoi(row[12]);
            pMonster->minSkillAttack = atoi(row[13]);
            pMonster->maxSkillAttack = atoi(row[14]);
            pMonster->xiang = atoi(row[15]);
            
            if(row[16] != NULL)
            {
                num = SplitLine(p,100,row[16]);
                if(num > 2)
                {
                    pMonster->dropNum = num/2;
                    pMonster->pDropItem = new SDropItem[pMonster->dropNum];
                    int region = 0;
                    for(int i = 0; i < num/2; i++)
                    {
                        pMonster->pDropItem[i].begin = region;
                        region += atoi(p[2*i]) - 1;
                        pMonster->pDropItem[i].end = region;
                        region++;
                        pMonster->pDropItem[i].itemId = atoi(p[2*i+1]);
                    }
                }
            }
            if(row[17] != NULL)
            {
                num = SplitLine(p,100,row[17]);
                if(num > 2)
                {
                    pMonster->headDropNum = num/2;
                    pMonster->pHeadDropItem = new SDropItem[pMonster->headDropNum];
                    int region = 0;
                    for(int i = 0; i < num/2; i++)
                    {
                        pMonster->pHeadDropItem[i].begin = region;
                        region += atoi(p[2*i]) - 1;
                        pMonster->pHeadDropItem[i].end = region;
                        region++;
                        pMonster->pHeadDropItem[i].itemId = atoi(p[2*i+1]);
                    }
                }
            }
            if(row[18] != NULL)
            {
                memset(pMonster->petSkillId,0,sizeof(pMonster->petSkillId));
                num = SplitLine(p,SMonsterTmpl::MAX_SKILL_NUM,row[18]);
                for(uint8 i = 0; i < num; i++)
                {
                    pMonster->petSkillId[i] = atoi(p[i]);
                }
            }
            SingletonMonsterManager::instance().AddMonster(pMonster);
        }
    }
}

void ReadItem()
{
    CGetDbConnect getDb;
    CDatabaseSql *pDb = getDb.GetDbConnect();
                       //0 1    2    3    4   5       6     7       8       9       10
    string sql("select id,type,level,`mod`,sex,fangyu,gongji,naijiu,jiage,add_fali,add_qixue,"\
                //11            12      13  14  15  
                "add_xueliang,add_sudu,pic,name,des from item_template");
                
    if ((pDb != NULL)
        && (pDb->Query(sql.c_str())))
    {
        char **row;
        while ((row = pDb->GetRow()) != NULL)
        {
            SItemTemplate *pItem  = new SItemTemplate;
            pItem->id = atoi(row[0]);
            pItem->type = atoi(row[1]);//种类          
            pItem->level = atoi(row[2]);//需求等级	   
            pItem->mod = atoi(row[3]);//模式           
            pItem->sex = atoi(row[4]);//性别           
            pItem->fangYu = atoi(row[5]);//防御       
            pItem->gongji = atoi(row[6]);//攻击力     
            pItem->naijiu = atoi(row[7]);//耐久       
            pItem->jiage = atoi(row[8]);//价格        
            pItem->addFaLi = atoi(row[9]);//增加法力  
            pItem->addQiXue = atoi(row[10]);//增加气血 
            pItem->addXue = atoi(row[11]);//增加血量   
            pItem->addSudu = atoi(row[12]);//增加速度              
            pItem->pic = atoi(row[13]);//图片          
            pItem->name = row[14];//名字         
            pItem->describe = row[15];//说明 
            /*if(atoi(row[16]) == 1)
                pItem->canDieJia = true;
            else
                pItem->canDieJia = false;*/
            if((pItem->type == EITTeShu) || (pItem->type == EITNvWaShi))
            {
                pItem->pScript = new CCallScript(pItem->id+20000);
            }
            SingletonItemManager::instance().AddItem(pItem);
            
            
        }
    }
}

static vector<string> sAllMission;

void InitMission()
{
    CGetDbConnect getDb;
    CDatabaseSql *pDb = getDb.GetDbConnect();
    sAllMission.clear();
    sAllMission.resize(500);
    if ((pDb != NULL)
        && pDb->Query("select id,name from mission order by id"))
    {
        char **row;
        while ((row = pDb->GetRow()) != NULL)
        {
            uint32 id = (uint32)atoi(row[0]);
            if(id > sAllMission.capacity())
                sAllMission.resize(id);
            if(id <= 0)
                continue;
            sAllMission.insert(sAllMission.begin()+id-1,row[1]);
        }
    }
}

char *GetMissionName(uint32 id)
{
    if(id >= 10000)
    {
        static char p[] = "通缉任务";
        return p;
    }
    if(id > sAllMission.capacity())
    {
        return NULL;
    }
    return (char*)sAllMission[id-1].c_str();
}

uint8 GetQuality(int fen,uint8 num)
{
    if(fen == 0)
        return 0;
    int jizhun = 5*num;
    if(num == 0)
        return 0;
    else if(fen <= jizhun)
        return 1;
    else if(fen <= jizhun + 40)
        return 2;
    else if(fen <= jizhun + 100)
        return 3;
    else if(fen <= jizhun + 130)
        return 4;            
    else if(fen <= jizhun + 170)
        return 5;
    else if(fen <= jizhun + 250)
        return 6;
    else if(fen <= jizhun + 330)
        return 7;
    else
        return 8;
}

bool MakeItemInfo(SItemInstance *item,CNetMessage &msg)
{
    msg<<item->tmplId;
    if(item->tmplId == 0)
        return true;
    CItemTemplateManager &itemMgr = SingletonItemManager::instance();
    SItemTemplate *pItem = itemMgr.GetItem(item->tmplId);
    if(pItem == NULL)
    {
        return false;
    }
    
    if(pItem->type <= EITGun)
    {//武器
        /************
        +------+----------+--------+----------+------+----------+--------+----------+------+----------+------+
       | 类型 | 需求等级 | 攻击力 | 强化等级 | 品质 | 属性需求 | 属性数 | 附加属性 | 耐久 | 说明长度 | 说明 |
       +------+----------+--------+----------+------+----------+--------+----------+------+----------+------+
       |  1   |    1     |    2   |    1     |   1  |     3    |    1   | 3*属性数 |   2  |     2    |  Var |  
       +------+----------+--------+----------+------+----------+--------+----------+------+----------+------+
       ***************/
        uint16 gongji;
        if(item->level > 0)
            gongji = (int)(pItem->gongji * GetQiangHuaBeiLv(item->level-1));
        else 
            gongji = pItem->gongji;
        msg<<pItem->type<<pItem->level<<gongji<<item->level<<item->quality;
        
        uint16 qPos = msg.GetDataLen();
        uint8 quality = 0;
        int fen = 0;
        uint16 min,mid,max = 0;
        msg<<quality<<item->bangDing
            <<item->reqAttrType<<item->reqAttrVal;
        uint16 pos = msg.GetDataLen();
        msg<<item->addAttrNum;
        uint8 num = 0;
        
        for(uint8 i = 0; i < SItemInstance::MAX_KAIJIA_ATTR_NUM; i++)
        {
            if(item->addAttrType[i] != 0)
            {
                num++;
                msg<<item->addAttrType[i]<<item->addAttrVal[i];
                GetAddAttrVal(item->addAttrType[i],min,mid,max);
                if(max != 0)
                {
                    fen += item->addAttrVal[i]*100/max;
                }
            }
        }
        msg.WriteData(pos,&num,1);
        quality = GetQuality(fen,6);
        msg.WriteData(qPos,&quality,1);
        msg<<item->naijiu;
    }
    else if(pItem->type <= EITYaoDai)
    {
        /*************
        帽子,衣服,腰带格式为：
       +------+----------+------+--------+----------+----------+------+----------+--------+----------+------+----------+------+
       | 类型 | 需求等级 | 防御 | 增加hp | 增加法力 | 强化等级 | 品质 | 属性需求 | 属性数 | 附加属性 | 耐久 | 说明长度 | 说明 |
       +------+----------+------+--------+----------+----------+------+----------+--------+----------+------+----------+------+
       |  1   |    1     |   2  |    2   |    2     |    1     |   1  |    3     |    1   | 3*属性数 |   2  |    2     |  Var |
       +------+----------+------+--------+----------+----------+------+----------+--------+----------+------+----------+------+
       **************/
        uint16 fangYu;
        if(item->level > 0)
            fangYu = (int)(pItem->fangYu * GetQiangHuaBeiLv(item->level-1));
        else 
            fangYu = pItem->fangYu;
        msg<<pItem->type<<pItem->level<<fangYu<<pItem->addQiXue<<pItem->addFaLi
            <<item->level<<item->quality;
        
        uint16 qPos = msg.GetDataLen();
        uint8 quality = 0;
        int fen = 0;
        uint16 min,mid,max = 0;
        msg<<quality<<item->bangDing<<item->reqAttrType<<item->reqAttrVal;
        
        uint16 pos = msg.GetDataLen();
        msg<<item->addAttrNum;
        uint8 num = 0;
        for(uint8 i = 0; i < SItemInstance::MAX_ADD_ATTR_NUM; i++)
        {
            if(item->addAttrType[i] != 0)
            {
                num++;
                msg<<item->addAttrType[i]<<item->addAttrVal[i];
                GetAddAttrVal(item->addAttrType[i],min,mid,max);
                if(max != 0)
                {
                    fen += item->addAttrVal[i]*100/max;
                }
            }
        }
        msg.WriteData(pos,&num,1);
        quality = GetQuality(fen,6);
        msg.WriteData(qPos,&quality,1);
        msg<<item->naijiu;
    }
    else if(pItem->type == EITXieZi)
    {
        /****************
        鞋子格式为:
       +------+----------+------+----------+----------+------+----------+--------+----------+------+----------+------+
       | 类型 | 需求等级 | 防御 | 增加速度 | 强化等级 | 品质 | 属性需求 | 属性数 | 附加属性 | 耐久 | 说明长度 | 说明 |
       +------+----------+------+----------+----------+----------+------+--------+----------+------+----------+------+
       |  1   |    1     |   2  |    2     |    1     |   1  |    3     |    1   | 3*属性数 |   2  |    2     | Var  |
       +------+----------+------+----------+----------+----------+------+--------+----------+------+----------+------+
       ******************/
       uint16 fangYu;
        if(item->level > 0)
            fangYu = (int)(pItem->fangYu * GetQiangHuaBeiLv(item->level-1));
        else 
            fangYu = pItem->fangYu;
        msg<<pItem->type<<pItem->level<<fangYu<<pItem->addSudu
            <<item->level<<item->quality;
            
        uint16 qPos = msg.GetDataLen();
        uint8 quality = 0;
        int fen = 0;
        uint16 min,mid,max = 0;
        msg<<quality<<item->bangDing<<item->reqAttrType<<item->reqAttrVal;
        
        uint16 pos = msg.GetDataLen();
        msg<<item->addAttrNum;
        uint8 num = 0;
        for(uint8 i = 0; i < SItemInstance::MAX_ADD_ATTR_NUM; i++)
        {
            if(item->addAttrType[i] != 0)
            {
                num++;
                msg<<item->addAttrType[i]<<item->addAttrVal[i];
                GetAddAttrVal(item->addAttrType[i],min,mid,max);
                if(max != 0)
                {
                    fen += item->addAttrVal[i]*100/max;
                }
            }
        }
        msg.WriteData(pos,&num,1);
        quality = GetQuality(fen,6);
        msg.WriteData(qPos,&quality,1);
        msg<<item->naijiu;
    }    
    else if(pItem->type == EITXiangLian)
    {
        /***********
        项链格式为：
        +------+----------+----------+--------+----------+----------+------+
        | 类型 | 需求等级 | 增加法力 | 属性数 | 附加属性 | 说明长度 | 说明 |
        +------+----------+----------+--------+----------+----------+------+
        |   1  |    1     |    2     |    1   | 3*属性数 |     2    |  Var |
        +------+----------+----------+--------+----------+----------+------+    
        *************/
        uint16 addFaLi = pItem->addFaLi;
        if(item->naijiu > 0)
            addFaLi += CalculateRate((int)addFaLi,(int)item->naijiu,100);
        //msg<<pItem->type<<pItem->level<<addFaLi<<(uint8)item->naijiu<<item->addAttrNum;
        msg<<pItem->type<<pItem->level<<addFaLi<<(uint8)item->naijiu
            <<(uint8)item->quality;
        
        uint16 qPos = msg.GetDataLen();
        uint8 quality = 0;
        int fen = 0;
        uint16 min,mid,max = 0;
        msg<<quality<<(uint8)item->bangDing;
        
        uint16 pos = msg.GetDataLen();
        uint8 num = 0;
        msg<<item->addAttrNum;
        for(uint8 i = 0; i < SItemInstance::MAX_ADD_ATTR_NUM; i++)
        {
            if(item->addAttrType[i] != 0)
            {
                num++;
                msg<<item->addAttrType[i]<<item->addAttrVal[i];
                GetAddAttrVal(item->addAttrType[i],min,mid,max);
                if(max != 0)
                {
                    fen += item->addAttrVal[i]*100/max;
                }
            }
        }
        msg.WriteData(pos,&num,1);
        quality = GetQuality(fen,6);
        msg.WriteData(qPos,&quality,1);
    }
    else if(pItem->type == EITYuPei)
    {/***********
        戒指格式为:
       +------+----------+----------+--------+----------+----------+------+
       | 类型 | 需求等级 | 增加气血 | 属性数 | 附加属性 | 说明长度 | 说明 |
       +------+----------+----------+--------+----------+----------+------+
       |   1  |    1     |    2     |    1   | 3*属性数 |     2    |  Var |
       +------+----------+----------+--------+----------+----------+------+
       ***************/
        uint16 addQiXue = pItem->addQiXue;
        if(item->naijiu > 0)
            addQiXue += CalculateRate((int)addQiXue,(int)item->naijiu,100);
        msg<<pItem->type<<pItem->level<<addQiXue<<(uint8)item->naijiu
            <<(uint8)item->quality;
        
        uint16 qPos = msg.GetDataLen();
        uint8 quality = 0;
        int fen = 0;
        uint16 min,mid,max = 0;
        msg<<quality<<(uint8)item->bangDing;
        
        uint16 pos = msg.GetDataLen();
        msg<<item->addAttrNum;
        uint8 num = 0;
        for(uint8 i = 0; i < SItemInstance::MAX_ADD_ATTR_NUM; i++)
        {
            if(item->addAttrType[i] != 0)
            {
                num++;
                msg<<item->addAttrType[i]<<item->addAttrVal[i];
                GetAddAttrVal(item->addAttrType[i],min,mid,max);
                if(max != 0)
                {
                    fen += item->addAttrVal[i]*100/max;
                }
            }
        }
        msg.WriteData(pos,&num,1);
        quality = GetQuality(fen,6);
        msg.WriteData(qPos,&quality,1);
        msg<<item->name;
    }
    else if(pItem->type == EITShouZhuo)
    {
        /**********
        护腕格式为:
       +------+----------+----------+--------+----------+----------+------+
       | 类型 | 需求等级 | 增加攻击 | 属性数 | 附加属性 | 说明长度 | 说明 |
       +------+----------+----------+--------+----------+----------+------+
       |   1  |    1     |    2     |    1   | 3*属性数 |     2    |  Var |
       +------+----------+----------+--------+----------+----------+------+
        **************/
        uint16 gongji = pItem->gongji;
        
        if(item->naijiu > 0)
            gongji += CalculateRate((int)gongji,(int)item->naijiu,100);
        
        msg<<pItem->type<<pItem->level<<gongji<<(uint8)item->naijiu
            <<(uint8)item->quality;
        uint16 qPos = msg.GetDataLen();
        uint8 quality = 0;
        int fen = 0;
        uint16 min,mid,max = 0;
        msg<<quality<<(uint8)item->bangDing;//<<item->addAttrNum;
        uint16 pos = msg.GetDataLen();
        msg<<item->addAttrNum;
        uint8 num = 0;
        for(uint8 i = 0; i < SItemInstance::MAX_ADD_ATTR_NUM; i++)
        {
            if(item->addAttrType[i] != 0)
            {
                num++;
                msg<<item->addAttrType[i]<<item->addAttrVal[i];
                GetAddAttrVal(item->addAttrType[i],min,mid,max);
                if(max != 0)
                {
                    fen += item->addAttrVal[i]*100/max;
                }
            }
        }
        msg.WriteData(pos,&num,1);
        quality = GetQuality(fen,6);
        msg.WriteData(qPos,&quality,1);
    }
    else if(pItem->type == EITAddAttr)
    {
        msg<<pItem->type<<item->addAttrType[0]<<item->addAttrVal[0]<<item->level
            <<item->bangDing;
    /*********    
    类别13IDEF为
      +------+----------+--------+------+
      | 类型 | 属性类别 | 属性值 | 等级 |
      +------+----------+--------+------+
      |   1  |     1    |   2    |  1   |
      +------+----------+--------+------+
    ***********/
    }
    else if((pItem->type == EITPKYaoPin) || (pItem->type == EITNormalYaoPin))
    {
        /*
        +------+------+----+----+
        | 类型 | 模式 | hp | mp |
        +------+------+----+----+
        |   1  |   1  | 4  |  4 |
        +------+------+----+----+
        */
        msg<<pItem->type<<pItem->mod<<pItem->addQiXue<<pItem->addFaLi<<item->bangDing;
    }
    else if(pItem->type == EITNvWaShi)
    {
        int val;
        val = item->naijiu;
        msg<<pItem->type<<item->level<<val;
    }
    else if(pItem->type == EITTeShu) 
    {
        int val = item->naijiu;
        if(pItem->id == 1005)
        {
            val = *(int*)item->addAttrVal;
        }
        msg<<pItem->type<<item->level<<val;
    }
    else if((pItem->type == EITMission) || (pItem->type == EITCanDelMiss))
    {
        /*
        +------+----------+------+
        | 类型 | 说明长度 | 说明 |
        +------+----------+------+
        |   1  |     2    |  Var |
        +------+----------+------+
        */
        msg<<pItem->type;
    }
    else if(pItem->type == EITMissionCanSave)
    {
        msg<<pItem->type<<item->bangDing;
    }
    else if(pItem->type == EITChargeItem)
    {
        int *pAdd = (int*)item->addAttrVal;
        if((pItem->id == 1809) || (pItem->id == 1815))
        {
#ifdef DEBUG
            cout<<"回归符耐久："<<item->naijiu<<endl;
#endif 
            *pAdd = item->naijiu;
        }
        msg<<pItem->type<<*pAdd<<item->bangDing;
        msg<<item->name;
    }
    else if(pItem->type == EITPetBook)
    {
        msg<<pItem->type<<item->bangDing;
    }
    else if(pItem->type == EITPetKaiJia)
    {
        uint16 fangYu;
        if(item->level > 0)
            fangYu = (int)(pItem->fangYu * GetQiangHuaBeiLv(item->level-1));
        else 
            fangYu = pItem->fangYu;
        uint8 quality =  GetChongKaiQuality(item);
        msg<<pItem->type<<pItem->level<<fangYu<<pItem->addSudu<<item->level<<quality<<item->bangDing;
        item->MakeKongInfo(msg);//<<(uint8)0<<(uint8)0<<(uint8)0;
        
        msg<<item->naijiu;
    }
    /*if(pItem->mod & EIMTName)
    {
        msg<<item->name;
    }*/
    return true;
}

uint8 GetChongKaiQuality(SItemInstance *pItem)
{
    int fen = 0;
    uint16 min,mid,max = 0;
    for(uint8 i = 0; i < SItemInstance::MAX_KAIJIA_ATTR_NUM; i++)
    {
        if(pItem->addAttrType[i] != 0)
        {
            GetAddAttrVal(pItem->addAttrType[i],min,mid,max);
            if(max != 0)
            {
                fen += pItem->addAttrVal[i]*100/max;
            }
        }
    }
    uint8 num = 0;
    uint8 kongNum[] = {
        1,0,0,
        1,1,0,
        2,1,0,
        2,2,0,
        2,2,1,
        3,2,1,
        3,2,2,
        3,3,2,
        3,3,3,
        3,3,3,
        3,3,3};
    int minId = 800;
    uint8 pos = pItem->tmplId - minId;
    if(pos < sizeof(kongNum))
    {
        num = kongNum[3*pos]+kongNum[3*pos+1]+kongNum[3*pos+2];
    }
    return GetQuality(fen,num);
}

static time_t sSysTime;

void SetSysTime(time_t t)
{
    sSysTime = t;
}

time_t GetSysTime()
{
    return sSysTime;
}

void MD5String(string &str)
{
    uint8 out[16];
    md5((uint8*)str.c_str(),str.size(),out);
    str.clear();
    HexToStr(out,16,str);
}

bool UserCanGetPet(CUser *pUser,uint16 petId)
{
    SMonsterTmpl *pTmpl = SingletonMonsterManager::instance().GetTmpl(petId);
    if((pTmpl == NULL) || (pTmpl->minLevel > pUser->GetLevel()+5)
        || (pTmpl->id >= 100))
        return false;
    return true;
}

static boost::mutex    m_mutex;
int GbkToUnicode(char *to,size_t toLen,char *from,size_t fromLen)
{
    static iconv_t sCdGbkUnicode = iconv_open("UNICODELITTLE","GBK");
    boost::mutex::scoped_lock lk(m_mutex);
    size_t oldToLen = toLen;  
    iconv(sCdGbkUnicode,&from,&fromLen,&to,&toLen);
    return (oldToLen-toLen);
}

int UnicodeToGbk(char *to,size_t toLen,char *from,size_t fromLen)
{
    static iconv_t sCdUnicodeGbk = iconv_open("GBK","UNICODELITTLE");
    boost::mutex::scoped_lock lk(m_mutex);
    size_t oldToLen = toLen;
    iconv(sCdUnicodeGbk,&from,&fromLen,&to,&toLen);
    return (oldToLen-toLen);
}

const char *pIllegalString[] = {
    "管理",
    "系统",
    "充值",
    "在线",
    "客服",
    "反馈",
    "处理",
    "黄网",
    "技术",
    "服务",
    "@一区",
    "@二区",
    "@三区",
    "@四区",
    "@五区",
    "@六区",
    "@七区",
    "@八区",
    "@九区",
    "@十区",
    "『",
    "』"
};

bool IllegalStr(string &str)
{
    const char *p = str.c_str();
    
    for(uint32 i = 0; i < strlen(p); i++)
    {
        if((p[i] == '&') || (p[i] == '\'')
            || (p[i] == '\"') || (p[i] == '\\')
            || (p[i] == ' '))
        {
            str = "GM";
            return true;
        }
        else if(((p[i] == 'g') || (p[i] == 'G'))
            && ((p[i+1] == 'm') || (p[i+1] == 'M')))
        {
            str = "GM";
            return true;
        }
    }
    int num = sizeof(pIllegalString)/sizeof(char*);
    for(int i = 0; i < num; i++)
    {
        if(strstr(p,pIllegalString[i]) != NULL)
        {
            str = pIllegalString[i];
            return true;
        }
    }
    return false;
}

CCallScript *GetScript()
{
    static CCallScript *pScript;
    if(pScript == NULL)
    {
        pScript = new CCallScript(10000);
    }
    return pScript;
}

extern char *gConfigFile;
const char *GetScriptDir()
{
    static string scriptDir;
    if(scriptDir.size() <= 0)
    {
        scriptDir = CIniFile::GetValue("script_dir","server",gConfigFile);
    }
    return scriptDir.c_str();
}

static void SendNianShouInfo(CSocketServer *pSock,CNetMessage *pMsg,CUser *pUser)
{
    pSock->SendMsg(pUser->GetSock(),*pMsg);
}

void SendMsgToAllUser(CNetMessage &msg)
{
    CSocketServer &sock = SingletonSocket::instance();
    SingletonOnlineUser::instance().ForEachUser(boost::bind(SendNianShouInfo,&sock,&msg,_1));
}

void SendSceneMsg(CNetMessage &msg,CScene *pScene)
{
    CSocketServer &sock = SingletonSocket::instance();
    COnlineUser   &onlineUser =  SingletonOnlineUser::instance();
    if(pScene != NULL)
    {
        list<uint32> userList;
        pScene->GetUserList(userList);
        list<uint32>::iterator iter = userList.begin();
        for(; iter != userList.end(); iter++)
        {
            ShareUserPtr p = onlineUser.GetUserByRoleId(*iter);
            if(p.get() != NULL)
            {
                sock.SendMsg(p->GetSock(),msg);
            }
        }
    }
}

void SendSceneMsg(CNetMessage &msg,int sceneId)
{
    CSceneManager &scene = SingletonSceneManager::instance();
    CSocketServer &sock = SingletonSocket::instance();
    COnlineUser   &onlineUser =  SingletonOnlineUser::instance();
    
    CScene *pScene = scene.FindScene(sceneId);
    if(pScene != NULL)
    {
        list<uint32> userList;
        pScene->GetUserList(userList);
        list<uint32>::iterator iter = userList.begin();
        for(; iter != userList.end(); iter++)
        {
            ShareUserPtr p = onlineUser.GetUserByRoleId(*iter);
            if(p.get() != NULL)
            {
                sock.SendMsg(p->GetSock(),msg);
            }
        }
    }
}

void SysInfoToAllUser(const char *msg)
{
    CNetMessage sysMsg;
    sysMsg.SetType(PRO_SYSTEM_INFO);
    sysMsg<<msg;
    
    CSocketServer &sock = SingletonSocket::instance();
    SingletonOnlineUser::instance().ForEachUser(boost::bind(SendNianShouInfo,&sock,&sysMsg,_1));
}

static bool sFightHuoDong = false;
void BeginFightHuoDong()
{
    sFightHuoDong = true;
}
void EndFightHuoDong()
{
    sFightHuoDong = false;
}
bool InFightHuoDong()
{
    return sFightHuoDong;
}

static int sLeftNum = 0;
int GetLeftDropNum()
{
    return sLeftNum;
}
void SetLeftDropNum(int num)
{
    sLeftNum = num;
}

const int MAX_SCRIPT_VAL = 10;
static int SCRIPT_VAL[MAX_SCRIPT_VAL];

int GetScriptVal(int id)
{
    if(id >= MAX_SCRIPT_VAL)
        return 0;
    return SCRIPT_VAL[id];
}

void SetScriptVal(int id,int val)
{
    if(id >= MAX_SCRIPT_VAL)
        return;
    SCRIPT_VAL[id] = val;
}

void ReadScriptVal()
{
    FILE *file = fopen("script_val","r");
    if(file == NULL)
        return;
    fread(SCRIPT_VAL,sizeof(SCRIPT_VAL),1,file);
    fclose(file);
}

void WriteScriptVal()
{
    FILE *file = fopen("script_val","w");
    if(file == NULL)
        return;
    fwrite(SCRIPT_VAL,sizeof(SCRIPT_VAL),1,file);
    fclose(file);
}

time_t sClearTime = 0;
void SetClearTime(time_t t)
{
    sClearTime = t;
}

time_t GetClearTime()
{
    return sClearTime;
}

void AddDiscipleAward(CUser *pUser)
{
    if(pUser == NULL)
        return;
    int type = 0;
    if(pUser->GetLevel() == 30)
        type = 1;
    else if(pUser->GetLevel() == 40)
        type = 2;
    /*else if(pUser->GetLevel() == 50)
        type = 3;*/
    else 
        return;
    uint32 master = GetMaster(pUser);
    if(master == 0)
        return;
        
    CGetDbConnect getDb;
    CDatabaseSql *pDb = getDb.GetDbConnect();
    if(pDb == NULL)
        return;
                
    char sql[256];
    snprintf(sql,256,"INSERT INTO mp_award (role_id,type,award_type) VALUES (%d,1,%d)",
        pUser->GetRoleId(),type);
    pDb->Query(sql);
    
    snprintf(sql,256,"INSERT INTO mp_award (role_id,type,award_type) VALUES (%d,0,%d)",
        master,type);
    pDb->Query(sql);
}
/*
void AddMasterAward(CUser *pUser,int type)
{
    if(pUser == NULL)
        return;
    
    CGetDbConnect getDb;
    CDatabaseSql *pDb = getDb.GetDbConnect();
    if(pDb == NULL)
        return;
                
    char sql[256];
    snprintf(sql,256,"INSERT INTO mp_award (role_id,type,award_type) VALUES (%d,0,%d)",
        pUser->GetRoleId(),type);
    pDb->Query(sql);
}
*/

uint8 GetPetQuality(SPet *pPet)
{
    if(pPet == NULL)
        return 0;
    SMonsterTmpl *pTmpl = SingletonMonsterManager::instance().GetTmpl(pPet->tmplId);
    if(pTmpl == NULL)
        return 0;
    double rate; 
    if(pPet->xiang > 0)
    {//法宠
        rate = pPet->hpCZ-pTmpl->minHp;//pTmpl->minHp,pTmpl->maxHp
        rate += pPet->mpCZ-pTmpl->minMp;// = Random(pTmpl->minMp,pTmpl->minMp+()/10);
        rate += pPet->speedCZ-pTmpl->minSpeed;// = Random(pTmpl->minSpeed,pTmpl->minSpeed+(pTmpl->maxSpeed-pTmpl->minSpeed)/10);
        //rate += pPet->attackCZ;// = Random(pTmpl->minAttack,pTmpl->minAttack+(pTmpl->maxAttack-pTmpl->minAttack)/10);
        rate += pPet->skillAttackCZ-pTmpl->minSkillAttack;
        int maxCZ = pTmpl->maxHp - pTmpl->minHp
                    + pTmpl->maxHp-pTmpl->minMp
                    + pTmpl->maxSpeed-pTmpl->minSpeed
                    + pTmpl->maxSkillAttack-pTmpl->minSkillAttack;
        if(maxCZ <= 0)
            maxCZ = 1;
        rate = rate/maxCZ;
    }
    else
    {//物攻宠
        rate = pPet->hpCZ-pTmpl->minHp;//pTmpl->minHp,pTmpl->maxHp
        rate += pPet->mpCZ-pTmpl->minMp;// = Random(pTmpl->minMp,pTmpl->minMp+()/10);
        rate += pPet->speedCZ-pTmpl->minSpeed;// = Random(pTmpl->minSpeed,pTmpl->minSpeed+(pTmpl->maxSpeed-pTmpl->minSpeed)/10);
        rate += pPet->attackCZ-pTmpl->minAttack;// = Random(pTmpl->minAttack,pTmpl->minAttack+(pTmpl->maxAttack-pTmpl->minAttack)/10);
        //rate += pPet->skillAttackCZ;
        int maxCZ = pTmpl->maxHp - pTmpl->minHp
                    + pTmpl->maxMp-pTmpl->minMp
                    + pTmpl->maxSpeed-pTmpl->minSpeed
                    + pTmpl->maxAttack-pTmpl->minAttack;
        if(maxCZ <= 0)
            maxCZ = 1;
        rate = rate/maxCZ;
    }
    
    if(rate >= 0.8)
        return EPQdingji;
    else if(rate >= 0.6)
        return EPQxiyou;
    else if(rate >= 0.4)
        return EPQyouxiu;
    else if(rate >= 0.2)
        return EPQputong;
    else
        return EPQpingyong;
    return 0;
}

static uint32 sShopItemId;
//产生物品个人店铺物品id，每次+1
uint32 GetShopItemId()
{
    return sShopItemId++;
}

void SendSysMail(CUser *pUser,const char *info)
{
    if((pUser == NULL) || (info == NULL))
        return;
    if(pUser->IsLogout())
    {
        CGetDbConnect getDb;
        CDatabaseSql *pDb = getDb.GetDbConnect();
        
        char sql[1024];
        sprintf(sql,"INSERT INTO role_mail (from_id,to_id,from_name,time,msg) VALUES ("\
                    "%d,%d,'%s',%lu,'%s')",
                    0,pUser->GetRoleId(),"【系统】",GetSysTime(),info);                    
        pDb->Query(sql);
    }
    else
    {
        CNetMessage msg;
        msg.SetType(PRO_ONLINE_MAIL);
        uint8 month,day,hour,min;
        GetTime(GetSysTime(),month,day,hour,min);
        msg<<0<<"【系统】"<<info<<month<<day<<hour<<min;
        CSocketServer &sock = SingletonSocket::instance();
        sock.SendMsg(pUser->GetSock(),msg);
    }
}

static const char* IllegalChatMsg[] = 
{
    "藏旗","藏青会","堵截","希望OL","明慧","朱蒙","渊改苏文","大祚荣","太王四神记","爆炸","示威","自焚","垮台","倒台","分裂","反社会","洛神","大唐风云","凤舞天骄","没有人权","法轮功","大法","新疆独立","西藏独立","反党","反政府","反国家","圆满","法轮","李洪志","转法轮","大法轮","轮功","共铲党","九评共产党","明慧网","退党","无网界浏览","无网界","美国之音","自由亚洲","文化大革命","六四","大纪元","真善忍","新唐人","六四事件","反华","共产党","共匪","政府软弱","政府无能","达赖","警察殴打","中央军委","针对台湾","中共走狗","中共小丑","共奴","中共恶霸","共产无赖","右派","流氓政府","内斗退党","江理论","党内分裂","新生网","圆明网","修炼之歌","发正念","和平修炼","放下生死","大法大福","大法弟子","支联会","共产专制","共产极权","专政机器","共产王朝","大法洪传","毛派","法网恢恢","邓派","五套功法","宇宙最高法理","法轮佛法","谁是新中国","法正人间","法正乾坤","正法时期","海外护法","洪发交流","报禁","党禁","鸽派","鹰派","赣江学院暴动","全国退党","绝食抗暴","维权抗暴","活体器官","中共暴政","中共当局","胡温政府","江罗集团","师傅法身","正派民运","中华联邦政府","亲共行动","联邦政府","流氓民运","特务民运","江贼民","中共警察","中共监狱","中共政权","中共迫害","自由联邦","中共独枭","流氓无产者","中共专制","明慧周刊","共狗","大圆满","真、善、忍","护法","集会","游行","性交","一党专制","雪山狮子","西藏人权","藏独","性爱","赤裸","性交易","性服务","特殊服务","处女膜","透码","AV片","AV女","迷奸药","爱液","波霸","肥穴","后庭","交媾","菊花蕾","浪穴","美穴","蜜穴","嫩穴","女优","蓬门","屁眼","兽交","肉洞","色情网站","骚穴","情色","酥乳","素人","小穴","穴穴","阴精","淫姐姐","淫女","淫水","淫穴","淫汁","应召女郎","龟头","鸡巴","精液","喷精","裸聊","阴茎","阴毛","性虐待","黄色网站","成人网站","成人小说","成人文学","成人影视","成人漫画","成人图片","肉棒","阳具","爆浆","插穴","肛交","开苞","口交","乱伦","轮奸","淫","群奸","群交","乳交","舔","舔奶","舔穴","吸鸟","性派对","颜射","诱奸","援交","长舌象","成人用品","吹气娃娃","催情","滚珠棒","蜜蜂棒","气球男孩","跳蛋","兔女郎","超级性虐","成人电影","成人书库","成人图","成人网","炮图","三级片","偷窥","无码","性爱图","淫图","激情","诱惑","黄色影视","黄色电影","黄色小说","黄色文学","黄色图片","黄色漫画","坐台小姐","应招妓女","成人论坛","性免费电影","小电影","阳萎","迷药","日你","摸奶","破鞋","三点","避孕套","卫生巾","艳照门","男女公关","感情陪护","三陪先生","三陪小姐","买春","隐私图片","美腿写真","丝袜写真","陈冠希","阿娇","艳照","熟女","美少妇","换妻","月薪万元","玉蒲团","台湾18DY电影","H动漫","金瓶梅","网络色情网址大全","H漫画","勾引","他妈的","江湖淫娘","粗口歌","A片","毛片","黄片","禁区","风骚","高潮","云雨","销魂","乳房","调情","双腿间的禁地","少儿不宜","X夜激情","叫床","一夜情","两性狂情","夜激情","干柴烈火","咪咪","小弟弟","命根","色情","六合彩","嫖娼","卖淫","淫秽","你妈","你娘","他妈","他娘","妈的","你全家","我日","我操","我奸","干你","操你","射你","强奸","奸你","插你","婊子","妓","傻B","傻比","傻逼","骚逼","妈B","妈逼","逼样","妈比","骚货","祖宗","狗日","屁股","鸡8","鸡奸","阴唇","交配","淫荡","贱货","卖B","卖比","浪叫","奶子","乳头","姘头","TMD","仆街","白痴","老二","笨蛋","射精","爹娘","雞巴","王八蛋","你是鸡","你是鸭","烂货","烂逼","阴道","吃屎","妓院","扒光","干你妈","做爱","外阴","妈妈的","我靠","操你妈","干你娘","鸡吧","鸡八","外挂","他奶奶","狗娘养的","日你妈","龟儿子","杂种","下三烂","下贱","口肯","他奶奶的","他奶娘的","他马的","他妈ㄉ王八蛋","白烂","她奶奶的","她马的","她妈的","老母","老味","你他马的","你他妈的","你它马的","你它妈的","你她马的","你娘卡好","你娘咧","你马的","你妈的","妳妈的","狗屁","肥西","姣西","奸","强奸你","淫西","淫妇","几八","几巴","几叭","几芭","插死你","塞你公","塞你母","塞你老母","塞你老师","塞你爸","塞你娘","妈的B","妈个B","干x娘","干七八","干死GM","干死客服","干死CS","干死你","干你老母","干你良","干您娘","干机掰","贱B","贱人","靠北","靠母","靠爸","靠背","靠腰","驶你公","驶你母","驶你老母","驶你老师","驶你爸","驶你娘","操78","操你全家","操你老母","操你老妈","操你娘","操你祖宗","操机掰","机八","机巴","机机歪歪","赛你老母","赛妳阿母","鸡叭","鸡芭","懒8","懒八","懒叫","懒教","赣您娘","灨你娘","他吗的","吗的","激情电影","十八禁","18禁","耽美","丝袜","写真","亚热","花花公子",
    "大禁","H电影","龙虎豹","啄木鸟公司","丁度·巴拉斯","阁楼","成人杂志","ANDREWBLAKE","藏春阁","PRIVATE","痴汉是犯罪","美女野球拳","裸女对对碰","偷窥有罪","伪装美女","虎胆雄心","买春堂","护士24点","成人配色","尾行","性感扑克","夜场女模特","网友自拍","玉女心经","阴蒂","淫欲小说","裸体美女","浪妇妹妹","美女走光","性城","奶交","免费激情片","浪妇阿姨","日本女学生图片","阿姨女殖器","射进女儿的子宫","性交电影","阴道图","美女自慰","处女性交","处女做爱","操处女逼","美女操逼","性交图","日本美女","丝袜美腿","美女乱交","阴道写真","美女射精","操逼","自由亚州","无界浏览","极景","无界","免费铃声下载","铃声下载","手机铃声","美女走光图片","走光照片","美女全脱图","美女内射","色文小说","露底走光","文心藏阁","少女发春","丝袜电影","办公室艳情小说","艳情小说","色豹网强奸","性文学小说","成人图书馆","情欲奸美女","人妖杂交","黄影院","色乱小说","色爽爽性文学","实弹做爱","美女撒尿图","少女乱插","全奶美女","免费禁播电影","美女地图","色十八","美少女交奸","熟妇高潮","成人武侠艳情","超色日本卡通漫画","林心如走光","少女发育图","小穴插插","粉嫩小穴","美穴贴图区","骚妹浪穴","少女阴穴","小嫩穴","操穴","女人的b","少女嫩穴","女人的洞","插妈妈","兽奸","成人贴图","日逼","骚比","火星文","撑粤语","革命","茉莉花","博讯","中心广场","五月花广场","81386360"
};


bool IsIllegalMsg(const char *msg)
{
    for(int i = 0; i < (int)(sizeof(IllegalChatMsg)/sizeof(IllegalChatMsg[0])); i++)
    {
        if(strstr(msg,IllegalChatMsg[i]) != NULL)
			return true;
    }
    return false;
}

void SysInfoToGroupUser(int sceneGroup,const char *info)
{
    CNetMessage msg;
    msg.SetType(PRO_SYSTEM_INFO);
    msg<<info;
    
    CSceneManager &scene = SingletonSceneManager::instance();
    CSocketServer &sock = SingletonSocket::instance();
    COnlineUser   &onlineUser =  SingletonOnlineUser::instance();
    
    list<int> sceneList;
    scene.GetGroupScene(sceneGroup,sceneList);
    for(list<int>::iterator i = sceneList.begin(); i != sceneList.end(); i++)
    {
        CScene *pScene = scene.FindScene(*i);
        if(pScene == NULL)
            continue;
        list<uint32> userList;
        pScene->GetUserList(userList);
        list<uint32>::iterator iter = userList.begin();
        for(; iter != userList.end(); iter++)
        {
            ShareUserPtr p = onlineUser.GetUserByRoleId(*iter);
            if(p.get() != NULL)
            {
                sock.SendMsg(p->GetSock(),msg);
            }
        }
    }
}

void SendSysInfoToGroup(int sceneGroup,const char *info)
{
    CSceneManager &scene = SingletonSceneManager::instance();
    CSocketServer &sock = SingletonSocket::instance();
    COnlineUser   &onlineUser =  SingletonOnlineUser::instance();
    CNetMessage msg;
    msg.SetType(PRO_SYS_INFO);
    msg<<info;
    
    list<int> sceneList;
    scene.GetGroupScene(sceneGroup,sceneList);
    for(list<int>::iterator i = sceneList.begin(); i != sceneList.end(); i++)
    {
        CScene *pScene = scene.FindScene(*i);
        if(pScene == NULL)
            continue;
        list<uint32> userList;
        pScene->GetUserList(userList);
        list<uint32>::iterator iter = userList.begin();
        for(; iter != userList.end(); iter++)
        {
            ShareUserPtr p = onlineUser.GetUserByRoleId(*iter);
            if(p.get() != NULL)
            {
                sock.SendMsg(p->GetSock(),msg);
            }
        }
    }
}
/*
void Compress(uint8 *p,uint32 len,string &compress)
{
    void *res = alloca(len);
    uLongf outLen = len;
    compress2((Bytef*)p,(uLongf*)&outLen,(Bytef*)res,len,9);
    HexToStr((uint8*)res,outLen,compress);
}

void UnCompress(char *src,uint8 *p,uint32 *pLen)
{
    uint32 outLen = 1024*8;
    uint32 len = strlen(src)/2;
    uint8 *pSrc = (uint8*)alloca(len);
    StrToHex(src,p,*pLen);
    
    uncompress((Bytef*)p,(uLongf*)&outLen,(Bytef*)pSrc,len);
}
*/

void SaveGuiJieJl(CUser *pUser,int item)
{
    CGetDbConnect getDb;
    CDatabaseSql *pDb = getDb.GetDbConnect();
    
    if(pDb == NULL)
        return;
        
    char buf[512];
    snprintf(buf,512,"INSERT INTO guijie_jl(role_id,item) VALUES (%u,%d)",
            pUser->GetRoleId(),item);
    pDb->Query(buf);
}

void AddTongBao(uint32 roleId,int tongbao)
{
    COnlineUser   &onlineUser =  SingletonOnlineUser::instance();
    ShareUserPtr p = onlineUser.GetUserByRoleId(roleId);
    CUser *pUser = p.get();
    if(pUser != NULL)
    {
        pUser->AddTongBao(tongbao);
    }
    else
    {
        CGetDbConnect getDb;
        CDatabaseSql *pDb = getDb.GetDbConnect();
        
        char sqlBuf[256];
        snprintf(sqlBuf,256,"update user_info set money=money+%d where role0=%u or role1=%u or role2=%u or role3=%u or role4=%u or role5=%u",
            tongbao,roleId,roleId,roleId,roleId,roleId,roleId);
        if (pDb != NULL)
        {
            pDb->Query(sqlBuf);
        }
    }
}

uint8 GetPetSpeed(int qinmi)
{
    if(qinmi < 100000)
        return 8;
    else if(qinmi < 300000)
        return (uint8)(1.5*8);
    else
        return 16;
}

uint64 GetTime()
{
    timeval tv;
    gettimeofday(&tv,NULL);
    uint64 t = 1000000*tv.tv_sec;
    t += tv.tv_usec;
    return t;
}

uint8 GetRoleName(uint32 id,char *name)
{
    CGetDbConnect getDb;
    CDatabaseSql *pDb = getDb.GetDbConnect();
    if(pDb == NULL)
        return 0;
    char sql[128];
    snprintf(sql,128,"select name,level from role_info where id=%u",id);
    if(!pDb->Query(sql))
        return 0;
    char **row = pDb->GetRow();
    if(row == NULL)
        return 0;
    strcpy(name,row[0]);
    return atoi(row[1]);
}

uint32 GetRoleId(const char *name,uint8 &level)
{
    CGetDbConnect getDb;
    CDatabaseSql *pDb = getDb.GetDbConnect();
    if(pDb == NULL)
        return 0;
    char sql[128];
    snprintf(sql,128,"select id,level from role_info where name='%s'",name);
    if(!pDb->Query(sql))
        return 0;
    char **row = pDb->GetRow();
    if(row == NULL)
        return 0;
    level = atoi(row[1]);
    return atoi(row[0]);
}

void DelMission(uint32 roleId,int missId)
{
    COnlineUser   &onlineUser =  SingletonOnlineUser::instance();
    ShareUserPtr pUser = onlineUser.GetUserByRoleId(roleId);
    CUser *p = pUser.get();
    if(p != NULL)
    {
        p->DelMission(missId);
        return;
    }
    CGetDbConnect getDb;
    CDatabaseSql *pDb = getDb.GetDbConnect();
    if(pDb == NULL)
        return;
    char sql[128];
    snprintf(sql,128,"delete from role_mission where role_id=%u and mission_id=%d",roleId,missId);
    pDb->Query(sql);
}

void AddMoney(uint32 roleId,int money)
{
    COnlineUser   &onlineUser =  SingletonOnlineUser::instance();
    ShareUserPtr pUser = onlineUser.GetUserByRoleId(roleId);
    CUser *p = pUser.get();
    if(p != NULL)
    {
        p->AddMoney(money);
        return;
    }
    CGetDbConnect getDb;
    CDatabaseSql *pDb = getDb.GetDbConnect();
    if(pDb == NULL)
        return;
    char sql[128];
    snprintf(sql,128,"update role_info set money=money+%d where id=%u",money,roleId);
    pDb->Query(sql);
}

void SendSysMail(uint32 roleId,const char *info)
{
    COnlineUser   &onlineUser =  SingletonOnlineUser::instance();
    ShareUserPtr pToPtr = onlineUser.GetUserByRoleId(roleId);
    CUser *pTo = pToPtr.get();
    
    if(pTo == NULL)
    {
        CGetDbConnect getDb;
        CDatabaseSql *pDb = getDb.GetDbConnect();
        
        char sql[1024];
        sprintf(sql,"INSERT INTO role_mail (from_id,to_id,from_name,time,msg) VALUES ("\
                    "%d,%d,'%s',%lu,'%s')",
                    0,roleId,"【系统】",GetSysTime(),info);                    
        pDb->Query(sql);
    }
    else
    {
        uint8 month,day,hour,min;
        GetTime(GetSysTime(),month,day,hour,min);
        CNetMessage msg;
        msg.SetType(PRO_ONLINE_MAIL);
        msg<<0<<"【系统】"<<info<<month<<day<<hour<<min;
        CSocketServer &sock = SingletonSocket::instance();
        sock.SendMsg(pTo->GetSock(),msg);
    }
}

int UnHexify(unsigned char *obuf, const char *ibuf)
{
    unsigned char c, c2;
    int len = strlen(ibuf) / 2;
    assert(!(strlen(ibuf) %1)); // must be even number of bytes

    while (*ibuf != 0)
    {
        c = *ibuf++;
        if( c >= '0' && c <= '9' )
            c -= '0';
        else if( c >= 'a' && c <= 'f' )
            c -= 'a' - 10;
        else if( c >= 'A' && c <= 'F' )
            c -= 'A' - 10;
        else
            assert( 0 );

        c2 = *ibuf++;
        if( c2 >= '0' && c2 <= '9' )
            c2 -= '0';
        else if( c2 >= 'a' && c2 <= 'f' )
            c2 -= 'a' - 10;
        else if( c2 >= 'A' && c2 <= 'F' )
            c2 -= 'A' - 10;
        else
            assert( 0 );

        *obuf++ = ( c << 4 ) | c2;
    }

    return len;
}

void Hexify(unsigned char *obuf, const unsigned char *ibuf, int len)
{
    unsigned char l, h;

    while (len != 0)
    {
        h = (*ibuf) / 16;
        l = (*ibuf) % 16;

        if( h < 10 )
            *obuf++ = '0' + h;
        else
            *obuf++ = 'a' + h - 10;

        if( l < 10 )
            *obuf++ = '0' + l;
        else
            *obuf++ = 'a' + l - 10;

        ++ibuf;
        len--;
    }
}

bool AddPackage(uint32 roleId,SItemInstance &item)
{
    COnlineUser   &onlineUser =  SingletonOnlineUser::instance();
    ShareUserPtr p = onlineUser.GetUserByRoleId(roleId);
    CUser *pUser = p.get();
    if(pUser != NULL)
    {
        return pUser->AddPackage(item);
    }
    CGetDbConnect getDb;
    CDatabaseSql *pDb = getDb.GetDbConnect();
    if (pDb == NULL)
        return false;
    
    char sqlBuf[128];
    sprintf(sqlBuf,"select package from role_info where id=%u",roleId);
    if(!pDb->Query(sqlBuf))
        return false;
    char **row = pDb->GetRow();
    if(row == NULL)
        return false;
    
    pUser = new CUser;
    ShareUserPtr user(pUser);
    
    pUser->SetPackage(row[0]);
    pUser->SetSock(-1);
    if(pUser->AddPackage(item))
    {
        string str;
        pUser->GetPackage(str);
        boost::format fmt("update role_info set package='%1%' where id=%2%");
        fmt % str.c_str() % roleId;
        pDb->Query(fmt.str().c_str());
        return true;
    }
    return false;
}

bool Compress(uint8 *pInBuf,uint32 inLen,string &compress)
{
    uLongf outLen = compressBound(inLen);
    uint8 *res = new uint8[outLen];
    boost::scoped_ptr<uint8> ptr(res);
    int ret = compress2((Bytef*)res,(uLongf*)&outLen,(Bytef*)pInBuf,inLen,9);
    if(ret != Z_OK)
        return false;
    
    compress.resize(2*outLen);
    Hexify((uint8*)compress.c_str(),res,outLen);
    return true;
}

bool UnCompress(const char *inStr,uint8 *pOutBuf,uint32 &outLen)
{
    uint8 *str = new uint8[strlen(inStr)*2];
    boost::scoped_ptr<uint8> ptr(str);
    uLongf len = UnHexify(str,inStr);
    uLongf oLen = outLen;
    int ret = uncompress((Bytef*)pOutBuf,(uLongf*)&oLen,(Bytef*)str,len);
    if(ret != Z_OK)
        return false;
    
    return true;
}
