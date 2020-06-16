#include "EditerScene.h"
#include "CCIMGUI.h"
#include <iostream>
#include <fstream>
#include <vector>
#include "nlohmann/json.hpp"
USING_NS_CC;
using namespace std;
using json = nlohmann::json;

cocos2d::Scene* EditerScene::createScene()
{
	return EditerScene::create();
}

bool EditerScene::init()
{
    if (!Scene::init())
    {
        return false;
    }
    if (_planeMgr.ReadPlanes("plane_model.json")) {
        _planeMgr.LoadPlanes("plane_model.data");
    }
    else {
        cout << "read plane_model.json error" << endl;
        return true;
    }
    _planeMgr.GetAllPlane(_allPlanes);
    for (auto i = _allPlanes.begin(); i != _allPlanes.end(); i++)
    {
        _allPlaneNames.push_back((*i)->name.c_str());
    }
    
    _planeNode = DrawNode::create();
    this->addChild(_planeNode);
    auto winSize = Director::getInstance()->getWinSize();
    _planeNode->setPosition(winSize.width / 3, winSize.height/2);
    _planeNode->setGlobalZOrder(2);
    //_planeNode->setPosition(winSize.width-100 , winSize.height-100);

    /*auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();*/

    CCIMGUI::getInstance()->addCallback(std::bind(&EditerScene::initGUI,this), "PlaneEditer");

    readData();

    //环境光
    Color3B col;
    convertToColor(_ambientCol, col);
    _ambientLight = AmbientLight::create(col);
    addChild(_ambientLight);

    //定向光
    convertToColor(_directionCol, col);
    _directionLight = DirectionLight::create(_directionVec, col);
    addChild(_directionLight);

    convertToColor(_spotCol, col);
    _spotLight = PointLight::create(_spotVec, col, _spotRange);
    addChild(_spotLight);
    
    /*auto map = Sprite::create("map.jpg");
    map->setAnchorPoint(Vec2(0,0));
    map->setOpacity(80);
    addChild(map);*/
    
    /*_camera = Camera::createPerspective(60.0f, winSize.width / winSize.height, 1.0f, 10000.0f);
    _camera->setPosition3D(Vec3(winSize.width/2,winSize.height/2,0));
    _camera->setCameraFlag(CameraFlag::USER1);
    addChild(_camera);*/

    setCurSelPlaneCfg();
    //_planeNode->drawPoint(Vec2(0, 0), 10, Color4F::RED);
    drawPlaneBox();

    auto listener1 = EventListenerTouchOneByOne::create();
    listener1->onTouchBegan = [&](Touch* touch, Event* event) {
        return this->onTouchBegin(touch);
    };
    listener1->onTouchMoved = [&](Touch* touch, Event* event) {
        this->onTouchMoved(touch);
    };
    listener1->onTouchEnded = [&](Touch* touch, Event* event) {
        this->onTouchEnded(touch);
    };

    getEventDispatcher()->addEventListenerWithSceneGraphPriority(listener1, this);

    return true;
}

void EditerScene::onExit()
{
    this->savePlanes();
    this->saveData();
    Scene::onExit();
}

void EditerScene::convertToColor(float fCol[3], Color3B& col)
{
    col.r = uint8_t(fCol[0] * 255);
    col.g = uint8_t(fCol[1] * 255);
    col.b = uint8_t(fCol[2] * 255);
}

bool EditerScene::setLightColDir(const char *name,float col[3], cocos2d::BaseLight* light,cocos2d::Vec3 &vec,float min,float max)
{
    ImGui::Separator();
    if (ImGui::ColorEdit3(name, col)) {
        Color3B color;
        convertToColor(col, color);
        light->setColor(color);
    }
    bool ret = false;
    char subName[128];
    snprintf(subName, sizeof(subName), "%s_X", name);
    if (ImGui::SliderFloat(subName, &vec.x, min, max)) {
        ret = true;
    }
    snprintf(subName, sizeof(subName), "%s_Y", name);
    if (ImGui::SliderFloat(subName, &vec.y, min, max)) {
        ret = true;
    }
    snprintf(subName, sizeof(subName), "%s_Z", name);
    if (ImGui::SliderFloat(subName, &vec.z, min, max)) {
        ret = true;
    }
    return ret;
}

void EditerScene::drawMuzzles()
{
    _planeNode->clear();
    for (int i = 0; i < PlaneConf::MUZZLES_NUM; i++)
    {
        Vec2 p((float)_planeCfg->muzzles[i].x, (float)_planeCfg->muzzles[i].y);
        _planeNode->drawPoint(p, 10, Color4F::RED);
    }
}

void EditerScene::drawPlaneBox()
{
    _planeNode->clear();
    Vec2 p1, p2;
    const float width = 96;
    const float height = 128;
    p1.x = -width / 2;
    p1.y = -height / 2;
    p2.x = -p1.x;
    p2.y = -p1.y;
    _planeNode->drawRect(p1,p2 ,Color4F::WHITE);
}

void EditerScene::drawBox()
{
    _planeNode->clear();

    if (_planeCfg->polygon.size() <= 0)
        return;

    vector<Vec2> verts;
    auto i = _planeCfg->polygon.begin();
    auto j = i;
    Vec2 p((float)i->x, (float)i->y);
    _planeNode->drawPoint(p, 3, Color4F::RED);
    j++;
    for (; j != _planeCfg->polygon.end(); i++,j++)
    {
        Vec2 p((float)i->x, (float)i->y);
        Vec2 p1((float)j->x, (float)j->y);
        _planeNode->drawSegment(p,p1, 1, Color4F::GREEN);
        _planeNode->drawPoint(Vec2((float)i->x, (float)i->y), 3, Color4F::RED);
    }

    /*Color4F fCol;
    fCol.a = 0;
    Color4F bColor(0,1,0,1);
    _planeNode->drawPolygon(&(verts[0]), verts.size(), fCol, 1, bColor);*/
}

bool EditerScene::onTouchBegin(cocos2d::Touch* touch)
{
    if (_editSelect == EDIT_MUZZLE) {

    }
    else if (_editSelect == EDIT_BOX) {

    }
    return true;
}

void EditerScene::onTouchMoved(cocos2d::Touch* touch)
{
}

void EditerScene::onTouchEnded(cocos2d::Touch* touch)
{
    if (_editSelect == EDIT_BOX) {
        if (_planeCfg->polygon.size() >= PlaneConf::MAX_POLYGON_SIZE)
            return;
        Vec2 pos = _planeNode->convertToNodeSpace(touch->getLocation());
        if (pos.length() > 256) {
            return;
        }
        Position p;
        p.x = (int)pos.x;
        p.y = (int)pos.y;
        _planeCfg->polygon.push_back(p);
        drawBox();
    }
}

void EditerScene::initGUI()
{
    setGUIPos();
    ImGui::Begin("Plane mod", nullptr, 0);// ImGuiWindowFlags_NoTitleBar);
    
    if (ImGui::ColorEdit3("Ambient", _ambientCol)) {
        Color3B col;
        convertToColor(_ambientCol, col);
        _ambientLight->setColor(col);
    }
    
    if (setLightColDir("Direction", _directionCol, _directionLight, _directionVec,-1,1))
        _directionLight->setDirection(_directionVec);
    if (setLightColDir("Spot", _spotCol, _spotLight, _spotVec,-1000,1000))
        _spotLight->setPosition3D(_spotVec);
    if (ImGui::SliderFloat("Spot range", &_spotRange,0,10000)) {
        _spotLight->setRange(_spotRange);
    }

    if (ImGui::Combo("##combo", &_curSelPlane, &(_allPlaneNames[0]), _allPlaneNames.size())) {
        setCurSelPlaneCfg();
    }
    ImGui::Separator();

    if (ImGui::Combo("Texture", &_curSelTexture, &(_planeTextNames[0]), _planeTextNames.size())) {
        _plane->setTexture("plane/" + _planeCfg->textureNames[_curSelTexture]);
    }

    if (ImGui::SliderInt("Rotate X", &_planeCfg->rotateX, 0, 360)) {
        updatePlaneCfg();
    }
    if (ImGui::SliderInt("Rotate Y", &_planeCfg->rotateY, 0, 360)) {
        updatePlaneCfg();
    }
    if (ImGui::SliderInt("Rotate Z", &_planeCfg->rotateZ, 0, 360)) {
        updatePlaneCfg();
    }
    if (ImGui::SliderInt("Pos X", &_planeCfg->pos.x, -100, 100)) {
        //_plane->setAnchorPoint(Vec2(_anchorX, _anchorY));
        _plane->setPosition3D(Vec3(_planeCfg->pos.x, _planeCfg->pos.y, 0));
    }
    if (ImGui::SliderInt("Pos Y", &_planeCfg->pos.y, -100, 100)) {
        //_plane->setAnchorPoint(Vec2(_anchorX, _anchorY));
        _plane->setPosition3D(Vec3(_planeCfg->pos.x, _planeCfg->pos.y, 0));
    }

    if (ImGui::SliderInt("Scale ( %% )", &_planeCfg->scale, 10, 1000)) {
        updatePlaneCfg();
    }
    
    //ImGui::NewLine();
    ImGui::Separator();
    //ImGui::NewLine();
    //static ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_Reorderable;
    if (ImGui::CheckboxFlags("Edit muzzle", &_editSelect, EDIT_MUZZLE)) {
        if ((_editSelect & EDIT_MUZZLE) != 0) {
            _editSelect = EDIT_MUZZLE;
            drawMuzzles();
        }
    }
    ImGui::SameLine();
    if (ImGui::CheckboxFlags("Edit box", &_editSelect, EDIT_BOX)) {
        if ((_editSelect & EDIT_BOX) != 0) {
            _editSelect = EDIT_BOX;
        }
    }
    if (_editSelect == EDIT_MUZZLE) {
        bool drawMuzzle = false;
        char muzzleName[64];
        for (int i = 0; i < PlaneConf::MUZZLES_NUM; i++)
        {
            snprintf(muzzleName, sizeof(muzzleName), "Muzzle x%d", i + 1);
            if (ImGui::SliderInt(muzzleName, &(_planeCfg->muzzles[i].x), -100, 100)) {
                drawMuzzle = true;
            }
            snprintf(muzzleName, sizeof(muzzleName), "Muzzle y%d", i + 1);
            if (ImGui::SliderInt(muzzleName, &(_planeCfg->muzzles[i].y), -100, 100)) {
                drawMuzzle = true;
            }
        }
        if (drawMuzzle) {
            drawMuzzles();
        }
    }else if (_editSelect == EDIT_BOX) {
        if (ImGui::Button("Del last")) {
            if(_planeCfg->polygon.size() > 0)
                _planeCfg->polygon.pop_back();
        }
        ImGui::SameLine();
        if (ImGui::Button("Clear box")) {
            _planeCfg->polygon.clear();
            _planeNode->clear();
        }
        else {
            drawBox();
        }
    }
    else {
        //_planeNode->drawPoint(Vec2(0, 0), 10, Color4F::RED);
        drawPlaneBox();
    }

    ImGui::End();
}

void EditerScene::setGUIPos()
{
    if (_initPos)
        return;
    _initPos = true;
    
    /*ImFont* font_current = ImGui::GetFont();
    font_current->Scale = 1.5f;*/

    auto winSize = Director::getInstance()->getWinSize();
    ImVec2 toolWinSize;
    toolWinSize.x = 300;
    toolWinSize.y = winSize.height;
    ImGui::SetNextWindowSize(toolWinSize, ImGuiCond_FirstUseEver);
    ImVec2 toolWinPos;
    toolWinPos.x = winSize.width - toolWinSize.x;
    toolWinPos.y = 0;
    ImGui::SetNextWindowPos(toolWinPos, ImGuiCond_FirstUseEver);
}

void EditerScene::savePlanes()
{
    _planeMgr.SavePlanes("plane_model.data");
}

void EditerScene::setCurSelPlaneCfg()
{
    if (_curSelPlane < 0 || _curSelPlane >= _allPlanes.size()) {
        return;
    }
    _planeCfg = _allPlanes[_curSelPlane];
    _planeTextNames.clear();
    _curSelTexture = 0;
    for (auto i = _planeCfg->textureNames.begin(); i != _planeCfg->textureNames.end(); i++)
    {
        _planeTextNames.push_back(i->c_str());
    }
    if (_plane != nullptr) {
        //if not run action, will crash
        auto action = Sequence::create(
            CallFunc::create([=]() {
                _plane->removeFromParent();
                _plane = Sprite3D::create("plane/" + _planeCfg->mod);
                _plane->setTexture("plane/" + _planeCfg->textureNames[_curSelTexture]);
                _planeNode->addChild(_plane);
                _plane->setPosition3D(Vec3(_planeCfg->pos.x, _planeCfg->pos.y, 0));
                _plane->setGlobalZOrder(1);
                updatePlaneCfg();
                }),
            nullptr);
        _plane->runAction(action);
    }
    else {
        _plane = Sprite3D::create("plane/" + _planeCfg->mod);
        _plane->setTexture("plane/" + _planeCfg->textureNames[_curSelTexture]);
        _planeNode->addChild(_plane);
        _plane->setPosition3D(Vec3(_planeCfg->pos.x, _planeCfg->pos.y, 0));
        _plane->setGlobalZOrder(1);
        updatePlaneCfg();
    }
}

void EditerScene::updatePlaneCfg()
{
    _plane->setScale(_planeCfg->scale / 1000.0f);
    _plane->setRotation3D(Vec3(_planeCfg->rotateX, _planeCfg->rotateY, _planeCfg->rotateZ));
}

void EditerScene::saveData()
{
    json j;
    j["ambient_col"] = _ambientCol;
    j["direction_col"] = _directionCol;
    j["spot_col"] = _spotCol;
    j["direction_x"] = _directionVec.x;
    j["direction_y"] = _directionVec.y;
    j["direction_z"] = _directionVec.z;
    j["spot_x"] = _spotVec.x;
    j["spot_y"] = _spotVec.y;
    j["spot_z"] = _spotVec.z;
    j["spot_range"] = _spotRange;

    auto msgpack = json::to_msgpack(j);
    std::ofstream msgPackJs(SCENE_DATA_FILE, ios::binary);
    msgPackJs.write((const char*)(&msgpack[0]), msgpack.size());
    msgPackJs.flush();
}

void EditerScene::readData()
{
    std::ifstream jsFile(SCENE_DATA_FILE, ios::binary);
    if (!jsFile.is_open()) {
        return;
    }
    jsFile.seekg(0, jsFile.end);
    auto num = jsFile.tellg();
    jsFile.seekg(0, jsFile.beg);
    vector<uint8_t> buf;
    buf.resize(num);
    jsFile.read((char*)&(buf[0]), num);

    json j = json::from_msgpack(buf);

    for (int i = 0; i < 3; i++)
    {
        _ambientCol[i] = j["ambient_col"][i];
    }
    for (int i = 0; i < 3; i++)
    {
        _directionCol[i] = j["direction_col"][i];
    }
    for (int i = 0; i < 3; i++)
    {
        _spotCol[i] = j["spot_col"][i];
    }
    _directionVec.x = j["direction_x"];
    _directionVec.y = j["direction_y"];
    _directionVec.z = j["direction_z"];
    _spotVec.x = j["spot_x"];
    _spotVec.y = j["spot_y"];
    _spotVec.z = j["spot_z"];
    _spotRange = j["spot_range"];
}