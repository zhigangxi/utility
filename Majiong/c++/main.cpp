#include "MahjongAI.h"
#include <algorithm>
#include <iostream>
using namespace std;

int main()
{
	//1万	2万	2万	3万	3万	3万	4万	4万	5万	东风	东风	白板	白板
	std::vector<MahjongAI::MJType> cards =
	{ 
		MahjongAI::YiWan,
		MahjongAI::LiangWan,
		MahjongAI::LiangWan,
		MahjongAI::SanWan,
		MahjongAI::SanWan,
		MahjongAI::SanWan,
		MahjongAI::SiWan,
		MahjongAI::SiWan,
		MahjongAI::WuWan,
		MahjongAI::DongFeng,
		MahjongAI::DongFeng,
		MahjongAI::BaiBan,
		MahjongAI::BaiBan,
	};
	bool need = MahjongAI::NeedPeng(cards, MahjongAI::LiangWan);
	cout << need << endl;
	
	//1万	1万	1万	2万	3万	4万	5万	6万	7万	8万	9万	9万	9万	东风
	cards =
	{
		MahjongAI::YiWan,
		MahjongAI::YiWan,
		MahjongAI::YiWan,
		MahjongAI::LiangWan,
		MahjongAI::SanWan,
		MahjongAI::SiWan,
		MahjongAI::WuWan,
		MahjongAI::LiuWan,
		MahjongAI::QiWan,
		MahjongAI::BaWan,
		MahjongAI::JiuWan,
		MahjongAI::JiuWan,
		MahjongAI::JiuWan,
		MahjongAI::DongFeng,
	};
	std::vector<MahjongAI::MJType> canSeeCards;
	auto chuPai = MahjongAI::GetChuPai(cards, canSeeCards);
	cout << chuPai << endl;

	//1万	2万	2万	3万	3万	4万	4万	5万	5万	6万	6万	7万	7万	9万
	cards =
	{
		MahjongAI::YiWan,
		MahjongAI::LiangWan,
		MahjongAI::LiangWan,
		MahjongAI::SanWan,
		MahjongAI::SanWan,
		MahjongAI::SiWan,
		MahjongAI::SiWan,
		MahjongAI::WuWan,
		MahjongAI::WuWan,
		MahjongAI::LiuWan,
		MahjongAI::LiuWan,
		MahjongAI::QiWan,
		MahjongAI::QiWan,
		MahjongAI::JiuWan,
	};
	chuPai = MahjongAI::GetChuPai(cards, canSeeCards);
	cout << chuPai << endl;
	return 0;
}