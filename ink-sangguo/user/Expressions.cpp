#include "Expressions.h"
#include <assert.h>

namespace DesignData {
    // expression help functions
    float Expressions::sAdvantureConfigBaseAwardGold(int idx, const AdvantureData* advanture) {
		return Expressions::sGetInstance().expAdvantureConfigBaseAwardGold(idx)(advanture);
	}
    
    float Expressions::sAdvantureConfigBaseAwardExp(int idx, const AdvantureData* advanture) {
		return Expressions::sGetInstance().expAdvantureConfigBaseAwardExp(idx)(advanture);
	}
    
    float Expressions::sAdvantureEventDelay(int idx, const AdvantureEventData* advantureevent) {
		return Expressions::sGetInstance().expAdvantureEventDelay(idx)(advantureevent);
	}
    
    float Expressions::sAwardAwardExp(int idx, const AwardData* award) {
		return Expressions::sGetInstance().expAwardAwardExp(idx)(award);
	}
    
    float Expressions::sAwardAwardExpHero(int idx, const AwardData* award) {
		return Expressions::sGetInstance().expAwardAwardExpHero(idx)(award);
	}
    
    float Expressions::sAwardAwardCoin(int idx, const AwardData* award) {
		return Expressions::sGetInstance().expAwardAwardCoin(idx)(award);
	}
    
    float Expressions::sAwardAwardGold(int idx, const AwardData* award) {
		return Expressions::sGetInstance().expAwardAwardGold(idx)(award);
	}
    
    float Expressions::sAwardAwardContribution(int idx, const AwardData* award) {
		return Expressions::sGetInstance().expAwardAwardContribution(idx)(award);
	}
    
    float Expressions::sAwardAwardReputation(int idx, const AwardData* award) {
		return Expressions::sGetInstance().expAwardAwardReputation(idx)(award);
	}
    
    bool Expressions::sEnemyTeamMember1(int idx, const EnemyFilterData* enemyfilter) {
		return Expressions::sGetInstance().condEnemyTeamMember1(idx)(enemyfilter);
	}
    
    bool Expressions::sEnemyTeamMember2(int idx, const EnemyFilterData* enemyfilter) {
		return Expressions::sGetInstance().condEnemyTeamMember2(idx)(enemyfilter);
	}
    
    bool Expressions::sEnemyTeamMember3(int idx, const EnemyFilterData* enemyfilter) {
		return Expressions::sGetInstance().condEnemyTeamMember3(idx)(enemyfilter);
	}
    
    bool Expressions::sEnemyTeamMember4(int idx, const EnemyFilterData* enemyfilter) {
		return Expressions::sGetInstance().condEnemyTeamMember4(idx)(enemyfilter);
	}
    
    bool Expressions::sEnemyTeamMember5(int idx, const EnemyFilterData* enemyfilter) {
		return Expressions::sGetInstance().condEnemyTeamMember5(idx)(enemyfilter);
	}
    
    float Expressions::sHeroLearnPropertyLearnPropertyExp(int idx, const LearnPropertyData* learnproperty) {
		return Expressions::sGetInstance().expHeroLearnPropertyLearnPropertyExp(idx)(learnproperty);
	}
    
    float Expressions::sHeroLearnPropertyLearnPropertyCritAdd(int idx, const LearnPropertyData* learnproperty) {
		return Expressions::sGetInstance().expHeroLearnPropertyLearnPropertyCritAdd(idx)(learnproperty);
	}
    
    float Expressions::sHeroLearnSkillLearnSkillExp(int idx, const LearnSkillData* learnskill) {
		return Expressions::sGetInstance().expHeroLearnSkillLearnSkillExp(idx)(learnskill);
	}
    
    float Expressions::sHeroLearnSkillLearnSkillCritAdd(int idx, const LearnSkillData* learnskill) {
		return Expressions::sGetInstance().expHeroLearnSkillLearnSkillCritAdd(idx)(learnskill);
	}
    
    float Expressions::sQuestQuestEffect(int idx, const QuestData* quest) {
		return Expressions::sGetInstance().expQuestQuestEffect(idx)(quest);
	}
    
    float Expressions::sResourcePlayerNumber(int idx, const ResourceNumberData* resourcenumber) {
		return Expressions::sGetInstance().expResourcePlayerNumber(idx)(resourcenumber);
	}
    
    float Expressions::sResourcePlayerGenerateProduct(int idx, const ResourceNumberData* resourcenumber) {
		return Expressions::sGetInstance().expResourcePlayerGenerateProduct(idx)(resourcenumber);
	}
    
    float Expressions::sResourcePlayerGenerateMax(int idx, const ResourceNumberData* resourcenumber) {
		return Expressions::sGetInstance().expResourcePlayerGenerateMax(idx)(resourcenumber);
	}
    
    float Expressions::sWorldCityPopulationSpeedPopulationSpeed(int idx, const PopulationData* population) {
		return Expressions::sGetInstance().expWorldCityPopulationSpeedPopulationSpeed(idx)(population);
	}
    
	Expressions* Expressions::sInstance = nullptr;

	Expressions& Expressions::sGetInstance() {
		if (sInstance == nullptr) {
			sInstance = new Expressions();
		}

		return *sInstance;
	}

    // expression default functions
    float Expressions::sExpDefaultAdvantureConfigBaseAwardGold(const AdvantureData*) { return 0.0f; }
    float Expressions::sExpDefaultAdvantureConfigBaseAwardExp(const AdvantureData*) { return 0.0f; }
    float Expressions::sExpDefaultAdvantureEventDelay(const AdvantureEventData*) { return 0.0f; }
    float Expressions::sExpDefaultAwardAwardExp(const AwardData*) { return 0.0f; }
    float Expressions::sExpDefaultAwardAwardExpHero(const AwardData*) { return 0.0f; }
    float Expressions::sExpDefaultAwardAwardCoin(const AwardData*) { return 0.0f; }
    float Expressions::sExpDefaultAwardAwardGold(const AwardData*) { return 0.0f; }
    float Expressions::sExpDefaultAwardAwardContribution(const AwardData*) { return 0.0f; }
    float Expressions::sExpDefaultAwardAwardReputation(const AwardData*) { return 0.0f; }
    bool Expressions::sCondDefaultEnemyTeamMember1(const EnemyFilterData*) { return false; }
    bool Expressions::sCondDefaultEnemyTeamMember2(const EnemyFilterData*) { return false; }
    bool Expressions::sCondDefaultEnemyTeamMember3(const EnemyFilterData*) { return false; }
    bool Expressions::sCondDefaultEnemyTeamMember4(const EnemyFilterData*) { return false; }
    bool Expressions::sCondDefaultEnemyTeamMember5(const EnemyFilterData*) { return false; }
    float Expressions::sExpDefaultHeroLearnPropertyLearnPropertyExp(const LearnPropertyData*) { return 0.0f; }
    float Expressions::sExpDefaultHeroLearnPropertyLearnPropertyCritAdd(const LearnPropertyData*) { return 0.0f; }
    float Expressions::sExpDefaultHeroLearnSkillLearnSkillExp(const LearnSkillData*) { return 0.0f; }
    float Expressions::sExpDefaultHeroLearnSkillLearnSkillCritAdd(const LearnSkillData*) { return 0.0f; }
    float Expressions::sExpDefaultQuestQuestEffect(const QuestData*) { return 0.0f; }
    float Expressions::sExpDefaultResourcePlayerNumber(const ResourceNumberData*) { return 0.0f; }
    float Expressions::sExpDefaultResourcePlayerGenerateProduct(const ResourceNumberData*) { return 0.0f; }
    float Expressions::sExpDefaultResourcePlayerGenerateMax(const ResourceNumberData*) { return 0.0f; }
    float Expressions::sExpDefaultWorldCityPopulationSpeedPopulationSpeed(const PopulationData*) { return 0.0f; }

	Expressions::Expressions() :
        mAdvantureConfigBaseAwardGoldExpressions(nullptr),
        mAdvantureConfigBaseAwardExpExpressions(nullptr),
        mAdvantureEventDelayExpressions(nullptr),
        mAwardAwardExpExpressions(nullptr),
        mAwardAwardExpHeroExpressions(nullptr),
        mAwardAwardCoinExpressions(nullptr),
        mAwardAwardGoldExpressions(nullptr),
        mAwardAwardContributionExpressions(nullptr),
        mAwardAwardReputationExpressions(nullptr),
        mEnemyTeamMember1Conditions(nullptr),
        mEnemyTeamMember2Conditions(nullptr),
        mEnemyTeamMember3Conditions(nullptr),
        mEnemyTeamMember4Conditions(nullptr),
        mEnemyTeamMember5Conditions(nullptr),
        mHeroLearnPropertyLearnPropertyExpExpressions(nullptr),
        mHeroLearnPropertyLearnPropertyCritAddExpressions(nullptr),
        mHeroLearnSkillLearnSkillExpExpressions(nullptr),
        mHeroLearnSkillLearnSkillCritAddExpressions(nullptr),
        mQuestQuestEffectExpressions(nullptr),
        mResourcePlayerNumberExpressions(nullptr),
        mResourcePlayerGenerateProductExpressions(nullptr),
        mResourcePlayerGenerateMaxExpressions(nullptr),
        mWorldCityPopulationSpeedPopulationSpeedExpressions(nullptr)
	{}

    AdvantureConfigBaseAwardGoldExpression Expressions::expAdvantureConfigBaseAwardGold(int idx) {
		if (idx >= 0 && idx < NUM_ROWS_ADVANTURE_CONFIG_BASEAWARDGOLD) {
			return mAdvantureConfigBaseAwardGoldExpressions[idx];
		} else {
            assert(false);
			return sExpDefaultAdvantureConfigBaseAwardGold;
		}
    }
    
    AdvantureConfigBaseAwardExpExpression Expressions::expAdvantureConfigBaseAwardExp(int idx) {
		if (idx >= 0 && idx < NUM_ROWS_ADVANTURE_CONFIG_BASEAWARDEXP) {
			return mAdvantureConfigBaseAwardExpExpressions[idx];
		} else {
            assert(false);
			return sExpDefaultAdvantureConfigBaseAwardExp;
		}
    }
    
    AdvantureEventDelayExpression Expressions::expAdvantureEventDelay(int idx) {
		if (idx >= 0 && idx < NUM_ROWS_ADVANTURE_EVENT_DELAY) {
			return mAdvantureEventDelayExpressions[idx];
		} else {
            assert(false);
			return sExpDefaultAdvantureEventDelay;
		}
    }
    
    AwardAwardExpExpression Expressions::expAwardAwardExp(int idx) {
		if (idx >= 0 && idx < NUM_ROWS_AWARD_AWARD_EXP) {
			return mAwardAwardExpExpressions[idx];
		} else {
            assert(false);
			return sExpDefaultAwardAwardExp;
		}
    }
    
    AwardAwardExpHeroExpression Expressions::expAwardAwardExpHero(int idx) {
		if (idx >= 0 && idx < NUM_ROWS_AWARD_AWARD_EXPHERO) {
			return mAwardAwardExpHeroExpressions[idx];
		} else {
            assert(false);
			return sExpDefaultAwardAwardExpHero;
		}
    }
    
    AwardAwardCoinExpression Expressions::expAwardAwardCoin(int idx) {
		if (idx >= 0 && idx < NUM_ROWS_AWARD_AWARD_COIN) {
			return mAwardAwardCoinExpressions[idx];
		} else {
            assert(false);
			return sExpDefaultAwardAwardCoin;
		}
    }
    
    AwardAwardGoldExpression Expressions::expAwardAwardGold(int idx) {
		if (idx >= 0 && idx < NUM_ROWS_AWARD_AWARD_GOLD) {
			return mAwardAwardGoldExpressions[idx];
		} else {
            assert(false);
			return sExpDefaultAwardAwardGold;
		}
    }
    
    AwardAwardContributionExpression Expressions::expAwardAwardContribution(int idx) {
		if (idx >= 0 && idx < NUM_ROWS_AWARD_AWARD_CONTRIBUTION) {
			return mAwardAwardContributionExpressions[idx];
		} else {
            assert(false);
			return sExpDefaultAwardAwardContribution;
		}
    }
    
    AwardAwardReputationExpression Expressions::expAwardAwardReputation(int idx) {
		if (idx >= 0 && idx < NUM_ROWS_AWARD_AWARD_REPUTATION) {
			return mAwardAwardReputationExpressions[idx];
		} else {
            assert(false);
			return sExpDefaultAwardAwardReputation;
		}
    }
    
    EnemyTeamMember1Condition Expressions::condEnemyTeamMember1(int idx) {
		if (idx >= 0 && idx < NUM_ROWS_ENEMY_TEAM_MEMBER1) {
			return mEnemyTeamMember1Conditions[idx];
		} else {
            assert(false);
			return sCondDefaultEnemyTeamMember1;
		}
    }
    
    EnemyTeamMember2Condition Expressions::condEnemyTeamMember2(int idx) {
		if (idx >= 0 && idx < NUM_ROWS_ENEMY_TEAM_MEMBER2) {
			return mEnemyTeamMember2Conditions[idx];
		} else {
            assert(false);
			return sCondDefaultEnemyTeamMember2;
		}
    }
    
    EnemyTeamMember3Condition Expressions::condEnemyTeamMember3(int idx) {
		if (idx >= 0 && idx < NUM_ROWS_ENEMY_TEAM_MEMBER3) {
			return mEnemyTeamMember3Conditions[idx];
		} else {
            assert(false);
			return sCondDefaultEnemyTeamMember3;
		}
    }
    
    EnemyTeamMember4Condition Expressions::condEnemyTeamMember4(int idx) {
		if (idx >= 0 && idx < NUM_ROWS_ENEMY_TEAM_MEMBER4) {
			return mEnemyTeamMember4Conditions[idx];
		} else {
            assert(false);
			return sCondDefaultEnemyTeamMember4;
		}
    }
    
    EnemyTeamMember5Condition Expressions::condEnemyTeamMember5(int idx) {
		if (idx >= 0 && idx < NUM_ROWS_ENEMY_TEAM_MEMBER5) {
			return mEnemyTeamMember5Conditions[idx];
		} else {
            assert(false);
			return sCondDefaultEnemyTeamMember5;
		}
    }
    
    HeroLearnPropertyLearnPropertyExpExpression Expressions::expHeroLearnPropertyLearnPropertyExp(int idx) {
		if (idx >= 0 && idx < NUM_ROWS_HERO_LEARNPROPERTY_LEARNPROPERTYEXP) {
			return mHeroLearnPropertyLearnPropertyExpExpressions[idx];
		} else {
            assert(false);
			return sExpDefaultHeroLearnPropertyLearnPropertyExp;
		}
    }
    
    HeroLearnPropertyLearnPropertyCritAddExpression Expressions::expHeroLearnPropertyLearnPropertyCritAdd(int idx) {
		if (idx >= 0 && idx < NUM_ROWS_HERO_LEARNPROPERTY_LEARNPROPERTYCRITADD) {
			return mHeroLearnPropertyLearnPropertyCritAddExpressions[idx];
		} else {
            assert(false);
			return sExpDefaultHeroLearnPropertyLearnPropertyCritAdd;
		}
    }
    
    HeroLearnSkillLearnSkillExpExpression Expressions::expHeroLearnSkillLearnSkillExp(int idx) {
		if (idx >= 0 && idx < NUM_ROWS_HERO_LEARNSKILL_LEARNSKILLEXP) {
			return mHeroLearnSkillLearnSkillExpExpressions[idx];
		} else {
            assert(false);
			return sExpDefaultHeroLearnSkillLearnSkillExp;
		}
    }
    
    HeroLearnSkillLearnSkillCritAddExpression Expressions::expHeroLearnSkillLearnSkillCritAdd(int idx) {
		if (idx >= 0 && idx < NUM_ROWS_HERO_LEARNSKILL_LEARNSKILLCRITADD) {
			return mHeroLearnSkillLearnSkillCritAddExpressions[idx];
		} else {
            assert(false);
			return sExpDefaultHeroLearnSkillLearnSkillCritAdd;
		}
    }
    
    QuestQuestEffectExpression Expressions::expQuestQuestEffect(int idx) {
		if (idx >= 0 && idx < NUM_ROWS_QUEST_QUEST_EFFECT) {
			return mQuestQuestEffectExpressions[idx];
		} else {
            assert(false);
			return sExpDefaultQuestQuestEffect;
		}
    }
    
    ResourcePlayerNumberExpression Expressions::expResourcePlayerNumber(int idx) {
		if (idx >= 0 && idx < NUM_ROWS_RESOURCE_PLAYER_NUMBER) {
			return mResourcePlayerNumberExpressions[idx];
		} else {
            assert(false);
			return sExpDefaultResourcePlayerNumber;
		}
    }
    
    ResourcePlayerGenerateProductExpression Expressions::expResourcePlayerGenerateProduct(int idx) {
		if (idx >= 0 && idx < NUM_ROWS_RESOURCE_PLAYERGENERATE_PRODUCT) {
			return mResourcePlayerGenerateProductExpressions[idx];
		} else {
            assert(false);
			return sExpDefaultResourcePlayerGenerateProduct;
		}
    }
    
    ResourcePlayerGenerateMaxExpression Expressions::expResourcePlayerGenerateMax(int idx) {
		if (idx >= 0 && idx < NUM_ROWS_RESOURCE_PLAYERGENERATE_MAX) {
			return mResourcePlayerGenerateMaxExpressions[idx];
		} else {
            assert(false);
			return sExpDefaultResourcePlayerGenerateMax;
		}
    }
    
    WorldCityPopulationSpeedPopulationSpeedExpression Expressions::expWorldCityPopulationSpeedPopulationSpeed(int idx) {
		if (idx >= 0 && idx < NUM_ROWS_WORLDCITY_POPULATIONSPEED_POPULATIONSPEED) {
			return mWorldCityPopulationSpeedPopulationSpeedExpressions[idx];
		} else {
            assert(false);
			return sExpDefaultWorldCityPopulationSpeedPopulationSpeed;
		}
    }
    }
