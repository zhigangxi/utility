#pragma once
#include<vector>

class MahjongAI
{
public:
	enum MJType
	{
		YiWan,//一万
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
	//判断是否胡牌
	//handCards手里的牌
	//curCard当前出的牌，或者摸的牌
	static bool CanHu(const std::vector<MJType> &handCards, MJType curCard);

	//获取要出的牌
	//handCards手里的牌
	//canSeeCards包括：玩家碰/杠/吃的牌，和打出的牌，还有自己手里的牌(如果作弊的话，可以包括其他玩家手里的牌)
	static MJType GetChuPai(const std::vector<MJType> &handCards,const std::vector<MJType> &canSeeCards);
	
	//是否需要吃
	static bool NeedChi(const std::vector<MJType> &handCards, MJType curCard);
	//是否可以吃
    static bool CanChi(const std::vector<MJType> &handCards,MJType curCard);

	static bool NeedPeng(const std::vector<MJType> &handCards, MJType curCard);

    static bool CanPeng(const std::vector<MJType> &handCards,MJType curCard);

	static bool NeedGang(const std::vector<MJType> &handCards, MJType curCard);

    static bool CanGang(const std::vector<MJType> &handCards,MJType curCard);
private:
	MahjongAI();
	~MahjongAI();
};

