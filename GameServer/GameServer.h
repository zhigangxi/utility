class Player;
class NetPackage;

class GameServer
{
public:
    void AddPlayerToThread(Player*);
    void DispatchNetPackage(NetPackage*);
private:

};

class GameLogic
{
public:
    
};