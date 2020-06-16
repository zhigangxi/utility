#include "CCImGuiLayer.h"
#include "imgui/imgui.h"
#include "imgui_impl_cocos2dx.h"
#include "CCIMGUI.h"

using namespace cocos2d;

ImGuiLayer* ImGuiLayer::create()
{
	ImGuiLayer* pRet = new(std::nothrow) ImGuiLayer();
	if (pRet && pRet->init())
	{
		pRet->autorelease();
		return pRet;
	}
	delete pRet;
	return nullptr;
}

bool ImGuiLayer::init()
{
	if (!Layer::init() || !CCIMGUI::getInstance())
		return false;
	// note: when at the first click to focus the window, this will not take effect
    auto listener = EventListenerTouchOneByOne::create();
    listener->setSwallowTouches(true);
    listener->onTouchBegan = [this](Touch* touch, Event*) -> bool {
		if (!_visible)
			return false;
        return ImGui::IsAnyWindowHovered();
    };
    getEventDispatcher()->addEventListenerWithSceneGraphPriority(listener, this);
    return true;
}

void ImGuiLayer::visit(Renderer *renderer, const Mat4 &parentTransform, uint32_t parentFlags)
{
	if (!_visible)
		return;
	Layer::visit(renderer, parentTransform, parentFlags);
	onDraw();
}

void ImGuiLayer::onDraw()
{
    // create frame
    ImGui_ImplCocos2dx_NewFrame();
    // draw all gui
    CCIMGUI::getInstance()->update();
    // render
    ImGui::Render();
    ImGui_ImplCocos2dx_RenderDrawData(ImGui::GetDrawData());
}

void ImGuiLayer::createAndKeepOnTop()
{
    // delay call, once.
    auto director = Director::getInstance();
    director->getScheduler()->schedule([=](float dt)
        {
            std::string layerName = "ImGUILayer";
            auto order = INT_MAX;
            auto layer = ImGuiLayer::create();
            auto runningScene = Director::getInstance()->getRunningScene();
            if (runningScene && !runningScene->getChildByName(layerName))
            {
                runningScene->addChild(layer, INT_MAX, layerName);
            }

            auto e = Director::getInstance()->getEventDispatcher();
            layer->detached = false;
            e->addCustomEventListener(Director::EVENT_BEFORE_SET_NEXT_SCENE, [&, layerName](EventCustom*) {
                layer = dynamic_cast<ImGuiLayer*>(Director::getInstance()->getRunningScene()->getChildByName(layerName));
                if (layer) {
                    layer->retain();
                    layer->removeFromParent();
                    layer->detached = true;
                }
                });
            e->addCustomEventListener(Director::EVENT_AFTER_SET_NEXT_SCENE, [&, layer, layerName](EventCustom*) {
                if (layer && layer->detached) {
                    Director::getInstance()->getRunningScene()->addChild(layer, order, layerName);
                    layer->release();
                    layer->detached = false;
                }
                });
        }, director, 0, 0, 0, false, "checkIMGUI");
}