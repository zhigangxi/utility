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
    void AddLevel(bool callScript = true);         //�ȼ�
    void AddExp(int exp,bool callScript = true);            //����
    
    void AddTiZhi(uint16 tizhi);        //����
    void AddLiLiang(uint16 liliang);    //����
    void AddMinJie(uint16 minjie);      //����
    void AddLingLi(uint16 lingli);      //����
    void AddNaiLi(uint16 naili);        //����
    void AddShuXinDian(int shuxindian);//δ�������Ե�
    void AddJin(uint16 jin);            //������
    void AddMu(uint16 mu);              //ľ����
    void AddShui(uint16 shui);          //ˮ����
    void AddHuo(uint16 huo);            //������
    void AddTu(uint16 tu);              //������
    void AddXiangXinDian(uint16 xiangxindian);  //δ�������Ե�
    void AddHp(int hp);              //��Ѫ
    void AddMp(int mp);              //����
    
    int GetDamage();//�����˺�
    
    int GetRecovery();//����
    int GetSpeed();//�ٶ�
    
    void AddDamage(int damage);
    void AddSkillDamage(int skillDamage);
    void AddRecovery(int recovery);
    void AddSpeed(int speed);
    
    bool AddPackage(SItemInstance &item);
    bool AddPackage(int itemId,int num = 1);	//������Ʒ��ָ����Ʒid��Ŀǰ��֧��ָ����Ʒ����            
    bool DelPackage(int pos,int num = 1); //ɾ����Ʒ��ָ����Ʒ�ڰ������λ�ã�0-35��ֱ���������        
    
    uint32 GetMoney();
    int GetTili();
    void SetMoney(uint32 money);	
    void SetTili(int tili);
    void AddMoney(int add);			//���ӽ�Ǯ
    void AddTili(int add);			//��������
    
    int GetItemfangYu();//����
    int GetItemGongji();//������
    int GetItemFaLi();//���ӷ���
    int GetItemQiXue();//������Ѫ
    int GetItemSudu();//�����ٶ�
    
    uint32 GetTeam();
    
    bool AddPet(SharePetPtr &pPet);	//���ӳ�����ݺ�������
    
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
    
    const char *GetMission(int id);				//��ȡ����
    bool AddMission(int id,const char *pMiss); 	//��������
    void UpdateMission(int id,const char *pMiss);//��������
    void DelMission(int id);					//ɾ������
    SPet GetPetByPos(int pos);
    void UpdatePetToBaby(uint8 pos);
    
    //������ʱ������������洢���ݿ�
    void SetVal(int id,int val);
    //��ȡ�������
    int GetVal(int id);
    
    //������������ߺ󱣴����ݿ�
    void SetSaveVal(uint8 index,int val);
    int GetSaveVal(uint8 index);
    
    //����λ���洢���ݿ�
    void SetBitSet(int ind);
    void ClearBitSet(int ind);
    bool HaveBitSet(int ind);
    
    SItemInstance *GetItem(uint8 pos);
    
    //0�ɹ���1��ʧ�ܣ�2����װ��
    //int QiangHuaPackage(uint8 itemPos,uint8 stonePos);
    
    //0�ɹ���1��ʧ�ܣ�2����������stone1,stone2,stone3Ϊ��ˮ��λ��
    int LianHuaPackage(uint8 itemPos,uint8 stone1,uint8 stone2,uint8 stone3);
    
    //0�ɹ���1��ʧ�ܣ�2����������tianHuanShi�컻ʯ(623)Ϊλ��()
    int TianHuanPackage(uint8 itemPos,uint8 tianHuanShi);//,uint8 lianHuaShi);
    
    //0�ɹ���1��ʧ�ܣ�2��������
    int RepairItem(uint8 pos);
    
    int GetRepairFee();
    void RepairTotel();
    
    //����������num����-1
    bool DelPackageById(int id,int num);
    
    int GetMenPai();
    void SetMenPai(int menpai);
    
    int GetDaoHang();
    
    //���ý��볡���Ƿ���ýű�
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
    
    //����λ�ã�-1����ʧ��
    int JianDingItem(uint8 pos);
    
    void SetQianNeng(int qianNeng);
    int GetQianNeng();
    
    bool HaveItem(int id);
    
    //waitTime��λΪ����
    void AddTimer(int id,int waitTime);
    void DelTimer(int id);    
    
    //�ϳ�����ʯ
    bool CompoundStone(uint8 pos,uint8 num);
    
    //�ϳ���Ʒ�����غϳ���Ʒid��0ʱ��ʾ�ϳ�ʧ��,-1������
    int CompoundItem(uint8 pos1,uint8 pos2,uint8 pos3);
    int CompoundItem1(int id);
    
    //������Ʒ
    bool JianDingShiPin(uint8 pos);
    
    //��Ʒ��Ƕ
    bool XiangQian(uint8 pos,uint8 stone);
    
    bool HavePet(int id);
    bool HaveBaobaoPet(int id);
    
    //��ó�ս����,����idΪ0ʱ��ʾû�г�ս����
    SPet GetCZPet();
    //���ó�ս������ѧ
    void SetCZPetWuXue(int wuxue);
    //�ж������Ƿ���
    bool MissionFull();
    
    //����а��ɷ���id������޷���0
    uint32 GetBangPai();
    
    int GetBangState();
    int GetBangRank();
    void DismissBang();// ��ɢ����
    void UndismissBang();// �����ɢ״̬
    int GetItemNum(int id);
    
    SItemInstance *GetEquip(uint8 pos);
    
    //�������Ʒ
    bool AddBangDingPackage(int itemId,int num=1);
    //������ˮ��          ����      ����ֵ
    bool AddBlueCrystal(int attr,int attrVal);
    //��������ʯ
    bool AddLianHuaStone(int level,int num = 1);
    //����ָ���ȼ�ǿ��װ��
    bool AddLevelPackage(int itemId,int level);
    
    bool AddAttrPackage(int itemId,int attr,int attrVal);
    
    int EmptyPackage();
    
    void AddTongBao(int tongbao);
    
    void SaveSellItem(uint8 pos,uint8 num);
    
    //0���ɹ���1 û�г��������2 ����ǰ󶨡�3 ���ʱ��δ��
    int PetJieBang(uint8 petPos);   
    
    int GetLeftDoubleTime();
    void SetDouble(int hour);
    
    //�򿪱�����1��������2���ĸ�
    void OpenPackage(int tab);
    
    int GetShengWang();
    
    int GetJiFen();
    int GetDieTimes();
    
    void SetShiFu();
    
    int GetChuShiNum();
    
    //0 ʦ���ȼ�
    //1 ������������
    //2ʹ��title
    //3��ʦͽ������
    void SetData8(uint8 pos,uint8 data);
    uint8 GetData8(uint8 pos);
    
    void SetShengWang(int sw);
    
    //0 ��ɢʦͽ��ϵʱ��
    //1 ״Ԫ��̽��������titleʱ��
    //2 �뿪����ʱ��
    //3 ����id
    //4 ͵��ʱ��
    //5 �ﹱ
    void SetData32(uint8 pos,uint32 data);
    uint32 GetData32(uint8 pos);
    
    //0 �������ƶ�
    //1 ��������
    void SetData16(uint8 pos,uint16 data);
    uint16 GetData16(uint8 pos);
    
    void SetTitle(uint16 title);
    
    int GetHeChengVal(uint8 pos1,uint8 pos2);
    bool HeChengLanShuiJing(uint8 pos1,uint8 pos2,uint8 hechengfu);
    
    //�ϳɳ������ף�kaijiaΪ���ף�pos1��3Ϊ�޹ǣ�����ľ��
    int HeChengKaiJia(int target ,uint8 kaijia);
    
    //�õ�����λ��
    int GetPetById(int id);
    
    SItemInstance *GetItemById(int id);
    
    void AddPetQinMi(uint8 pos,int qinmi);
    
    //type = 3��ʾ�����
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
