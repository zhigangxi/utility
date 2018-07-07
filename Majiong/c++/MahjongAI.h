#pragma once
#include<vector>

class MahjongAI
{
public:
	enum MJType
	{
		YiWan,//һ��
		LiangWan,
		SanWan,
		SiWan,
		WuWan,
		LiuWan,
		QiWan,
		BaWan,
		JiuWan,
		DongFeng = 27,
		NanFeng,
		XiFeng,
		BeiFeng,
		HongZhong,
		FaCai,
		BaiBan
	};
	//�ж��Ƿ����
	//handCards�������
	//curCard��ǰ�����ƣ�����������
	static bool CanHu(const std::vector<MJType> &handCards, MJType curCard);

	//��ȡҪ������
	//handCards�������
	//canSeeCards�����������/��/�Ե��ƣ��ʹ�����ƣ������Լ��������(������׵Ļ������԰�����������������)
	static MJType GetChuPai(const std::vector<MJType> &handCards,const std::vector<MJType> &canSeeCards);
	
	//�Ƿ���Ҫ��
	static bool NeedChi(const std::vector<MJType> &handCards, MJType curCard);
	//�Ƿ���Գ�
    static bool CanChi(const std::vector<MJType> &handCards,MJType curCard);

	static bool NeedPeng(const std::vector<MJType> &handCards, MJType curCard);

    static bool CanPeng(const std::vector<MJType> &handCards,MJType curCard);

	static bool NeedGang(const std::vector<MJType> &handCards, MJType curCard);

    static bool CanGang(const std::vector<MJType> &handCards,MJType curCard);
private:
	MahjongAI();
	~MahjongAI();
};

