#ifndef _NCP_MANAGER_H_
#define _NCP_MANAGER_H_
#include "self_typedef.h"
#include "hash_table.h"
#include "net_msg.h"
#include <time.h>
#include <string>
#include <list>
#include <iostream>

using namespace std;
class CCallScript;

struct SNpcTemplate
{
    SNpcTemplate():pScript(NULL)
    {
    }
    uint16 id;
    string name;
    uint16 pic;
    uint8 type;
    //int script;
    CCallScript *pScript;
};

/*
   +-----+--------+--------+--------+-------+-------+-------+
   | SEX | WEAPON | ULEVEL | HELMET | CLASS | ARMOR | CLASS |
   +-----+--------+--------+--------+-------+-------+-------+
   |  1  |    2   |    1   |   2    |    1  |   2   |   1   |
   +-----+--------+--------+--------+-------+-------+-------+
*/
struct HumanData
{
    uint32 roleId;
    uint16 weapon;
    uint16 helmet;
    uint16 armor;
    uint8 level;
    uint8 sex;
    uint8 helmetClass;
    uint8 armorClass;
};

struct SNpcInstance
{
    SNpcInstance():id(0),
        templateId(0),
        sceneId(0),
        x(0),
        y(0),
        timeOut(0),
        pNpc(NULL),
        type(0),
        direct(8),
        pHumanData(NULL)
    {
    }
    void MakeNpcInfo(CNetMessage &msg);
    
    uint16 id;
    uint16 templateId;
    int sceneId;
    uint8 x;
    uint8 y;
    time_t timeOut;
    SNpcTemplate *pNpc;
    uint8 type;
    uint8 direct;//·½Ïò
    HumanData *pHumanData;
};

class CNpcManager
{
public:
    CNpcManager();
    ~CNpcManager();
    void GetSceneNpc(int sceneId,list<uint16>*);
    bool Init();
    SNpcTemplate *GetNpcTemplate(uint16 tempId);
    SNpcInstance *GetNpcInstance(uint16 id);
private:
    bool m_isInit;
    bool ForEachInsFun(uint16 instanceId,SNpcInstance *,uint16 sceneId,list<uint16>*);
    CHashTable<uint16,SNpcTemplate*> m_npcTemplate;
    CHashTable<uint16,SNpcInstance*> m_npcInstance;
};
#endif
