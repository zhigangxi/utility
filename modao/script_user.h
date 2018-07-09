class CScene;

class CUser
{
public:
    uint32 GetUserId();
    uint32 GetRoleId();
    uint8 GetFace();
    CScene *GetScene();
    
    uint32 GetFightId();
    uint8 GetFightPos();
    
    const char *GetName();
    uint8 GetSex();
    uint8 GetHead();
    uint8 GetXiang();
    uint8 GetX();
    uint8 GetY();
    uint8 GetLevel();
    uint32 GetExp();
    uint16 GetTiZhi();
    uint16 GetLiLiang();
    uint16 GetMinJie();
    uint16 GetLingLi();
    uint16 GetNaiLi();
    uint16 GetShuXinDian();
    uint16 GetJin();
    uint16 GetMu();
    uint16 GetShui();
    uint16 GetHuo();
    uint16 GetTu();
    uint16 GetXiangXinDian();
    int GetHp();
    int GetMp();
    int GetMaxHp();
    int GetMaxMp();
    
    void SetName(const char *name);
    void AddLevel(bool callScript = true);         //等级
    void AddExp(int exp,bool callScript = true);            //经验
    
    void AddTiZhi(uint16 tizhi);        //体质
    void AddLiLiang(uint16 liliang);    //力量
    void AddMinJie(uint16 minjie);      //敏捷
    void AddLingLi(uint16 lingli);      //灵力
    void AddNaiLi(uint16 naili);        //耐力
    void AddShuXinDian(int shuxindian);//未分配属性点
    void AddJin(uint16 jin);            //金相性
    void AddMu(uint16 mu);              //木相性
    void AddShui(uint16 shui);          //水相性
    void AddHuo(uint16 huo);            //火相性
    void AddTu(uint16 tu);              //土相性
    void AddXiangXinDian(uint16 xiangxindian);  //未分配相性点
    void AddHp(int hp);              //气血
    void AddMp(int mp);              //法力
    
    int GetDamage();//物理伤害
    
    int GetRecovery();//防御
    int GetSpeed();//速度
    
    void AddDamage(int damage);
    void AddSkillDamage(int skillDamage);
    void AddRecovery(int recovery);
    void AddSpeed(int speed);
    
    bool AddPackage(SItemInstance &item);
    bool AddPackage(int itemId,int num = 1);	//增加物品，指定物品id，目前不支持指定物品数量            
    bool DelPackage(int pos,int num = 1); //删除物品，指定物品在包裹里的位置，0-35，直接清除格子        
    
    uint32 GetMoney();
    int GetTili();
    void SetMoney(uint32 money);	
    void SetTili(int tili);
    void AddMoney(int add);			//增加金钱
    void AddTili(int add);			//增加体力
    
    int GetItemfangYu();//防御
    int GetItemGongji();//攻击力
    int GetItemFaLi();//增加法力
    int GetItemQiXue();//增加气血
    int GetItemSudu();//增加速度
    
    uint32 GetTeam();
    
    bool AddPet(SharePetPtr &pPet);	//增加宠物，根据函数创建
    
    void AssignPetAttr(uint8 pos,uint8 attr);
    void AssignPetKangXing(uint8 pos,uint8 attr);
    void SetGuanKanPet(uint8 pos);
    void SetChuZhanPet(uint8 pos);
    void SetPetIdle(uint8 pos);
    //void UseItemToPet(uint8 pos,uint8 itemPos);
    void XunYangPet(uint8 pos);
    void DelPet(uint8 pos);
    
    void SetPkVal(uint16 pk);
    uint16 GetPkVal();
    void AddPkVal(int val);
    
    void SetCall(int script,const char *call);
    
    uint8 GetChuZhanPet();
    
    const char *GetMission(int id);				//获取任务
    bool AddMission(int id,const char *pMiss); 	//增加任务
    void UpdateMission(int id,const char *pMiss);//更新任务
    void DelMission(int id);					//删除任务
    SPet GetPetByPos(int pos);
    void UpdatePetToBaby(uint8 pos);
    
    //用于临时保存变量，不存储数据库
    void SetVal(int id,int val);
    //获取保存变量
    int GetVal(int id);
    
    //保存变量，下线后保存数据库
    void SetSaveVal(uint8 index,int val);
    int GetSaveVal(uint8 index);
    
    //设置位，存储数据库
    void SetBitSet(int ind);
    void ClearBitSet(int ind);
    bool HaveBitSet(int ind);
    
    SItemInstance *GetItem(uint8 pos);
    
    //0成功，1，失败，2不能装备
    //int QiangHuaPackage(uint8 itemPos,uint8 stonePos);
    
    //0成功，1，失败，2不能炼化，stone1,stone2,stone3为黑水晶位置
    int LianHuaPackage(uint8 itemPos,uint8 stone1,uint8 stone2,uint8 stone3);
    
    //0成功，1，失败，2不能炼化，tianHuanShi天换石(623)为位置()
    int TianHuanPackage(uint8 itemPos,uint8 tianHuanShi);//,uint8 lianHuaShi);
    
    //0成功，1，失败，2不能修理
    int RepairItem(uint8 pos);
    
    int GetRepairFee();
    void RepairTotel();
    
    //不限制数量num传入-1
    bool DelPackageById(int id,int num);
    
    int GetMenPai();
    void SetMenPai(int menpai);
    
    int GetDaoHang();
    
    //设置进入场景是否调用脚本
    //void SetEnterSceneCall(int id);
    
    int AddDaoHang(int val);
    void AddQianNeng(int qianNeng);
    
    void SetCallScript(int script);
    void SetCallFun(const char *call);
    void SetMPGongXian(int val);
    int GetMPGongXian();
    
    void SetSkill(int skillId,int level);
    int GetSkillLevel(int skillId);
    
    void SetPetZhongCheng(uint8 pos,int zhongcheng);
    
    void PetRestore(uint8 pos);
    
    //返回位置，-1鉴定失败
    int JianDingItem(uint8 pos);
    
    void SetQianNeng(int qianNeng);
    int GetQianNeng();
    
    bool HaveItem(int id);
    
    //waitTime单位为分钟
    void AddTimer(int id,int waitTime);
    void DelTimer(int id);    
    
    //合成炼化石
    bool CompoundStone(uint8 pos,uint8 num);
    
    //合成物品，返回合成物品id，0时表示合成失败,-1包裹满
    int CompoundItem(uint8 pos1,uint8 pos2,uint8 pos3);
    int CompoundItem1(int id);
    
    //鉴定饰品
    bool JianDingShiPin(uint8 pos);
    
    //饰品镶嵌
    bool XiangQian(uint8 pos,uint8 stone);
    
    bool HavePet(int id);
    bool HaveBaobaoPet(int id);
    
    //获得出战宠物,宠物id为0时表示没有出战宠物
    SPet GetCZPet();
    //设置出战宠物武学
    void SetCZPetWuXue(int wuxue);
    //判断任务是否满
    bool MissionFull();
    
    //如果有帮派返回id，如果无返回0
    uint32 GetBangPai();
    
    int GetBangState();
    int GetBangRank();
    void DismissBang();// 解散帮派
    void UndismissBang();// 解除解散状态
    int GetItemNum(int id);
    
    SItemInstance *GetEquip(uint8 pos);
    
    //给予绑定物品
    bool AddBangDingPackage(int itemId,int num=1);
    //给予蓝水晶          属性      属性值
    bool AddBlueCrystal(int attr,int attrVal);
    //给予炼化石
    bool AddLianHuaStone(int level,int num = 1);
    //给予指定等级强化装备
    bool AddLevelPackage(int itemId,int level);
    
    bool AddAttrPackage(int itemId,int attr,int attrVal);
    
    int EmptyPackage();
    
    void AddTongBao(int tongbao);
    
    void SaveSellItem(uint8 pos,uint8 num);
    
    //0解绑成功，1 没有宠物解绑符，2 宠物非绑定、3 解绑时间未到
    int PetJieBang(uint8 petPos);   
    
    int GetLeftDoubleTime();
    void SetDouble(int hour);
    
    //打开背包，1第三个、2第四个
    void OpenPackage(int tab);
    
    int GetShengWang();
    
    int GetJiFen();
    int GetDieTimes();
    
    void SetShiFu();
    
    int GetChuShiNum();
    
    //0 师傅等级
    //1 比赛死亡次数
    //2使用title
    //3出师徒弟数量
    void SetData8(uint8 pos,uint8 data);
    uint8 GetData8(uint8 pos);
    
    void SetShengWang(int sw);
    
    //0 解散师徒关系时间
    //1 状元、探花、榜眼title时间
    //2 离开帮派时间
    //3 副本id
    //4 偷菜时间
    //5 帮贡
    void SetData32(uint8 pos,uint32 data);
    uint32 GetData32(uint8 pos);
    
    //0 声望（善恶）
    //1 比赛积分
    void SetData16(uint8 pos,uint16 data);
    uint16 GetData16(uint8 pos);
    
    void SetTitle(uint16 title);
    
    int GetHeChengVal(uint8 pos1,uint8 pos2);
    bool HeChengLanShuiJing(uint8 pos1,uint8 pos2,uint8 hechengfu);
    
    //合成宠物铠甲，kaijia为铠甲，pos1－3为兽骨，飞羽，木料
    int HeChengKaiJia(int target ,uint8 kaijia);
    
    //得到宠物位置
    int GetPetById(int id);
    
    SItemInstance *GetItemById(int id);
    
    void AddPetQinMi(uint8 pos,int qinmi);
    
    //type = 3表示可骑乘
    void SetPetType(uint8 pos,uint8 type);
    
    void XiShouPet(uint8 ind);
    bool AddQiLing();
    
    void AddTextTitle(const char *pTitle);
    void UseTextTitle(const char *pTitle);
    
    uint8 GetExtData8(uint16 pos);
    void SetExtData8(uint16 pos,uint8 val);
    
    uint16 GetExtData16(uint16 pos);
    void SetExtData16(uint16 pos,uint16 val);
    
    uint32 GetExtData32(uint16 pos);
    void SetExtData32(uint16 pos,uint32 val);
};
