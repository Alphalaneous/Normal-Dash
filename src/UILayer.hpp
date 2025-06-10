#pragma once

#include "Geode/cocos/layers_scenes_transitions_nodes/CCLayer.h"
#include <Geode/Geode.hpp>
#include <Geode/modify/UILayer.hpp>
#include <Geode/modify/CCEGLView.hpp>

using namespace geode::prelude;

class $modify(MyUILayer, UILayer) {

	struct Fields {
		bool m_shiftHeld = false;
        CCLayerColor* m_chatboxBG;
        TextInput* m_input;
        std::unordered_map<std::string, std::function<std::pair<std::string, bool>(std::vector<std::string>)>> m_commands;
        std::string m_username;
        std::vector<std::pair<std::string, bool>> m_chatHistory;
        std::vector<std::string> m_inputHistory;
        CCNode* m_chatWindow;
        CCNode* m_chatWindowHistory;
        int m_historyCursor = -1;
        bool m_disableInput = false;
	};

    enum GameMode {
        UNKNOWN,
		CUBE,
		SHIP,
		BALL,
		UFO,
		WAVE,
		ROBOT,
		SPIDER,
		SWING
	};


    bool init(GJBaseGameLayer* p0);
    void setupCommands();
	GameMode getGameMode();
    std::string gameModeToString(GameMode mode);
    GameMode stringToGamemode(std::string gamemode);
    void setGameMode(GameMode mode);
    void handleKeypress(cocos2d::enumKeyCodes p0, bool p1);
    void keyDown(enumKeyCodes p0);
    void keyUp(cocos2d::enumKeyCodes p0);
	void handleJumpPress(enumKeyCodes key, bool down);
    void onPause(cocos2d::CCObject* sender);
    void sendMessage();
    void toggleInput(bool toggle);
    void executeCommand(const std::string& str);
    void postMessage(const std::string& str, bool err = false);
    void showChat(bool show, bool startWithSlash = false);
    void fixBlinkLabelPos();
    void updateBlink(float dt);
    CCLayerColor* createChatCell(const std::string& str, bool error, bool dontFade = false);
};

