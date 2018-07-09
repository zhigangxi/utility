#ifndef _GLOBAL_VAL_MANAGER_H_
#define _GLOBAL_VAL_MANAGER_H_
#include <boost/any.hpp>

enum EGlobalValueType
{
    EGV_ONLINE_USER_LIST,
    EGV_CDESPATCH_COMMAND,
    EGV_NPC_MANAGER,//CNpcManager

    //如果添加此类管理的全局变量，请在此前添加	
    EGV_GLOBAL_NUMBER
};

class CGlobalValManager
{
public:
    static CGlobalValManager *CreateInstance();

    template<typename AddType>
    void AddVal(AddType *val,EGlobalValueType index)
    {
        assert(index < EGV_GLOBAL_NUMBER);
        m_val[index] = val;
    }

    template<typename GetType>
    void GetVal(GetType **val,EGlobalValueType index)
    {
        *val = boost::any_cast<GetType*>(m_val[index]);
    }

private:
    CGlobalValManager(){}
    ~CGlobalValManager(){}
    boost::any  m_val[EGV_GLOBAL_NUMBER];
    static CGlobalValManager *m_manager;
};
#endif