struct SItemTemplate
{
    SItemTemplate():
        type(0),//种类
        level(0),//需求等级	
        mod(0),//模式
        sex(0),//性别
        fangYu(0),//防御
        id(0),
        gongji(0),//攻击力
        naijiu(0),//耐久
        addFaLi(0),//增加法力
        addQiXue(0),//增加气血
        addSudu(0),//增加速度
        pic(0),//图片
        addXue(0),//增加血量
        jiage(0),//价格
        pScript(NULL)
    {
    }
    
    uint8 type;//种类
    uint8 level;//需求等级	
    uint8 mod;//模式
    uint8 sex;//性别
    uint16 fangYu;//防御
    uint16 id;
    uint16 gongji;//攻击力
    uint16 naijiu;//耐久
    uint16 addFaLi;//增加法力
    uint16 addQiXue;//增加气血
    uint16 addSudu;//增加速度
    uint16 pic;//图片
    int addXue;//增加血量
    int jiage;//价格    
    string name;//名字
    string describe;//说明
};

#define MAX_ADD_ATTR_NUM  9

struct SItemInstance
{
    uint8 level;
    uint8   strengthenLevel;//强化需求
    uint8   quality;//品质
    uint8   addAttrNum;//附加属性数量
    uint8   num;
    uint8   addAttrType[MAX_ADD_ATTR_NUM];//附加属性类型
    uint8   reqAttrType;//属性需求类型
    uint16  reqAttrVal;//属性需求值
    uint16  addAttrVal[MAX_ADD_ATTR_NUM];//附加属性值
    uint16  naijiu;//耐久
    uint16 tmplId;
    int GetItemValue();
    int GetAddAttrType(uint8 pos);
    int GetAddAttrVal(uint8 pos);
};

