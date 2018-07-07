#include "MahjongAI.h"
#include "split.h"
#include <algorithm>

void CopyCards(const std::vector<MahjongAI::MJType> &vecCards, char *cards)
{
	for (size_t i = 0; i < vecCards.size(); i++)
	{
		int pos = vecCards[i] - MahjongAI::YiWan;
		if (pos >= 0 && pos < split::CardTypeNum)
		{
			cards[pos]++;
		}
	}
}

bool HaveCard(char *cards, int pos)
{
    if(pos >= 0 && pos < split::CardTypeNum)
    {
        return (cards[pos] > 0);
    }
    return false;
}

bool MahjongAI::CanChi(const std::vector<MJType> &handCards,MJType curCard)
{
    char cards[split::CardTypeNum] = { 0 };
	CopyCards(handCards, cards);
    int pos = (int)curCard;
    if(pos < 0 || pos >= split::CardTypeNum)
    {
        return false;
    }
    if(HaveCard(cards,pos-1) && HaveCard(cards,pos-2))
    {
        return true;
    }
    if(HaveCard(cards,pos-1) && HaveCard(cards,pos+1))
    {
        return true;
    }
    if(HaveCard(cards,pos+1) && HaveCard(cards,pos+2))
    {
        return true;
    }
    return false;
}

bool MahjongAI::NeedPeng(const std::vector<MJType>& handCards, MJType curCard)
{
	char cards[split::CardTypeNum] = { 0 };
	CopyCards(handCards, cards);
	
	int pos = (int)curCard;
	if (pos < 0 || pos >= split::CardTypeNum)
	{
		return false;
	}
	if (cards[pos] < 2)
		return false;
	int beforePendNeedCard = split::GetHuPaiNeedCardNum(cards);
	cards[pos] -= 2;
	int afterPengNeedCard = split::GetHuPaiNeedCardNum(cards);
	return  afterPengNeedCard <= beforePendNeedCard;
}

bool MahjongAI::CanPeng(const std::vector<MJType> &handCards,MJType curCard)
{
    char cards[split::CardTypeNum] = { 0 };
	CopyCards(handCards, cards);
    int pos = (int)curCard;
    if(pos < 0 || pos >= split::CardTypeNum)
    {
        return false;
    }
    return (cards[pos] >= 2);
}

bool MahjongAI::NeedGang(const std::vector<MJType>& handCards, MJType curCard)
{
	char cards[split::CardTypeNum] = { 0 };
	CopyCards(handCards, cards);

	int pos = (int)curCard;
	if (pos < 0 || pos >= split::CardTypeNum)
	{
		return false;
	}

	if (cards[pos] < 3)
		return false;
	int beforeGangdNeedCard = split::GetHuPaiNeedCardNum(cards);
	cards[pos] -= 3;
	int afterGangNeedCard = split::GetHuPaiNeedCardNum(cards);

	return afterGangNeedCard <= beforeGangdNeedCard;
}

bool MahjongAI::CanGang(const std::vector<MJType> &handCards,MJType curCard)
{
    char cards[split::CardTypeNum] = { 0 };
	CopyCards(handCards, cards);
    int pos = (int)curCard;
    if(pos < 0 || pos >= split::CardTypeNum)
    {
        return false;
    }
    return (cards[pos] >= 3);
}

bool MahjongAI::CanHu(const std::vector<MJType>& handCards, MJType curCatd)
{
	char cards[split::CardTypeNum] = { 0 };
	CopyCards(handCards, cards);
	int pos = curCatd - YiWan;
	if (pos >= 0 && pos < split::CardTypeNum)
		cards[pos]++;
	return split::get_hu_info(cards);
}

MahjongAI::MJType MahjongAI::GetChuPai(const std::vector<MJType>& handCards, const std::vector<MJType> &canSeeCards)
{	
	struct HuPai {
		MJType type;
		int curStepNeed;
		int nextStepNeed;
	};
	int cardNum = handCards.size();
	std::vector<MJType> tempCards = handCards;
	char cards[split::CardTypeNum] = { 0 };

	char allLeftCards[split::CardTypeNum];
	int pos;
	for (int i = 0; i < split::CardTypeNum; i++)
	{
		allLeftCards[i] = 4;
	}
	for (auto i = canSeeCards.begin(); i != canSeeCards.end(); i++)
	{
		pos = *i;
		if (pos >= 0 && pos < split::CardTypeNum)
		{
			allLeftCards[pos] -= 1;
		}
	}
	std::vector<HuPai> huPaiNums;
	for (int i = 0; i < cardNum; i++)
	{
		memset(cards, 0, sizeof(cards));
		tempCards = handCards;
		tempCards.erase(tempCards.begin() + i);
		CopyCards(tempCards, cards);
		int num = split::GetHuPaiNeedCardNum(cards);
		int needCardNum = 0;
		for (int j = 0; j < split::CardTypeNum; j++)
		{
			if (allLeftCards[j] <= 0)
				continue;
			cards[j]++;
			if (split::GetHuPaiNeedCardNum(cards) < num)
				needCardNum++;
			cards[j]--;
		}
		HuPai info;
		info.type = handCards[i];
		info.nextStepNeed = needCardNum;
		info.curStepNeed = num;
		huPaiNums.push_back(info);
	}
	std::sort(huPaiNums.begin(), huPaiNums.begin() + huPaiNums.size(), [](const HuPai info1, const HuPai info2) {
		int val1 = (split::CardTypeNum - info1.curStepNeed) << 8 | info1.nextStepNeed;
		int val2 = (split::CardTypeNum - info2.curStepNeed) << 8 | info2.nextStepNeed;
		return val1 > val2;
	});
	return huPaiNums[0].type;
}

bool MahjongAI::NeedChi(const std::vector<MJType>& handCards, MJType curCard)
{
	if (!CanChi(handCards, curCard))
		return false;

	int pos = (int)curCard;
	if (pos < 0 || pos >= split::CardTypeNum)
	{
		return false;
	}
	char cards[split::CardTypeNum] = { 0 };
	CopyCards(handCards, cards);
	int beforeChiNeedCard = split::GetHuPaiNeedCardNum(cards);
	cards[pos]++;
	int afterChiNeedCard = split::GetHuPaiNeedCardNum(cards);
	return  afterChiNeedCard < beforeChiNeedCard;
}

MahjongAI::MahjongAI()
{
}


MahjongAI::~MahjongAI()
{
}
