#pragma once

#include "cocos2d.h"
#include "PlaneConf.h"
#include <vector>
#include <string>

class EditerScene: public cocos2d::Scene
{
public:
    static cocos2d::Scene* createScene();

    virtual bool init();

    virtual void onExit();

    // implement the "static create()" method manually
    CREATE_FUNC(EditerScene);
private:
    void initGUI();
    void setGUIPos();
    void savePlanes();

    void setCurSelPlaneCfg();
    void updatePlaneCfg();

    bool setLightColDir(const char *name,float col[3],cocos2d::BaseLight *light,cocos2d::Vec3 &vec,float min,float max);

    void drawMuzzles();
    void drawBox();

    void drawPlaneBox();

    void convertToColor(float fCol[3], cocos2d::Color3B& col);

    bool onTouchBegin(cocos2d::Touch* touch);
    void onTouchMoved(cocos2d::Touch* touch);
    void onTouchEnded(cocos2d::Touch* touch);

    void saveData();
    void readData();

    cocos2d::DrawNode* _planeNode;
    cocos2d::Sprite3D* _plane = nullptr;

    cocos2d::AmbientLight *_ambientLight;//环境光
    cocos2d::DirectionLight *_directionLight;//定向光
    cocos2d::PointLight* _spotLight;//点光源

    float _ambientCol[3];
    float _directionCol[3];
    float _spotCol[3];

    cocos2d::Vec3 _directionVec;
    cocos2d::Vec3 _spotVec;
    float _spotRange;
    //cocos2d::Camera* _camera;

    PlaneConfMgr _planeMgr;
    bool _initPos = false;
    PlaneConf *_planeCfg;
    std::vector<const char*> _allPlaneNames;
    std::vector<const char*> _planeTextNames;
    std::vector<PlaneConf*> _allPlanes;

    int _curSelPlane = 0;
    uint32_t _editSelect = 0;
    int _curSelTexture = 0;

    static const uint32_t EDIT_MUZZLE = 1;
    static const uint32_t EDIT_BOX = 2;
    const char* SCENE_DATA_FILE = "scene_data.db";
};
