#include "MahjongAI.h"
#include <algorithm>
#include <iostream>
using namespace std;

int main()
{
	//1��	2��	2��	3��	3��	3��	4��	4��	5��	����	����	�װ�	�װ�
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
	
	//1��	1��	1��	2��	3��	4��	5��	6��	7��	8��	9��	9��	9��	����
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

	//1��	2��	2��	3��	3��	4��	4��	5��	5��	6��	6��	7��	7��	9��
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