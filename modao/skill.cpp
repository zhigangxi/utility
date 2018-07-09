#include "skill.h"
#include <algorithm>
using namespace std;

uint8 GetSkillType(uint8 id)
{
    if(id <= 50)
    {
        return ESTDamageHp;
    }
    else if(id <= 100)
    {
        id = id - 51;
        return id / 4 + ESTForget;
    }
    else if(id <= 150)
    {
        id = id - 101;
        return id / 4 + ESTImproveDamage;
    }
    else 
    {
        return id - 151 + ESTshenshengzhiguang;
    }
}

uint8 GetSkillXiang(uint8 id)
{
    if(id <= 50)
    {
        id -= 1;
    }
    else if(id <= 100)
    {
        id -= 51;
    }
    else if(id <= 150)
    {
        id -= 101;;
    }
    else
    {
        return 0;
    }
    return id/4+1;
}

uint8 GetSkillJie(uint16 id)
{
    uint8 num = 0;
    if(id <= 50)
    {
        num = id % 4;
    }
    else if(id <= 100)
    {
        id = id - 50;
        num = id % 4;
    }
    else
    {
        id = id - 100;
        num = id % 4;
    }
    if(num == 0)
        num = 4;
    return num;
}

uint8 GetSkillAttackNum(uint8 id,uint8 level)
{
    uint8 maxNum = 1;
    if(level < 30)
        return maxNum;
    else if(level < 60)
        maxNum = 2;
    else if(level < 90)
        maxNum = 3;
    else
        maxNum = 4;
    
    switch(GetSkillJie(id))
    {
    case 1:
        return 1;
    case 2:
        return min(2,(int)maxNum);
    case 3:
        return min(4,(int)maxNum);
    case 4:
        return 1;
    default:
        return 1;
    }
}
