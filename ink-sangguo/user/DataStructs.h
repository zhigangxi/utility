#pragma once
#include <stdint.h>
#include <list>

struct AwardData
{
	uint32_t Level = 0;
	float Rate = 0;
	uint32_t Times = 0;
	uint32_t Round = 0;
	int Random(int min, int max) const;
};

struct AdvantureData
{
	uint32_t Level;
};

struct AdvantureEventData
{
	int Random(int min, int max) const;
};

struct QuestData
{
	QuestData() :Strength(0), Wisdom(0), Leadership(0)
	{

	}
	float Strength;
	float Wisdom;
	float Leadership;
	float Charm;
};

struct PopulationData
{

};
struct LearnPropertyData
{
	int Property = 0;
	int Luck = 0;
};

struct LearnSkillData
{
	int Wisdom = 0;
	int Luck = 0;
};

struct ResourceNumberData
{
	int Level = 0;
	int Soldier = 0;
};
class CEnemy;

class EnemyFilterData
{
public:
	EnemyFilterData(CEnemy *enemy) :m_enemy(enemy)
	{

	}
	bool id(int) const;
	bool soldier(int) const;
	bool force(int) const;
private:
	CEnemy *m_enemy;
};