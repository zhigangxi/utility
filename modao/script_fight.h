
class CFight
{
public:
    int GetMonsterNum(int id); 
    int monsterId1;
    int monsterId2;
    
    /*
    5 每一个任务的潜能提供：
	10~29级  怪物提供潜能=（10+轮次）*0.7*（头目等级*100+3000）/155+喽罗个数*（10+轮次）*0.06*（喽罗等级*100+3000）/155
	30~59级  怪物提供潜能=（10+轮次）*0.65*（头目等级*150+5000）/155+喽罗个数*（10+轮次）*0.07*（喽罗等级*150+5000）/155
	60~90级  怪物提供潜能=（10+轮次）*0.6*（头目等级*200+7000）/155+喽罗个数*（10+轮次）*0.08*（喽罗等级*200+7000）/155
	type 0 10~29级
	type 1 30~59级
	type 2 60~90级
	*/
    int GetDaoQian5(int turn,int type);
    
    /*
    6 每一个任务的道行提供：
	 10~29级  怪物提供道行=（10+轮次）*0.7*（头目等级*10+300）/155+喽罗个数*（10+轮次）*0.06*（喽罗等级*10+300）/155
	30~59级  怪物提供道行=（10+轮次）*0.65*（头目等级*15+500）/155+喽罗个数*（10+轮次）*0.07*（喽罗等级*15+500）/155
	60~90级  怪物提供道行=（10+轮次）*0.6*（头目等级*20+700）/155+喽罗个数*（10+轮次）*0.08*（喽罗等级*20+700）/155
	type 0 10~29级
	type 1 30~59级
	type 2 60~90级
	*/
    int GetDaoQian6(int turn,int type);
    
    int GetUseTime();
};
