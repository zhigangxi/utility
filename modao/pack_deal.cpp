#include "pack_deal.h"
#include "singleton.h"
#include "protocol.h"
#include "net_msg.h"
#include "singleton.h"
#include "database.h"
#include "online_user.h"
#include "scene_manager.h"
#include "npc_manager.h"
#include "call_script.h"
#include "md5.h"
#include "script_call.h"
#include "main.h"
#include "ini_file.h"
#include "huo_dong.h"
#include "script_call.h"
#include <boost/bind.hpp>
#include <boost/format.hpp>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

extern CMainClass *gpMain;
extern char *gConfigFile;

CPackageDeal::CPackageDeal():
m_socketServer(SingletonSocket::instance())
,m_onlineUser(SingletonOnlineUser::instance())
,m_npcManager(SingletonNpcManager::instance())
,m_sceneManager(SingletonSceneManager::instance())
,m_monsterManager(SingletonMonsterManager::instance())
,m_bangPaiMgr(SingletonCBangPaiManager::instance())
{
    CDespatchCommand &despatch = SingletonDespatch::instance();

    SCommand cmdFun[] = {
        {PRO_USER_LOGIN,boost::bind(&CPackageDeal::UserLogin,this,_1,_2)},
#ifndef QQ
        {PRO_REG_USER,boost::bind(&CPackageDeal::RegUser,this,_1,_2)},
#endif
        {PRO_CREATE_ROLE,boost::bind(&CPackageDeal::CreateRole,this,_1,_2)},
        {PRO_SELECT_ROLE,boost::bind(&CPackageDeal::SelectRole,this,_1,_2)},
        {PRO_ROLE_EQUIPMENT,boost::bind(&CPackageDeal::GetEquipment,this,_1,_2)},
        {PRO_ROLE_PACKAGE,boost::bind(&CPackageDeal::GetPackage,this,_1,_2)},
        {PRO_ROLE_MOVE,boost::bind(&CPackageDeal::RoleMove,this,_1,_2)},
        {PRO_OPEN_INTERACT,boost::bind(&CPackageDeal::OpenNpcInteract,this,_1,_2)},
        {PRO_INTERACT,boost::bind(&CPackageDeal::NpcInteract,this,_1,_2)},
        {PRO_OUTFIT,boost::bind(&CPackageDeal::EquipItem,this,_1,_2)},
        {PRO_GET_ITEM_INFO,boost::bind(&CPackageDeal::GetItemInfo,this,_1,_2)},
        {PRO_USER_TEAM,boost::bind(&CPackageDeal::UserTeam,this,_1,_2)},
        {PRO_UPDATE_CHAR,boost::bind(&CPackageDeal::UpdateChar,this,_1,_2)},
        {PRO_PLAYER_INFO,boost::bind(&CPackageDeal::GetPlayerInfo,this,_1,_2)},
        {PRO_USER_PK,boost::bind(&CPackageDeal::PlayerPk,this,_1,_2)},
        {PRO_PLYAER_MATCH,boost::bind(&CPackageDeal::PlayerMatch,this,_1,_2)},
        {PRO_NEAR_PLAYER_LIST,boost::bind(&CPackageDeal::NearPlayerList,this,_1,_2)},
        {PRO_QUERY_PET,boost::bind(&CPackageDeal::QueryPet,this,_1,_2)},
        {PRO_UPDATE_PET,boost::bind(&CPackageDeal::PetOption,this,_1,_2)},
        {PRO_TASK_LIST,boost::bind(&CPackageDeal::GetMissionList,this,_1,_2)},
        {PRO_TASK_INFO,boost::bind(&CPackageDeal::GetMissionInfo,this,_1,_2)},
        {PRO_USER_SKILL,boost::bind(&CPackageDeal::GetUserSkill,this,_1,_2)},
        {PRO_PET_SKILL,boost::bind(&CPackageDeal::GetPetSkill,this,_1,_2)},
        {PRO_MSG_CHAT,boost::bind(&CPackageDeal::UserChat,this,_1,_2)},
        {PRO_OTHER_ITEM_INFO,boost::bind(&CPackageDeal::GetOtherUserItemInfo,this,_1,_2)},
        {PRO_HOT_LIST,boost::bind(&CPackageDeal::GetHotList,this,_1,_2)},
        {PRO_ADD_HOT,boost::bind(&CPackageDeal::AddHot,this,_1,_2)},
        {PRO_DEL_HOT,boost::bind(&CPackageDeal::DelHot,this,_1,_2)},
        {PRO_ONLINE_MAIL,boost::bind(&CPackageDeal::OnlineMail,this,_1,_2)},
        {RPO_QUERY_MAIL,boost::bind(&CPackageDeal::QueryMail,this,_1,_2)},
        {PRO_PSHOP_LIST,boost::bind(&CPackageDeal::UserShop,this,_1,_2)},
        {PRO_PSHOP_BUY,boost::bind(&CPackageDeal::BuyShopItem,this,_1,_2)},
        {PRO_PSHOP_PUT,boost::bind(&CPackageDeal::PutItemToShop,this,_1,_2)},
        {PRO_PSHOP_DETAIL,boost::bind(&CPackageDeal::ShopItemInfo,this,_1,_2)},
        {PRO_PLAYER_DETAIL,boost::bind(&CPackageDeal::GetUserDetail,this,_1,_2)},
        {PRO_SAFE_TRADE,boost::bind(&CPackageDeal::SafeTrade,this,_1,_2)},
        {PRO_CLOSE_PLAYER,boost::bind(&CPackageDeal::CloseUser,this,_1,_2)},
        {PRO_BANGPAI,boost::bind(&CPackageDeal::BangPai,this,_1,_2)},
        {PRO_UPDATE_PACK,boost::bind(&CPackageDeal::UpdatePackage,this,_1,_2)},
        {PRO_LIST_ESHOP,boost::bind(&CPackageDeal::GetEShopItem,this,_1,_2)},
        {PRO_BUY_EITEM,boost::bind(&CPackageDeal::BuyEShopItem,this,_1,_2)},
        {PRO_JUMP_SCENE,boost::bind(&CPackageDeal::UserJumpOk,this,_1,_2)},
        {PRO_UPDATE_NPC,boost::bind(&CPackageDeal::GetNpcState,this,_1,_2)},
        {PRO_EQUIP_ENFORCE,boost::bind(&CPackageDeal::QiangHuaEquip,this,_1,_2)},
        {PRO_ENFORCE_QUERY,boost::bind(&CPackageDeal::QueryQiangHua,this,_1,_2)},
        {PRO_EQUIP_BLUED,boost::bind(&CPackageDeal::MakeBlueItem,this,_1,_2)},
        {PRO_EQUIP_GREEND,boost::bind(&CPackageDeal::MakeGreenItem,this,_1,_2)},
        {PRO_EQUIP_REFINE,boost::bind(&CPackageDeal::SelectGreenItemAttr,this,_1,_2)},
        {PRO_ORNAMENT_CARVE,boost::bind(&CPackageDeal::SelectGreenItemAttr,this,_1,_2)},
        {PRO_ORNAMENT_SETTING,boost::bind(&CPackageDeal::XiangQian,this,_1,_2)},
        {PRO_SKILL_DESC,boost::bind(&CPackageDeal::QuerySkillDesc,this,_1,_2)},
        {PRO_SWITCH_CHANNEL,boost::bind(&CPackageDeal::ChatChannel,this,_1,_2)},
        {PRO_EASY_RECOVER,boost::bind(&CPackageDeal::EasyUseYaoPin,this,_1,_2)},
        {PRO_SWITCH_INFO,boost::bind(&CPackageDeal::SwitchInfo,this,_1,_2)},
        {PRO_OTHER_PET,boost::bind(&CPackageDeal::QueryPetInfo,this,_1,_2)},
        {PRO_MY_BANG,boost::bind(&CPackageDeal::MyBangPai,this,_1,_2)},
        {PRO_CHANGE_FACE,boost::bind(&CPackageDeal::ChangeUserFace,this,_1,_2)},
        {PRO_CANCEL_TASK,boost::bind(&CPackageDeal::CancelTask,this,_1,_2)},
        {PRO_ITEM_DESC,boost::bind(&CPackageDeal::ItemDesc,this,_1,_2)},
        {PRO_CHARGE,boost::bind(&CPackageDeal::Charge,this,_1,_2)},
        {PRO_QUERY_RLOC,boost::bind(&CPackageDeal::QuerySavePos,this,_1,_2)},
        {PRO_CLIENT_PATH,boost::bind(&CPackageDeal::QueryPath,this,_1,_2)},
        {PRO_AVAILABLE_TASK,boost::bind(&CPackageDeal::AvailableTask,this,_1,_2)},
        {PRO_SCENE_POS,boost::bind(&CPackageDeal::GetScenePos,this,_1,_2)},
        {PRO_SPEC_CHAT,boost::bind(&CPackageDeal::SpecChat,this,_1,_2)},
        {MSG_ANSWER_XING,boost::bind(&CPackageDeal::UserAnswer,this,_1,_2)},
        {MSG_CLIENT_MONSTER_BATTLE,boost::bind(&CPackageDeal::MonsterBattle,this,_1,_2)},
        {MSG_QUERY_SCENE,boost::bind(&CPackageDeal::QueryScene,this,_1,_2)},
        {MSG_CLIENT_ITEM_DEF,boost::bind(&CPackageDeal::QueryItem,this,_1,_2)},
        {MSG_SERVER_HEART_BEAT,boost::bind(&CPackageDeal::HeartBeat,this,_1,_2)},
        {MSG_LIST_SERVER,boost::bind(&CPackageDeal::GetServerList,this,_1,_2)},
        {MSG_CLIENT_LIST_TITLE,boost::bind(&CPackageDeal::GetTitle,this,_1,_2)},
        {MSG_CLIENT_USE_TITLE,boost::bind(&CPackageDeal::UseTitle,this,_1,_2)},
        {MSG_CLIENT_SYS_MAIL,boost::bind(&CPackageDeal::SysMail,this,_1,_2)},
        {MSG_SERVER_QUERY_BILL,boost::bind(&CPackageDeal::CXGongGao,this,_1,_2)},
        {MSG_CLIENT_LIST_SHITU,boost::bind(&CPackageDeal::ShiTu,this,_1,_2)},
        {MSG_CLIENT_DO_OBJECT,boost::bind(&CPackageDeal::DoObject,this,_1,_2)},
        {MSG_CLIENT_XIANGQIAN,boost::bind(&CPackageDeal::KaiJiaXiangQian,this,_1,_2)},
        {MSG_CLIENT_LIST_FUQI,boost::bind(&CPackageDeal::FuQi,this,_1,_2)},
        {MSG_CLIENT_DEL_CHAR,boost::bind(&CPackageDeal::DelRole,this,_1,_2)},
        {MSG_CLIENT_UNDEL_CHAR,boost::bind(&CPackageDeal::CancelDelRole,this,_1,_2)},
        {MSG_SERVER_LIST_BLACK,boost::bind(&CPackageDeal::IgnoreList,this,_1,_2)},
        {MSG_CLIENT_ADD_BLACK,boost::bind(&CPackageDeal::AddIgnore,this,_1,_2)},
        {MSG_CLIENT_DEL_BLACK,boost::bind(&CPackageDeal::DelIgnore,this,_1,_2)},
        {MSG_SERVER_INVITE_ALLY,boost::bind(&CPackageDeal::WWAgreeYaoQing,this,_1,_2)},
#ifdef QQ
        {MSG_QQ_LOGIN,boost::bind(&CPackageDeal::QQServerLogin,this,_1,_2)},
#endif
        {MSG_MGR,boost::bind(&CPackageDeal::ServerMgr,this,_1,_2)},
        {MSG_SERVER_XINSHI,boost::bind(&CPackageDeal::XinShi,this,_1,_2)},
        {MSG_CLIENT_LEARN_SKILL,boost::bind(&CPackageDeal::LearnSkill,this,_1,_2)},
        {MSG_SERVER_PROMPT,boost::bind(&CPackageDeal::YeWaiShiYao,this,_1,_2)},
        {GUANZHAN_ENTER_BATTLE,boost::bind(&CPackageDeal::GuanZhan,this,_1,_2)},
        {LEAVE_GUANZHAN,boost::bind(&CPackageDeal::LeaveGuanZhan,this,_1,_2)},
        {MSG_TEXT_TITLE,boost::bind(&CPackageDeal::QueryTextTitle,this,_1,_2)},
        {MSG_USE_TEXT_TITLE,boost::bind(&CPackageDeal::UseTextTitle,this,_1,_2)},
        {MSG_NEW_USER_GIFT,boost::bind(&CPackageDeal::GetNewUserGift,this,_1,_2)},
        {MSG_NORMAL_USER_GIFT,boost::bind(&CPackageDeal::GetNormalGift,this,_1,_2)}
        
    };
    m_socketServer.ObserveConnectClose(boost::bind(&CPackageDeal::OnSockClose,this,_1));
    despatch.AddCommandDeal(cmdFun,sizeof(cmdFun)/sizeof(SCommand));
    
    //m_pAdminScript = new CCallScript(999999999);
    
    m_pSLMsg = new CNetMessage;
    m_readSLTime = 0;
    if(!m_bangPaiMgr.Init())
    {
        exit(0);
    }
    
	CGetDbConnect getDb;
    CDatabaseSql *pDb = getDb.GetDbConnect();
    if ((pDb != NULL)
        && (pDb->Query("select id,des from skill")))
    {
        char **row;
        while ((row = pDb->GetRow()) != NULL)
        {
            m_skillDescMap.insert(make_pair(atoi(row[0]),row[1]));
        }
    }
    
    if((pDb != NULL) && (pDb->Query("select item_id,level,money,page from shop_item order by id")))
    {
        char **row;
        EShopItem item;
        while((row = pDb->GetRow()) != NULL)
        {
            item.id = atoi(row[0]);
            item.level = atoi(row[1]);
            item.money = atoi(row[2]);
            item.page = atoi(row[3]);
            m_eShopItemList.push_back(item);
        }
    }
#ifdef QQ
    m_qqLoginServer = CIniFile::GetValue("server_ip","qq_server",gConfigFile);
    string port = CIniFile::GetValue("server_port","qq_server",gConfigFile);
    m_qqLoginPort = atoi(port.c_str());
    m_qqServerSocket = 0;
#else
    string user = CIniFile::GetValue("username","login_db",gConfigFile);
    string password = CIniFile::GetValue("password","login_db",gConfigFile);
    string host = CIniFile::GetValue("host","login_db",gConfigFile);
    string db = CIniFile::GetValue("dbname","login_db",gConfigFile);
    string port = CIniFile::GetValue("port","login_db",gConfigFile);
    if(!m_loginDb.Connect(user.c_str(),password.c_str(),host.c_str(),
        db.c_str(),atoi(port.c_str())))
    {
        cout<<"connect login db error"<<endl;
        exit(0);
    }
#endif
}

#define GET_MSG if(pMsg == NULL)\
                    return;\
                CNetMessage &msg = *pMsg;
#define GET_USER ShareUserPtr ptr = m_onlineUser.GetUserBySock(sock);\
                 CUser *pUser = ptr.get();\
                 if((pUser == NULL) || (pUser->GetRoleId() == 0))\
                    return;

void CPackageDeal::QuerySkillDesc(CNetMessage *pMsg,int sock)
{
	GET_MSG
    GET_USER
    
    uint16 id = 0;
    msg>>id;
    std::map<uint16,string>::iterator i = m_skillDescMap.find(id);
    if(i != m_skillDescMap.end())
    {
        msg<<i->second;
        m_socketServer.SendMsg(pUser->GetSock(),msg);
    }
}

void CPackageDeal::MD5(string &str)
{
    uint8 out[32] = {0};
    md5((uint8*)str.c_str(),str.size(),out);
    str.clear();
    HexToStr(out,16,str);
}

void CPackageDeal::SetVerInfo(string &version,string &forceUpdate,string &updateMsg,string &updateUrl)
{
    m_version = version;
    m_forceUpdate = atoi(forceUpdate.c_str());
    m_updateMsg = updateMsg;
    m_updateUrl = updateUrl;
}

#ifndef QQ
uint32 CPackageDeal::LoginShareUser(string &name,string &passwd)
{
#ifdef NO_REG_USER
    return 0;
#endif
    char buf[256];
    snprintf(buf,255,"select passwd from t_user_basic where login_name='%s'"
        ,name.c_str());
        
    bool insertUser = false;
    {
        boost::mutex::scoped_lock lk(m_dbLock);
        if(m_loginDb.Query(buf))
        {
            char **row = m_loginDb.GetRow();
            if((row != NULL) && (passwd == row[0]))
            {
                insertUser = true;
            }
        }
    }
    if(insertUser)
    {
        CGetDbConnect getDb;
        CDatabaseSql *pDb = getDb.GetDbConnect();
        //boost::format fmt("INSERT INTO user_info (name,password,type,reg_time) VALUES('%1%','%2%',%3%,%4%)");
        snprintf(buf,255,"INSERT INTO user_info (name,password) VALUES('%s','%s')",
            name.c_str(),passwd.c_str());
        if(pDb->Query(buf))
            return pDb->InsertId();
    }
    return 0;
}
#endif

struct SRoleDel
{
    uint32 id;
    time_t dT;//删除时间
};

struct SSortRole
{
    bool operator()(const SRoleDel &r1,const SRoleDel &r2)
    {
        return r1.id < r2.id;
    }
};

const int CAN_SAVE_TIME = 7*24*3600;

#ifdef QQ
void CPackageDeal::AddQQLoginUser(string &name,int sock)
{
    SQQLoginUser qqUser;
    qqUser.name = name;
    qqUser.sock = sock;
    qqUser.loginTime = GetSysTime();
    boost::mutex::scoped_lock lk(m_dbLock);
    for(list<SQQLoginUser>::iterator i = m_qqLoginUserList.begin(); i != m_qqLoginUserList.end(); i++)
    {
        if(i->name == name)
        {
            i->sock = sock;
            i->loginTime = GetSysTime();
            return;
        }
    }
    m_qqLoginUserList.push_back(qqUser);
}
int CPackageDeal::FindQQLoginUser(string &name)
{
    boost::mutex::scoped_lock lk(m_dbLock);
    for(list<SQQLoginUser>::iterator i = m_qqLoginUserList.begin(); i != m_qqLoginUserList.end(); i++)
    {
        if(i->name == name)
        {
            return i->sock;
        }
    }
    return 0;
}

void CPackageDeal::DelQQLoginUser(int sock)
{
    boost::mutex::scoped_lock lk(m_dbLock);
    for(list<SQQLoginUser>::iterator i = m_qqLoginUserList.begin(); i != m_qqLoginUserList.end(); i++)
    {
        if(i->sock == sock)
        {
            m_qqLoginUserList.erase(i);
            break;
        }
        else if(GetSysTime() - i->loginTime > 24*3600)
        {
            m_qqLoginUserList.erase(i);
            break;
        }
    }
}

void CPackageDeal::QQServerLogin(CNetMessage *pMsg,int sock)
{
    if(sock != m_qqServerSocket)
        return;
    GET_MSG
    

    /*PoolString *str = pMsg->GetMsgData();
    for(int i = 0; i < (int)str->length(); i++)
    {
        cout<<(int)str->at(i)<<" ";
    }
    cout<<endl;*/

    string name;
    msg>>name;
    
    sock = FindQQLoginUser(name);
    if(sock == 0)
        return;
    
    msg.ReWrite();
    msg.SetType(PRO_USER_LOGIN);
    CGetDbConnect getDb;
    CDatabaseSql *pDb = getDb.GetDbConnect();
    string sql = "select id,role0,role1,role2,role3,role4,role5,money,"\
        "del_time0,del_time1,del_time2,del_time3,del_time4,del_time5 from user_info where name='" + name + "'";
        
    if(pDb == NULL)
    {
        msg<<PRO_ERROR<<(uint8)0;
        m_socketServer.SendMsg(sock,msg);
        return;
    }
    if(!pDb->Query(sql.c_str()))
    {
        return;
    }
    char **row = pDb->GetRow();
    if(row == NULL)
    {
        boost::format fmt("INSERT INTO user_info (name,reg_time) VALUES('%1%',%2%)");
        fmt % name % GetSysTime();
        pDb->Query(fmt.str().c_str());
        msg<<PRO_SUCCESS<<(uint8)0;
        m_socketServer.SendMsg(sock,msg);
        m_onlineUser.AddUser(sock,pDb->InsertId());
        return;
    }
    
    uint32 userId = (uint32)atoi(row[0]);
    CUser *pUser = m_onlineUser.AddUser(sock,userId);
    if(pUser == NULL)
    {
        msg<<PRO_ERROR<<(uint8)0;
        m_socketServer.SendMsg(sock,msg,true);
        return;
    }
    
    //pUser->SetTongBao(atoi(row[7]));
    msg<<PRO_SUCCESS<<userId;
    SRoleDel roleId[MAX_ROLE_NUM] = {{0,0}};
    int i = 0;
    uint8 roleNum = 0;
    uint32 id;
    time_t dT;
    
    for (; i < MAX_ROLE_NUM; i++)
    {
        id = atoi(row[1+i]);
        dT = atoi(row[8+i]);
        if((id != 0) && ((dT == 0) || (GetSysTime() - dT < CAN_SAVE_TIME)))
        {
            roleId[roleNum].id = id;
            roleId[roleNum].dT = dT;
            pUser->AddRole(roleId[roleNum].id);
            roleNum++;
        }
    }
    
    if(roleNum > 0)
    {
        SSortRole sortRole;
        std::sort(roleId,roleId+roleNum,sortRole);
    }
    //pUser->SetRole(roleId);
    msg<<roleNum;
    for (i = 0; i < roleNum; i++)
    {                               //0     1   2       3   4    5   6
        sql = (boost::format("select id ,name ,head ,xiang,level,sex,equipment from "\
            "role_info where id=%1%") % roleId[i].id).str();
        if((roleId[i].id != 0)
            && pDb->Query(sql.c_str()) 
            && (row =pDb->GetRow()) != NULL)
        {
            SItemInstance equip[EETShouZhuo2+1];
            memset(equip,0,sizeof(equip));
            if(row[6] != NULL)
            {
                StrToHex(row[6],(uint8*)equip,sizeof(equip));
                //uint32 len = 0;
                //UnCompress(row[6],(uint8*)equip,&len);
            }
            msg<<atoi(row[0])<<row[1]<<(uint8)atoi(row[2])<<(uint8)atoi(row[3])<<(uint8)atoi(row[4])
                <<(uint8)atoi(row[5])
                //WEAPON | HELMET | ARMOR
                <<(uint16)equip[EETWuQi].tmplId
                <<(uint16)equip[EETMaoZi].tmplId
                <<(uint16)equip[EETKuiJia].tmplId;
            if(roleId[i].dT != 0)
            {
                msg<<(uint8)1<<(uint32)(CAN_SAVE_TIME-(GetSysTime() - roleId[i].dT));
            }
            else
            {
                msg<<(uint8)0<<0;
            }
        }
        else
        {
            msg<<0;
        }
    }
    m_socketServer.SendMsg(sock,msg);
}

void CPackageDeal::QQLogin(string &name,string &password,int sock)
{
    if(m_qqServerSocket == 0)
    {
        struct sockaddr_in addr;
        memset(&addr,0,sizeof(addr));
        addr.sin_addr.s_addr    = inet_addr(m_qqLoginServer.c_str());
        addr.sin_port           = htons(m_qqLoginPort);
        addr.sin_family         = AF_INET;
        int sock                = socket(AF_INET,SOCK_STREAM,0);
        if(sock < 0)
        {
            close(sock);
        }
        struct timeval timev;
        timev.tv_sec = 6;
        timev.tv_usec = 0;
        setsockopt(sock,SOL_SOCKET,SO_RCVTIMEO,&timev,sizeof(timev));
        setsockopt(sock,SOL_SOCKET,SO_SNDTIMEO,&timev,sizeof(timev));
        if(connect(sock,(sockaddr*)&addr,sizeof(addr)) != 0)
        {
            close(sock);
            cout<<"connect login server error"<<endl;
            cout<<strerror(errno)<<endl;
            return;
        }
        m_qqServerSocket = sock;
        m_socketServer.SetSock(m_qqServerSocket);
        m_socketServer.AddEvent(m_qqServerSocket);
    }
    CNetMessage msg;
    msg.SetType(PRO_USER_LOGIN);
    msg<<name<<password;
    m_socketServer.SendMsg(m_qqServerSocket,msg);
    AddQQLoginUser(name,sock);
}
#endif
void CPackageDeal::MDLogin(string &name,string &password,int sock)
{
    CNetMessage msg;
    msg.SetType(PRO_USER_LOGIN);
    CGetDbConnect getDb;
    CDatabaseSql *pDb = getDb.GetDbConnect();
    string sql = "select password,id,role0,role1,role2,role3,role4,role5,money,"\
        "del_time0,del_time1,del_time2,del_time3,del_time4,del_time5 from user_info where name='" + name + "'";
        
    if((pDb == NULL) || !pDb->Query(sql.c_str()))
    { 
        msg<<PRO_ERROR<<(uint8)0;
        m_socketServer.SendMsg(sock,msg);
        return;
    }
    
    char **row = pDb->GetRow();
    if(row == NULL)
    {
#ifdef QQ
        msg<<PRO_ERROR<<(uint8)0;
        m_socketServer.SendMsg(sock,msg);
        return;
#else
        uint32 userId = LoginShareUser(name,password);
        if(userId != 0)
        {
            msg<<PRO_SUCCESS<<userId;
            for (uint8 i = 0; i < MAX_ROLE_NUM; i++)
            {
                msg<<0;
            }
            m_onlineUser.AddUser(sock,userId);
        }
        else
        {
            msg<<PRO_ERROR<<(uint8)0;
        }
        m_socketServer.SendMsg(sock,msg);
        return;
#endif
    }
    if(password == row[0])
    {
        uint32 userId = (uint32)atoi(row[1]);
        CUser *pUser = m_onlineUser.AddUser(sock,userId);
        if(pUser == NULL)
        {
            msg<<PRO_ERROR<<(uint8)0;
            m_socketServer.SendMsg(sock,msg,true);
            return;
        }
        
        //pUser->SetTongBao(atoi(row[8]));
        msg<<PRO_SUCCESS<<userId;
        SRoleDel roleId[MAX_ROLE_NUM] = {{0}};
        int i = 0;
        uint8 roleNum = 0;
        uint32 id;
        time_t dT;
        
        for (; i < MAX_ROLE_NUM; i++)
        {
            id = atoi(row[2+i]);
            dT = atoi(row[9+i]);
            if((id != 0) && ((dT == 0) || (GetSysTime() - dT < CAN_SAVE_TIME)))
            {
                roleId[roleNum].id = id;
                roleId[roleNum].dT = dT;
                pUser->AddRole(roleId[roleNum].id);
                roleNum++;
            }
        }
        
        if(roleNum > 0)
        {
            SSortRole sortRole;
            std::sort(roleId,roleId+roleNum,sortRole);
        }
        //pUser->SetRole(roleId);
        msg<<roleNum;
        for (i = 0; i < roleNum; i++)
        {                               //0     1   2       3   4    5   6
            sql = (boost::format("select id ,name ,head ,xiang,level,sex,equipment from "\
                "role_info where id=%1%") % roleId[i].id).str();
            if((roleId[i].id != 0)
                && pDb->Query(sql.c_str()) 
                && (row =pDb->GetRow()) != NULL)
            {
                SItemInstance equip[EETShouZhuo2+1];
                memset(equip,0,sizeof(equip));
                if(row[6] != NULL)
                {
                    StrToHex(row[6],(uint8*)equip,sizeof(equip));
                    //uint32 len = 0;
                    //UnCompress(row[6],(uint8*)equip,&len);
                }
                msg<<atoi(row[0])<<row[1]<<(uint8)atoi(row[2])<<(uint8)atoi(row[3])<<(uint8)atoi(row[4])
                    <<(uint8)atoi(row[5])
                    //WEAPON | HELMET | ARMOR
                    <<(uint16)equip[EETWuQi].tmplId
                    <<(uint16)equip[EETMaoZi].tmplId
                    <<(uint16)equip[EETKuiJia].tmplId;
                if(roleId[i].dT != 0)
                {
                    msg<<(uint8)1<<(uint32)(CAN_SAVE_TIME-(GetSysTime() - roleId[i].dT));
                }
                else
                {
                    msg<<(uint8)0<<0;
                }
            }
            else
            {
                msg<<0;
            }
        }
        m_socketServer.SendMsg(sock,msg);
    }
    else
    {
        msg<<PRO_ERROR<<(uint8)0;
        m_socketServer.SendMsg(sock,msg);
        close(sock);
        OnSockClose(sock);
    }
}


void CPackageDeal::UserLogin(CNetMessage *pMsg,int sock)
{
    GET_MSG
    ShareUserPtr pUserOld = m_onlineUser.GetUserBySock(sock);
    CUser *pOld = pUserOld.get();
    if(pOld != NULL)
    {
        //close(pOld->GetSock());
        //OnSockClose(pOld->GetSock());
        return;
    }
    string version;
    string name;
    string password;
    msg>>name>>password>>version;
#ifdef QQ
    uint8 adminLogin = 0;
    msg>>adminLogin;
#endif
    msg.ReWrite();
    msg.SetType(PRO_USER_LOGIN);
    
    if((version.length() == 8) && (atoi((version.c_str() + 5)) < atoi(m_version.c_str())))
    {
        //cout<<version<<endl<<m_version<<endl;
        CNetMessage vMsg;
        vMsg.SetType(PRO_SERVER_VERSION);
        vMsg<<m_forceUpdate<<m_updateMsg<<m_updateUrl;
        m_socketServer.SendMsg(sock,vMsg);
    }
#ifdef QQ
    if(adminLogin == 0xff)
    {
        if(password.size() <= 0)
            return;
        MDLogin(name,password,sock);
    }
    else
    {
        QQLogin(name,password,sock);
    }
#else
    MDLogin(name,password,sock);
#endif
}

bool CPackageDeal::RegShareUser(string &name,string &passwd)//,string &trueName,string &rcode)
{
    char buf[256];
    snprintf(buf,255,
        "select UNIX_TIMESTAMP(uplogin) as t1, register_date as t2 from t_user_basic "\
        "where login_name='%s'"
        ,name.c_str());
    boost::mutex::scoped_lock lk(m_dbLock);
    if(!m_loginDb.Query(buf))
        return false;
    char **row = m_loginDb.GetRow();
    if(row == NULL)
    {
        snprintf(buf,255,"insert into t_user_basic (login_name,nick,passwd,register_date) "\
            "values ('%s','%s','%s',%lu)"
            ,name.c_str(),name.c_str(),passwd.c_str(),GetSysTime());//,trueName.c_str(),rcode.c_str());
        return m_loginDb.Query(buf);
    }
    /*else
    {
        if((atoi(row[0]) < 1167584400) && (atoi(row[1]) < 1167584400))
        {
            snprintf(buf,255,"update t_user_basic set passwd='%s',register_date=%lu where login_name='%s'",
                passwd.c_str(),GetSysTime(),name.c_str());
            return m_loginDb.Query(buf);
        }
    }*/
    return false;
}

static bool CheckPass(char *pass)
{
  while(*pass)
  {
    if(*pass>122)
      return false;
    if(*pass<48)
      return false;
    if(*pass>57 && *pass<65)
      return false;
    if(*pass>90 && *pass<97)
      return false;
    pass++;
  }
  return true;
}

void CPackageDeal::RegUser(CNetMessage *pMsg,int sock)
{
	GET_MSG

    string version;
    string name;
    string password;
    uint8 type = 0;
    uint8 ext = 0;
    //string trueName;
    //string rcode;
    
    //msg>>name>>password>>trueName>>rcode>>version>>type>>ext;
    msg>>name>>password>>version>>type>>ext;
    msg.ReWrite();
    msg.SetType(PRO_REG_USER);
#ifdef NO_REG_USER
    msg<<PRO_ERROR<<"新用户注册暂停，3月18日上午10点公测时开放";
    m_socketServer.SendMsg(sock,msg);
    return;
#endif   
    
    if((name.size() < 4) || (name.size() > 16) ||
        (password.size() < 4) || (password.size() > 16))
    {
        msg<<PRO_ERROR<<"用户名密码不符合要求";
        m_socketServer.SendMsg(sock,msg);
        return;
    }
    
    char *p = (char*)name.c_str();
    if(!CheckPass(p))
    {
        msg<<PRO_ERROR<<"用户名有非法字符";
        m_socketServer.SendMsg(sock,msg);
        return;
    }
    
    if(!RegShareUser(name,password))//,trueName,rcode))
    {
        msg<<PRO_ERROR<<"用户名已存在";
        m_socketServer.SendMsg(sock,msg);
        return;
    }
    //MD5(password);
    CGetDbConnect getDb;
    CDatabaseSql *pDb = getDb.GetDbConnect();
    boost::format fmt("INSERT INTO user_info (name,password,type,ext,reg_time) VALUES('%1%','%2%',%3%,%4%,%5%)");
    fmt % name % password % (int)type % (int)ext % GetSysTime();
    //cout<<fmt.str()<<endl;
#ifdef DEBUG
    cout<<"reg user"<<name<<":"<<password<<endl;
#endif

    if ((pDb != NULL) && (pDb->Query(fmt.str().c_str())))
    {
        uint32 userId = pDb->InsertId();
        ShareUserPtr ptr = m_onlineUser.GetUserBySock(sock);
        CUser *pUser = ptr.get();
        if(pUser != NULL)
        {
            close(pUser->GetSock());
            OnSockClose(pUser->GetSock());
            return;
        }
        if(m_onlineUser.AddUser(sock,userId) == NULL)
            msg<<PRO_ERROR<<"注册失败";
        else
            msg<<PRO_SUCCESS;
    }
    else
    {
        msg<<PRO_ERROR<<"用户已存在";
    }
    m_socketServer.SendMsg(sock,msg);
}

void CPackageDeal::CreateRole(CNetMessage *pMsg,int sock)
{
	GET_MSG
    //GET_USER
    
    ShareUserPtr ptr = m_onlineUser.GetUserBySock(sock);
    CUser *pUser = ptr.get();
    if(pUser == NULL)
        return;
    
    uint8 position;
    string name;
    uint8 head = 0;
    uint8 xiang = 0;
    msg>>position>>name>>head>>xiang;
#ifdef DEBUG
    cout<<"create role "<<name<<endl;
#endif
    msg.ReWrite();
    msg.SetType(PRO_CREATE_ROLE);
    
    int nameLen = name.size();
    if((head == 0) || (xiang == 0) 
        || (nameLen < 2) || (nameLen > 16))
    {
        msg<<PRO_ERROR<<"名称长度不符要求哦";
        m_socketServer.SendMsg(pUser->GetSock(),msg);
        return;
    }
    if(IllegalStr(name))
    {
        char buf[64];
        snprintf(buf,64,"\"%s\"字为限制使用字符，请换一个哦",name.c_str());
        msg<<PRO_ERROR<<buf;
        m_socketServer.SendMsg(pUser->GetSock(),msg);
        return;
    }
    uint8 sex = 1;
    //头像1、3、5、7男0
    if(head % 2 != 0)
        sex = 0;
    char sqlBuf[512];
    CGetDbConnect getDb;
    CDatabaseSql *pDb = getDb.GetDbConnect();

    boost::format fmt("select role0,role1,role2,role3,role4,role5,"\
        "del_time0,del_time1,del_time2,del_time3,del_time4,del_time5 from user_info where id=%1%");
    fmt%pUser->GetUserId();
    
    string sql = fmt.str();
    
    if(!pDb->Query(sql.c_str()))
    {
        return;
    }
    char **row = pDb->GetRow();
    if(row == NULL)
        return;
    uint8 i = 0;
    uint8 roleNum = 0;
    uint32 id;
    time_t dT;
    
    position = MAX_ROLE_NUM;
    for (; i < MAX_ROLE_NUM; i++)
    {
        id = atoi(row[i]);
        dT = atoi(row[i+6]);
        if((id != 0) && ((dT == 0) || (GetSysTime() - dT < CAN_SAVE_TIME)))
        {
            roleNum++;
        }
        else if(position == MAX_ROLE_NUM)
        {
            position = i;
        }
    }
    if(roleNum >= 3)
    {
        msg<<PRO_ERROR<<"角色数过多";
        m_socketServer.SendMsg(pUser->GetSock(),msg);
        return;
    }
    
    sprintf(sqlBuf,"insert into role_info (position,name,sex,head,xiang,reg_time) "\
        "values (%d,'%s',%d,%d,%d,%lu)",position,name.c_str(),sex,head,xiang,GetSysTime());

    if ((position < MAX_ROLE_NUM) && (pDb != NULL) && (pDb->Query(sqlBuf)))
    {
        uint32 roleId = pDb->InsertId();
        boost::format fmt("update user_info set role%1% = %2%,del_time%1%=0 where id = %3%");
        fmt % (int)position % pDb->InsertId() % pUser->GetUserId();
        if(pDb->Query(fmt.str().c_str()))
        {
            //cout<<"add role:"<<roleId<<endl;
            pUser->AddRole(roleId);
            msg<<PRO_SUCCESS<<position<<roleId<<name<<head<<xiang;
            m_socketServer.SendMsg(pUser->GetSock(),msg);
            return;
        }
    }
    msg<<PRO_ERROR<<"名称已被占用啦~换一个吧";
    m_socketServer.SendMsg(pUser->GetSock(),msg);
}

void CPackageDeal::SelectRole(CNetMessage *pMsg,int sock)
{
	GET_MSG

    ShareUserPtr ptr = m_onlineUser.GetUserBySock(sock);
    CUser *pUser = ptr.get();
    uint32 roleId = 0;
    
    msg>>roleId;
    msg.ReWrite();
    msg.SetType(PRO_SELECT_ROLE);
    //需要对user id进行判断
    if ((pUser == NULL) || (roleId == 0) || !pUser->HaveRole(roleId))
    {
        msg<<PRO_ERROR<<"登录失败";
        m_socketServer.SendMsg(sock,msg);
        return;
    }
    else
    {
        ShareUserPtr ptrOther = m_onlineUser.GetUserByRoleId(roleId);
        CUser *pOther = ptrOther.get();
        if((pOther != NULL) && (pOther->GetSock() == sock))
        {
            return;
        }
    }
    uint16 sceneId = 0;//pUser->GetSceneId();
    
    bool useSrcInfo = false;//使用已在线用户的信息
    uint32 userRoles[MAX_ROLE_NUM] = {0};
    pUser->GetRoles(userRoles);
    for(uint8 i = 0; i < MAX_ROLE_NUM; i++)
    {
        if(userRoles[i] != 0)
        {
            ShareUserPtr ptrOther = m_onlineUser.GetUserByRoleId(userRoles[i]);
            CUser *pOther = ptrOther.get();
            if(pOther != NULL)
            {
                if(userRoles[i] != roleId)
                {
                    msg<<PRO_ERROR<<"您有其他角色在游戏中";
                    m_socketServer.SendMsg(pUser->GetSock(),msg);
                    return;
                }
                else if(userRoles[i] == roleId)
                {
                    if(pOther->GetFightId() != 0)
                    {
                        int oldSock = pOther->GetSock();
                        if((oldSock >= 0) && (oldSock != sock))
                        {
                            close(oldSock);
                            //m_onlineUser.DelUser(sock);
                            OnSockClose(oldSock);
                        }
                        pUser = m_onlineUser.ReLogin(sock,roleId);
                        if(pUser == NULL)
                            return;
                        useSrcInfo = true;
                    }
                    else
                    {
                        if(pOther->GetSock() > 0)
                            close(pOther->GetSock());
                        UserLogout(pOther);
                    }
                    break;
                }
            }
        }
    }
    /*if(pUser->GetFightId() != 0)
    {
        msg<<PRO_ERROR;
        m_socketServer.SendMsg(sock,msg);
        return;
    }*/
    
    /*if((pUser->GetRoleId() == roleId) && (pUser->GetFightId() != 0))
    {
        useSrcInfo = true;
    }*/
    
    if(useSrcInfo || pUser->ReadData(roleId,&sceneId))
    {
        if(!useSrcInfo)
        {
            pUser->Init();
            m_onlineUser.SetRoleId(sock,roleId);
        }
        else
        {
            sceneId = pUser->GetMapId();
        }
        if((sceneId == LAN_RUO_DI_GONG_ID) || (sceneId == 304))
        {
            CSceneManager &sceneMgr = SingletonSceneManager::instance();
            CScene *pScene = sceneMgr.FindScene(pUser->GetData32(3));
            if(pScene == NULL)
            {
                sceneId = 301;
                pUser->SetPos(8,19);
            }
        }
        if(sceneId == 250)
        {
            CScene *pScene = m_sceneManager.FindMarryHall(pUser->GetData32(3));
            if(pScene == NULL)
            {
                sceneId = 20;
                pUser->SetPos(22,14);
            }
        }
        
        int fagong = 0;
        uint16 mingzhong = 0;
        uint16 huibi = 0;
        pUser->GetViewPara(fagong,mingzhong,huibi);
        if(pUser->GetHp() <= 0)
            pUser->AddHp(1);
        
        int state = 0;
        if(pUser->GetBangPai() != 0)
        {
            CBangPaiManager &bPMgr = SingletonCBangPaiManager::instance();
            CBangPai *pBangPai = bPMgr.FindBangPai(pUser->GetBangPai());
            if((pBangPai == NULL) || (pBangPai->GetMemberRank(roleId) == 0))
                pUser->SetBangPai(0);
            if(pBangPai != NULL)
            {
                if(pBangPai->GetBangZhu() == pUser->GetRoleId())
                {
                    pUser->AddTextTitle("一帮之主");
                    if((int)pUser->GetBangPai() == WWGetWinBang())
                        state |= 2;
                }
            }
        }
        if(pUser->GetData8(0) > 0)
        {
            pUser->AddTextTitle("初为人师");
        }
        if(pUser->GetData8(3) >= 20)
        {
            pUser->AddTextTitle("桃李天下");
        }
        if(pUser->GetDaoHang() >= 1000000)
        {
            pUser->AddTextTitle("修真者");
        }
        if(pUser->GetDaoHang() >= 5000000)
        {
            pUser->AddTextTitle("修仙者");
        }
        if(pUser->HaveAllWanMei())
        {
            pUser->AddTextTitle("无冕之王");
        }
        if(pUser->HaveLevel12Equip())
        {
            pUser->AddTextTitle("幸运之星");
        }
        uint8 xiang = pUser->GetXiang();
        switch(xiang)
        {
        case EXJinXiang://金
            if((pUser->GetSkillLevel(1) >= 60)
                && (pUser->GetSkillLevel(2) >= 60)
                && (pUser->GetSkillLevel(3) >= 60)
                && (pUser->GetSkillLevel(51) >= 60)
                && (pUser->GetSkillLevel(52) >= 60)
                && (pUser->GetSkillLevel(53) >= 60)
                && (pUser->GetSkillLevel(101) >= 60)
                && (pUser->GetSkillLevel(102) >= 60)
                && (pUser->GetSkillLevel(103) >= 60))
            {
                pUser->AddTextTitle("炉火纯青");
            }
            break;
        case EXMuXiang://2－木6.21.23.44.45.47.58
            if((pUser->GetSkillLevel(6) >= 60)
                && (pUser->GetSkillLevel(21) >= 60)
                && (pUser->GetSkillLevel(23) >= 60)
                && (pUser->GetSkillLevel(55) >= 60)
                && (pUser->GetSkillLevel(56) >= 60)
                && (pUser->GetSkillLevel(57) >= 60)
                && (pUser->GetSkillLevel(105) >= 60)
                && (pUser->GetSkillLevel(106) >= 60)
                && (pUser->GetSkillLevel(107) >= 60))
            {
                pUser->AddTextTitle("炉火纯青");
            }
            break;
        case EXShuiXiang://3－水9.10.11.59.60.61.62
            if((pUser->GetSkillLevel(9) >= 60)
                && (pUser->GetSkillLevel(10) >= 60)
                && (pUser->GetSkillLevel(11) >= 60)
                && (pUser->GetSkillLevel(59) >= 60)
                && (pUser->GetSkillLevel(60) >= 60)
                && (pUser->GetSkillLevel(61) >= 60)
                && (pUser->GetSkillLevel(121) >= 60)
                && (pUser->GetSkillLevel(110) >= 60)
                && (pUser->GetSkillLevel(111) >= 60))
            {
                pUser->AddTextTitle("炉火纯青");
            }
            break;
        case EXHuoXiang://4－火13.14.15.63.64.65.66
            if((pUser->GetSkillLevel(13) >= 60)
                && (pUser->GetSkillLevel(14) >= 60)
                && (pUser->GetSkillLevel(15) >= 60)
                && (pUser->GetSkillLevel(63) >= 60)
                && (pUser->GetSkillLevel(64) >= 60)
                && (pUser->GetSkillLevel(65) >= 60)
                && (pUser->GetSkillLevel(113) >= 60)
                && (pUser->GetSkillLevel(114) >= 60)
                && (pUser->GetSkillLevel(115) >= 60))
            {
                pUser->AddTextTitle("炉火纯青");
            }
            break;
        case EXTuXiang://5－土22.24.18.67.68.69.70
            if((pUser->GetSkillLevel(22) >= 60)
                && (pUser->GetSkillLevel(24) >= 60)
                && (pUser->GetSkillLevel(18) >= 60)
                && (pUser->GetSkillLevel(67) >= 60)
                && (pUser->GetSkillLevel(68) >= 60)
                && (pUser->GetSkillLevel(69) >= 60)
                && (pUser->GetSkillLevel(117) >= 60)
                && (pUser->GetSkillLevel(118) >= 60)
                && (pUser->GetSkillLevel(119) >= 60))
            {
                pUser->AddTextTitle("炉火纯青");
            }
            break;
        }
        
        msg<<PRO_SUCCESS<<roleId<<pUser->GetName()<<pUser->GetHead()<<pUser->GetSex()
            <<pUser->GetXiang()<<sceneId<<pUser->GetX()<<pUser->GetY()
            <<pUser->GetLevel()<<pUser->GetExp()<<pUser->GetTiZhi()
            <<pUser->GetLiLiang()<<pUser->GetMinJie()<<pUser->GetLingLi()
            <<pUser->GetNaiLi()<<pUser->GetShuXinDian()<<pUser->GetJin()
            <<pUser->GetMu()<<pUser->GetShui()<<pUser->GetHuo()<<pUser->GetTu()<<pUser->GetXiangXinDian()
            <<pUser->GetHp()<<pUser->GetMaxHp()<<pUser->GetMp()<<pUser->GetMaxMp()
            <<(uint16)pUser->GetTili()
            <<pUser->GetDamage()
            <<fagong
            <<(uint16)pUser->GetSpeed()
            <<pUser->GetRecovery()
            <<mingzhong
            <<huibi
            //<<(uint8)pUser->GetItemMingzhong()
            //<<(uint8)pUser->GetItemHuibi()
            <<(uint16)pUser->GetItemLianjiLv()
            <<(uint16)pUser->GetItemBaoJiLv()
            <<(uint16)pUser->GetItemFanJiLv()
            <<(uint16)pUser->GetItemFanZhenLv()
            <<(uint16)pUser->GetFaShuFanTanLv()
            <<(uint16)pUser->GetFaShuBaoJi()
            <<pUser->GetMoney()
            <<pUser->GetTongBao()
            <<pUser->GetQianNeng()<<pUser->GetDaoHang()
            <<pUser->GetMenPai()
            <<pUser->GetPkVal()
            <<pUser->GetData16(0)
            <<pUser->GetChatTime()
            <<pUser->AdminLevel()
            <<pUser->GetOpenPack();
        
        if(pUser->GetData8(6) != 0)
            state |= 1;
            
        msg<<state;
        
        //msg<<(byte)第四行囊开启数<<(int)结束时间<<(byte)仓库开启数目<<(int)结束时间<<(byte)宠物仓库开启数<<(int)结束时间
        uint8 openPack = 0;
        uint32 tPack = 0;
        uint8 openBank = 0;
        uint32 tBank = 0;
        uint8 openPetBank = 0;
        uint32 tPetBank = 0;
        if(pUser->GetExtData32(0) > (uint32)GetSysTime())
        {
            openPack = pUser->GetExtData8(0);
            tPack = pUser->GetExtData32(0);
        }
        if(pUser->GetExtData32(1) > (uint32)GetSysTime())
        {
            openBank = pUser->GetExtData8(1);
            tBank = pUser->GetExtData32(1);
        }
        if(pUser->GetExtData32(2) > (uint32)GetSysTime())
        {
            openPetBank = pUser->GetExtData8(2);
            tPetBank = pUser->GetExtData32(2);
        }
        msg<<openPack<<tPack<<openBank<<tBank<<openPetBank<<tPetBank<<pUser->GetTongBao(1);
            
        list<uint32> hotList;
        pUser->GetHotList(hotList);
        for(list<uint32>::iterator i = hotList.begin(); i != hotList.end(); i++)
        {
            AddNotifyUser(pUser->GetRoleId(),*i);
        }
    }
    else
    {
        msg<<PRO_ERROR<<"此用户已封号,请与管理员联系";
        m_socketServer.SendMsg(pUser->GetSock(),msg);
        return;
    }
    CGetDbConnect getDb;
    CDatabaseSql *pDb = getDb.GetDbConnect();
    /*if(pDb != NULL)
    {
        char sqlBuf[64];
        snprintf(sqlBuf,63,"update role_info set state = 1 where id=%d",roleId);
        pDb->Query(sqlBuf);
    }*/
    m_socketServer.SendMsg(pUser->GetSock(),msg);
    
    if(useSrcInfo)
    {
        CScene *pScene = pUser->GetScene();
        if(pScene != NULL)
            pScene->SendUserList(pUser);
            
        ShareFightPtr pFight = SingletonFightManager::instance().FindFight(pUser->GetFightId());
        if(pFight.get() != NULL)
        {
            ShareUserPtr ptr = m_onlineUser.GetUserBySock(sock);
            if(!pFight->ReBegin(m_socketServer,ptr))
                pUser->SetFight(0,0);
        }
        else
        {
            pUser->SetFight(0,0);
        }
    }
    else
    {
        if(sceneId != 0)
        {
            if((sceneId == BANG_PAI_SCENE_ID) || (sceneId == 305))
            {
                int bangpai = pUser->GetData32(3)>>16;
                pUser->EnterScene(m_sceneManager.GetBangPaiScene(sceneId,bangpai));
            }
            else if(sceneId == 250)
            {
                CScene *pScene = m_sceneManager.FindMarryHall(pUser->GetData32(3));
                if(pScene == NULL)
                {
                    pScene = m_sceneManager.FindScene(20);
                }
                pUser->EnterScene(pScene);
            }
            else if((sceneId == LAN_RUO_DI_GONG_ID) || (sceneId == 304)
                || (sceneId == BANG_PAI_SCENE_ID) || (sceneId == 305))
            {
                pUser->EnterFuBen(sceneId);
            }
            else
            {
                pUser->EnterScene(m_sceneManager.FindScene(sceneId));
            }
        }
        
        NotifyUser(pUser,true);
        pUser->UserJump(false);
        pUser->LoadMission();
        CCallScript *pScript = GetScript();//("10000.lua");
        if(pScript != NULL)
        {
            pUser->SetCallScript(pScript->GetScriptId());
            pScript->Call("Logon","u",pUser);
        }
    }
    
    if(pDb != NULL)
    {
        char sqlBuf[64];
        snprintf(sqlBuf,63,"select msg from cz_notice where user_id =%d",pUser->GetUserId());
        if(pDb->Query(sqlBuf))
        {
            char **row = pDb->GetRow();
            if(row != NULL)
            {
                SendPopMsg(pUser,row[0]);
                snprintf(sqlBuf,63,"delete from cz_notice where user_id =%d",pUser->GetUserId());
                pDb->Query(sqlBuf);
            }
            else
            {
                gpMain->SendGongGao(pUser->GetSock());
            }
        }
    }
     
    if(pUser->HaveBitSet(0))
    {
        SendPopMsg(pUser,"你惹怒了星灵仙子，快去星寿村向她道歉吧。否则打怪做任务得不到任何奖励");
    }
}

void CPackageDeal::AddNotifyUser(uint32 userId,uint32 hotId)
{
    boost::mutex::scoped_lock lk(m_mutex);
    list<uint32> *pList = NULL;
    if(!m_inOutNotify.Find(hotId,pList))
    {
        pList = new list<uint32>;
        m_inOutNotify.Insert(hotId,pList);
    }
    pList->push_back(userId);
}

void CPackageDeal::DelNotifyUser(uint32 userId,uint32 hotId)
{
    boost::mutex::scoped_lock lk(m_mutex);
    list<uint32> *pList = NULL;
    if(m_inOutNotify.Find(hotId,pList))
    {
        pList->remove(userId);
        if(pList->empty())
        {
            m_inOutNotify.Erase(hotId);
            delete pList;
        }
    }
}

void CPackageDeal::NotifyUser(CUser *pUser,bool online)
{
    list<uint32> *pHotList = NULL;    
    boost::mutex::scoped_lock lk(m_mutex);
    m_inOutNotify.Find(pUser->GetRoleId(),pHotList);
    if(pHotList != NULL)
    {    
        CNetMessage msg;
        msg.SetType(PRO_HOT_ONLINE);
        msg<<pUser->GetRoleId()<<pUser->GetName();
        if(online)
            msg<<(uint8)1;
        else 
            msg<<(uint8)0;
        for(list<uint32>::iterator i = pHotList->begin(); i != pHotList->end(); i++)
        {
            ShareUserPtr p = m_onlineUser.GetUserByRoleId(*i);
            CUser *pU = p.get();
            if(pU != NULL)
            {
                m_socketServer.SendMsg(pU->GetSock(),msg);
            }
        }
    }
}

void CPackageDeal::GetEquipment(CNetMessage *pMsg,int sock)
{
	GET_MSG
    GET_USER
    
    msg.ReWrite();
    msg.SetType(PRO_ROLE_EQUIPMENT);
    pUser->MakeEquip(msg);
    m_socketServer.SendMsg(pUser->GetSock(),msg);
}

void CPackageDeal::GetPackage(CNetMessage *pMsg,int sock)
{
	GET_MSG

    msg.ReWrite();
    msg.SetType(PRO_ROLE_PACKAGE);
    ShareUserPtr ptr = m_onlineUser.GetUserBySock(sock);
    CUser *pUser = ptr.get();
    if (pUser == NULL)
        return;
    
    pUser->MakePack(msg);
    m_socketServer.SendMsg(pUser->GetSock(),msg);
}
extern bool InWorldWar();

void CPackageDeal::UserMoveOneStep(CUser *pUser,CScene *pScene,ShareUserPtr &ptr,uint8 x,uint8 y)
{
    SJumpTo *pJump = NULL;
    if(pScene->GetJumpPoint(x,y,pJump))
    {
        if((pScene->GetId() == 1) && (pUser->GetLevel() <= 4))
        {
            SendPopMsg(pUser,"你初来此地，还是先到处逛逛吧。向这里的每个人请教一下");
            return;
        }
        if(pJump->sceneId == 275)// && (pJump->sceneId <= 276))
        {
            if((WWGetWinBang() == 0) || (WWGetWinBang() != (int)pUser->GetBangPai()))
            {
                if(!InWorldWar())
                {
                    SendPopMsg(pUser,"非活动期间只有占领帮派方可进入");
                    return;
                }
            }
        }
        if(pJump->sceneId == 276)
        {
            if(InWorldWar())
            {
                if(!IsFangShou(pUser->GetBangPai()))
                {
                    SendPopMsg(pUser,"非占领帮派成员不能进入场景");
                    return;
                }
            }
            else if((WWGetWinBang() == 0) || (WWGetWinBang() != (int)pUser->GetBangPai()))
            {
                SendPopMsg(pUser,"非占领帮派成员不能进入场景");
                return;
            }
        }
        
        CNetMessage msg;
        CScene *pOld = pUser->GetScene();
        if((pJump->sceneId == BANG_PAI_SCENE_ID) || (pJump->sceneId == 305))
        {
            msg.SetType(PRO_JUMP_SCENE);
            msg<<pJump->sceneId<<pJump->x<<pJump->y<<pJump->face;
            m_socketServer.SendMsg(pUser->GetSock(),msg);
            pUser->SetPos(pJump->x,pJump->y);
            pUser->SetFace(pJump->face);
            int bpId = pUser->GetBangPai();
            if(pOld != NULL)
                bpId = pOld->GetId()>>16;
            pUser->EnterScene(m_sceneManager.GetBangPaiScene(pJump->sceneId,bpId));
        }
        else if((pOld != NULL) && (pOld->GetGroupId() != 0))
        {
            CScene *pScene = m_sceneManager.FindScene(pJump->sceneId,pOld->GetGroupId());
            if(pScene == NULL)
            {
                msg.ReWrite();
                msg.SetType(PRO_JUMP_SCENE);
                msg<<pJump->sceneId<<pJump->x<<pJump->y<<pJump->face;
                m_socketServer.SendMsg(pUser->GetSock(),msg);
                pUser->SetPos(pJump->x,pJump->y);
                pUser->SetFace(pJump->face);
                pUser->EnterScene(m_sceneManager.FindScene(pJump->sceneId));
                return;
            }
            if(pScene->GetMapId() == 260)
            {
                if((0xffff & pScene->GetId()) == (int)pUser->GetBangPai())
                {
                    SendPopMsg(pUser,"敌帮领地，无法进入");
                    return;
                }
            }
            if(pScene->GetMapId() == 261)
            {
                if((pScene->GetGroupId()>>16) == (int)pUser->GetBangPai())
                {
                    SendPopMsg(pUser,"敌帮领地，无法进入");
                    return;
                }
            }
            msg.ReWrite();
            msg.SetType(PRO_JUMP_SCENE);
            msg<<pJump->sceneId<<pJump->x<<pJump->y<<pJump->face;
            m_socketServer.SendMsg(pUser->GetSock(),msg);
            pUser->SetPos(pJump->x,pJump->y);
            pUser->SetFace(pJump->face);
            pUser->EnterScene(pScene);
        }
        else
        {
            msg.ReWrite();
            msg.SetType(PRO_JUMP_SCENE);
            msg<<pJump->sceneId<<pJump->x<<pJump->y<<pJump->face;
            m_socketServer.SendMsg(pUser->GetSock(),msg);
            pUser->SetPos(pJump->x,pJump->y);
            pUser->SetFace(pJump->face);
            pUser->EnterScene(m_sceneManager.FindScene(pJump->sceneId));
        }
#ifdef DEBUG
        boost::format fmt("jump to:%1%,x:%2%,y%3%,face:%4%");
        fmt % (int)pJump->sceneId % (int)pJump->x % (int)pJump->y % (int)pJump->face;
        cout<<fmt<<endl;
#endif
    }
    else
    {
        if(pUser->GetFightId() == 0)
        {
            if(pUser->GetAskTime() != 0)
            {
                return;
            }
            if(pUser->GetTeam() != 0)
            {
                CScene *pScene = pUser->GetScene();
                if(pScene != NULL)
                {
                    CUser *pUser1 = pScene->GetTeamMember1(pUser->GetTeam());
                    CUser *pUser2 = pScene->GetTeamMember2(pUser->GetTeam());
                    if((pUser1 != NULL) && (pUser1->GetAskTime() != 0))
                        return;
                    if((pUser2 != NULL) && (pUser2->GetAskTime() != 0))
                        return;
                }
            }
            if(pUser->GetAnswerSpace() > ANSWER_QUESTION_SAPCE)//ANSWER_QUESTION_SAPCE)
            {
                if(!pUser->HaveBitSet(0))
                {
                    AskQuestion(pUser);
                    return;
                }
            }
            pUser->SetAskTime(0);
            CScene *pScene = pUser->GetScene();
            int step = 0;
            if(pScene != NULL)
                step = pScene->GetFightStep();// && ((pUser->GetStep() > 
            if(pUser->IsFewMonster())
            {
                SItemTemplate *pItem = SingletonItemManager::instance().GetItem(1810);
                if(pItem != NULL)
                {
                    step = CalculateRate(step,(int)pItem->naijiu,100);
                    //step = (int)(step * 100 / pItem->naijiu);
                }
            }
            if(step <= 0)
                step = 20;
            if(pUser->CanMeetEnemy() && (pUser->GetStep() % step == 0))
            {
                //ShareUserPtr ptr = m_onlineUser.GetUserByRoleId(pUser->GetRoleId());
                if(InFightHuoDong() 
                    && pUser->CanFightHuoDong() && (pUser->GetLevel() > 20)
                    && (pScene->GetMonsterNum() > 0)
                    && (Random(1,5) == 1))
                {
                    pScene->NianShouBattle(ptr);
                }
                else
                {
                    pScene->MeetEnemy(ptr);
                }
            }
        }
        pUser->AddStep(1);
    }
}

void CPackageDeal::RoleMove(CNetMessage *pMsg,int sock)
{
	GET_MSG
    GET_USER
    
    uint8 dir = 0;
    uint8 step = 1;
    msg>>dir>>step;
    
    if((pUser->GetFightId() != 0) || pUser->InJump())
        return;
    if((pUser->GetTeam() != 0) && (pUser->GetTeam() != pUser->GetRoleId())
        && !pUser->TempLeaveTeam())
    {
        return;
    }
        
    if(pUser->GetStep() % 10 == 0)
    {
        uint64 t = GetTime();
        if(t - pUser->GetMoveTime() < (uint64)951759)
        {
            pUser->SetErrMoveTimes(pUser->GetMoveErrTimes()+1);
            if(pUser->GetMoveErrTimes() >= 5)
            {
                //shutdown(sock,SHUT_RD);
                SendPopMsg(pUser,"你移动速度过快,引起了星灵仙子的注意,她现在很想见见你。");
                pUser->SetBitSet(0);
                pUser->SetErrMoveTimes(0);
                pUser->SetMoveTime(t);
                return;
            }
        }
        else
        {
            pUser->SetErrMoveTimes(0);
        }
        pUser->SetMoveTime(t);
    }
    uint8 x,y;
    pUser->GetPos(x,y);
    pUser->Move(dir,step);
    
    CScene *pScene = pUser->GetScene();
    if(pScene == NULL)
    {
        return;
    }
        
    pScene->UserMove(pUser,dir,step);
    
    for(uint8 i = 0; i < step; i++)
    {
        switch(dir)
        {
        case 2:
            y--;
            break;
        case 4:
            x--;
            break;
        case 6:
            x++;
            break;
        case 8:
            y++;
        }
        UserMoveOneStep(pUser,pScene,ptr,x,y);
    }
}

void CPackageDeal::OnSockClose(int sock)
{
#ifdef QQ
    DelQQLoginUser(sock);
    if(sock == m_qqServerSocket)
        m_qqServerSocket = 0;
#endif
    ShareUserPtr ptr = m_onlineUser.GetUserBySock(sock);
    CUser *pUser = ptr.get();
    if (pUser != NULL)
    {
        UserLogout(pUser);    
    }
}

void CPackageDeal::UserLogout(CUser *pUser)
{
    uint32 fightId = pUser->GetFightId();
    
    uint32 guanzhanFight = pUser->GetGuanZhan();
    if(guanzhanFight != 0)
    {
        ShareFightPtr ptr = SingletonFightManager::instance().FindFight(guanzhanFight);
        if(ptr.get() != NULL)
        {
            ptr->LeaveGuanZhan(pUser);
        }
    }
    if(fightId != 0)
    {
        ShareFightPtr ptr = SingletonFightManager::instance().FindFight(fightId);
        if(ptr.get() != NULL)
        {
            m_onlineUser.DelUser(pUser);//->GetSock(),pUser->GetRoleId());
            pUser->SetAutoFightTurn(0xffff);
            pUser->UserLogout(true);
            return;
        }
    }
    
    m_onlineUser.DelUser(pUser);//->GetSock(),pUser->GetRoleId());
    
    CScene *pScene = pUser->GetScene();
    if(pScene == NULL)
        return;
            
    if(pUser->GetTradeUser() != 0)
    {
        CNetMessage msg;
        msg.SetType(PRO_SAFE_TRADE);
        ShareUserPtr pTotr = m_onlineUser.GetUserByRoleId(pUser->GetTradeUser());
        pUser->SetTradeUser(0);
        CUser *pTo = pTotr.get();
        if(pTo != NULL)
        {
            msg<<(uint8)3<<pUser->GetRoleId()<<pUser->GetName();
            m_socketServer.SendMsg(pTo->GetSock(),msg);        
            pTo->SetTradeUser(0);
        }
    }
    
    pScene->LeaveTeam(pUser);
    pScene->Exit(pUser);
    
    NotifyUser(pUser,false);
    list<uint32> hotList;
    pUser->GetHotList(hotList);
    for(list<uint32>::iterator i = hotList.begin(); i != hotList.end(); i++)
    {
        DelNotifyUser(pUser->GetRoleId(),*i);
    }
    pUser->UserLogout(true);
}

void CPackageDeal::OpenNpcInteract(CNetMessage *pMsg,int sock)
{
	GET_MSG

    GET_USER

    uint16 npcId = 0;
    msg>>npcId;
    
    CScene *pScene = pUser->GetScene();
    if(pScene == NULL)
        return;
    
    list<uint16> *pList = pScene->GetNpcList();
    if(pList == NULL)
        return;
    for(list<uint16>::iterator i = pList->begin(); i != pList->end(); i++)
    {
        SNpcInstance *pNpc = m_npcManager.GetNpcInstance(*i);
        if(pNpc == NULL)
            continue;
        uint8 x,y;
        pUser->GetFacePos(x,y);
        if(((x == pNpc->x) && (y == pNpc->y)) || (npcId == pNpc->id))
        {
            pUser->SetCall(0,"");
            msg.ReWrite();
            msg.SetType(PRO_OPEN_INTERACT);
            msg<<PRO_SUCCESS;
            m_socketServer.SendMsg(pUser->GetSock(),msg);
        
            CCallScript *pScript = NULL;
            if(pNpc->pNpc != NULL)
            {
                pScript = pNpc->pNpc->pScript;
            }
            if(pScript != NULL)
            {
                pUser->SetCallScript(pScript->GetScriptId());
                pScript->Call("NpcMain","u",pUser);
            }
            
            return;
        }
    }
    
    SNpcInstance npc ;
    if(pUser->FindNpcNear(npc))
    {
        pUser->SetCall(0,"");
        msg.ReWrite();
        msg.SetType(PRO_OPEN_INTERACT);
        msg<<PRO_SUCCESS;
        m_socketServer.SendMsg(pUser->GetSock(),msg);
        
        CCallScript *pScript = NULL;
        if(npc.pNpc != NULL)
        {
            pScript = npc.pNpc->pScript;
        }
        if(pScript != NULL)
        {
            pUser->SetCallScript(pScript->GetScriptId());
            pScript->Call("NpcMain","u",pUser);
        }
        return;
    }
    uint8 x,y;
    pUser->GetFacePos(x,y);
    SNpcInstance *pNpc = NULL;
    if(npcId != 0)
        pNpc = pScene->FindNpc(npcId);
    else
        pNpc = pScene->FindNpc(x,y);
        
    if(pNpc != NULL)
    {
        pUser->SetCall(0,"");
        msg.ReWrite();
        msg.SetType(PRO_OPEN_INTERACT);
        msg<<PRO_SUCCESS;
        m_socketServer.SendMsg(pUser->GetSock(),msg);
        
        CCallScript *pScript = NULL;
        if(pNpc->pNpc != NULL)
        {
            pScript = pNpc->pNpc->pScript;
        }
        if(pScript != NULL)
        {
            pUser->SetCallScript(pScript->GetScriptId());
            pScript->Call("NpcMain","u",pUser);
        }
        return;
    }
    msg.ReWrite();
    msg.SetType(PRO_OPEN_INTERACT);
    msg<<PRO_ERROR;
    m_socketServer.SendMsg(pUser->GetSock(),msg);
}

void CPackageDeal::NpcInteract(CNetMessage *pMsg,int sock)
{
	GET_MSG

    GET_USER
    
    if(pUser->GetFightId() != 0)
        return;
            
    uint8 op = 0;
    uint8 num = 0;
    uint8 type;
    msg>>op>>num;//>>input;
    
    if(op == 0)
    {
        CloseInteract(pUser);
        return;
    }
        
    if(num > 6)
        return;
        
    ArgList argList;
    argList.push_back(boost::any(pUser));
    
    for(uint8 i = 0; i < num; i++)
    {
        msg>>type;
        if(type == 0)
        {
            int inputInt = 0;
            msg>>inputInt;
            argList.push_back(boost::any(inputInt));
        }
        else
        {
            string input;
            msg>>input;
            argList.push_back(boost::any(input));
        }
    }
    
    int script = 0;
    string call = pUser->GetCall(script);

    if(call.empty())
    {
#ifdef DEBUG
        cout<<"could fond find script:"<<script<<endl;
#endif
        CloseInteract(pUser);
        return;
    }
    pUser->SetCall(0,"");
    
    CCallScript *pCallScript = FindScript(script);//(name);
    if(pCallScript != NULL)
    {
        pUser->SetCallScript(pCallScript->GetScriptId());
        pCallScript->Call(call.c_str(),&argList);
    }
}

//加属性点、向性点……
void CPackageDeal::UpdateChar(CNetMessage *pMsg,int sock)
{
	GET_MSG

    GET_USER
        
    boost::mutex::scoped_lock lk(m_mutex);
    //cout<<&m_mutex<<endl;
    uint8 op = 0;
    uint8 val = 0;
    msg>>op>>val;
    //msg.ReWrite();
    if(op == 1)
    {//属性点分配
        if((pUser->GetShuXinDian() > 0) && (val > 0) && (val <= 5))
        {
            pUser->AddShuXinDian(-1);
            switch(val)
            {
            case 1:// 体质
                pUser->AddTiZhi(1);
                pUser->Init();
                pUser->SendUpdateInfo(10,pUser->GetMaxHp());
                break;
            case 2:// 力量
                pUser->AddLiLiang(1);
                pUser->Init();
                pUser->SendUpdateInfo(13,pUser->GetDamage());
                pUser->SendUpdateInfo(34,pUser->GetItemBaoJiLv());
                break;
            case 3:// 敏捷
                pUser->AddMinJie(1);
                pUser->Init();
                pUser->SendUpdateInfo(14,pUser->GetSpeed());
                break;
            case 4:// 灵力
                pUser->AddLingLi(1);
                pUser->Init();
                pUser->SendUpdateInfo(12,pUser->GetMaxMp());
                pUser->SendUpdateInfo(38,pUser->GetFaShuBaoJi());
                break;
            case 5:// 耐力
                pUser->AddNaiLi(1);
                pUser->Init();
                pUser->SendUpdateInfo(15,pUser->GetRecovery());
                break;
            }
            int fagong;
            uint16 mingzhong,huibi;
            pUser->GetViewPara(fagong,mingzhong,huibi);
            pUser->SendUpdateInfo(28,fagong);
            pUser->SendUpdateInfo(29,mingzhong);
            pUser->SendUpdateInfo(30,huibi);
        }
        //msg<<(uint8)4<<(int)pUser->GetShuXinDian();
    }
    else if(op == 2)
    {//向性点分配
        if((pUser->GetXiangXinDian() > 0) && (val > 0) &&(val <= 5))
        {
            pUser->AddXiangXinDian(-1);
            switch(val)
            {
            case 1://金
                pUser->AddJin(1);
                pUser->Init();
                break;
            case 2://木
                pUser->AddMu(1);
                pUser->Init();
                break;
            case 3://水
                pUser->AddShui(1);
                pUser->Init();
                break;
            case 4://火
                pUser->AddHuo(1);
                pUser->Init();
                break;
            case 5://土
                pUser->AddTu(1);
                pUser->Init();
                break;
            }
            pUser->UpdateInfo();
        }
        //msg<<(uint8)5<<(int)pUser->GetXiangXinDian();
    }
    else
    {
        return;
    }
}

void CPackageDeal::EquipItem(CNetMessage *pMsg,int sock)
{
	GET_MSG

    GET_USER
        
    bool flag = false;
    uint8 op = 0;
    uint8 loc = 0;
    msg>>op>>loc;
    msg.ReWrite();
    msg.SetType(PRO_OUTFIT);
    if(op == 0)
    {//解除装备
        int pos = pUser->UnEquipItem(loc);
        if(pos >= 0)
        {
            msg<<op<<loc<<(uint8)pos<<PRO_SUCCESS;
            flag = true;            
        }
        else
        {
            msg<<op<<loc<<(uint8)pos<<PRO_ERROR;
        }
        m_socketServer.SendMsg(pUser->GetSock(),msg);
    }
    else if(op == 1)
    {//装备
        string errMsg;
        int pos = pUser->EquipItem(loc,errMsg);
        if(pos >= 0)
        {
            msg<<op<<(uint8)pos<<loc<<PRO_SUCCESS;
            flag = true;
        }
        else
        {
            msg<<op<<(uint8)pos<<loc<<PRO_ERROR<<errMsg;
        }
        m_socketServer.SendMsg(pUser->GetSock(),msg);
    }
    CScene *pScene = pUser->GetScene();
    if(flag && (pScene != NULL))
    {
        pScene->UpdateUserInfo(pUser);
    }
}

void CPackageDeal::GetItemInfo(CNetMessage *pMsg,int sock)
{
	GET_MSG
    GET_USER
        
    uint8 op = 0;
    uint8 ind = 0;
    msg>>op>>ind;
    
    if(op == 0)
    {//查看装备信息
        if(!pUser->MakeEquitInfo(ind,msg))
            return;
    }
    else if(op == 1)
    {//查看背包信息
        if(!pUser->MakePackInfo(ind,msg))
            return;
    }
    else
    {
        return;
    }
    m_socketServer.SendMsg(pUser->GetSock(),msg);
}

void CPackageDeal::GetOtherUserItemInfo(CNetMessage *pMsg,int sock)
{
	GET_MSG

    uint8 op = 0;
    uint32 userId = 0;
    uint8 ind = 0;
    ShareUserPtr ptrMe = m_onlineUser.GetUserBySock(sock);
    CUser *pMe = ptrMe.get();
    if((pMe == NULL) || (pMe->GetRoleId() == 0))
        return;
        
    msg>>op>>userId>>ind;
    
    ShareUserPtr ptr = m_onlineUser.GetUserByRoleId(userId);
    CUser *pUser = ptr.get();
    if(pUser == NULL)
        return;
        
    if(op == 0)
    {//查看装备信息
        if(!pUser->MakeEquitInfo(ind,msg))
            return;
    }
    else if(op == 1)
    {//查看背包信息
        if(!pUser->MakePackInfo(ind,msg))
            return;
    }
    else
    {
        return;
    }
    m_socketServer.SendMsg(pMe->GetSock(),msg);
}

extern bool InWorldWar();

void CPackageDeal::UserTeam(CNetMessage *pMsg,int sock)
{
	GET_MSG
    GET_USER
    
    CScene *pScene = pUser->GetScene();
    if(pScene == NULL)
        return;
    
    if(InWorldWar())
    {
        if(pUser->HaveItem(1558) || 
            pUser->HaveItem(1559) || 
            pUser->HaveItem(1560) || 
            pUser->HaveItem(1561) || 
            pUser->HaveItem(1562))
        {
            SendSysInfo(pUser,"携矿中不能组队");
            return;
        }
    }
            
    if(pUser->HaveBitSet(156))
    {
        SendSysInfo(pUser,"任务中不能组队");
        return;
    }
    if((pScene->GetMapId() >= MATCH_SCENE_ID) && (GetHour() == MATCH_BEGIN_TIME))
    {
        SendSysInfo(pUser,"本场景不能组队");
        return;
    }
    uint8 op = 0;
    msg>>op;
    switch(op)
    {
        case 1:
        {//创建队伍
            if(pUser->TempLeaveTeam() != 0)
                return;
            uint16 mapId = pScene->GetMapId();
            if((mapId == 300) || (mapId == 305))
            {
                if((pScene->GetId()>>16) != (int)pUser->GetBangPai())
                {
                    SendSysInfo(pUser,"非本帮领地，此功能暂时无法使用");
                    return;
                }
            }
            pScene->CreateTeam(pUser);
            break;
        }
        case 2:
        {//请求加入队伍
            uint32 headId = 0;
            msg>>headId;
            if(pUser->TempLeaveTeam() != 0)
            {
                if(headId == pUser->TempLeaveTeam())
                {
                    pScene->ReturnTeam(pUser);
                    return;
                }
                SendSysInfo(pUser,"加入失败，您已有队伍");
                return;
            }
            uint16 mapId = pScene->GetMapId();
            if((mapId == 300) || (mapId == 305))
            {
                if((pScene->GetId()>>16) != (int)pUser->GetBangPai())
                {
                    SendSysInfo(pUser,"非本帮领地，此功能暂时无法使用");
                    return;
                }
            }
            pScene->AskForJoinTeam(pUser,headId);
            break;
        }
        case 3:
        {//队伍列表
            uint8 page = 0;
            msg>>page;
            pScene->GetTeamList(pUser,page);
            break;
        }
        case 4:
        {//队长批准加入队伍
            if(pUser->TempLeaveTeam() != 0)
                return;
            uint32 memberId = 0;
            uint8 agree = 0;
            msg>>agree;
            msg>>memberId;
            if(agree == 1)
                pScene->AllowJoinTeam(pUser,memberId);
            else 
                pScene->NotAllowJoin(pUser,memberId);
            break;
        }
        case 5:
        {//创建队伍并邀请玩家
            if(GetSysTime() - pUser->GetActivityTime() < 3)
            {
                SendPopMsg(pUser,"你邀请组队过快");
                return;
            }
            pUser->SetActivityTime(GetSysTime());
            
            if(pUser->TempLeaveTeam() != 0)
                return;
            uint16 mapId = pScene->GetMapId();
            if((mapId == 300) || (mapId == 305))
            {
                if((pScene->GetId()>>16) != (int)pUser->GetBangPai())
                {
                    SendSysInfo(pUser,"非本帮领地，此功能暂时无法使用");
                    return;
                }
            }
            uint32 requestId = 0;
            msg>>requestId;
            pScene->CreateTeam(pUser,requestId);
            break;
        }
        case 6:
        {//接受邀请
            if(pUser->TempLeaveTeam() != 0)
                return;
            uint32 headId = 0;
            uint8 agree = 0;
            msg>>agree;
            msg>>headId;
            if(agree == 1)
                pScene->AskForJoinTeam(pUser,headId);
            break;
        }
        case 7:
        {
            uint8 page = 0;
            msg>>page;
            pScene->NotInTeamUser(page,msg);
            m_socketServer.SendMsg(pUser->GetSock(),msg);
            break;
        }
        /*case :
        {//队长请求申请列表
            pScene->GetAskForUserList(pUser);
            break;
        }*/
        case 8:
        {//请求队伍成员列表
            pScene->GetTeamMembers(pUser);
            break;
        }
        case 9:
        {//离开队伍
            pScene->LeaveTeam(pUser);
            break;
        }
        /*
        case 8:
        {//队长拒绝加入队伍
            uint32 memberId;
            msg>>memberId;
            pScene->NotAllowJoin(pUser,memberId);
            break;
        }*/
        case 10:
        {//队长剔除队员
            uint32 memberId;
            msg>>memberId;
            pScene->DelTeamMember(pUser,memberId);
            break;
        }
        //更换队长
        case 11:
        {
            uint32 memberId = 0;
            msg>>memberId;
            pScene->SetNewHead(pUser,memberId);
            break;
        }
        //暂离队伍
        case 12:
        {
            if(pUser->TempLeaveTeam())
                return;
            if(pUser->GetRoleId() == pUser->GetTeam())
                return;
            pScene->TempLeaveTeam(pUser);
            break;
        }
        //队员归队
        case 13:
        {
            if(!pUser->TempLeaveTeam())
                return;
            pScene->ReturnTeam(pUser);
            break;
        }
        case 14:
        {
            if(!pUser->TempLeaveTeam())
                return;
            uint8 accept = 0;
            msg>>accept;
            if(accept == 1)
                pScene->ReturnTeam(pUser);
            break;
        }
    }
}

void CPackageDeal::GetPlayerInfo(CNetMessage *pMsg,int sock)
{
	GET_MSG
    uint32 roleId = 0;
    msg>>roleId;
    if(roleId == 0)
    {
        //cout<<"GetPlayerInfo roleId:"<<roleId<<endl;
        return;
    }
    ShareUserPtr ptrMe = m_onlineUser.GetUserBySock(sock);
    CUser *pMe = ptrMe.get();
    if((pMe == NULL) || (pMe->GetRoleId() == 0))
        return;
    
    ShareUserPtr ptr = m_onlineUser.GetUserByRoleId(roleId);
    CUser *pUser = ptr.get();
    if(pUser == NULL)
    {
        //cout<<"GetPlayerInfo roleId:"<<roleId<<endl;
        return;
    }
    
    msg.ReWrite();
    msg.SetType(PRO_PLAYER_INFO);
    pUser->MakePlayerInfo(msg,pMe);
    m_socketServer.SendMsg(pMe->GetSock(),msg);
}

void CPackageDeal::PlayerPk(CNetMessage *pMsg,int sock)
{
	GET_MSG
    GET_USER
    
    CScene *pScene = pUser->GetScene();
    if(pScene == NULL)
        return;
    
    uint32 roleId = 0;
    uint8 op = 0;
    msg>>op;
    bool yaoqing = false;
    if(op == 0)
    {
        /*if(!pScene->InGuiYu() 
            && !((pScene->GetMapId() >= 260) && (pScene->GetMapId() <= 266))
            && (pScene->GetMapId() != 300) && (pScene->GetMapId() != 305))
        {    
            if(!pUser->CanPk())
            {
                SendSysInfo(pUser,"15分钟内禁止PK");
                return;
            }
        }*/
        if((pScene->GetMapId() >= 260) && (pScene->GetMapId() <= 266))
        {
            if(pUser->HaveItem(1543) || pUser->HaveItem(1544))
            {
                SendSysInfo(pUser,"运输矿产过程中无法主动PK ");
                return;
            }
        }
        uint16 mapId = pScene->GetMapId();
        if((mapId == 300) || (mapId == 305))
        {
            if((pScene->GetId()>>16) != (int)pUser->GetBangPai())
            {
                SendSysInfo(pUser,"非本帮领地，此功能暂时无法使用");
                return;
            }
        }
        
        msg>>roleId;
        yaoqing = true;
        if(roleId == pUser->GetRoleId())
            return;
    }
    else if(op == 1)
    {
        uint8 res;
        msg>>res;
        msg>>roleId;
        if(res == 0)
            return;
    }
    pScene->PlayerPk(ptr,roleId,yaoqing);
}

void CPackageDeal::PlayerMatch(CNetMessage *pMsg,int sock)
{
	GET_MSG
    GET_USER
    
    CScene *pScene = pUser->GetScene();
    if(pScene == NULL)
        return;

    uint32 roleId = 0;
    uint8 op = 0;
    msg>>op;
    switch(op)
    {
    case 0://发起切磋
        {
            uint16 mapId = pScene->GetMapId();
            if((mapId == 300) || (mapId == 305))
            {
                if((pScene->GetId()>>16) != (int)pUser->GetBangPai())
                {
                    SendSysInfo(pUser,"非本帮领地，此功能暂时无法使用");
                    return;
                }
            }
            uint8 mod = 0;
            msg>>mod;
            if(mod == 1)
            {
                msg>>roleId;
                if(roleId == pUser->GetRoleId())
                    return;
            }
            pScene->PlayerAskForMatch(ptr,roleId);
            break;
        }
    case 1://被邀请方接受或者拒绝
        {
            uint8 accept = 0;
            msg>>accept;
            msg>>roleId;
            pScene->AcceptAskForMatch(ptr,accept!=0,roleId);
            break;
        }
    }
}

void CPackageDeal::NearPlayerList(CNetMessage *pMsg,int sock)
{
	GET_MSG
    uint8 page = 0;
    msg>>page;
    
    if(page < 1)
        return;
        
    GET_USER
    
    CScene *pScene = pUser->GetScene();
    if(pScene == NULL)
        return;

    msg.ReWrite();
    msg.SetType(PRO_NEAR_PLAYER_LIST);
    pScene->MakeNearPlayerList(pUser,page,msg);
    m_socketServer.SendMsg(pUser->GetSock(),msg);
}

void CPackageDeal::QueryPet(CNetMessage *pMsg,int sock)
{
	GET_MSG
    GET_USER

    pUser->MakePet(msg);
    m_socketServer.SendMsg(pUser->GetSock(),msg);
}

void CPackageDeal::PetOption(CNetMessage *pMsg,int sock)
{
	GET_MSG
    GET_USER
        
    uint8 op = 0;
    uint8 ind = 0;
    msg>>op>>ind;
    switch(op)
    {
    case 0://分配属性点
        {
            uint8 attr = 0;
            msg>>attr;
            pUser->AssignPetAttr(ind,attr);
            break;
        }
    case 1:// 分配抗性点
        {
            uint8 attr = 0;
            msg>>attr;
            pUser->AssignPetKangXing(ind,attr);
            break;
        }
    case 2:// 分配抗异常点 
        {
            uint8 attr = 0;
            msg>>attr;
            attr += 5;
            pUser->AssignPetKangXing(ind,attr);
            break;
        }
    case 3:// 观看 跟随玩家显示在地图上
        {
            if(pUser->HaveBitSet(157))
            {
                msg.ReWrite();
                msg.SetType(MSG_SERVER_USE_RESULT);
                msg<<(uint8)3<<PRO_ERROR<<"任务中不能使用此功能";
                m_socketServer.SendMsg(pUser->GetSock(),msg);
                return;
            }
            pUser->SetGuanKanPet(ind);
            CScene *pScene = pUser->GetScene();
            if(pScene != NULL)
            {
                pScene->UpdateUserInfo(pUser);
            }
        }
        break;
    case 4:// 出战
        {
            if(pUser->HaveBitSet(157))
            {
                msg.ReWrite();
                msg.SetType(MSG_SERVER_USE_RESULT);
                msg<<(uint8)4<<PRO_ERROR<<"任务中不能使用此功能";
                m_socketServer.SendMsg(pUser->GetSock(),msg);
                return;
            }
            pUser->SetChuZhanPet(ind);
            CScene *pScene = pUser->GetScene();
            if(pScene != NULL)
            {
                pScene->UpdateUserInfo(pUser);
            }
        }
        break;
    case 5:// 休息
        {
            if(pUser->HaveBitSet(157))
            {
                //SendSysInfo(pUser,"任务中不能使用此功能");
                msg.ReWrite();
                msg.SetType(MSG_SERVER_USE_RESULT);
                msg<<(uint8)5<<PRO_ERROR<<"任务中不能使用此功能";
                m_socketServer.SendMsg(pUser->GetSock(),msg);
                return;
            }
            pUser->SetPetIdle(ind);
            CScene *pScene = pUser->GetScene();
            if(pScene != NULL)
            {
                pScene->UpdateUserInfo(pUser);
            }
        }
        break;
    case 6://使用物品
        {
            /*
            +----+-----+------+-------+
            | OP | IND | PIND | VALUE |
            +----+-----+------+-------+
            |  1 |  1  |   1  |   4   |
            +----+-----+------+-------+
            */
            uint8 itemInd = 0;
            int val = 0;
            msg>>itemInd>>val;
            msg.ReWrite();
            msg.SetType(PRO_UPDATE_PET);
            msg<<op;
            if(pUser->UseItemToPet(ind,itemInd,NULL,NULL,val))
                msg<<PRO_SUCCESS;
            else
                msg<<PRO_ERROR;
            m_socketServer.SendMsg(pUser->GetSock(),msg);
            break;
        }
    case 7:// 驯养
        //用玩家体力增加宠物忠诚度
        pUser->XunYangPet(ind);
        break;
    case 8:// 遗弃
        {
            SharePetPtr pet = pUser->GetPet(ind);
            SPet *pPet = pet.get();
            if((pPet != NULL) && (pPet->tmplId >= 100))
            {
                return;
            }
            pUser->DelPet(ind);
            CScene *pScene = pUser->GetScene();
            if(pScene != NULL)
            {
                pScene->UpdateUserInfo(pUser);
            }
        }
        break;
    case 9://改名字
        {
            //SPet *pPet = pUser->GetPetByPos(ind);
            string name;
            msg>>name;
            string errMsg;
            if(name.size() > 1)
            {
                msg.ReWrite();
                msg.SetType(MSG_SERVER_DO_PET);
                if(pUser->ModifyPetName(ind,name,errMsg))
                {
                    msg<<PRO_SUCCESS;
                }
                else
                {
                    msg<<PRO_ERROR<<errMsg;
                }
                m_socketServer.SendMsg(pUser->GetSock(),msg);
            }
        }
        break;
    case 10:
        {//升级宠物技能
            uint16 skillId = 0;
            msg>>skillId;
            pUser->PetStudySkill(ind,skillId);
        }
        break;
    case 11:
        {//装备宠物铠甲
            uint8 pos;
            msg>>pos;
            if(pUser->PetKaiJia(ind,pos))
            {
                CScene *pScene = pUser->GetScene();
                if(pScene != NULL)
                {
                    pScene->UpdateUserInfo(pUser);
                }
            }
        }
        break;
    case 12:
        {
            if(pUser->HaveBitSet(157))
            {
                //SendSysInfo(pUser,"任务中不能使用此功能");
                msg.ReWrite();
                msg.SetType(MSG_SERVER_USE_RESULT);
                msg<<(uint8)12<<PRO_ERROR<<"任务中不能使用此功能";
                m_socketServer.SendMsg(pUser->GetSock(),msg);
                return;
            }
            pUser->SetQiPet(ind);
            CScene *pScene = pUser->GetScene();
            if(pScene != NULL)
            {
                pScene->UpdateUserInfo(pUser);
            }
        }
        break;
    default:
        return;
    }
}

void CPackageDeal::GetMissionList(CNetMessage *pMsg,int sock)
{
	GET_MSG

    GET_USER

    pUser->MakeMission(msg);
    m_socketServer.SendMsg(pUser->GetSock(),msg);
}

extern void DelTongJi(int id,uint32 finishedUser = 0);

void CPackageDeal::GetMissionInfo(CNetMessage *pMsg,int sock)
{
	GET_MSG
    GET_USER    
    uint16 missionId;
    msg>>missionId;
    
    if(missionId >= 10000)
    {
        STongJiInfo info;
        char buf[256] = " | | ";
        if(GetTongJiInfo(missionId,&info))
        {
            if(info.t < GetSysTime())
            {
                //DelTongJi(info.id);
                msg<<"任务已过期||";
                m_socketServer.SendMsg(pUser->GetSock(),msg);
                return;
            }
            ShareUserPtr p = m_onlineUser.GetUserByRoleId(info.roleId);
            if(p.get() != NULL)
            {
                CScene *pScene = p->GetScene();
                if(pScene == NULL)
                    return;
                snprintf(buf,256,"%s现在%s,请速去消灭目标。",p->GetName(),pScene->GetName());
            }
            else
            {
                GetRoleName(info.roleId,buf);
                strcat(buf,"离线。");
            }
            int len = strlen(buf);
            int end = info.t - GetSysTime();
            snprintf(buf+len,256-len,"|%d天%d小时%d分。",
                end / (3600*24),end % (3600*24) / 3600,end % (3600) / 60);
            len = strlen(buf);
            snprintf(buf+len,256-len,"|%d金币",info.money);
        }
        msg<<buf;
        m_socketServer.SendMsg(pUser->GetSock(),msg);
        return;
    }
    CCallScript *pScript = GetScript();//script("10000.lua");
    
    char *pInfo = NULL;
    if(pScript != NULL)
    {
        char fun[64];
        sprintf(fun,"GetMissionInfo%d",missionId);
        pUser->SetCallScript(pScript->GetScriptId());
        pScript->Call(fun,"us>s",pUser,pUser->GetMission(missionId),&pInfo);
    }
    
    if(pInfo != NULL)
    {
        msg<<pInfo;
        m_socketServer.SendMsg(pUser->GetSock(),msg);
    }
}

void CPackageDeal::GetUserSkill(CNetMessage *pMsg,int sock)
{
	GET_MSG

    GET_USER
    pUser->MakeUserSkill(msg);
    
    m_socketServer.SendMsg(pUser->GetSock(),msg);
}

void CPackageDeal::GetPetSkill(CNetMessage *pMsg,int sock)
{
	GET_MSG

    uint8 pos = 0;
    msg>>pos;
    
    GET_USER
    pUser->MakePetSkill(pos,msg);
    
    m_socketServer.SendMsg(pUser->GetSock(),msg);
}

void CPackageDeal::BroadcastChat(CUser *pUser,CNetMessage *msg)
{
    if((pUser->GetChatChannel() & 1) == 1)
        m_socketServer.SendMsg(pUser->GetSock(),*msg);
}

void CPackageDeal::ChatChannel(CNetMessage *pMsg,int sock)
{
	GET_MSG

    GET_USER
    uint8 chatVal = pUser->GetChatChannel();
    uint8 channel;
    uint8 open;
    msg>>channel>>open;
    
    if((channel < 1) ||(channel > 6))
        return;
    channel--;
    if(open == 0)
    {//关
        chatVal &= ~(1<<channel);
    }
    else if(open == 1)
    {//开
        chatVal |= 1<<channel;
    }

    //printf("聊天通道设置:%02x,%d\n",chatVal,channel);

    pUser->SetChatChannel(chatVal);
}

//以前是潜能聊天，后来改为管理员功能

static bool AllowIp(in_addr_t ip)
{
#ifdef QQ
    return (ip == inet_addr("10.147.20.215")) || (ip == inet_addr("127.0.0.1"));
#else
    return (ip == inet_addr("117.135.138.39")) || (ip == inet_addr("127.0.0.1"));
#endif
}

void CPackageDeal::SpecChat(CNetMessage *pMsg,int sock)
{
    GET_MSG
    sockaddr_in addr;
    socklen_t len = sizeof(addr);
    getpeername(sock, (sockaddr*)&addr,&len);
    if(!AllowIp(addr.sin_addr.s_addr))
    {
        GET_USER
        if(pUser->AdminLevel() <= 0)
            return;
    }
    uint32 roleId;
    uint8 type;
    msg>>type>>roleId;
    
    switch(type)
    {
    case 1:
        {
            CGetDbConnect getDb;
            CDatabaseSql *pDb = getDb.GetDbConnect();
            if(pDb != NULL)
            {
                char buf[64];
                snprintf(buf,63,"update role_info set state=2 where id=%d",roleId);
                pDb->Query(buf);
            }
            ShareUserPtr p = m_onlineUser.GetUserByRoleId(roleId);
            if(p.get() != NULL)
            {
                msg.ReWrite();
                msg.SetType(PRO_MSG_CHAT);
                char buf[64];
                snprintf(buf,63,"【%s】被封号",p->GetName());
                msg<<(uint8)1<<0<<"系统管理员"<<buf;
                m_onlineUser.ForEachUser(boost::bind(&CPackageDeal::BroadcastChat,this,_1,&msg));
            }
        }
        break;
    case 2:
        {
            ShareUserPtr p = m_onlineUser.GetUserByRoleId(roleId);
            if(p.get() != NULL)
            {
                p->SetChatTime(GetSysTime()+1800);
                msg.ReWrite();
                msg.SetType(PRO_MSG_CHAT);
                char buf[64];
                snprintf(buf,63,"【%s】被禁言半小时",p->GetName());
                msg<<(uint8)1<<0<<"系统管理员"<<buf;
                m_onlineUser.ForEachUser(boost::bind(&CPackageDeal::BroadcastChat,this,_1,&msg));
            }
        }
        break;
    case 3:
        {
            ShareUserPtr p = m_onlineUser.GetUserByRoleId(roleId);
            if(p.get() != NULL)
            {
                shutdown(p->GetSock(),SHUT_RD);
            }
        }
        break;
    case 4:
        {//跟新npc脚本
            /*SNpcTemplate *pNpc = m_npcManager.GetNpcTemplate(roleId);
            if((pNpc != NULL) && (pNpc->pScript != NULL))
                pNpc->pScript->ReLoad();*/
            CCallScript *pScript = FindScript(roleId);
            if(pScript != NULL)
                pScript->ReLoad();
        }
        break;
    case 5:
        {//跟新物品脚本
            CItemTemplateManager &itemMgr = SingletonItemManager::instance();
            SItemTemplate *pItem = itemMgr.GetItem(roleId);
            if((pItem != NULL) && (pItem->pScript != NULL))
            {
                pItem->pScript->ReLoad();
            }
        }
        break;
    case 6:
        {
            string info;
            msg>>info;
            //cout<<info<<endl;
            ShareUserPtr p = m_onlineUser.GetUserByRoleId(roleId);
            if(p.get() != NULL)
            {
                SendSysMail(p.get(),info.c_str());
            }
            else
            {
                CGetDbConnect getDb;
                CDatabaseSql *pDb = getDb.GetDbConnect();
                if(pDb != NULL)
                {
                    char sql[1024];
                    sprintf(sql,"INSERT INTO role_mail (from_id,to_id,from_name,time,msg) VALUES ("\
                                "%d,%d,'%s',%lu,'%s')",
                                0,roleId,"【系统】",GetSysTime(),info.c_str());
                    pDb->Query(sql);
                }
            }
        }
        break;
    case 7:
        /*OP=7 添加称号
        +-----+-----+-----+
        | OP  | CID | TID |
        +-----+-----+-----+
        |  1  |  4  |  2  |
        +-----+-----+-----+*/
        {
            uint16 title = 0;
            msg>>title;
            COnlineUser &onlineUser = SingletonOnlineUser::instance();
            ShareUserPtr ptr = onlineUser.GetUserByRoleId(roleId);
            if(ptr.get() != NULL)
            {
                ptr->AddTitle(title);
            }
            else
            {
                CUser *pUser = new CUser;
                auto_ptr<CUser> user(pUser);
                CGetDbConnect getDb;
                CDatabaseSql *pDb = getDb.GetDbConnect();
                if(pDb == NULL)
                    return;
                char sql[4096];
                snprintf(sql,512,"select title from role_info where id=%u",roleId);
                if(!pDb->Query(sql))
                    return;
                char **row = pDb->GetRow();
                if(row == NULL)
                    return;
                pUser->ReadTitle(row[0]);  
                pUser->AddTitle(title);
                string str;
                pUser->GetTitleStr(str);
                snprintf(sql,4096,"update role_info set title='%s' where id=%u",
                    str.c_str(),roleId);
                pDb->Query(sql);
            }
        }
    }
    msg.ReWrite();
    msg.SetType(PRO_SPEC_CHAT);
    m_socketServer.SendMsg(sock,msg);
}

void CPackageDeal::UserChat(CNetMessage *pMsg,int sock)
{
	GET_MSG

    GET_USER
        
    uint8 chanel = 0;
    string chatMsg;
    msg>>chanel>>chatMsg;
    if(chatMsg.size() <= 0)
        return;
        
    msg.ReWrite();
    msg.SetType(PRO_MSG_CHAT);
    msg<<chanel<<pUser->GetRoleId()<<pUser->GetName()<<chatMsg;
    
    if(IsIllegalMsg(chatMsg.c_str()))
    {
        SendSysChannelMsg(pUser,"您的发言中有屏蔽文字");
        return;
    }
    /***********************************
    1 世界 服务器收到此条消息 需向全部在线玩家广播
    2 附近 服务器向当前地图上的玩家广播
    3 队伍 服务器在队伍中广播
    4 门派 服务器向同一门派的玩家广播
    5 帮派 服务器向同一帮派中的玩家广播
    ************************************/
    switch(chanel)
    {
    case 1://世界
        if(pUser->AdminLevel() > 0)
        {
            m_onlineUser.ForEachUser(boost::bind(&CPackageDeal::BroadcastChat,this,_1,&msg));
        }
        else if(pUser->CanChat())
        {
            if(pUser->GetTili() >= USE_TILI_CHAT)
            {
                pUser->AddTili(-USE_TILI_CHAT);
                pUser->SetChatTime(GetSysTime());
                m_onlineUser.ForEachUser(boost::bind(&CPackageDeal::BroadcastChat,this,_1,&msg));
            }
        }
        break;
    
    case 2://附近
        {
            if(chatMsg.size() > 20)
                return;
            CScene *pScene = pUser->GetScene();
            if(pScene == NULL)
                return;
            pScene->SceneChat(msg);
            break;
        }
    case 3://队伍
        {
            CScene *pScene = pUser->GetScene();
            if(pScene == NULL)
                return;
            if(pUser->GetTeam() != 0)
                pScene->TeamChat(pUser->GetTeam(),msg);
            break;
        }
    case 4:// 帮派 服务器向同一帮派中的玩家广播
        {
            if(pUser->GetBangPai() == 0)
                break;
            CBangPai *pBangPai = m_bangPaiMgr.FindBangPai(pUser->GetBangPai());
            if(pBangPai == NULL)
                break;
            list<uint32> userList;
            pBangPai->GetMember(userList);
            for(list<uint32>::iterator i = userList.begin(); i != userList.end(); i++)
            {
                ShareUserPtr ptr = m_onlineUser.GetUserByRoleId(*i);
                if((ptr.get() != NULL) //(pUser->GetRoleId() != ptr->GetRoleId())
                    && ((ptr->GetChatChannel() & (1<<3)) == (1<<3)))
                {
                    if(ptr->GetBangPai() != pUser->GetBangPai())
                        ptr->SetBangPai(0);
                    else
                        m_socketServer.SendMsg(ptr->GetSock(),msg);
                }
            }
            break;
        }
    case 5://战斗聊天
        {
            if(chatMsg.size() > 20)
                return;
            uint32 fightId = pUser->GetFightId();
            if(fightId != 0)
            {
                ShareFightPtr ptr = SingletonFightManager::instance().FindFight(fightId);
                if(ptr.get() != NULL)
                {
                    ptr->BroadcastMsg(msg);
                }
            }
            else
            {
                CScene *pScene = pUser->GetScene();
                if(pScene == NULL)
                    return;
                msg.ReWrite();
                msg.SetType(PRO_MSG_CHAT);
                msg<<(uint8)3<<pUser->GetRoleId()<<pUser->GetName()<<chatMsg;
                if(pUser->GetTeam() != 0)
                    pScene->TeamChat(pUser->GetTeam(),msg);
            }
            break;
        }
    case 6://系统信件
        break;
    }
}

void CPackageDeal::FuQi(CNetMessage *pMsg,int sock)
{
    GET_MSG
    GET_USER

    uint32 roleId = pUser->GetData32(6);
    if(roleId == 0)
    {
        msg<<(uint8)0;
        m_socketServer.SendMsg(pUser->GetSock(),msg);
        return;
    }
    else
    {
        msg<<(uint8)1;
        ShareUserPtr ptr = m_onlineUser.GetUserByRoleId(roleId);
        CUser *pUser = ptr.get();
        if(pUser == NULL)
        {
            CGetDbConnect getDb;
            CDatabaseSql *pDb = getDb.GetDbConnect();
            char sql[128];
            sprintf(sql,"select name,sex from role_info where id=%d",roleId);
            if ((pDb != NULL) && (pDb->Query(sql)))
            {
                char **row = pDb->GetRow();
                if(row == NULL)
                {
                    return;
                }
                msg<<(uint8)atoi(row[1])<<roleId<<row[0]<<(uint16)0;
            }
        }
        else
        {
            msg<<pUser->GetSex()<<roleId<<pUser->GetName()<<pUser->GetSceneId();
        }
    }
    uint16 hotVal = 0;
    pUser->GetHotVal(roleId,hotVal);
    msg<<hotVal;
    m_socketServer.SendMsg(pUser->GetSock(),msg);
}

void CPackageDeal::GetHotList(CNetMessage *pMsg,int sock)
{
	GET_MSG

    GET_USER

    list<HotInfo> hotList;
    pUser->GetHotList(hotList);
    msg<<(uint8)hotList.size();
    
    char **row;
    char sql[128];
    CGetDbConnect getDb;
    CDatabaseSql *pDb = getDb.GetDbConnect();
        
    for(list<HotInfo>::iterator i = hotList.begin(); i != hotList.end(); i++)
    {
        msg<<i->hotId;
        ShareUserPtr ptr = m_onlineUser.GetUserByRoleId(i->hotId);
        CUser *pUser = ptr.get();
        if(pUser == NULL)
        {
            sprintf(sql,"select name,level from role_info where id=%d",i->hotId);
            if ((pDb == NULL) || !(pDb->Query(sql)))
            {
                msg<<" "<<(uint8)0<<(uint16)0;
            }
            else
            {
                row = pDb->GetRow();
                if(row == NULL)
                {
                    msg<<" "<<(uint8)0<<(uint16)0;
                }
                else
                {
                    msg<<row[0]<<(uint8)atoi(row[1])<<(uint16)0;
                }
            }
        }
        else
        {
            msg<<pUser->GetName()<<(uint8)pUser->GetLevel()<<pUser->GetMapId();
        }
        msg<<i->hotVal;
    }
    m_socketServer.SendMsg(pUser->GetSock(),msg);
}

void CPackageDeal::AddHot(CNetMessage *pMsg,int sock)
{
	GET_MSG

    GET_USER

    char sql[128];
    CGetDbConnect getDb;
    CDatabaseSql *pDb = getDb.GetDbConnect();
    if(pDb == NULL)
    {
        return;
    }
    
    uint8 type = 0;
    uint32 userId = 0;
    msg>>type;
    char **row;
    
    if(type == 0)
    {
        msg>>userId;
        if(userId == pUser->GetRoleId())
            return;
    }
    else if(type == 1)
    {
        string name;
        msg>>name;
        if(strcmp(pUser->GetName(),name.c_str()) == 0)
            return;
        sprintf(sql,"select id from role_info where name=\"%s\"",name.c_str());
        if(!pDb->Query(sql) || ((row = pDb->GetRow()) == NULL))
        {
            msg.ReWrite();
            msg.SetType(PRO_ADD_HOT);
            msg<<PRO_ERROR;
            m_socketServer.SendMsg(pUser->GetSock(),msg);
            return;
        }
        userId = atoi(row[0]);
    }
    msg.ReWrite();
    msg.SetType(PRO_ADD_HOT);
    if(userId == 0)
    {
        msg<<PRO_ERROR;
        m_socketServer.SendMsg(pUser->GetSock(),msg);
        return;
    }
    //增加对id合法性的判断
    sprintf(sql,"select id from role_info where id=%u",userId);
    if(!pDb->Query(sql) || ((row = pDb->GetRow()) == NULL))
    {
        msg<<PRO_ERROR;
        m_socketServer.SendMsg(pUser->GetSock(),msg);
        return;
    }
    if(!pUser->AddHot(userId))
    {
        msg<<PRO_ERROR;
        m_socketServer.SendMsg(pUser->GetSock(),msg);
        return;
    }
    
    msg<<PRO_SUCCESS;
    m_socketServer.SendMsg(pUser->GetSock(),msg);
}

void CPackageDeal::DelHot(CNetMessage *pMsg,int sock)
{
	GET_MSG

    GET_USER

    uint32 roleId = 0;
    msg>>roleId;
    
    pUser->DelHot(roleId);
    
    DelNotifyUser(pUser->GetRoleId(),roleId);
}

void CPackageDeal::OnlineMail(CNetMessage *pMsg,int sock)
{
	GET_MSG

    GET_USER
    uint32 userId = 0;
    uint8 type;
    string info;
    msg>>type;
    
    if(GetSysTime() - pUser->GetActivityTime() < 3)
    {
        SendPopMsg(pUser,"你发信过快");
        return;
    }
    pUser->SetActivityTime(GetSysTime());
    
    if(type == 0)
    {
        msg>>userId>>info;
        if(userId == 0)
        {
            CGetDbConnect getDb;
            CDatabaseSql *pDb = getDb.GetDbConnect();
            boost::format fmt("INSERT INTO sys_mail (role_id,msg,time,name) "\
                            "VALUES (%1%,'%2%',FROM_UNIXTIME(%3%),'%4%')");
            fmt%pUser->GetRoleId()
                %info.c_str()
                %GetSysTime()
                %pUser->GetName();
                
            if(pDb != NULL)
                pDb->Query(fmt.str().c_str());
            return;
        }
        if(userId == pUser->GetRoleId())
            return;
    }
    else if(type == 1)
    {
        string name;
        msg>>name>>info;
        if(strcmp(pUser->GetName(),name.c_str()) == 0)
            return;
        char sql[128];
        sprintf(sql,"select id from role_info where name=\"%s\"",name.c_str());
        CGetDbConnect getDb;
        CDatabaseSql *pDb = getDb.GetDbConnect();
        char **row;
        if(!pDb->Query(sql) || ((row = pDb->GetRow())) == NULL)
        {
            return;
        }
        userId = atoi(row[0]);
    }
    else 
    {
        return;
    }
    
    ShareUserPtr pToPtr = m_onlineUser.GetUserByRoleId(userId);
    CUser *pTo = pToPtr.get();
    string toName;
    
    uint8 month,day,hour,min;
    GetTime(GetSysTime(),month,day,hour,min);
    
    if(pTo == NULL)
    {
        CGetDbConnect getDb;
        CDatabaseSql *pDb = getDb.GetDbConnect();
        
        char sql[1024];
        sprintf(sql,"select name from role_info where id=%d",userId);
        char **row;
        if((pDb != NULL) && pDb->Query(sql) && ((row = pDb->GetRow()) != NULL))
        {
            toName = row[0];
        }
        else
        {
            return;
        }
        
        sprintf(sql,"INSERT INTO role_mail (from_id,to_id,from_name,time,msg) VALUES ("\
                    "%d,%d,'%s',%lu,'%s')",
                    pUser->GetRoleId(),userId,pUser->GetName(),GetSysTime(),info.c_str());
                    
        pDb->Query(sql);
    }
    else
    {
        if(!pTo->HaveIgnore(pUser->GetRoleId()))
        {
            msg.ReWrite();
            msg.SetType(PRO_ONLINE_MAIL);
            msg<<pUser->GetRoleId()<<pUser->GetName()<<info<<month<<day<<hour<<min;
            m_socketServer.SendMsg(pTo->GetSock(),msg);
        }
        toName = pTo->GetName();
    }
    msg.ReWrite();
    msg.SetType(PRO_MAIL_ACK);
    msg<<userId<<toName<<info<<month<<day<<hour<<min;
    m_socketServer.SendMsg(pUser->GetSock(),msg);
}

void CPackageDeal::QueryMail(CNetMessage *pMsg,int sock)
{
	GET_MSG

    GET_USER
    
    CGetDbConnect getDb;
    CDatabaseSql *pDb = getDb.GetDbConnect();
    
    char sql[256];
    sprintf(sql,"select from_id,from_name,time,msg from role_mail where to_id=%d order by id limit 50",pUser->GetRoleId());
    
    if ((pDb == NULL) || !pDb->Query(sql))
    {
        return;
    }
    char **row;
    uint8 num = pDb->GetRowNum();
    msg<<num;
    uint8 month,day,hour,min;
    uint32 fromId = 0;
    while((row = pDb->GetRow()) != NULL)
    {
        fromId = atoi(row[0]);
        if(!pUser->HaveIgnore(fromId))
        {
            GetTime(atoi(row[2]),month,day,hour,min);
            msg<<fromId<<row[1]<<row[3]<<month<<day<<hour<<min;
        }
    }
    m_socketServer.SendMsg(pUser->GetSock(),msg);
    sprintf(sql,"delete from role_mail where to_id=%d",pUser->GetRoleId());
    pDb->Query(sql);
}

void CPackageDeal::UserShop(CNetMessage *pMsg,int sock)
{
	GET_MSG

    GET_USER
    
    uint32 userId = 0;
    msg>>userId;
    msg.ReWrite();
    msg.SetType(PRO_PSHOP_LIST);
    //if(userId == pUser->GetRoleId()
    //msg<<userId;
    if(userId == 0)
    {
        msg<<pUser->GetRoleId();
        pUser->MakeShopItemList(msg);
    }
    else
    {
        ShareUserPtr ptr = m_onlineUser.GetUserByRoleId(userId);
        CUser *p = ptr.get();
        if(p != NULL)
        {
            msg<<userId;
            p->MakeShopItemList(msg);
        }
        else
        {
            msg<<0;
        }
    }
    m_socketServer.SendMsg(pUser->GetSock(),msg);
}

void CPackageDeal::BuyShopItem(CNetMessage *pMsg,int sock)
{
	GET_MSG

    GET_USER
    
    CScene *pScene = pUser->GetScene();
    if((pScene != NULL) && pScene->InMatchScene())
    {
        SendSysInfo(pUser,"本场景不允许交易");
        return;
    }
    
    uint32 userId;
    uint8 pos;
    uint32 shopItemId;
    msg>>userId>>pos>>shopItemId;
    ShareUserPtr pTo = m_onlineUser.GetUserByRoleId(userId);
    CUser *p = pTo.get();
    msg.ReWrite();
    msg.SetType(PRO_PSHOP_BUY);
    if(p != NULL)
    {
        string errMsg;
        if(pUser->BuyUserShopItem(p,pos,shopItemId,errMsg))
        {
            msg<<PRO_SUCCESS<<pos;
        }
        else
        {
            msg<<PRO_ERROR<<pos<<errMsg;
        }
    }
    else
    {
        msg<<PRO_ERROR<<pos<<"物品不存在";
    }
    m_socketServer.SendMsg(pUser->GetSock(),msg);
}

void CPackageDeal::PutItemToShop(CNetMessage *pMsg,int sock)
{
	GET_MSG

    GET_USER
    
    uint8 op = 0;
    uint8 ind = 0;
    msg>>op;
    if(op == 0)
    {//下架
        msg>>ind;
        msg.ReWrite();
        msg.SetType(PRO_PSHOP_PUT);
        if(pUser->TakeDownShopItem(ind))
            msg<<PRO_SUCCESS;
        else
            msg<<PRO_ERROR;
        msg<<op<<ind;
        m_socketServer.SendMsg(pUser->GetSock(),msg);
    }
    else if(op == 1)
    {//上架
        uint8 type;
        uint8 num;
        int money;
        msg>>type>>ind>>num>>money;
        uint8 recvType = type;
        if(money < 0)
            return;
        if(type == 0)
            type = SSITItem;
        else if(type == 1)
            type = ESITPet;
        else 
            return;
        msg.ReWrite();
        msg.SetType(PRO_PSHOP_PUT);
        uint16 id = pUser->PutItemToShop(type,ind,num,money);
        if(id != 0)
            msg<<PRO_SUCCESS;
        else
            msg<<PRO_ERROR;
        //ind商店中位置
        msg<<op<<ind<<id<<recvType<<num<<money;
        m_socketServer.SendMsg(pUser->GetSock(),msg);
    }
}

void CPackageDeal::ShopItemInfo(CNetMessage *pMsg,int sock)
{
	GET_MSG

    GET_USER
    
    uint8 src;
    uint32 userId;
    uint8 ind;
    msg>>src>>userId>>ind;
    msg.ReWrite();
    msg.SetType(PRO_PSHOP_DETAIL);
    
    CUser *pTo = pUser;
    ShareUserPtr pToPtr;
    if(userId != 0)
    {
        pToPtr = m_onlineUser.GetUserByRoleId(userId);
        pTo = pToPtr.get();
    }
    
    if(pTo == NULL)
    {
        msg<<PRO_ERROR;
    }
    else
    {
        msg<<PRO_SUCCESS<<src;
        pTo->MakeShopItemInfo(ind,msg);
    }
    m_socketServer.SendMsg(pUser->GetSock(),msg);
    //cout<<"send shop item info"<<endl;
}

void CPackageDeal::GetUserDetail(CNetMessage *pMsg,int sock)
{
	GET_MSG

    GET_USER

    uint32 userId = 0;
    msg>>userId;
    ShareUserPtr pToPtr = m_onlineUser.GetUserByRoleId(userId);
    CUser *pTo = pToPtr.get();
    msg.ReWrite();
    msg.SetType(PRO_PLAYER_DETAIL);
    
    if(pTo == NULL)
    {
        msg<<PRO_ERROR;
    }
    else
    {
        /*
+-----+-----+------+-----+------+------+------+------+--------+--------+-----+--------
| RES | CID | HEAD | SEX |  LEN | 名称 | 等级 | 门派 | 宠物ID | 帮派ID | LEN | 帮派名 
+-----+-----+------+-----+------+------+------+------+--------+--------+-----+--------
|  1  |  4  |   1  |  1  |   2  |  Var |  1   |   1  |    2   |    4   |  2  |   Var  
+-----+-----+------+-----+------+------+------+------+--------+--------+-----+--------
+--------+--------+--------+--------+--------+--------+--------+--------+--------+------+------+
| 帽子ID | 衣服ID | 腰带ID | 鞋子ID | 武器ID | 项链ID | 玉佩ID | 手镯ID | 手镯ID | 道行 | 魔障 |
+--------+--------+--------+--------+--------+--------+--------+--------+--------+------+------+
|    2   |   2    |    2   |    2   |    2   |   2    |    2   |   2    |   2    |   4  |   2  |
+--------+--------+--------+--------+--------+--------+--------+--------+--------+------+------+
        */
        uint16 petId = 0;
        SharePetPtr pet;
        if(pTo->GetChuZhanPet(pet))
        {
            petId = pet->tmplId;
        }
        msg<<PRO_SUCCESS<<userId<<pTo->GetHead()<<pTo->GetSex()<<pTo->GetName()
            <<pTo->GetLevel()<<pTo->GetMenPai()<<petId<<pTo->GetBangPai();
        CBangPai *pBangPai = m_bangPaiMgr.FindBangPai(pTo->GetBangPai());
        if(pBangPai != NULL)
            msg<<pBangPai->GetName();
        pTo->MakeOtherEquip(msg);
        msg<<pTo->GetDaoHang()<<pTo->GetPkVal()<<pTo->AdminLevel();
    }
    m_socketServer.SendMsg(pUser->GetSock(),msg);
}

void CPackageDeal::SafeTrade(CNetMessage *pMsg,int sock)
{
	GET_MSG

    GET_USER
    
    CScene *pScene = pUser->GetScene();
    if((pScene != NULL) && pScene->InMatchScene())
    {
        SendSysInfo(pUser,"本场景不允许交易");
        return;
    }
    uint8 op = 0;
    msg>>op;
    switch(op)
    {
    case 1://请求交易
        {
            if(GetSysTime() - pUser->GetActivityTime() < 3)
            {
                SendPopMsg(pUser,"你交易速度过快");
                return;
            }
            pUser->SetActivityTime(GetSysTime());
            
            uint16 mapId = pScene->GetMapId();
            if((mapId == 300) || (mapId == 305))
            {
                if((pScene->GetId()>>16) != (int)pUser->GetBangPai())
                {
                    SendSysInfo(pUser,"非本帮领地，此功能暂时无法使用");
                    return;
                }
            }
            uint32 userId = 0;
            msg>>userId;
            ShareUserPtr pToPtr = m_onlineUser.GetUserByRoleId(userId);
            CUser *pTo = pToPtr.get();
            if(pTo == NULL)
            {
                SendSysInfo(pUser,"对方已下线不能交易");
                return;
            }
            if(pTo->HaveIgnore(pUser->GetRoleId()))
                return;
            if(pTo->GetFightId() != 0)
            {
                SendSysInfo(pUser,"对方在战斗中不能交易");
                return;
            }
            if(pTo->IsInTrade())
            {
                SendSysInfo(pUser,"对方正在和别人交易");
                return;
            }
            pUser->SetTradeUser(userId);
            pTo->SetTradeUser(pUser->GetRoleId());
            /*string name = "【";
            name.append(pUser->GetName());
            name.append("】");*/
            msg<<pUser->GetName()<<pUser->AdminLevel()<<pUser->GetLevel();
            m_socketServer.SendMsg(pTo->GetSock(),msg);
            return;
        }
    case 2://同意开始交易
        {
            uint8 answer = 0;
            msg>>answer;
            ShareUserPtr pToPtr = m_onlineUser.GetUserByRoleId(pUser->GetTradeUser());
            CUser *pTo = pToPtr.get();
            if(pTo == NULL)
            {
                SendSysInfo(pUser,"对方已下线");
                pUser->SetTradeUser(0);
                return;
            }
            if(pTo->GetFightId() != 0)
            {
                SendSysInfo(pUser,"对方在战斗中不能交易");
                pUser->SetTradeUser(0);
                pTo->SetTradeUser(0);
                return;
            }
            if(pTo->GetTradeUser() != pUser->GetRoleId())
            {
                pUser->SetTradeUser(0);
                return;
            }
            msg.ReWrite();
            msg.SetType(PRO_SAFE_TRADE);
            if(answer == 0)
            {
                msg<<(uint8)2<<answer<<pUser->GetRoleId()<<pUser->GetName();
                m_socketServer.SendMsg(pTo->GetSock(),msg);
                pUser->SetTradeUser(0);
                pTo->SetTradeUser(0);
                return;
            }
            pUser->SetInTrade();
            pTo->SetInTrade();
            msg<<(uint8)4<<pUser->GetRoleId()<<pUser->GetName()<<pTo->GetRoleId()<<pTo->GetName();
            m_socketServer.SendMsg(pTo->GetSock(),msg);
            m_socketServer.SendMsg(pUser->GetSock(),msg);
            return;
        }
    case 3://取消交易
        {
            ShareUserPtr pToPtr = m_onlineUser.GetUserByRoleId(pUser->GetTradeUser());
            CUser *pTo = pToPtr.get();
            if(pTo == NULL)
                return;
            if(!pTo->IsInTrade())
            {
                pUser->SetTradeUser(0);
                return;
            }
            msg<<pTo->GetRoleId()<<pTo->GetName();
            m_socketServer.SendMsg(pTo->GetSock(),msg);
            pUser->SetTradeUser(0);
            pTo->SetTradeUser(0);
            return;
        }
    case 5://发送交易物品给服务器
        {
            //msg.ReWrite();
            //msg.SetType(PRO_SAFE_TRADE);
            //pUser->GetTradeInfo(msg);
            ShareUserPtr pToPtr = m_onlineUser.GetUserByRoleId(pUser->GetTradeUser());
            CUser *pTo = pToPtr.get();
            if(pTo == NULL)
                return;
            if(!pTo->IsInTrade())
            {
                return;
            }
            if(!pUser->SetTradeItem(msg))
            {
                pUser->SetTradeUser(0);
                pTo->SetTradeUser(0);
                msg.ReWrite();
                msg.SetType(PRO_SAFE_TRADE);
                msg<<(uint8)7;
                msg<<PRO_ERROR;
                m_socketServer.SendMsg(pUser->GetSock(),msg);
                m_socketServer.SendMsg(pTo->GetSock(),msg);
            }
            else
            {
                m_socketServer.SendMsg(pTo->GetSock(),msg);
            }
            return;
        }
    case 6://确认交易
        {
            pUser->TradeOk();
            ShareUserPtr pToPtr = m_onlineUser.GetUserByRoleId(pUser->GetTradeUser());
            CUser *pTo = pToPtr.get();
            if(pTo == NULL)
                return;
            m_socketServer.SendMsg(pTo->GetSock(),msg);
            if(!pTo->IsInTrade())
            {
                return;
            }
            if(!pTo->TradeIsOk())
                return;
            msg.ReWrite();
            msg.SetType(PRO_SAFE_TRADE);
            msg<<(uint8)7;
            
            if(pUser->TradeItem(pTo))
            {
                msg<<PRO_SUCCESS;
            }
            else
            {
                msg<<PRO_ERROR;
            }
            pUser->SetTradeUser(0);
            pTo->SetTradeUser(0);
            //m_socketServer.SendMsg(pUser->GetSock(),msg);
            //m_socketServer.SendMsg(pTo->GetSock(),msg);
            
            m_socketServer.SendMsg(pUser->GetSock(),msg);
            m_socketServer.SendMsg(pTo->GetSock(),msg);
            return;
        }
    /*case 7://完成交易
        {
            pUser->TradeComplate();
            ShareUserPtr pToPtr = m_onlineUser.GetUserByRoleId(pUser->GetTradeUser());
            CUser *pTo = pToPtr.get();
            if(pTo == NULL)
                return;
            if(!pTo->TradeIsComplate())
                return;
            
            if(pUser->TradeItem(pTo))
            {
                msg<<PRO_SUCCESS;
            }
            else
            {
                msg<<PRO_ERROR;
            }
            pUser->SetTradeUser(0);
            pTo->SetTradeUser(0);
            m_socketServer.SendMsg(pUser->GetSock(),msg);
            m_socketServer.SendMsg(pTo->GetSock(),msg);
            return;
        }*/
    }
}

void CPackageDeal::CloseUser(CNetMessage *pMsg,int sock)
{
	GET_MSG

    GET_USER
    
    CScene *pScene = pUser->GetScene();
    if(pScene == NULL)
        return;
    pScene->MakeCloseUser(pUser,msg,pUser->GetBangPai(),pUser->GetXZBangId());
    m_socketServer.SendMsg(pUser->GetSock(),msg);
}

static int GetUserName(uint32 id,string &name,uint8 *pLevel = NULL)
{
    CGetDbConnect getDb;
    CDatabaseSql *pDb = getDb.GetDbConnect();
    if(pDb == NULL)
        return 0;
    
    char sql[128];
    sprintf(sql,"select name,level,save_data from role_info where id=%u",id);
    if(!pDb->Query(sql))
        return 0;
    char **row =pDb->GetRow();
    int bangGong = 0;
    if(row != NULL)
    {
        name = row[0];
        if(pLevel != NULL)
            *pLevel = atoi(row[1]);
        CUser *pUser = new CUser;
        pUser->ReadSaveData(row[2]);
        bangGong = pUser->GetData32(5);
        delete pUser;
    }
    return bangGong;
}
                                                          //true add,false del
static void UpdateBangPai(uint32 userId,CBangPai *pBangPai,bool flag)
{
    if(pBangPai == NULL)
        return;
        
    CGetDbConnect getDb;
    CDatabaseSql *pDb = getDb.GetDbConnect();
    if(pDb == NULL)
        return;
    
    char sql[128];
    sprintf(sql,"select bangpai from role_info where id=%u",userId);
    if(!pDb->Query(sql))
        return;
    char **row =pDb->GetRow();
    
    if(row == NULL)
        return;
    
    if(flag && (atoi(row[0]) != 0))
    {
        return;
    }
    
    if(flag)
    {
        if(!pBangPai->AddMember(userId,EBRBangZhong))
            return;
        sprintf(sql,"update role_info set bangpai=%u where id=%u",pBangPai->GetId(),userId);
    }
    else
    {
        char buf[4096];
        snprintf(buf,4096,"select save_data,name from role_info where id=%u",userId);
        if(!pDb->Query(buf))
            return;
        char **row = pDb->GetRow();
        if(row == NULL)
            return;
            
        CUser *p = new CUser;
        p->ReadSaveData(row[0]);  
        p->SetData32(2,GetSysTime());
        string str;
        p->WriteSaveData(str);
        snprintf(buf,4096,"update role_info set save_data='%s',bangpai=0 where id=%u",str.c_str(),userId);
        pDb->Query(buf);
        
        p->LoadMission();
        p->DelMission(14);
        delete p;
    }
    pDb->Query(sql);
}

void CPackageDeal::BangPai(CNetMessage *pMsg,int sock)
{
	GET_MSG
    GET_USER
        
    uint8 op = 0;
    msg>>op;
    //boost::mutex::scoped_lock lk(m_bpMutex);
    switch(op)
    {
    case 1://创建帮派
        {
            string name;
            msg>>name;
            msg.ReWrite();
            msg.SetType(PRO_BANGPAI);
            msg<<(uint8)1;
            CBangPai *pBangPai = NULL;
            if(name.size() > 2)
                pBangPai = m_bangPaiMgr.CreateBangPai(pUser,name.c_str());
            if(pBangPai != NULL)
            {
                msg<<PRO_SUCCESS;
                pUser->SetBangPai(pBangPai->GetId());
                pUser->UpdateBangPai();
            }
            else
            {
                msg<<PRO_ERROR;
            }
            m_socketServer.SendMsg(pUser->GetSock(),msg);
            break;
        }
    case 2://帮派列表
        {
            uint8 page;
            msg>>page;
            msg.ReWrite();
            msg.SetType(PRO_BANGPAI);
            msg<<(uint8)2<<page;
            m_bangPaiMgr.MakeBangPaiList(page,msg,pUser->GetBangPai());
            m_socketServer.SendMsg(pUser->GetSock(),msg);
            break;
        }
    case 3://申请加入帮派
        {
            if(pUser->GetLevel() < 30)
            {
                SendSysInfo(pUser,"等级低于30级不能加入帮派");
                return;
            }
            if(pUser->GetBangPai() != 0)
            {
                SendSysInfo(pUser,"目前有帮派");
                return;
            }
            if(pUser->GetData32(2) + 3*3600*24 > (uint32)GetSysTime())
            {
                SendSysInfo(pUser,"离开帮派三天内不得加入新帮");
                return;
            }
            uint32 bangpaiId = 0;
            msg>>bangpaiId;
            if(bangpaiId == 0)
                break;
            CBangPai *pBangPai = m_bangPaiMgr.FindBangPai(bangpaiId);
            if(pBangPai == NULL)
            {
                break;
            }
            if(!pBangPai->AddAskJoin(pUser->GetRoleId()))
            {
                break;
            }
            ShareUserPtr p = m_onlineUser.GetUserByRoleId(pBangPai->GetBangZhu());
            CUser *pU = p.get();
            if(pU != NULL)
            {
                string str = pUser->GetName();
                str += ":申请加入帮派";
                SendSysInfo(pU,str.c_str());
            }
            break;
        }
    case 4://向帮派中有帮众管理权限的玩家请求加入帮派
        {
            uint32 userId = 0;
            msg>>userId;
            ShareUserPtr p = m_onlineUser.GetUserByRoleId(userId);
            CUser *pU = p.get();
            msg.ReWrite();
            msg.SetType(PRO_BANGPAI);
            msg<<(uint8)4;
            if((pU == NULL) || (pU->GetBangPai() == 0))
            {
                msg<<PRO_ERROR;
                m_socketServer.SendMsg(pUser->GetSock(),msg);
                break;
            }
            CBangPai *pBangPai = m_bangPaiMgr.FindBangPai(userId);
            if((pBangPai == NULL) || (!pBangPai->IsAdmin(userId)))
            {
                msg<<PRO_ERROR;
                m_socketServer.SendMsg(pUser->GetSock(),msg);
                break;
            }
            msg<<PRO_SUCCESS;
            m_socketServer.SendMsg(pUser->GetSock(),msg);
            msg.ReWrite();
            msg.SetType(PRO_BANGPAI);
            msg<<(uint8)5;
            msg<<pUser->GetRoleId()<<pUser->GetName();
            m_socketServer.SendMsg(pU->GetSock(),msg);
            break;
        }
    case 6://帮派中有帮众管理权限的玩家发出邀请
        {
            uint32 userId;
            msg>>userId;
            ShareUserPtr p = m_onlineUser.GetUserByRoleId(userId);
            CUser *pU = p.get();
            
            msg.ReWrite();
            msg.SetType(PRO_BANGPAI);
            msg<<(uint8)6;
            if(pU == NULL)
            {
                msg<<PRO_ERROR;
                m_socketServer.SendMsg(pUser->GetSock(),msg);
                break;
            }
            if(pU->GetBangPai() != 0)
            {
                SendSysInfo(pUser,"目前有帮派");
                return;
            }
            msg<<PRO_SUCCESS;
            m_socketServer.SendMsg(pUser->GetSock(),msg);
            msg.ReWrite();
            msg.SetType(PRO_BANGPAI);
            msg<<(uint8)7;
            msg<<pUser->GetRoleId()<<pUser->GetName();
            m_socketServer.SendMsg(pU->GetSock(),msg);
            break;
        }
    case 7:
        {
            uint32 userId;
            uint8 accept = 0;
            msg>>userId>>accept;
            ShareUserPtr p = m_onlineUser.GetUserByRoleId(userId);
            if(p.get() == NULL)
                return;
            CBangPai *pBangPai = m_bangPaiMgr.FindBangPai(p->GetBangPai());
            if((pBangPai == NULL) || !(pBangPai->IsAdmin(userId)))
                break;
            if(accept == 0)
            {
                pBangPai->DelAskForJoin(userId);
                break;
            }
            if(pUser->GetLevel() < 30)
            {
                SendSysInfo(pUser,"等级低于30级不能加入帮派");
                return;
            }
            if(pUser->GetData32(2) + 3*3600*24 > (uint32)GetSysTime())
            {
                SendSysInfo(pUser,"离开帮派三天内不得加入新帮");
                return;
            }
            if(pUser->GetBangPai() == 0)
            {
                if(pBangPai->AddMember(pUser->GetRoleId(),EBRBangZhong))
                {
                    pUser->SetBangPai(pBangPai->GetId());
                    pUser->UpdateBangPai();
                }
            }
        }
        break;
    case 8://申请加入帮派列表
        {
            if(pUser->GetBangPai() != 0)
            {
                CBangPai *pBangPai = m_bangPaiMgr.FindBangPai(pUser->GetBangPai());
                if(pBangPai == NULL)
                    break;
                list<uint32> askJoin;
                pBangPai->GetAskForJoin(askJoin);
                list<uint32>::iterator i = askJoin.begin();
                uint8 num = 0;
                uint16 pos = msg.GetDataLen();
                msg<<num;//(uint8)askJoin.size();
                for(; i != askJoin.end(); i++)
                {
                    msg<<*i;
                    ShareUserPtr p = m_onlineUser.GetUserByRoleId(*i);
                    CUser *pU = p.get();
                    if(pU != NULL)
                    {
                        msg<<pU->GetName()<<pU->GetLevel();
                    }
                    else
                    {
                        string name;
                        uint8 level = 0;
                        GetUserName(*i,name,&level);
                        msg<<name<<level;
                    }
                    if(++num >= 20)
                        break;
                }
                msg.WriteData(pos,&num,1);
                m_socketServer.SendMsg(pUser->GetSock(),msg);
            }
            break;
        }
    case 9://批准加入帮派
        {
            uint8 res = 0;
            msg>>res;//0不同意，1同意
            if(res == 1)
            {
                uint32 userId = 0;
                msg>>userId;
                CBangPai *pBangPai = m_bangPaiMgr.FindBangPai(pUser->GetBangPai());
                if((pBangPai == NULL) || !(pBangPai->IsAdmin(pUser->GetRoleId())))
                    break;
                if(pBangPai->IsAskJoin(userId))
                {
                    ShareUserPtr p = m_onlineUser.GetUserByRoleId(userId);
                    CUser *pU = p.get();
                    if(pU != NULL)
                    {
                        if((pU->GetBangPai() == 0) || (pU->GetBangPai() == pBangPai->GetId()))
                        {
                            if(pBangPai->AddMember(userId,EBRBangZhong))
                            {
                                pU->SetBangPai(pBangPai->GetId());
                                pU->UpdateBangPai();
                                string str = pUser->GetName();
                                str += ":批准你加入帮派";
                                SendSysInfo(pU,str.c_str());
                            }
                        }
                    }
                    else    
                    {
                        UpdateBangPai(userId,pBangPai,true);
                        //pBangPai->AddMember(userId,EBRBangZhong);
                    }
                }
            }
            else if(res == 0)
            {
                uint32 userId = 0;
                msg>>userId;
                CBangPai *pBangPai = m_bangPaiMgr.FindBangPai(pUser->GetBangPai());
                if((pBangPai == NULL) || !(pBangPai->IsAdmin(pUser->GetRoleId())))
                    break;
                if(pBangPai->IsAskJoin(userId))
                {
                    ShareUserPtr p = m_onlineUser.GetUserByRoleId(userId);
                    CUser *pU = p.get();
                    if(pU != NULL)
                    {
                        string str = pUser->GetName();
                        str += ":不同意你加入帮派";
                        SendSysInfo(pU,str.c_str());
                    }
                    pBangPai->DelAskForJoin(userId);
                }
            }
            break;
        }
    case 10://帮派成员列表
        {
            uint8 page = 0;
            msg>>page;
            if(page > 0)
                page--;
            const int ONE_PAGE_NUM = 20;
            if(pUser->GetBangPai() != 0)
            {
                CBangPai *pBangPai = m_bangPaiMgr.FindBangPai(pUser->GetBangPai());
                if(pBangPai == NULL)
                    break;
                list<uint32> member;
                pBangPai->GetMember(member);
                list<uint32>::iterator i = member.begin();
                for(uint8 j = 0; j < page*ONE_PAGE_NUM; j++)
                {
                    if(i == member.end())
                        break;
                    i++;
                }
                uint8 num = 0;
                uint16 pos = msg.GetDataLen();
                msg<<num;
                for(; i != member.end(); i++)
                {
                    msg<<*i;
                    uint8 level = pBangPai->GetMemberRank(*i);
                    ShareUserPtr p = m_onlineUser.GetUserByRoleId(*i);
                    CUser *pU = p.get();
                    int bangGong = 0;
                    if(pU != NULL)
                    {
                        msg<<pU->GetName();
                        bangGong = pU->GetData32(5);
                    }
                    else
                    {
                        string name;
                        bangGong = GetUserName(*i,name);
                        msg<<name;
                    }
                    msg<<level<<bangGong;
                    ++num;
                    if(num >= ONE_PAGE_NUM)
                        break;
                }
                //cout<<(int)num<<endl;
                msg.WriteData(pos,&num,1);
                m_socketServer.SendMsg(pUser->GetSock(),msg);
            }
            break;
        }
    case 11://逐出帮派
        {
            uint32 userId;
            msg>>userId;
            if(userId == pUser->GetRoleId())
                break;
            CBangPai *pBangPai = m_bangPaiMgr.FindBangPai(pUser->GetBangPai());
            if((pBangPai == NULL) || !(pBangPai->IsAdmin(pUser->GetRoleId())))
                break;
            ShareUserPtr p = m_onlineUser.GetUserByRoleId(userId);
            CUser *pU = p.get();
            pBangPai->DelMember(userId);
            if((pU != NULL) && (pU->GetBangPai() == pUser->GetBangPai()))
            {
                pU->SetBangPai(0);
                pU->UpdateBangPai();
                pU->SetData32(2,GetSysTime());
                pBangPai->DelMember(userId);
                string str = "你被逐出帮派";
                SendSysInfo(pU,str.c_str());
                pU->DelMission(14);
            }
            else
            {
                UpdateBangPai(userId,pBangPai,false);
                //if(UpdateBangPai(userId,pBangPai->GetId(),pUser->GetBangPai()))
                    //pBangPai->DelMember(userId);
            }
            break;
        }
    case 12://退出帮派
        {
            msg<<PRO_SUCCESS;
            m_socketServer.SendMsg(pUser->GetSock(),msg);
            
            CBangPai *pBangPai = m_bangPaiMgr.FindBangPai(pUser->GetBangPai());
            if(pBangPai == NULL)
                break;            
            pBangPai->DelMember(pUser->GetRoleId());
            pUser->SetBangPai(0);
            const char *pMission = pUser->GetMission(14);
            if(pMission != NULL)
            {
                char buf[64];
                snprintf(buf,64,"%s",pMission);
                char *p[5];
                int num = SplitLine(p,5,buf);
                if(num >= 4)
                {
                    if(atoi(p[0]) == 3)
                    {
                        DelNpc(pUser,42);
                    }
                    else if(atoi(p[0]) == 4)
                    {
                        RemoveBattleRes(pUser,180,atoi(p[3]));
                    }
                }
                pUser->DelMission(14);
            }

            ShareUserPtr p = m_onlineUser.GetUserByRoleId(pBangPai->GetBangZhu());
            CUser *pU = p.get();
            if(pU != NULL)
            {
                string str = pUser->GetName();
                str += ":退出帮派";
                SendSysInfo(pU,str.c_str());
            }
            pUser->SetData32(2,GetSysTime());
            break;
        }
    case 13://帮派成员信息
        {
            CBangPai *pBangPai = m_bangPaiMgr.FindBangPai(pUser->GetBangPai());
            if(pBangPai == NULL)
            {
                msg<<PRO_ERROR;
                m_socketServer.SendMsg(pUser->GetSock(),msg);
                return;
            }
            string bangzhu;
            GetUserName(pBangPai->GetBangZhu(),bangzhu);
            //GetUserName(pBangPai->GetBangCreater(),creater);
            msg<<PRO_SUCCESS<<pBangPai->GetId()<<pBangPai->GetName()
                <<bangzhu<<pBangPai->GetLevel()<<(uint16)pBangPai->GetId()<<pBangPai->GetUserNum()
                <<pBangPai->GetFanRong()<<pBangPai->GetGongGao()
                <<pBangPai->GetKouHao();
            m_socketServer.SendMsg(pUser->GetSock(),msg);
            break;
        }
    case 14:
        {
            uint32 bangpaiId = 0;
            msg>>bangpaiId;
            CBangPai *pBangPai = m_bangPaiMgr.FindBangPai(bangpaiId);
            msg.ReWrite();
            msg.SetType(PRO_BANGPAI);
            msg<<(uint8)14;
            if(pBangPai == NULL)
            {
                msg<<PRO_ERROR;
                m_socketServer.SendMsg(pUser->GetSock(),msg);
                return;
            }
            string bangzhu;
            string creater;
            GetUserName(pBangPai->GetBangZhu(),bangzhu);
            GetUserName(pBangPai->GetBangCreater(),creater);
            msg<<PRO_SUCCESS<<pBangPai->GetId()<<pBangPai->GetName()
                <<bangzhu<<pBangPai->GetLevel()<<pBangPai->GetPaiMing()
                <<pBangPai->GetHuoYue()<<pBangPai->GetKouHao();
            m_socketServer.SendMsg(pUser->GetSock(),msg);
            break;
        }
    case 15:
        {
            CBangPai *pBangPai = m_bangPaiMgr.FindBangPai(pUser->GetBangPai());
            if(pBangPai == NULL)
            {
                return;
            }
            if(!pBangPai->IsAdmin(pUser->GetRoleId()))
                return;
            string name;
            msg>>name;
            pBangPai->SetKouHao(name.c_str());
            break;
        }
    case 16:
        {
            CBangPai *pBangPai = m_bangPaiMgr.FindBangPai(pUser->GetBangPai());
            if(pBangPai == NULL)
            {
                return;
            }
            if(!pBangPai->IsAdmin(pUser->GetRoleId()))
                return;
            string name;
            msg>>name;
            pBangPai->SetGongGao(name.c_str());
            break;
        }
        break;
    case 17:
        {
            CBangPai *pBangPai = m_bangPaiMgr.FindBangPai(pUser->GetBangPai());
            if(pBangPai == NULL)
            {
                return;
            }
            if(!pBangPai->IsAdmin(pUser->GetRoleId()))
                return;
            uint8 huodongTime;
            msg>>huodongTime;
            pBangPai->SetHuoDongTime(huodongTime);
            break;
        }
    case 18://解散帮派
        {
            CBangPai *pBangPai = m_bangPaiMgr.FindBangPai(pUser->GetBangPai());
            if(pBangPai == NULL)
            {
                return;
            }
            if(pBangPai->GetMemberRank(pUser->GetRoleId()) != EBRBangZhu)
                return;
            m_bangPaiMgr.DelBangPai(pUser->GetBangPai());
            break;
        }
    case 19://帮主传位
        break;
    case 20://调整位阶
        {
            uint32 userId;
            uint8 rank;
            msg>>userId>>rank;
            if((rank < EBRBangZhu) || (rank > EBRBangZhong))
                return;
            if(pUser->GetRoleId() == userId)
                return;
            CBangPai *pBangPai = m_bangPaiMgr.FindBangPai(pUser->GetBangPai());
            if(pBangPai == NULL)
            {
                return;
            }
            if(!pBangPai->IsAdmin(pUser->GetRoleId()))
                return;
            if(pBangPai->GetMemberRank(pUser->GetRoleId()) != EBRBangZhu)
                return;
            if((rank < EBRBangZhong) && (pBangPai->GetRankMemNum(rank) >= 1))
            {
                SendSysInfo(pUser,"此位阶人数已满");
                return;
            }
            pBangPai->SetMemberRank(userId,rank);
            ShareUserPtr p = m_onlineUser.GetUserByRoleId(userId);
            CUser *pU = p.get();
            if((pU != NULL) && (pU->GetBangPai() == pUser->GetBangPai()))
            {
                pU->UpdateBangPai();
            }
            break;
        }
        break;
    case 21://查询帮派信息
        {
            uint32 userId;
            msg>>userId;
            ShareUserPtr p = m_onlineUser.GetUserByRoleId(userId);
            CUser *pU = p.get();
            if((pU == NULL) || (pU->GetBangPai() == 0))
                break;
            uint32 bangpaiId = pU->GetBangPai();
            msg.ReWrite();
            msg.SetType(PRO_BANGPAI);
            msg<<(uint8)21;
            CBangPai *pBangPai = m_bangPaiMgr.FindBangPai(bangpaiId);
            if(pBangPai == NULL)
            {
                msg<<PRO_ERROR;
                m_socketServer.SendMsg(pUser->GetSock(),msg);
                break;
            }
            string bangzhu;
            string creater;
            GetUserName(pBangPai->GetBangZhu(),bangzhu);
            GetUserName(pBangPai->GetBangCreater(),creater);
            msg<<PRO_SUCCESS<<pBangPai->GetId()<<pBangPai->GetName()
                <<bangzhu<<pBangPai->GetLevel()<<pBangPai->GetPaiMing()
                <<pBangPai->GetHuoYue()<<pBangPai->GetKouHao();
            m_socketServer.SendMsg(pUser->GetSock(),msg);
            break;
        }
        break;
    }
}

void CPackageDeal::GetNpcState(CNetMessage *pMsg,int sock)
{
	GET_MSG

    GET_USER
    
    uint16 npcId = 0;
    msg>>npcId;
    SNpcInstance *pNpc = m_npcManager.GetNpcInstance(npcId);
    CCallScript *pScript = NULL;
    pScript = pUser->FindNpcScript(npcId);
    
    if((pScript == NULL) && (pNpc != NULL) && (pNpc->pNpc != NULL))
    {
        pScript = pNpc->pNpc->pScript;
    }
    if(pScript == NULL)
        return;
    int state = 0;
    pScript->Call("GetState","u>i",pUser,&state);
    
    msg<<(uint8)state;
    m_socketServer.SendMsg(pUser->GetSock(),msg);
}

void CPackageDeal::UpdatePackage(CNetMessage *pMsg,int sock)
{
	GET_MSG
    GET_USER
    if(pUser->GetFightId() != 0)
        return;
    uint8 op = 0xff;
    msg>>op;
    if(op == 0)
    {//丢弃
        uint8 pos = 0xff;
        uint8 num = 1;
        msg>>pos>>num;
        if(pUser->CanDelPackage(pos))
        {
            pUser->SaveDelItem(pos);
            pUser->DelPackage(pos,num);
        }
    }
    else if(op == 1)
    {//使用物品
        uint8 pos = 0xff;
        uint8 num = 1;
        uint8 target = 0xff;
        uint8 val= 0;
        uint8 val1=0;
        msg>>pos>>num>>target>>val>>val1;
        if(target == 0)
        {
            pUser->UseItem(pos,NULL,NULL,val,val1,num);
        }
        else if(target == 1)
        {
            uint8 gensui = pUser->GetGenSuiPet();
            pUser->UseItemToPet(gensui,pos);
        }
    }
    else if(op == 2)
    {//移动包裹
        uint8 srcPos;
        uint8 tarPos;
        msg>>srcPos>>tarPos;
        pUser->MoveItem(srcPos,tarPos);
    }
    else if(op == 3)
    {//背包物品绑定
        uint8 pos = 0;
        msg>>pos;
        pUser->BangDingItem(pos);
    }
}

void CPackageDeal::GetEShopItem(CNetMessage *pMsg,int sock)
{
	GET_MSG
    GET_USER
    if(pUser->GetFightId() != 0)
        return;
    //商城分五个tab页，一页最多18个物品
    //msg<<(uint8)1<<(uint8)1<<(uint16)610<<(uint8)13<<(uint8)0<<(uint16)0<<(uint8)0<<1000<<(uint8)0;
    
    msg<<(uint8)m_eShopItemList.size();
    CItemTemplateManager &itemMgr = SingletonItemManager::instance();
    for(uint8 i = 0; i < m_eShopItemList.size(); i++)
    {
        SItemTemplate *pItem = itemMgr.GetItem(m_eShopItemList[i].id);
        if(pItem == NULL)
            continue;
        SItemInstance item = {0};
        item.tmplId = m_eShopItemList[i].id;
        item.level = m_eShopItemList[i].level;
        item.naijiu = pItem->naijiu;
        item.num = 1;
        
        msg<<i;
        MakeItemInfo(&item,msg);
        msg<<m_eShopItemList[i].money<<m_eShopItemList[i].page;
    }
    m_socketServer.SendMsg(pUser->GetSock(),msg);
}

void CPackageDeal::UserJumpOk(CNetMessage *pMsg,int sock)
{
	GET_USER
    pUser->UserJump(false);
}

void CPackageDeal::BuyEShopItem(CNetMessage *pMsg,int sock)
{
	GET_MSG
    GET_USER
    if(pUser->GetFightId() != 0)
        return;
    uint8 pos;
    uint8 num = 1;
    uint8 type = 0;
    //type == 1绑定通宝
    msg>>pos>>num>>type;
    if(num < 1)
        return;
        
    if(pos >= m_eShopItemList.size())
        return;
        
    msg.ReWrite();
    msg.SetType(PRO_BUY_EITEM);
    
    CItemTemplateManager &itemMgr = SingletonItemManager::instance();
    SItemTemplate *pItem = itemMgr.GetItem(m_eShopItemList[pos].id);
    if(pItem == NULL)
        return;
    if((pItem->id == 1804) || (pItem->id == 1805) || (pItem->id == 1809) || (pItem->id == 1815))
        num = 1;
    
    time_t t = GetSysTime();
    struct tm *pTm = localtime(&t);
    if(pTm == NULL)
        return;
    
    if(pUser->GetTongBao(type) >= (m_eShopItemList[pos].money * num))
    {
        SItemInstance item = {0};
        item.tmplId = m_eShopItemList[pos].id;
        item.level = m_eShopItemList[pos].level;
        item.naijiu = pItem->naijiu;
        item.num = num;
        if(type == 1)
            item.bangDing = 1;
        else if(item.tmplId == 1834)
            item.bangDing = 1;
            
        if((pItem->id == 1804) || (pItem->id == 1805) || (pItem->id == 1822))
        {
            int *p = (int*)item.addAttrVal;
            *p = pItem->addXue;
        }
        if((item.tmplId == 611) && (item.level == 2))
        {
            if(pUser->HaveEmptyPack() < 2)
            {
                msg<<PRO_ERROR;
                m_socketServer.SendMsg(pUser->GetSock(),msg);
                return;
            }
            else
            {
                if(type == 1)
                    pUser->AddBangDingPackage(610,num);
                else
                    pUser->AddPackage(610,num);
            }
        }
        int money = m_eShopItemList[pos].money*num;
        
        if(pUser->AddPackage(item))
        {
            pUser->AddTongBao(-money,type);
            msg<<PRO_SUCCESS;
            m_socketServer.SendMsg(pUser->GetSock(),msg);
            SaveBuyShopItem(pUser->GetRoleId(),item.tmplId,num,item.level,m_eShopItemList[pos].money*num,pUser->GetTongBao(type));
            return;
        }
    }
    msg<<PRO_ERROR;
    m_socketServer.SendMsg(pUser->GetSock(),msg);
}

void CPackageDeal::QiangHuaEquip(CNetMessage *pMsg,int sock)
{
	GET_MSG

    GET_USER

    uint8 item = 0;
    uint8 shengjiStone = 0;
    uint8 lianhuaStone = 0;
    uint8 num;//炼化石数量
    
    msg>>item>>shengjiStone>>lianhuaStone>>num;
    msg.ReWrite();
    msg.SetType(PRO_EQUIP_ENFORCE);
    int val = pUser->QiangHuaPackage(item,shengjiStone,lianhuaStone,num);
    if(val == 0)
    {
        msg<<PRO_SUCCESS;
    }
    else if(val == 1)
    {
        msg<<PRO_ERROR<<"强化失败";
    }
    else if(val == 2)
    {
        msg<<PRO_ERROR<<"不能强化";
    }
    m_socketServer.SendMsg(pUser->GetSock(),msg);
}

void CPackageDeal::QueryQiangHua(CNetMessage *pMsg,int sock)
{
	GET_MSG

    GET_USER

    uint8 item = 0;
    uint8 shengjiStone = 0;
    uint8 lianhuaStone = 0;
    uint8 num;//炼化石数量
    msg>>item>>shengjiStone>>lianhuaStone>>num;
    
    msg.ReWrite();
    msg.SetType(PRO_ENFORCE_QUERY);
    if(pUser->MakeQiangHuaInfo(item,shengjiStone,lianhuaStone,num,msg))
        m_socketServer.SendMsg(pUser->GetSock(),msg);
}

void CPackageDeal::MakeBlueItem(CNetMessage *pMsg,int sock)
{
	GET_MSG

    GET_USER

    uint8 item = 0;
    uint8 stone1 = 0;
    uint8 stone2= 0;
    uint8 stone3 = 0;
    
    msg>>item>>stone1>>stone2>>stone3;
    msg.ReWrite();
    msg.SetType(PRO_EQUIP_BLUED);
    int val = pUser->LianHuaPackage(item,stone1,stone2,stone3);
    if(val == 0)
    {
        msg<<PRO_SUCCESS;
    }
    else if(val == 1)
    {
        msg<<PRO_ERROR<<"制作篮装失败";
    }
    else if(val == 2)
    {
        msg<<PRO_ERROR<<"不能制作篮装";
    }
    m_socketServer.SendMsg(pUser->GetSock(),msg);
}

void CPackageDeal::MakeGreenItem(CNetMessage *pMsg,int sock)
{
	GET_MSG

    GET_USER

    uint8 item = 0;
    uint8 stone = 0;
    
    msg>>item>>stone;
    msg.ReWrite();
    msg.SetType(PRO_EQUIP_GREEND);
    int val = pUser->MakeGreenItem(item,stone);
    if(val == 0)
    {
        msg<<PRO_SUCCESS;
    }
    else if(val == 1)
    {
        msg<<PRO_ERROR<<"制作绿装失败";
    }
    else if(val == 2)
    {
        msg<<PRO_ERROR<<"不能制作绿装";
    }
    m_socketServer.SendMsg(pUser->GetSock(),msg);
}

void CPackageDeal::SelectGreenItemAttr(CNetMessage *pMsg,int sock)
{
	GET_MSG

    GET_USER

    uint8 item = 0;
    uint8 attrInd = 0;
    uint8 shuijing = 0;
    uint8 stone = 0;
    uint8 stoneNum = 0;
    msg>>item>>attrInd>>shuijing>>stone>>stoneNum;
    if(stoneNum == 0)
        stoneNum = 1;
    msg.ReWrite();
    msg.SetType(PRO_ORNAMENT_CARVE);
    int val = pUser->SelectGreenItemAttr(item,attrInd,shuijing,stone,stoneNum);
    if(val == 0)
    {
        msg<<PRO_SUCCESS;
    }
    else if(val == 1)
    {
        msg<<PRO_ERROR<<"升华失败";
    }
    else if(val == 2)
    {
        msg<<PRO_ERROR<<"不能升华绿装";
    }
    m_socketServer.SendMsg(pUser->GetSock(),msg);
}

void CPackageDeal::XiangQian(CNetMessage *pMsg,int sock)
{
	GET_MSG

    GET_USER

    uint8 item = 0;
    uint8 stone = 0;
    msg>>item>>stone;
    msg.ReWrite();
    msg.SetType(PRO_ORNAMENT_SETTING);
    if(pUser->XiangQian(item,stone))
    {
        msg<<PRO_SUCCESS;
    }
    else
    {
        msg<<PRO_ERROR<<"镶嵌失败";
    }
    m_socketServer.SendMsg(pUser->GetSock(),msg);
}

void CPackageDeal::EasyUseYaoPin(CNetMessage *pMsg,int sock)
{
	GET_MSG

    GET_USER
    uint8 type = 0;
    msg>>type;
    if(type == 0)
    {
        pUser->UseAddHpItem();
    }
    else if(type == 1)
    {
        pUser->UserAddMpItem();
    }
}

void CPackageDeal::SwitchInfo(CNetMessage *pMsg,int sock)
{
	GET_MSG

    GET_USER
    uint8 mask = pUser->GetChatChannel();
    msg.ReWrite();
    msg.SetType(PRO_SWITCH_INFO);
    msg<<mask;

    m_socketServer.SendMsg(pUser->GetSock(),msg);
}

void CPackageDeal::QueryPetInfo(CNetMessage *pMsg,int sock)
{
	GET_MSG

    GET_USER
    uint32 userId = 0;
    uint8 pos = 0;
    msg>>userId>>pos;
    
    msg.ReWrite();
    msg.SetType(PRO_OTHER_PET);
    
    ShareUserPtr p = m_onlineUser.GetUserByRoleId(userId);
    if(p.get() == NULL)
    {
        msg<<PRO_ERROR;
    }
    else
    {
        msg<<PRO_SUCCESS<<userId<<pos;
        if(!p->MakePetInfo(pos,msg))
        {
            msg.ReWrite();
            msg.SetType(PRO_OTHER_PET);
            msg<<PRO_ERROR;
        }
    }
    m_socketServer.SendMsg(pUser->GetSock(),msg);
}

void CPackageDeal::MyBangPai(CNetMessage *pMsg,int sock)
{
	GET_MSG
    GET_USER
    
    uint32 bangId = pUser->GetBangPai();
    if(bangId == 0)
    {
        msg<<bangId;
        m_socketServer.SendMsg(pUser->GetSock(),msg);
        return;
    }
    else
    {
        CBangPai *pBangPai = m_bangPaiMgr.FindBangPai(bangId);
        if(pBangPai == NULL)
        {
            msg<<0;
            m_socketServer.SendMsg(pUser->GetSock(),msg);
            return;
        }
        uint8 rank = pBangPai->GetMemberRank(pUser->GetRoleId());
        msg<<bangId<<pBangPai->GetName()<<rank;
        m_socketServer.SendMsg(pUser->GetSock(),msg);
    }
}

void CPackageDeal::ChangeUserFace(CNetMessage *pMsg,int sock)
{
	GET_MSG
    GET_USER
    
    uint8 face;
    msg>>face;
    pUser->SetFace(face);
}

void CPackageDeal::CancelTask(CNetMessage *pMsg,int sock)
{
	GET_MSG
    GET_USER
    uint16 taskId;
    msg>>taskId;
    /*CCallScript script("10000.lua");
    pUser->SetCallScript(10000);
    script.Call("CancelTask","ui",pUser,taskId);*/
    CCallScript *pScript = GetScript();
    if(pScript != NULL)
    {
        pUser->SetCallScript(pScript->GetScriptId());
        pScript->Call("CancelTask","ui",pUser,taskId);
    }
}

void CPackageDeal::ItemDesc(CNetMessage *pMsg,int sock)
{
	GET_MSG

    GET_USER
    uint16 itemId = 0;
    msg>>itemId;
    CItemTemplateManager &itemMgr = SingletonItemManager::instance();
    SItemTemplate *pItem = itemMgr.GetItem(itemId);
    if(pItem == NULL)
        return;
    msg<<pItem->describe;
    m_socketServer.SendMsg(pUser->GetSock(),msg);
}

//充值
void CPackageDeal::Charge(CNetMessage *pMsg,int sock)
{
	GET_MSG

    GET_USER
    uint8 type;
    string cardNum;
    string cardPassword;
    uint16 money;
    msg>>type>>cardNum>>cardPassword>>money;
    
    msg.ReWrite();
    msg.SetType(PRO_CHARGE);
    if((money <= 0) || (cardNum.size() < 10) || ( cardNum.size() > 21))
    {
        msg<<PRO_ERROR;
        m_socketServer.SendMsg(pUser->GetSock(),msg);
        return;
    }
    if((cardPassword.size() < 8) || (cardPassword.size() > 21))
    {
        msg<<PRO_ERROR;
        m_socketServer.SendMsg(pUser->GetSock(),msg);
        return;
    }
    const char *p = NULL;
    if(type != 2)
    {
        p = cardNum.c_str();
        while(*p != 0)
        {
            if((*p < '0') || (*p > '9'))
            {
                msg<<PRO_ERROR;
                m_socketServer.SendMsg(pUser->GetSock(),msg);
                return;
            }
            p++;
        }
    }
    p = cardPassword.c_str();
    while(*p != 0)
    {
        if((*p < '0') || (*p > '9'))
        {
            msg<<PRO_ERROR;
            m_socketServer.SendMsg(pUser->GetSock(),msg);
            return;
        }
        p++;
    }
    //cout<<cardNum<<endl
      //  <<cardPassword<<endl;
    CGetDbConnect getDb;
    CDatabaseSql *pDb = getDb.GetDbConnect();
    boost::format fmt("INSERT INTO chong_tongbao (type,card_num,card_password,money,time,"\
                    "user_id,role_id) "\
                    "VALUES (%1%,'%2%','%3%',%4%,FROM_UNIXTIME(%5%),%6%,%7%)");
    fmt%(int)type
        %cardNum
        %cardPassword
        %money
        %GetSysTime()
        %pUser->GetUserId()
        %pUser->GetRoleId();
        
    if ((pDb != NULL) && (pDb->Query(fmt.str().c_str())))
    {
        msg<<PRO_SUCCESS;
        m_socketServer.SendMsg(pUser->GetSock(),msg);
        return;
    }
    msg<<PRO_ERROR;
    m_socketServer.SendMsg(pUser->GetSock(),msg);
    return;
    /*
    pUser->AddTongBao(100*money);
    msg<<PRO_SUCCESS;
    m_socketServer.SendMsg(sock,msg);
    */
}

void CPackageDeal::QuerySavePos(CNetMessage *pMsg,int sock)
{
	GET_MSG

    GET_USER
    pUser->MakeSavePosInfo(msg);
    m_socketServer.SendMsg(pUser->GetSock(),msg);
}

void CPackageDeal::QueryPath(CNetMessage *pMsg,int sock)
{
    GET_USER
    CScene *pScene = pUser->GetScene();
    if(pScene != NULL)
        pScene->SendPathInfo(pUser->GetSock());
}

void CPackageDeal::AvailableTask(CNetMessage *pMsg,int sock)
{
	GET_MSG

    GET_USER
    
    char *pInfo = NULL;    
    CCallScript *pScript = GetScript();//("10000.lua");
    if(pScript != NULL)
    {
        pUser->SetCallScript(pScript->GetScriptId());
        pScript->Call("GetAvailableTask","u>s",pUser,&pInfo);
    }
    
    if(pInfo != NULL)
    {
        char *p[20];
        //cout<<pInfo<<endl;
        char buf[256];
        snprintf(buf,255,"%s",pInfo);
        uint8 num = SplitLine(p,20,buf);
        SNpcInstance *pNpc;
        char *name;
        int npcId,missionId;
        msg<<(uint8)(num/2);
        for(uint8 i = 0; i < num/2; i++)
        {
            missionId = atoi(p[2*i]);
            npcId = atoi(p[2*i+1]);
            pNpc = m_npcManager.GetNpcInstance(npcId);
            name = GetMissionName(missionId);
            if((pNpc != NULL) && (name != NULL))
            {
                msg<<name<<(uint16)pNpc->sceneId<<pNpc->x<<pNpc->y;
            }
            else if(pNpc == NULL)
            {
                pUser->MakeNpc(npcId,msg);
            }
        }
        m_socketServer.SendMsg(pUser->GetSock(),msg);
    }
}

void CPackageDeal::GetScenePos(CNetMessage *pMsg,int sock)
{
    GET_MSG
    GET_USER
    if((pUser->GetTeam() != 0) && (pUser->GetTeam() != pUser->GetRoleId()))
        return;
        
    CScene *pScene = pUser->GetScene();
    if(pScene != NULL)
    {
        pUser->SetPos(pScene->GetX(),pScene->GetY());
        msg.ReWrite();
        msg.SetType(PRO_JUMP_SCENE);
        msg<<(uint16)pScene->GetMapId()<<pScene->GetX()<<pScene->GetY()<<pUser->GetFace();
        m_socketServer.SendMsg(pUser->GetSock(),msg);
    }
}

void CPackageDeal::SendQuestion(CUser *pUser)
{
    //1至9随机取2数，然后在加法，减法，乘法里取一种方式。
    uint8 q1 = Random(0,9);
    uint8 q2 = Random(0,9);
    uint8 res[4] = {0};
    
    uint8 right = Random(0,3);
    pUser->SetRightAns(right);
    char buf[64];
    switch(Random(0,2))
    {
    case 0:
        sprintf(buf,"星灵仙子:|我来问你,%d+%d等于几?",q1,q2);
        res[right] = q1+q2;
        break;
    case 1:
        if(q1 >= q2)
            sprintf(buf,"星灵仙子:|我来问你,%d-%d等于几?",q1,q2);
        else
            sprintf(buf,"星灵仙子:|我来问你,%d-%d等于几?",q2,q1);
        res[right] = abs(q1-q2);
        break;
    case 2:
        sprintf(buf,"星灵仙子:|我来问你,%d×%d等于几?",q1,q2);
        res[right] = q1*q2;
        break;
    default:
        sprintf(buf,"星灵仙子:|我来问你,%d×%d等于几?",q1,q2);
        res[right] = q1*q2;
        break;
    }
    for(uint8 i = 0; i < 4; i++)
    {
        if(i == right)
            continue;
        for(uint8 j = 0; j < 10; j++)
        {
            uint8 r = Random(0,100);
            if(r != res[right])
            {
                res[i] = r;
                break;
            }
        }
    }
    /*for(uint8 i = 0; i < 4; i++)
    {
        if((i != right) && (res[i] == res[right]))
        {
        }
    }todo*/
    CNetMessage msg;
    msg.SetType(MSG_SERVER_BEGIN_XING);
    msg<<buf;
    for(uint8 i = 0; i < 4; i++)
    {
        sprintf(buf,"%d",res[i]);
        msg<<buf;
    }
    pUser->SetAskTime(GetSysTime());
    m_socketServer.SendMsg(pUser->GetSock(),msg);
}

void CPackageDeal::AskQuestion(CUser *pUser)
{
    if(pUser == NULL)
        return;
    SendQuestion(pUser);
    CScene *pScene = NULL;
    if(pUser->GetTeam() != 0)
    {
        pScene = pUser->GetScene();
    }
    if(pScene == NULL)
        return;
    uint32 members[MAX_TEAM_MEMBER];
    uint8 num = pScene->GetTeamMem(pUser->GetTeam(),members);
    
    for(uint8 i = 0; i < num; i++)
    {
        ShareUserPtr ptr = m_onlineUser.GetUserByRoleId(members[i]);
        CUser *pPtr = ptr.get();
        if(pPtr != NULL)
        {
            SendQuestion(pPtr);
        }
    }
}

void CPackageDeal::UserAnswer(CNetMessage *pMsg,int sock)
{
    GET_MSG
    GET_USER
        
    uint8 answer = 0xff;
    msg>>answer;
    msg.ReWrite();
    msg.SetType(MSG_ANSWER_XING);
    
    pUser->SetAnswerTimes(pUser->GetAnswerTimes()+1);
    pUser->SetAskTime(GetSysTime());
    
    if(answer == pUser->GetRightAns())
    {
        pUser->ClearAnswer();
        pUser->SetAskTime(0);
        msg<<PRO_SUCCESS;
        pUser->AddMp(pUser->GetMaxMp()-pUser->GetMp());
        pUser->AddHp(pUser->GetMaxHp()-pUser->GetHp());
        SharePetPtr pet;
        if(pUser->GetChuZhanPet(pet))
        {
            SPet *pPet = pet.get();
            if(pPet != NULL)
            {
                uint8 pos = pUser->GetChuZhanPet();
                pPet->hp = pPet->maxHp;
                pPet->mp = pPet->maxMp;
                pUser->UpdatePetInfo(pos,5,pPet->maxHp);
                pUser->UpdatePetInfo(pos,6,pPet->maxMp);
                pUser->UpdatePetInfo(pos,13,pPet->zhongcheng);
            }
        }
        /*恢复用户全部气血/法力
	    恢复出战宠物全部气血/法力
	    恢复出战宠物忠诚度*/
	    pUser->SetAnswerTimes(0);
	    m_socketServer.SendMsg(pUser->GetSock(),msg);
	    SendPopMsg(pUser,"星灵仙子对你的表现很满意，施法将你的血魔都补满了");
    }
    else
    {
        msg<<PRO_ERROR;
        m_socketServer.SendMsg(pUser->GetSock(),msg);
        if(pUser->GetAnswerTimes() >= 2)
        {
            string msg = pUser->GetName();
            msg.append("惹怒了星灵仙子，受到了惩罚");
            pUser->SendMsgToTeamMember(msg.c_str());
            SendPopMsg(pUser,"你答题错误,引起了星灵仙子的注意,她现在很想见见你。");
            //shutdown(pUser->GetSock(),SHUT_RD);
            pUser->SetBitSet(0);
            return;
        }
        else
        {
            SendQuestion(pUser);
        }
    }
}

void CPackageDeal::MonsterBattle(CNetMessage *pMsg,int sock)
{
    GET_MSG
    GET_USER
    uint16 monsterId = 0;
    msg>>monsterId;
    CScene *pScene = pUser->GetScene();
    if(pUser->GetFightId() != 0)
        return;
        
    if(pScene != NULL)
    {
        if(pScene->GetMapId() == 306)
            pScene->ShiYaoFight(pUser,monsterId);
        else if((pScene->GetId() > 300) || (pScene->InGuiYu()))
            pScene->GuiYuFight(ptr,monsterId);
        else if(monsterId == YE_ZHU_WANG_ID)
            pScene->YeZhuFight(ptr,monsterId);
        else if(monsterId == QIAN_NIAN_JIANG_SHI_ID)
            pScene->QianNianJiangShiFight(ptr,monsterId);
        else if(monsterId == THREE_SISTER_ID)
            pScene->ThreeSister(ptr,monsterId);
        else
            pScene->NianShouBattle(pUser,monsterId);
    }
}

void CPackageDeal::QueryScene(CNetMessage *pMsg,int sock)
{
    GET_MSG
    GET_USER
    uint16 sceneId = 0;
    msg>>sceneId;
    char name[32];
    snprintf(name,31,"dat/%d.map",sceneId);
    if(access(name,R_OK) != 0)
        return;
    FILE *file = fopen(name,"r");
    if(file == NULL)
        return;
    const int MAX_MAP_SIZE = 1024*4;
    char buf[MAX_MAP_SIZE];
    int len = fread(buf,1,MAX_MAP_SIZE,file);
    fclose(file);
    //cout<<"read scene len:"<<len<<endl;
    msg.WriteData(buf,len);
    m_socketServer.SendMsg(pUser->GetSock(),msg);
}

void CPackageDeal::QueryItem(CNetMessage *pMsg,int sock)
{
    GET_MSG
    GET_USER
    uint16 itemId = 0;
    msg>>itemId;
    if(itemId == 0)
        return;
    CItemTemplateManager &itemMgr = SingletonItemManager::instance();
    SItemTemplate *pItem = itemMgr.GetItem(itemId);
    if(pItem == NULL)
        return;
    msg<<pItem->type<<pItem->level<<pItem->sex<<pItem->name<<pItem->pic;
    m_socketServer.SendMsg(pUser->GetSock(),msg);
}

void CPackageDeal::HeartBeat(CNetMessage *pMsg,int sock)
{
    GET_MSG
    GET_USER
    if(pUser->HaveBitSet(0))
        return;
    //cout<<pUser->GetRoleId()<<" "<<GetSysTime()<<endl;
    pUser->SetHeartTimes(pUser->GetHeartTimes()+1);
    if(GetSysTime() - pUser->GetLastHeartTime() > 60)
    {
        pUser->SetLastHeartTime(GetSysTime());
        if(pUser->GetHeartTimes() > 4)
        {
            pUser->SetHBErrTimes(pUser->GetHBErrTimes()+1);
        }
        else
        {
            pUser->SetHBErrTimes(0);
        }
        pUser->SetHeartTimes(0);
        if(pUser->GetHBErrTimes() > 3)
        {
            SendPopMsg(pUser,"你行动过快，惹怒了星灵仙子，她不高兴了。快去星寿村向她道歉吧!");
            pUser->SetBitSet(0);
        }
    }
    //cout<<(int)pUser->GetLastHeartTime()<<endl;
    msg.ReWrite();
    msg.SetType(MSG_SERVER_HEART_BEAT);
    msg<<250;
    m_socketServer.SendMsg(pUser->GetSock(),msg);
}
void CPackageDeal::GetServerList(CNetMessage *msg,int sock)
{
    if(GetSysTime() - m_readSLTime > 600)
    {
        m_readSLTime = GetSysTime();
        CGetDbConnect getDb;
        CDatabaseSql *pDb = getDb.GetDbConnect();
        if ((pDb != NULL)
            && (pDb->Query("select ip,port,name,server_id from server_list order by id")))
        {
            char **row;
            boost::mutex::scoped_lock lk(m_slLock);
            m_pSLMsg->ReWrite();
            m_pSLMsg->SetType(MSG_LIST_SERVER);
            *m_pSLMsg<<(uint8)pDb->GetRowNum();
            while ((row = pDb->GetRow()) != NULL)
            {
#ifdef QQ
                *m_pSLMsg<<row[2]<<row[0]<<atoi(row[1])<<atoi(row[3]);
#else
                *m_pSLMsg<<row[2]<<row[0]<<atoi(row[1])<<(uint8)atoi(row[3]);
#endif
            }
        }
    }
    //cout<<"send server list"<<endl;
    boost::mutex::scoped_lock lk(m_slLock);
    m_socketServer.SendMsg(sock,*m_pSLMsg);
}

void CPackageDeal::GetTitle(CNetMessage *pMsg,int sock)
{
    GET_MSG
    GET_USER
    
    msg.ReWrite();
    msg.SetType(MSG_CLIENT_LIST_TITLE);
    pUser->GetTitleMsg(msg);
    m_socketServer.SendMsg(pUser->GetSock(),msg);
}

void CPackageDeal::UseTitle(CNetMessage *pMsg,int sock)
{
    GET_MSG
    GET_USER
    uint16 useTitle = 0;
    msg>>useTitle;    
    pUser->SetTitle(useTitle);
}

void CPackageDeal::SysMail(CNetMessage *pMsg,int sock)
{
    GET_MSG
    GET_USER
    uint8 type;
    string info;
    msg>>type>>info;
    
    CGetDbConnect getDb;
    CDatabaseSql *pDb = getDb.GetDbConnect();
    boost::format fmt("INSERT INTO sys_mail (role_id,type,msg,time,name) "\
                    "VALUES (%1%,%2%,'%3%',FROM_UNIXTIME(%4%),'%5%')");
    fmt%pUser->GetRoleId()
        %(int)type
        %info.c_str()
        %GetSysTime()
        %pUser->GetName();
        
    if(pDb != NULL)
        pDb->Query(fmt.str().c_str());
}

void CPackageDeal::CXGongGao(CNetMessage *pMsg,int sock)
{
    GET_MSG
    GET_USER
    
    msg<<gpMain->GetCZGongGao();
    m_socketServer.SendMsg(pUser->GetSock(),msg);
}

void CPackageDeal::ShiTu(CNetMessage *pMsg,int sock)
{
    GET_MSG
    GET_USER
    
    CGetDbConnect getDb;
    CDatabaseSql *pDb = getDb.GetDbConnect();
    if(pDb == NULL)
        return;
        
    char sql[256];
    snprintf(sql,256,"select master,m_id,prentice,p_id from master_prentice where p_id=%d or m_id=%d"
        ,pUser->GetRoleId(),pUser->GetRoleId());
    
    if(!pDb->Query(sql))
        return;
    char **row;
    uint8 num = 0;
    uint16 pos = msg.GetDataLen();
    msg<<num;
    
    while((row = pDb->GetRow()) != NULL)
    {
        uint16 hotVal = 0;
        int userId = 0;
        CUser *p = NULL;
        if(atoi(row[1]) == (int)pUser->GetRoleId())
        {
            userId = atoi(row[3]);
            pUser->GetHotVal(userId,hotVal);
            p = m_onlineUser.GetUserByRoleId(userId).get();
            if(p == NULL)
                msg<<(uint8)1<<userId<<row[2]<<(uint16)0<<hotVal;
            else
                msg<<(uint8)1<<userId<<row[2]<<(uint16)p->GetMapId()<<hotVal;
        }
        else if(atoi(row[3]) == (int)pUser->GetRoleId())
        {
            userId = atoi(row[1]);
            pUser->GetHotVal(userId,hotVal);
            p = m_onlineUser.GetUserByRoleId(userId).get();
            if(p == NULL)
                msg<<(uint8)0<<userId<<row[0]<<(uint16)0<<hotVal;
            else
                msg<<(uint8)0<<userId<<row[0]<<(uint16)p->GetMapId()<<hotVal;
        }
        num++;
    }
    msg.WriteData(pos,&num,1);
    m_socketServer.SendMsg(pUser->GetSock(),msg);
}

void CPackageDeal::DoObject(CNetMessage *pMsg,int sock)
{
    GET_MSG
    GET_USER
    
    uint8 op = 0;
    msg>>op;
    msg.ReWrite();
    msg.SetType(MSG_CLIENT_DO_OBJECT);
    switch(op)
    {
    case 1:
        {
            if(GetSysTime() - pUser->GetData32(4) > 10)
            {
                CScene *pScene = pUser->GetScene();
                if(pScene == NULL)
                    return;
                CBangPaiManager &bPMgr = SingletonCBangPaiManager::instance();
                CBangPai *pBangPai = bPMgr.FindBangPai(pScene->GetId()>>16);
                CBangPai *pBangPai1 = bPMgr.FindBangPai(pUser->GetBangPai());
                if((pBangPai == NULL) || (pBangPai1 == NULL))
                    return;
                if(pBangPai->ThieveResource(pUser,pBangPai1,msg))
                    pUser->SetData32(4,GetSysTime());
            }
            else
            {
                msg<<PRO_ERROR<<"偷窃间隔时间不能小于十秒";
            }
        }
        break;
    case 2:
        {
            CScene *pScene = pUser->GetScene();
            if(pScene == NULL)
                return;
            CBangPaiManager &bPMgr = SingletonCBangPaiManager::instance();
            CBangPai *pBangPai = bPMgr.FindBangPai(pScene->GetId()>>16);
            CBangPai *pBangPai1 = bPMgr.FindBangPai(pUser->GetBangPai());
            if((pBangPai == NULL) || (pBangPai1 == NULL))
                return;
            pBangPai->PoHuai(pUser,pBangPai1,msg);
        }
        break;
    default:
        return;
    }
    m_socketServer.SendMsg(pUser->GetSock(),msg);
}

void CPackageDeal::KaiJiaXiangQian(CNetMessage *pMsg,int sock)
{
    GET_MSG
    GET_USER
    
    uint8 kaijiaPos = 0;
    uint8 kongPos = 0;
    uint8 stonePos = 0;
    msg>>kaijiaPos>>kongPos>>stonePos;
    
    msg.ReWrite();
    msg.SetType(MSG_CLIENT_XIANGQIAN);
    string str;
    if(pUser->KaiJiaXiangQian(kaijiaPos,kongPos,stonePos,str))
        msg<<PRO_SUCCESS;
    else
        msg<<PRO_ERROR<<str;
    m_socketServer.SendMsg(pUser->GetSock(),msg);
}

void CPackageDeal::DelRole(CNetMessage *pMsg,int sock)
{
    GET_MSG
    
    ShareUserPtr ptr = m_onlineUser.GetUserBySock(sock);
    CUser *pUser = ptr.get();
    if(pUser == NULL)
        return;
        
    CGetDbConnect getDb;
    CDatabaseSql *pDb = getDb.GetDbConnect();
    if(pDb == NULL)
        return;
    
    uint32 roleId;
    msg>>roleId;    
    uint32 userId = pUser->GetUserId();
    
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
    
    sql = (boost::format("update user_info set del_time%1%=%2% where id=%3%")%(int)i%GetSysTime()%userId).str();
    pDb->Query(sql.c_str());
    msg.ReWrite();
    msg.SetType(MSG_CLIENT_DEL_CHAR);
    msg<<PRO_SUCCESS;
    m_socketServer.SendMsg(sock,msg);
}

void CPackageDeal::CancelDelRole(CNetMessage *pMsg,int sock)
{
    GET_MSG
    ShareUserPtr ptr = m_onlineUser.GetUserBySock(sock);
    CUser *pUser = ptr.get();
    if(pUser == NULL)
        return;
    
    CGetDbConnect getDb;
    CDatabaseSql *pDb = getDb.GetDbConnect();
    if(pDb == NULL)
        return;
    
    uint32 roleId;
    msg>>roleId;    
    uint32 userId = pUser->GetUserId();
    
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
    
    sql = (boost::format("update user_info set del_time%1%=0 where id=%2%")%(int)i%userId).str();
    pDb->Query(sql.c_str());
}

void CPackageDeal::IgnoreList(CNetMessage *pMsg,int sock)
{
    GET_MSG
    GET_USER
    
    list<HotInfo> ignoreList;
    
    pUser->ReadIgnore();
    
    pUser->GetIgnoreList(ignoreList);
    msg<<(uint8)ignoreList.size();
    
    char **row;
    char sql[128];
    CGetDbConnect getDb;
    CDatabaseSql *pDb = getDb.GetDbConnect();
    
    for(list<HotInfo>::iterator i = ignoreList.begin(); i != ignoreList.end(); i++)
    {
        msg<<i->hotId;
        ShareUserPtr ptr = m_onlineUser.GetUserByRoleId(i->hotId);
        CUser *pUser = ptr.get();
        if(pUser == NULL)
        {
            sprintf(sql,"select name,level from role_info where id=%d",i->hotId);
            if ((pDb == NULL) || !(pDb->Query(sql)))
            {
                msg<<" "<<(uint8)0<<(uint16)0;
                continue;
            }
            row = pDb->GetRow();
            if(row == NULL)
            {
                msg<<" "<<(uint8)0<<(uint16)0;
                continue;
            }
            msg<<row[0]<<(uint8)atoi(row[1])<<(uint16)0;
        }
        else
        {
            msg<<pUser->GetName()<<(uint8)pUser->GetLevel()<<pUser->GetMapId();
        }
        msg<<i->hotVal;
    }
    m_socketServer.SendMsg(pUser->GetSock(),msg);
}

void CPackageDeal::AddIgnore(CNetMessage *pMsg,int sock)
{
    GET_MSG
    GET_USER
    
    uint32 ignoreId;
    msg>>ignoreId;
    msg.ReWrite();
    msg.SetType(MSG_CLIENT_ADD_BLACK);
    
    pUser->ReadIgnore();
    
    uint16 val;
    if(pUser->GetHotVal(ignoreId,val))
    {
        msg<<PRO_ERROR<<"好友之间，无法加入黑名单";
        m_socketServer.SendMsg(pUser->GetSock(),msg);
        return;
    }
    char sql[128];
    CGetDbConnect getDb;
    CDatabaseSql *pDb = getDb.GetDbConnect();
    if(pDb == NULL)
        return;
    snprintf(sql,128,"select master from master_prentice where (p_id=%u and m_id=%u) or (m_id=%u and p_id=%u)"
        ,pUser->GetRoleId(),ignoreId,pUser->GetRoleId(),ignoreId);
    
    if(!pDb->Query(sql))
        return;
    char **row = pDb->GetRow();
    if(row != NULL)
    {
        msg<<PRO_ERROR<<"师徒之间，无法加入黑名单";
        m_socketServer.SendMsg(pUser->GetSock(),msg);
        return;
    }
    if(ignoreId == pUser->GetData32(6))
    {
        msg<<PRO_ERROR<<"夫妻之间，无法加入黑名单";
        m_socketServer.SendMsg(pUser->GetSock(),msg);
        return;
    }
    if(pUser->AddIgnore(ignoreId))
        msg<<PRO_SUCCESS;
    else
    {
        msg<<PRO_ERROR<<"黑名单超过上限";
        m_socketServer.SendMsg(pUser->GetSock(),msg);
        return;
    }
    m_socketServer.SendMsg(pUser->GetSock(),msg);
    
    snprintf(sql,128,"insert into role_ignore (role_id,ignore_id) values (%u,%u)",pUser->GetRoleId(),ignoreId);
    pDb->Query(sql);
}

void CPackageDeal::DelIgnore(CNetMessage *pMsg,int sock)
{
    GET_MSG
    GET_USER
    
    uint32 ignoreId;
    msg>>ignoreId;
    pUser->DelIgnore(ignoreId);
    
    CGetDbConnect getDb;
    CDatabaseSql *pDb = getDb.GetDbConnect();
    if(pDb == NULL)
        return;
    char sql[128];
    snprintf(sql,128,"delete from role_ignore where role_id=%u and ignore_id=%u",pUser->GetRoleId(),ignoreId);
    pDb->Query(sql);
}

void CPackageDeal::WWAgreeYaoQing(CNetMessage *pMsg,int sock)
{
    GET_MSG
    GET_USER
    
    uint8 agree = 0;
    uint32 bId = 0;
    
    msg>>agree>>bId;
    AgreeWWFangShou(pUser,agree,bId);
}

void CPackageDeal::ServerMgr(CNetMessage *pMsg,int sock)
{
    GET_MSG
    GET_USER

    if(pUser->AdminLevel() <= 0)
        return;    
        
    uint16 type = 0;//管理类型
    msg>>type;
    char errMsg[64];
    switch(type)
    {
    case 1://加通宝
        {
            uint32 roleId = 0;
            int tongbao = 0;
            msg>>roleId>>tongbao;
            AddTongBao(roleId,tongbao);
            WriteAdminLog(pUser->GetRoleId(),"%u加%d通宝",roleId,tongbao);
            strcpy(errMsg,"加通宝成功");
        }
        break;
    case 2://加物品
        {
            uint32 roleId = 0;
            uint16 tmplId = 0;
            uint8 num = 0;
            uint8 level = 0;
            uint8 bangding = 0;
            msg>>roleId>>tmplId>>num>>level>>bangding;
            if(num == 0)
            {
                return;
            }
            CItemTemplateManager &itemMgr = SingletonItemManager::instance();
            SItemTemplate *pItem = itemMgr.GetItem(tmplId);
            if(pItem == NULL)
            {
                strcpy(errMsg,"没有此物品");
            }
            else
            {
                SItemInstance item = {0};
                item.naijiu = pItem->naijiu;
                if(pItem->addXue != 0)
                {
                    int *p = (int*)item.addAttrVal;
                    *p = pItem->addXue;
                }
                item.tmplId = tmplId;
                item.num = num;
                item.level = level;
                item.bangDing = bangding;
                if(AddPackage(roleId,item))
                {
                    strcpy(errMsg,"添加物品成功");
                    WriteAdminLog(pUser->GetRoleId(),"%u加%d级%d物品%d个",roleId,level,tmplId,num);
                }
                else
                {
                    strcpy(errMsg,"玩家包裹已满");
                }
            }
        }
        break;
    case 3://踢下线，并且封号
        {
            uint32 roleId = 0;
            msg>>roleId;
            ShareUserPtr p = m_onlineUser.GetUserByRoleId(roleId);
            if(p.get() != NULL)
            {
                shutdown(p->GetSock(),SHUT_RD);
            }
            CGetDbConnect getDb;
            CDatabaseSql *pDb = getDb.GetDbConnect();
            if(pDb != NULL)
            {
                char buf[64];
                snprintf(buf,63,"update role_info set state=2 where id=%d",roleId);
                pDb->Query(buf);
            }
            strcpy(errMsg,"封号成功");
        }
        break;
    case 4://解封
        {
            uint32 roleId = 0;
            msg>>roleId;
            CGetDbConnect getDb;
            CDatabaseSql *pDb = getDb.GetDbConnect();
            if(pDb != NULL)
            {
                char buf[64];
                snprintf(buf,63,"update role_info set state=0 where id=%d",roleId);
                pDb->Query(buf);
            }
            strcpy(errMsg,"解封成功");
            break;
        }
    case 5://修改等级
        {
            uint32 roleId = 0;
            uint8 level = 0;
            msg>>roleId>>level;
            strcpy(errMsg,"修改等级成功");
            ShareUserPtr p = m_onlineUser.GetUserByRoleId(roleId);
            if(p.get() != NULL)
            {
                p->SetLevel(level);
                break;
            }
            CGetDbConnect getDb;
            CDatabaseSql *pDb = getDb.GetDbConnect();
            if(pDb != NULL)
            {
                char buf[64];
                snprintf(buf,63,"update role_info set level=%d where id=%d",level,roleId);
                pDb->Query(buf);
            }
        }
        break;
    }
    msg.ReWrite();
    msg.SetType(MSG_MGR);
    msg<<errMsg;
    m_socketServer.SendMsg(pUser->GetSock(),msg);
}

void CPackageDeal::WriteAdminLog(uint32 roleId,const char *fmt, ...)
{
    CGetDbConnect getDb;
    CDatabaseSql *pDb = getDb.GetDbConnect();
    if(pDb != NULL)
    {
        va_list ap;
        va_start(ap, fmt);
        char msg[512];
        vsnprintf(msg,512,fmt,ap);
        va_end (ap);        
        char buf[1024];
        snprintf(buf,1024,"INSERT INTO admin_log (role_id,msg) VALUES(%u,'%s')",roleId,msg);
        pDb->Query(buf);
    }
}

void CPackageDeal::XinShi(CNetMessage *pMsg,int sock)
{
    GET_MSG
    GET_USER
    
    uint8 type = 0;//1寄信，3收信
    msg>>type;
    
    CGetDbConnect getDb;
    CDatabaseSql *pDb = getDb.GetDbConnect();
    if(pDb == NULL)
        return;
        
    char sql[512];
    sprintf(sql,"delete from xin_shi where UNIX_TIMESTAMP(time) < %lu",GetSysTime() - 7*24*3600);
    pDb->Query(sql);
       
    //邮寄费=邮寄金钱*1%+每格道具3000 
    if(type == 1)
    {
        if(pUser->GetLevel() < 30)
        {
            msg.ReWrite();
            msg.SetType(MSG_SERVER_XINSHI);
            msg<<(uint8)1<<PRO_ERROR<<"您的等级未达到30级,不能邮寄";
            m_socketServer.SendMsg(pUser->GetSock(),msg);
            return;
        }
        msg>>type;
        uint32 roleId = 0;
        string roleName;
        if(type == 0)
        {
            msg>>roleId;
            char name[MAX_NAME_LEN] = {0};
            GetRoleName(roleId,name);
            roleName = name;
        }
        else if(type == 1)
        {
            msg>>roleName;
            uint8 level;
            roleId = GetRoleId(roleName.c_str(),level);
        }
        if((roleId == 0) || (roleName.size() <= 0))
        {
            msg.ReWrite();
            msg.SetType(MSG_SERVER_XINSHI);
            msg<<(uint8)1;
            msg<<PRO_ERROR<<"无此用户";
            m_socketServer.SendMsg(pUser->GetSock(),msg);
            return;
        }
        char pos1,pos2;
        uint8 num1,num2;
        int money;
        const int ITEM_FEI = 3000;
        string strMsg;
        msg>>pos1>>num1>>pos2>>num2>>money>>strMsg;
        int kouFei = 0;
        if(money < 0)
            return;
        if(money > 0)
        {
            int fei = money/100;
            kouFei = money + fei;
        }
        if(pos1 >= 0)
            kouFei += ITEM_FEI;
        if(pos2 >= 0)
            kouFei += ITEM_FEI;
        msg.ReWrite();
        msg.SetType(MSG_SERVER_XINSHI);
        msg<<(uint8)1;
        if(pUser->GetMoney() < kouFei)
        {
            msg<<PRO_ERROR<<"金钱不足";
            m_socketServer.SendMsg(pUser->GetSock(),msg);
            return;
        }
        SItemInstance item[2] = {{0},{0}};
        if(pos1 >= 0)
        {
            if((num1 <= 0) || !pUser->CanDelPackage(pos1))
            {
                msg<<PRO_ERROR<<"此物品不能邮寄";
                m_socketServer.SendMsg(pUser->GetSock(),msg);
                return;
            }
            SItemInstance *pItem = pUser->GetItem(pos1);
            if(pItem == NULL)
                return;
            item[0] = *pItem;
            if((num1 > item[0].num) || (item[0].bangDing == 1))
            {
                msg<<PRO_ERROR<<"此物品不能邮寄";
                m_socketServer.SendMsg(pUser->GetSock(),msg);
                return;
            }
            item[0].num = num1;
        }
        if(pos2 >= 0)
        {
            if((num2 <= 0) || !pUser->CanDelPackage(pos2))
            {
                msg<<PRO_ERROR<<"此物品不能邮寄";
                m_socketServer.SendMsg(pUser->GetSock(),msg);
                return;
            }
            SItemInstance *pItem = pUser->GetItem(pos2);
            if(pItem == NULL)
                return;
            item[1] = *pItem;
            if((num2 > item[1].num) || (item[1].bangDing == 1))
            {
                msg<<PRO_ERROR<<"此物品不能邮寄";
                m_socketServer.SendMsg(pUser->GetSock(),msg);
                return;
            }
            item[1].num = num2;
        }
        if((pos1 >= 0) && (pos2 >= 0) && (pos1 == pos2))
        {
            msg<<PRO_ERROR<<"不能邮寄同一格子道具";
            m_socketServer.SendMsg(pUser->GetSock(),msg);
            return;
        }
        if(pos1 >= 0)
        {
            char buf[128];
            snprintf(buf,sizeof(buf),"邮寄给%u",roleId);
            SaveUseItem(pUser->GetRoleId(),item[0],buf,item[0].num,"","");
            pUser->DelPackage(pos1,num1);
        }
        if(pos2 >= 0)
        {
            char buf[128];
            snprintf(buf,sizeof(buf),"邮寄给%u",roleId);
            SaveUseItem(pUser->GetRoleId(),item[0],buf,item[0].num,"","");
            pUser->DelPackage(pos2,num2);
        }
        pUser->AddMoney(-kouFei);
        string strCompress;
        Compress((uint8*)item,sizeof(item),strCompress);
        snprintf(sql,512,"insert into xin_shi (from_id,from_name,to_id,item,msg,money) values (%u,'%s',%u,'%s','%s',%d)",
            pUser->GetRoleId(),pUser->GetName(),roleId,strCompress.c_str(),strMsg.c_str(),money);
        pDb->Query(sql);
        msg<<PRO_SUCCESS<<"发送成功";
        m_socketServer.SendMsg(pUser->GetSock(),msg);
        snprintf(sql,512,"【%s】邮寄了一些东西给你，请及时到皇宫找信使收取，过期不取系统自动删除信件及信件内物品、金钱",pUser->GetName());
        SendSysMail(roleId,sql);
        return;
    }
    else if(type == 3)
    {
        uint32 id = 0;
        msg>>id;
        sprintf(sql,"SELECT item,money FROM xin_shi where id=%u and to_id=%u",id,pUser->GetRoleId());
    
        if(!pDb->Query(sql))
            return;
        msg.ReWrite();
        msg.SetType(MSG_SERVER_XINSHI);
        msg<<(uint8)3<<id;
        char **row = pDb->GetRow();
        if(row == NULL)
            return;
        if((row[0] != NULL) && (row[0][0] != 0))
        {
            SItemInstance item[2] = {{0}};
            uint32 len = sizeof(item);
            UnCompress(row[0],(uint8*)&item,len);
            uint8 num = 0;
            if(item[0].tmplId != 0)
                num ++;
            //pUser->AddPackage(item[0]);
            if(item[1].tmplId != 0)
                num++;
            //pUser->AddPackage(item[1]);
            if(pUser->EmptyPackage() < num)
            {
                msg<<PRO_ERROR<<"包裹已满无法收取";
                m_socketServer.SendMsg(pUser->GetSock(),msg);
                return;
            }
            if(item[0].tmplId != 0)
                pUser->AddPackage(item[0]);
            if(item[1].tmplId != 0)
                pUser->AddPackage(item[1]);
            pUser->AddMoney(atoi(row[1]));
            sprintf(sql,"delete from xin_shi where id=%u",id);
            pDb->Query(sql);
            msg<<PRO_SUCCESS<<"成功";
            m_socketServer.SendMsg(pUser->GetSock(),msg);
        }
    }
}

void CPackageDeal::LearnSkill(CNetMessage *pMsg,int sock)
{
    GET_MSG
    GET_USER
    
    uint16 skillId = 0;
    msg>>skillId;
    msg.ReWrite();
    msg.SetType(MSG_CLIENT_LEARN_SKILL);
    if(::LearnSkill(pUser,(int)skillId))
    {
        msg<<PRO_SUCCESS;
    }
    else
    {
        msg<<PRO_ERROR<<"学习失败";
    }
    m_socketServer.SendMsg(pUser->GetSock(),msg);
}

void CPackageDeal::YeWaiShiYao(CNetMessage *pMsg,int sock)
{
    GET_MSG
    GET_USER
    
    if(pUser->GetFightId() != 0)
        return;
        
    uint8 answer;
    msg>>answer;
    if(answer != 1)
    {
        return;
    }
    switch(Random(1,9))
    {
    case 1:
        ShiYaoYW2(pUser);
        break;    
    case 2:
        ShiYaoYW3(pUser);
        break;
    case 3:
        ShiYaoYW4(pUser);
        break;
    case 4:
        ShiYaoYW5(pUser);
        break;
    case 5:
        ShiYaoYW6(pUser);
        break;
    case 6:
        ShiYaoYW7(pUser);
        break;    
    case 7:
        ShiYaoYW8(pUser);
        break;
    case 8:
        ShiYaoYW9(pUser);
        break;
    case 9:
        ShiYaoYW10(pUser);
        break;
    }
}

void CPackageDeal::GuanZhan(CNetMessage *pMsg,int sock)
{
    GET_MSG
    GET_USER
    
    if(pUser->GetFightId() != 0)
        return;
    uint32 roleId = 0;
    msg>>roleId;
    msg.ReWrite();
    msg.SetType(GUANZHAN_ENTER_BATTLE);
    
    if(pUser->GetFightId() != 0)
    {
        msg<<PRO_ERROR<<"战斗中不能观战";
        m_socketServer.SendMsg(pUser->GetSock(),msg);
        return;
    }
    ShareUserPtr p = m_onlineUser.GetUserByRoleId(roleId);
    if(p.get() == NULL)
    {
        msg<<PRO_ERROR<<"玩家不在线";
        m_socketServer.SendMsg(pUser->GetSock(),msg);
        return;
    }
    if(p->GetScene() != pUser->GetScene())
    {
        msg<<PRO_ERROR<<"不在同一场景无法观战";
        m_socketServer.SendMsg(pUser->GetSock(),msg);
        return;
    }
    uint32 fightId = p->GetFightId();
    if(fightId == 0)
    {
        msg<<PRO_ERROR<<"玩家不在战斗中";
        m_socketServer.SendMsg(pUser->GetSock(),msg);
        return;
    }
    ShareFightPtr pFight = SingletonFightManager::instance().FindFight(fightId);
    if(pFight.get() == NULL)
    {
        msg<<PRO_ERROR<<"玩家不在战斗中";
        m_socketServer.SendMsg(pUser->GetSock(),msg);
        return;
    }
    msg<<PRO_SUCCESS;
    m_socketServer.SendMsg(pUser->GetSock(),msg);
    pFight->GuanZhan(pUser);
    pUser->SetGuanZhan(fightId);
}

void CPackageDeal::LeaveGuanZhan(CNetMessage *pMsg,int sock)
{
    GET_USER
    
    uint32 fightId = pUser->GetGuanZhan();
    if(fightId == 0)
        return;
        
    ShareFightPtr pFight = SingletonFightManager::instance().FindFight(fightId);
    if(pFight.get() != NULL)
    {
        pFight->LeaveGuanZhan(pUser);
    }
}

//查询称号
void CPackageDeal::QueryTextTitle(CNetMessage *pMsg,int sock)
{
    GET_MSG
    GET_USER
    
    pUser->QueryTextTitle(msg);
    m_socketServer.SendMsg(pUser->GetSock(),msg);
}

void CPackageDeal::UseTextTitle(CNetMessage *pMsg,int sock)
{
    GET_MSG
    GET_USER
    string title;
    msg>>title;
    pUser->UseTextTitle(title.c_str());
}

void CPackageDeal::GetNewUserGift(CNetMessage *pMsg,int sock)
{
    GET_USER
    pUser->GetGift(1);
}
void CPackageDeal::GetNormalGift(CNetMessage *pMsg,int sock)
{
    GET_USER
    pUser->GetGift();
}
