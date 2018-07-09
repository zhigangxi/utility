
class CScene
{
public:
    CScene(uint16 id,uint16 mapId,const char *name,char *monsters);
    ~CScene();
    void Exit(CUser*);
    uint16 GetId();
    uint16 GetMapId();
    const char *GetName();
    void AddJumpPoint(uint8 x,uint8 y,uint8 toX,uint8 toY,uint16 sceneId);
    int GetState();
    void SetState(int state);
    void Clear();
    //(301,14,20)
};
