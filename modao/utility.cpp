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
    msg<<(uint8)0<<0<<"��ϵͳ��"<<info;
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
            pItem->type = atoi(row[1]);//����          
            pItem->level = atoi(row[2]);//����ȼ�	   
            pItem->mod = atoi(row[3]);//ģʽ           
            pItem->sex = atoi(row[4]);//�Ա�           
            pItem->fangYu = atoi(row[5]);//����       
            pItem->gongji = atoi(row[6]);//������     
            pItem->naijiu = atoi(row[7]);//�;�       
            pItem->jiage = atoi(row[8]);//�۸�        
            pItem->addFaLi = atoi(row[9]);//���ӷ���  
            pItem->addQiXue = atoi(row[10]);//������Ѫ 
            pItem->addXue = atoi(row[11]);//����Ѫ��   
            pItem->addSudu = atoi(row[12]);//�����ٶ�              
            pItem->pic = atoi(row[13]);//ͼƬ          
            pItem->name = row[14];//����         
            pItem->describe = row[15];//˵�� 
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
        static char p[] = "ͨ������";
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
    {//����
        /************
        +------+----------+--------+----------+------+----------+--------+----------+------+----------+------+
       | ���� | ����ȼ� | ������ | ǿ���ȼ� | Ʒ�� | �������� | ������ | �������� | �;� | ˵������ | ˵�� |
       +------+----------+--------+----------+------+----------+--------+----------+------+----------+------+
       |  1   |    1     |    2   |    1     |   1  |     3    |    1   | 3*������ |   2  |     2    |  Var |  
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
        ñ��,�·�,������ʽΪ��
       +------+----------+------+--------+----------+----------+------+----------+--------+----------+------+----------+------+
       | ���� | ����ȼ� | ���� | ����hp | ���ӷ��� | ǿ���ȼ� | Ʒ�� | �������� | ������ | �������� | �;� | ˵������ | ˵�� |
       +------+----------+------+--------+----------+----------+------+----------+--------+----------+------+----------+------+
       |  1   |    1     |   2  |    2   |    2     |    1     |   1  |    3     |    1   | 3*������ |   2  |    2     |  Var |
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
        Ь�Ӹ�ʽΪ:
       +------+----------+------+----------+----------+------+----------+--------+----------+------+----------+------+
       | ���� | ����ȼ� | ���� | �����ٶ� | ǿ���ȼ� | Ʒ�� | �������� | ������ | �������� | �;� | ˵������ | ˵�� |
       +------+----------+------+----------+----------+----------+------+--------+----------+------+----------+------+
       |  1   |    1     |   2  |    2     |    1     |   1  |    3     |    1   | 3*������ |   2  |    2     | Var  |
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
        ������ʽΪ��
        +------+----------+----------+--------+----------+----------+------+
        | ���� | ����ȼ� | ���ӷ��� | ������ | �������� | ˵������ | ˵�� |
        +------+----------+----------+--------+----------+----------+------+
        |   1  |    1     |    2     |    1   | 3*������ |     2    |  Var |
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
        ��ָ��ʽΪ:
       +------+----------+----------+--------+----------+----------+------+
       | ���� | ����ȼ� | ������Ѫ | ������ | �������� | ˵������ | ˵�� |
       +------+----------+----------+--------+----------+----------+------+
       |   1  |    1     |    2     |    1   | 3*������ |     2    |  Var |
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
        �����ʽΪ:
       +------+----------+----------+--------+----------+----------+------+
       | ���� | ����ȼ� | ���ӹ��� | ������ | �������� | ˵������ | ˵�� |
       +------+----------+----------+--------+----------+----------+------+
       |   1  |    1     |    2     |    1   | 3*������ |     2    |  Var |
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
    ���13IDEFΪ
      +------+----------+--------+------+
      | ���� | ������� | ����ֵ | �ȼ� |
      +------+----------+--------+------+
      |   1  |     1    |   2    |  1   |
      +------+----------+--------+------+
    ***********/
    }
    else if((pItem->type == EITPKYaoPin) || (pItem->type == EITNormalYaoPin))
    {
        /*
        +------+------+----+----+
        | ���� | ģʽ | hp | mp |
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
        | ���� | ˵������ | ˵�� |
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
            cout<<"�ع���;ã�"<<item->naijiu<<endl;
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
    "����",
    "ϵͳ",
    "��ֵ",
    "����",
    "�ͷ�",
    "����",
    "����",
    "����",
    "����",
    "����",
    "@һ��",
    "@����",
    "@����",
    "@����",
    "@����",
    "@����",
    "@����",
    "@����",
    "@����",
    "@ʮ��",
    "��",
    "��"
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
    {//����
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
    {//�﹥��
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
//������Ʒ���˵�����Ʒid��ÿ��+1
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
                    0,pUser->GetRoleId(),"��ϵͳ��",GetSysTime(),info);                    
        pDb->Query(sql);
    }
    else
    {
        CNetMessage msg;
        msg.SetType(PRO_ONLINE_MAIL);
        uint8 month,day,hour,min;
        GetTime(GetSysTime(),month,day,hour,min);
        msg<<0<<"��ϵͳ��"<<info<<month<<day<<hour<<min;
        CSocketServer &sock = SingletonSocket::instance();
        sock.SendMsg(pUser->GetSock(),msg);
    }
}

static const char* IllegalChatMsg[] = 
{
    "����","�����","�½�","ϣ��OL","����","����","Ԩ������","������","̫�������","��ը","ʾ��","�Է�","��̨","��̨","����","�����","����","���Ʒ���","�����콾","û����Ȩ","���ֹ�","��","�½�����","���ض���","����","������","������","Բ��","����","���־","ת����","����","�ֹ�","������","����������","������","�˵�","���������","������","����֮��","��������","�Ļ������","����","���Ԫ","������","������","�����¼�","����","������","����","��������","��������","����","����Ź��","�����ί","���̨��","�й��߹�","�й�С��","��ū","�й����","��������","����","��å����","�ڶ��˵�","������","���ڷ���","������","Բ����","����֮��","������","��ƽ����","��������","�󷨴�","�󷨵���","֧����","����ר��","������Ȩ","ר������","��������","�󷨺鴫","ë��","�����ֻ�","����","���׹���","������߷���","���ַ�","˭�����й�","�����˼�","����Ǭ��","����ʱ��","���⻤��","�鷢����","����","����","����","ӥ��","�ӽ�ѧԺ����","ȫ���˵�","��ʳ����","άȨ����","��������","�й�����","�й�����","��������","���޼���","ʦ������","��������","�л���������","�׹��ж�","��������","��å����","��������","������","�й�����","�й�����","�й���Ȩ","�й��Ⱥ�","��������","�й�����","��å�޲���","�й�ר��","�����ܿ�","����","��Բ��","�桢�ơ���","����","����","����","�Խ�","һ��ר��","ѩɽʨ��","������Ȩ","�ض�","�԰�","����","�Խ���","�Է���","�������","��ŮĤ","͸��","AVƬ","AVŮ","�Լ�ҩ","��Һ","����","��Ѩ","��ͥ","����","�ջ���","��Ѩ","��Ѩ","��Ѩ","��Ѩ","Ů��","����","ƨ��","�޽�","�ⶴ","ɫ����վ","ɧѨ","��ɫ","����","����","СѨ","ѨѨ","����","�����","��Ů","��ˮ","��Ѩ","��֭","Ӧ��Ů��","��ͷ","����","��Һ","�羫","����","����","��ë","��Ű��","��ɫ��վ","������վ","����С˵","������ѧ","����Ӱ��","��������","����ͼƬ","���","����","����","��Ѩ","�ؽ�","����","�ڽ�","����","�ּ�","��","Ⱥ��","Ⱥ��","�齻","��","����","��Ѩ","����","���ɶ�","����","�ռ�","Ԯ��","������","������Ʒ","��������","����","�����","�۷��","�����к�","����","��Ů��","������Ű","���˵�Ӱ","�������","����ͼ","������","��ͼ","����Ƭ","͵��","����","�԰�ͼ","��ͼ","����","�ջ�","��ɫӰ��","��ɫ��Ӱ","��ɫС˵","��ɫ��ѧ","��ɫͼƬ","��ɫ����","��̨С��","Ӧ�м�Ů","������̳","����ѵ�Ӱ","С��Ӱ","��ή","��ҩ","����","����","��Ь","����","������","������","������","��Ů����","�����㻤","��������","����С��","��","��˽ͼƬ","����д��","˿��д��","�¹�ϣ","����","����","��Ů","���ٸ�","����","��н��Ԫ","������","̨��18DY��Ӱ","H����","��ƿ÷","����ɫ����ַ��ȫ","H����","����","�����","��������","�ֿڸ�","AƬ","ëƬ","��Ƭ","����","��ɧ","�߳�","����","����","�鷿","����","˫�ȼ�Ľ���","�ٶ�����","Xҹ����","�д�","һҹ��","���Կ���","ҹ����","�ɲ��һ�","����","С�ܵ�","����","ɫ��","���ϲ�","���","����","����","����","����","����","����","���","��ȫ��","����","�Ҳ�","�Ҽ�","����","����","����","ǿ��","����","����","���","��","ɵB","ɵ��","ɵ��","ɧ��","��B","���","����","���","ɧ��","����","����","ƨ��","��8","����","����","����","����","����","��B","����","�˽�","����","��ͷ","�ͷ","TMD","�ͽ�","�׳�","�϶�","����","�侫","����","�u��","���˵�","���Ǽ�","����Ѽ","�û�","�ñ�","����","��ʺ","��Ժ","�ǹ�","������","����","����","�����","�ҿ�","������","������","����","����","���","������","��������","������","�����","����","������","�¼�","�ڿ�","�����̵�","�������","�����","��������˵�","����","�����̵�","�����","�����","��ĸ","��ζ","�������","�������","�������","�������","�������","���￨��","������","�����","�����","�����","��ƨ","����","���","��","ǿ����","����","����","����","����","����","����","������","���㹫","����ĸ","������ĸ","������ʦ","�����","������","���B","���B","��x��","���߰�","����GM","�����ͷ�","����CS","������","������ĸ","������","������","�ɻ���","��B","����","����","��ĸ","����","����","����","ʻ�㹫","ʻ��ĸ","ʻ����ĸ","ʻ����ʦ","ʻ���","ʻ����","��78","����ȫ��","������ĸ","��������","������","��������","�ٻ���","����","����","��������","������ĸ","������ĸ","����","����","��8","����","����","����","������","������","�����","���","�����Ӱ","ʮ�˽�","18��","����","˿��","д��","����","��������",
    "���","H��Ӱ","������","��ľ��˾","���ȡ�����˹","��¥","������־","ANDREWBLAKE","�ش���","PRIVATE","�պ��Ƿ���","��ŮҰ��ȭ","��Ů�Զ���","͵������","αװ��Ů","��������","����","��ʿ24��","������ɫ","β��","�Ը��˿�","ҹ��Ůģ��","��������","��Ů�ľ�","����","����С˵","������Ů","�˸�����","��Ů�߹�","�Գ�","�̽�","��Ѽ���Ƭ","�˸�����","�ձ�Ůѧ��ͼƬ","����Ůֳ��","���Ů�����ӹ�","�Խ���Ӱ","����ͼ","��Ů��ο","��Ů�Խ�","��Ů����","�ٴ�Ů��","��Ů�ٱ�","�Խ�ͼ","�ձ���Ů","˿������","��Ů�ҽ�","����д��","��Ů�侫","�ٱ�","��������","�޽����","����","�޽�","�����������","��������","�ֻ�����","��Ů�߹�ͼƬ","�߹���Ƭ","��Ůȫ��ͼ","��Ů����","ɫ��С˵","¶���߹�","���Ĳظ�","��Ů����","˿���Ӱ","�칫������С˵","����С˵","ɫ����ǿ��","����ѧС˵","����ͼ���","��������Ů","�����ӽ�","��ӰԺ","ɫ��С˵","ɫˬˬ����ѧ","ʵ������","��Ů����ͼ","��Ů�Ҳ�","ȫ����Ů","��ѽ�����Ӱ","��Ů��ͼ","ɫʮ��","����Ů����","�츾�߳�","������������","��ɫ�ձ���ͨ����","�������߹�","��Ů����ͼ","СѨ���","����СѨ","��Ѩ��ͼ��","ɧ����Ѩ","��Ů��Ѩ","С��Ѩ","��Ѩ","Ů�˵�b","��Ů��Ѩ","Ů�˵Ķ�","������","�޼�","������ͼ","�ձ�","ɧ��","������","������","����","����","��Ѷ","���Ĺ㳡","���»��㳡","81386360"
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
                    0,roleId,"��ϵͳ��",GetSysTime(),info);                    
        pDb->Query(sql);
    }
    else
    {
        uint8 month,day,hour,min;
        GetTime(GetSysTime(),month,day,hour,min);
        CNetMessage msg;
        msg.SetType(PRO_ONLINE_MAIL);
        msg<<0<<"��ϵͳ��"<<info<<month<<day<<hour<<min;
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
