#ifndef _JIANGHU_PROTOCOL_H_
#define _JIANGHU_PROTOCOL_H_
#include "self_typedef.h"
const uint8 PRO_ERROR         = 0;
const uint8 PRO_SUCCESS       = 1;

//性别
const uint8 GENDER_MALE     = 0;
const uint8 GENDER_FEMALE   = 1;

//向性
const uint8 XIANG_JIN       = 1;
const uint8 XIANG_MU        = 2;
const uint8 XIANG_SHUI      = 3;
const uint8 XIANG_HUO       = 4;
const uint8 XIANG_TU        = 5;

//最大角色数
const int MAX_ROLE_NUM      = 6;

//最大用户数
const int MAX_CON_USER      = 4096;

//协议命令字
const int PRO_USER_LOGIN    = 1;
const int PRO_REG_USER      = 2;
const int PRO_CREATE_ROLE   = 3;
const int PRO_SELECT_ROLE   = 4;
const int PRO_NPC_LIST      = 5;
const int PRO_USER_LIST     = 6;
const int PRO_ROLE_EQUIPMENT = 7;
const int PRO_ROLE_PACKAGE  = 8;
const int PRO_ROLE_MOVE     = 9;
const int PRO_JUMP_SCENE    = 10;
const int PRO_IN_OUT_SCENE  = 11;
const int PRO_OPEN_INTERACT = 12;

//30宠物仓库,宠物信息
//31师徒榜
const int PRO_INTERACT      = 13;

const int PRO_CLOSE_INTERACT    = 14;

const int PRO_UPDATE_PACK   = 15;
const int PRO_GET_ITEM_INFO = 16;

const int PRO_OUTFIT        = 17;//装备物品
const int PRO_UPDATE_CHAR   = 18;

const int PRO_OTHER_ITEM_INFO = 19;//查询其他玩家物品信息

const int PRO_PLAYER_DETAIL    = 20;

const int PRO_ENTER_BATTLE  = 21;//进入战斗
const int PRO_BATTLE        = 22;//战斗过程
const int PRO_BATTLE_OVER   = 23;//战斗结束

const int PRO_QUERY_PET     = 24;//宠物列表

const int PRO_UPDATE_PET    = 25;//客户端宠物操作，服务器端更新宠物列表

const int PRO_MSG_CHAT      = 26;//聊天

const int PRO_USER_SKILL    = 27;//

const int PRO_UPDATE_SKILL  = 28;

const int PRO_USER_TEAM     = 29;// 队伍

const int PRO_UPDATE_TEAM   = 30;//更新队伍

const int PRO_SYS_INFO      = 31;//系统消息

const int PRO_USER_PK       = 32;//玩家pk

const int PRO_PLYAER_MATCH  = 33;//玩家切磋

const int PRO_PLAYER_INFO   = 34;//查询玩家信息

const int PRO_NEAR_PLAYER_LIST = 35;//附近玩家列表

const int PRO_UPDATE_PLAYER = 36;//更新玩家状态

const int PRO_TASK_LIST     = 37;//任务列表

const int PRO_TASK_INFO     = 38;//任务信息

const int PRO_UPDATE_TASK   = 39;//更新任务

const int PRO_PET_SKILL     = 40;

const int PRO_UPDATE_PET_SKILL = 41;//更新宠物技能

const int PRO_HOT_LIST      = 42;

const int PRO_ADD_HOT       = 43;

const int PRO_DEL_HOT       = 44;

const int PRO_HOT_ONLINE    = 45;//好友上下线通知

const int PRO_ONLINE_MAIL   = 46;//

const int RPO_QUERY_MAIL    = 47;//查询信件

const int PRO_MAIL_ACK      = 48;//信件

const int PRO_PSHOP_PUT     = 49;

const int PRO_PSHOP_LIST    = 50;//个人商店

const int PRO_PSHOP_BUY     = 51;//个人商店购买物品

const int PRO_PSHOP_DETAIL  = 52;//查询个人商店物品

const int PRO_SAFE_TRADE   = 53;//安全交易

const int PRO_BANGPAI       = 54;//帮派

const int PRO_CLOSE_PLAYER = 55;//3×3格子的玩家

const int PRO_LIST_ESHOP    = 56;//商城

const int PRO_BUY_EITEM = 57;//购买商城物品

const int PRO_UPDATE_NPC    = 58;

const int PRO_ADD_NPC = 59;//添加npc

const int PRO_DEL_NPC = 60;//del npc

const int PRO_SYNC_POS  = 61;//同步角色位置

const int PRO_SYSTEM_INFO  = 62;//系统消息

const int PRO_EQUIP_ENFORCE = 63;//装备强化

const int PRO_ENFORCE_QUERY = 64;//查询装备强化信息

const int PRO_EQUIP_BLUED = 65;//制作篮装

const int PRO_EQUIP_GREEND = 66;//制作绿装

const int PRO_EQUIP_REFINE = 67;//绿装属性重新筛选

const int PRO_SKILL_DESC = 68;//查询技能描述

const int PRO_OTHER_PET = 69;//查询玩家宠物

const int PRO_UPDATE_PET_PARAM = 70;//更新宠物信息

const int PRO_ORNAMENT_SETTING = 71;//镶嵌饰品
const int PRO_ORNAMENT_CARVE    = 72;//饰品属性重生成

const int PRO_SWITCH_CHANNEL    = 73;//开关聊天频道

const int PRO_SWITCH_INFO = 74;//查询聊天通道是否关闭

const int PRO_SYS_POP_MSG = 75;//系统弹出消息

const int PRO_SYS_INFO1 = 76;//

const int PRO_SERVER_UPDATE_EQUIP = 77;//跟新装备

/*
一键吃血，吃蓝							
使用包裹里的补血道具将玩家角色和出战宠物的血或蓝恢复至满。道具不
足的情况下优先对玩家角色进行回复。				
回血物品ID的使用优先顺序：					
1。1804							
2。651-682							
回蓝物品ID的使用优先顺序：					
1。1806							
2。701-730							
*/
const int PRO_EASY_RECOVER  = 78;//一键回血会蓝

const int PRO_PSHOP_SOLD    = 79;//购买物品，给卖出着放松更新消息

const int PRO_MY_BANG       = 80;//我的帮派

const int PRO_CHANGE_FACE   = 81;//更新角色方向

const int PRO_CANCEL_TASK   = 82;//取消任务

const int PRO_ITEM_DESC = 83;//物品描述

const int PRO_CHARGE = 84;//客户端充值

const int PRO_QUERY_RLOC = 85;//

const int PRO_CLIENT_PATH = 87;//寻路信息

const int PRO_GONGGAO   = 88;//系统公告

const int PRO_AVAILABLE_TASK = 89;//可接任务

const int PRO_SCENE_POS = 90;//脱离卡死坐标

const int PRO_SERVER_VERSION = 91;

const int PRO_SPEC_CHAT = 92;

const int MSG_SERVER_BEGIN_XING = 93;

const int MSG_ANSWER_XING = 94;

const int MSG_SERVER_END_XING = 95;

const int MSG_SERVER_MONSTER = 96;//场景明怪

const int MSG_SERVER_ADD_MONSTER = 97;//增加明怪

const int MSG_SERVER_REMOVE_MONSTER = 98;//删除明怪

const int MSG_CLIENT_MONSTER_BATTLE = 99;//明怪战斗

const int MSG_QUERY_SCENE   = 100;//获取场景文件

const int MSG_CLIENT_ITEM_DEF = 101;//

const int MSG_SERVER_HEART_BEAT = 102;

const int MSG_LIST_SERVER = 103;

const int MSG_CLIENT_SYS_MAIL = 104;

const int MSG_CLIENT_LIST_TITLE = 105;

const int MSG_CLIENT_USE_TITLE = 106;

const int MSG_SERVER_JUMP_POINT = 107;

const int MSG_SERVER_QUERY_BILL = 108;

const int MSG_CLIENT_LIST_SHITU = 109;

const int MSG_SERVER_LIST_OBJECT = 110;

const int MSG_SERVER_ADD_OBJECT = 111;

const int MSG_SERVER_UPDATE_OBJECT = 112;

const int MSG_SERVER_REMOVE_OBJECT = 113;

const int MSG_CLIENT_DO_OBJECT  = 114;

const int MSG_SERVER_USE_RESULT = 115;
/*
+----+-----+------+------+
| OP | RES | VAL1 | VAL2 |
+----+-----+------+------+
|  1 |  1  |   4  |   4  |
+----+-----+------+------+
OP=1 宠物学习技能
宠物打书 VAL1 学会的技能ID VAL2 覆盖的技能ID(等于0，表示没
有遗忘的技能)
*/

const int MSG_CLIENT_XIANGQIAN = 116;
/*
+------+------+-------+
| EIND | KONG | STONE |
+------+------+-------+
|   1  |   1  |    1  |
+------+------+-------+
EIND 装备位置
KONG 镶嵌位置
STONE 石头位置
服务器返回
MSG_SERVER_XIANGQIAN(116)
+-----+
| RES |
+-----+
|  1  |
+-----+
*/

const int MSG_CLIENT_LIST_FUQI = 117;
/*
+-------+------+-----+-----+------+------+-----+       
| STATE | FUQI | CID | LEN | NICK | CMAP | HOT |       
+-------+------+-----+-----+------+------+-----+       
|   1   |   1  |  4  |  2  |  Var |   2  |  2  |       
+-------+------+-----+-----+------+------+-----+       
STATE=0 没有结婚        
FUQI ＝0夫 ＝1妻       
CMAP 为在线好友当前位置,离线为0       
HOT  亲密度   
*/

const int MSG_SERVER_VISUAL_EFFECT = 119;
/*
+------+
| TYPE |
+------+
|   1  |
+------+
TYPE=1 普通礼花
TYPE=2 结婚礼花
*/

const int MSG_CLIENT_DEL_CHAR = 118;

const int MSG_CLIENT_UNDEL_CHAR = 120;

const int MSG_SERVER_LIST_BLACK = 121;

const int MSG_CLIENT_ADD_BLACK = 122;

const int MSG_CLIENT_DEL_BLACK = 123;

const int MSG_SERVER_INVITE_ALLY = 124;

const int MSG_SERVER_NPC_MOVE = 125;

const int MSG_SERVER_NPC_EFFECT = 126;

const int MSG_SERVER_NPC_SAY = 127;

const int MSG_SERVER_XINSHI = 128;

const int MSG_SERVER_LIST_SKILL = 129;

const int MSG_CLIENT_LEARN_SKILL = 130;

const int MSG_SERVER_PROMPT = 131;

const int MSG_SERVER_DO_PET = 132;

const int GUANZHAN_ENTER_BATTLE  = 133;//观战进入战斗
const int LEAVE_GUANZHAN = 134;//退出观战
const int ENTER_GUANZHAN = 135;//观战战斗开始
const int GUAGNZHAN_BATTLE        = 136;//观战战斗过程
const int GUANGZHAN_BATTLE_OVER   = 137;//观战战斗结束


const int UPDATE_OPSN_PACK = 138;//开启背包、仓库、宠物仓库,消息内容(byte)type,(byte)num,(int)time
                                //type:1开启第四背包数量，2开启仓库数量，3开启宠物仓库数量，num为开启的数量，time为结束时间

const int PRO_TASK_INFO_EXT = 139;

/*称号,
客户端发送此信息询问自己的称号，信息内容空
服务器返回玩家称号，内容：(byte)数量 （char*）称号(byte)type（0未获得，1获得，2使用）
*/
const int MSG_TEXT_TITLE = 140;

//使用称号，客户端发送此消息，消息内容为：(char*)称号文本
const int MSG_USE_TEXT_TITLE = 141;

//师徒榜玩家信息
//客户端发送内容(byte)0师傅榜，1徒弟榜
//服务器返回(char*)info,文字中|做为换行符
const int MSG_SHI_TU_USER_INFO = 142;

//新手礼盒
//客户端发送表示获取礼盒,无内容
//服务器发送表示可领取新手礼盒，内容(short)物品id，num(byte) (short)下一次领取剩余时间（分钟），(char*)物品描述
const int MSG_NEW_USER_GIFT = 143;

//每日礼盒
//客户端发送表示获取礼盒,无内容
//服务器发送表示可领取每日礼盒，type(byte)(1物品，2经验、3潜能，4道行)内容(short)获得物品id 或者数值，num(byte),(short)下一次领取剩余时间（分钟），(char*)物品描述
//(byte)显示物品数量,type(byte)(1物品，2经验、3潜能，4道行)内容(short)获得物品id或者数值,num(byte)
const int MSG_NORMAL_USER_GIFT = 144;

const int MSG_MGR = 0xfffe;

#ifdef QQ
const int MSG_QQ_LOGIN = 0xffff;
#endif

#endif
