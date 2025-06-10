#include "UILayer.hpp"
#include <Geode/modify/CCTextInputNode.hpp>
#include <algorithm>
#include <vector>
#include "Geode/cocos/actions/CCAction.h"
#include "Geode/cocos/actions/CCActionInterval.h"
#include "Geode/cocos/label_nodes/CCLabelBMFont.h"
#include "Geode/cocos/layers_scenes_transitions_nodes/CCLayer.h"
#include "Geode/loader/Log.hpp"
#include "Geode/utils/cocos.hpp"
#include "Geode/utils/string.hpp"
#include "PlayerObject.hpp"
#include "GJBaseGameLayer.hpp"

bool MyUILayer::init(GJBaseGameLayer* p0) {
    if (!UILayer::init(p0)) return false;

    auto fields = m_fields.self();

    CCSize winSize = CCDirector::get()->getWinSize();

    fields->m_username = GJAccountManager::sharedState()->m_username;
    if (fields->m_username.empty()) fields->m_username = "Steve";

    fields->m_chatboxBG = CCLayerColor::create({0, 0, 0, 100});
    fields->m_chatboxBG->setContentSize({winSize.width - 6, 15});
    fields->m_chatboxBG->setAnchorPoint({0, 0});
    fields->m_chatboxBG->setPosition({3, 3});
    fields->m_chatboxBG->setVisible(false);

    fields->m_chatWindow = CCNode::create();
    fields->m_chatWindow->setContentSize({425, 200});
    fields->m_chatWindow->setAnchorPoint({0, 0});
    fields->m_chatWindow->setPosition({0, 45});

    RowLayout* layout = RowLayout::create();
    layout->setAxisAlignment(AxisAlignment::Start);
    layout->setAutoScale(false);
    layout->setAxisReverse(true);
    layout->setCrossAxisOverflow(true);
    layout->setGrowCrossAxis(true);
    layout->setGap(0);

    fields->m_chatWindow->setLayout(layout);
    
    addChild(fields->m_chatWindow);

    fields->m_chatWindowHistory = CCNode::create();
    fields->m_chatWindowHistory->setContentSize({425, 200});
    fields->m_chatWindowHistory->setAnchorPoint({0, 0});
    fields->m_chatWindowHistory->setPosition({0, 45});
    fields->m_chatWindowHistory->setVisible(false);

    RowLayout* layout2 = RowLayout::create();
    layout2->setAxisAlignment(AxisAlignment::Start);
    layout2->setAutoScale(false);
    layout2->setAxisReverse(true);
    layout2->setCrossAxisReverse(true);
    layout2->setCrossAxisOverflow(true);
    layout2->setGrowCrossAxis(true);
    layout2->setGap(0);

    fields->m_chatWindowHistory->setLayout(layout2);
    
    addChild(fields->m_chatWindowHistory);

    float scale = 0.85f;

    CCFadeIn* blinkIn = CCFadeIn::create(0);
    CCDelayTime* delay = CCDelayTime::create(0.3);
    CCFadeOut* blinkOut = CCFadeOut::create(0);
    CCDelayTime* delay2 = CCDelayTime::create(0.3);

    CCSequence* sequence = CCSequence::create(blinkIn, delay, blinkOut, delay2, nullptr);
    CCRepeatForever* repeat = CCRepeatForever::create(sequence);

    fields->m_input = TextInput::create((fields->m_chatboxBG->getContentWidth() + 2) / scale, "", "minecraft.fnt"_spr);
    CCPoint pos = fields->m_chatboxBG->getContentSize()/2;
    fields->m_input->setPosition({pos.x, pos.y - 1});
    fields->m_input->setScale(scale);
    fields->m_input->getInputNode()->onClickTrackNode(false);
    fields->m_input->getInputNode()->m_cursor->setFntFile("minecraft.fnt"_spr);
    fields->m_input->getInputNode()->m_cursor->setString("_");
    fields->m_input->getInputNode()->m_cursor->setScale(0.6f);
    fields->m_input->getInputNode()->m_cursor->setColor({255, 255, 255});
    fields->m_input->getInputNode()->m_cursor->runAction(repeat);
    fields->m_input->setTextAlign(TextInputAlign::Left);
    fields->m_input->getBGSprite()->removeFromParent();
    fields->m_input->setEnabled(false);

    fields->m_chatboxBG->addChild(fields->m_input);

    setupCommands();
    addChild(fields->m_chatboxBG);
    schedule(schedule_selector(MyUILayer::updateBlink));

    return true;
}

void MyUILayer::updateBlink(float dt) {
    fixBlinkLabelPos();
}

void MyUILayer::setupCommands() {
    auto fields = m_fields.self();

    fields->m_commands["/kill"] = [fields, self = Ref(this)] (std::vector<std::string> args) -> std::pair<std::string, bool> {
        self->m_gameLayer->destroyPlayer(self->m_gameLayer->m_player1, nullptr);
        return {fmt::format("Killed {}", fields->m_username), true};
    };

    fields->m_commands["/help"] = [fields] (std::vector<std::string> args) -> std::pair<std::string, bool> {
        return 
        {
            "/gamemode (cube | ship | ball | ufo | wave | robot | spider | swing)\n"
            "/help\n"
            "/kill\n"
            "/teleport\n"
            "/tp -> teleport", 
        true};
    };

    fields->m_commands["/teleport"] = [fields, self = Ref(this)] (std::vector<std::string> args) -> std::pair<std::string, bool> {

        if (args.size() < 3) return {"Invalid coordinates", false};

        CCPoint currentPos = self->m_gameLayer->m_player1->m_position;

        float x = 0;
        if (geode::utils::string::startsWith(args[1], "~")) {
            if (args[1] == "~") {
                x = currentPos.x;
            }
            else {
                auto res = geode::utils::numFromString<float>(args[1].substr(1));
                if (!res.isOk()) {
                    return {"Invalid coordinates", false};
                }
                x = currentPos.x + res.unwrap();
            }
        }
        else {
            auto res = geode::utils::numFromString<float>(args[1]);
            if (!res.isOk()) {
                return {"Invalid coordinates", false};
            }
            x = res.unwrap();
        }

        float y = 0;
        if (geode::utils::string::startsWith(args[2], "~")) {
            if (args[2] == "~") {
                y = currentPos.y;
            }
            else {
                auto res = geode::utils::numFromString<float>(args[2].substr(1));
                if (!res.isOk()) {
                    return {"Invalid coordinates", false};
                }
                y = currentPos.y + res.unwrap();
            }
        }
        else {
            auto res = geode::utils::numFromString<float>(args[2]);
            if (!res.isOk()) {
                return {"Invalid coordinates", false};
            }
            y = res.unwrap();
        }

        GameMode mode = self->getGameMode();
        self->setGameMode(GameMode::CUBE);
        self->m_gameLayer->m_player1->m_position = CCPoint{x, y};
        self->m_gameLayer->resetCamera();
        self->setGameMode(mode);

        return {fmt::format("Teleported {} to {}, {}", fields->m_username, x, y), true};
    };

    fields->m_commands["/tp"] = [fields] (std::vector<std::string> args) -> std::pair<std::string, bool> {
        return fields->m_commands["/teleport"](args);
    };

    fields->m_commands["/gamemode"] = [fields, self = Ref(this)] (std::vector<std::string> args) -> std::pair<std::string, bool> {
        
        if (args.size() < 2) return {"", false};

        GameMode mode = self->stringToGamemode(args[1]);

        if (mode == GameMode::UNKNOWN) return {fmt::format("Unknown game mode: {}", args[1]), false};
        
        self->setGameMode(mode);

        return {fmt::format("Set own game mode to {}", self->gameModeToString(mode)), true};
    };
}

MyUILayer::GameMode MyUILayer::getGameMode() {
    PlayerObject* po1 = m_gameLayer->m_player1;

    if (po1->m_isShip) return GameMode::SHIP;
    if (po1->m_isBall) return GameMode::BALL;
    if (po1->m_isBird) return GameMode::UFO;
    if (po1->m_isDart) return GameMode::WAVE;
    if (po1->m_isRobot) return GameMode::ROBOT;
    if (po1->m_isSpider) return GameMode::SPIDER;
    if (po1->m_isSwing) return GameMode::SWING;

    return GameMode::CUBE;
}

std::string MyUILayer::gameModeToString(GameMode mode) {
    switch(mode) {
        case GameMode::CUBE: {
            return "Cube";
        }
        case GameMode::SHIP: {
            return "Ship";
        }
        case GameMode::BALL: {
            return "Ball";
        }
        case GameMode::UFO: {
            return "UFO";
        }
        case GameMode::WAVE: {
            return "Wave";
        }
        case GameMode::ROBOT: {
            return "Robot";
        }
        case GameMode::SPIDER: {
            return "Spider";
        }
        case GameMode::SWING: {
            return "Swing";
        }
        case GameMode::UNKNOWN: {
            return "Unknown";
        }
    }
    return "Unknown";
}

MyUILayer::GameMode MyUILayer::stringToGamemode(std::string gamemode) {
    geode::utils::string::toLowerIP(gamemode);
    if (gamemode == "cube") return GameMode::CUBE;
    if (gamemode == "ship") return GameMode::SHIP;
    if (gamemode == "ball") return GameMode::BALL;
    if (gamemode == "ufo") return GameMode::UFO;
    if (gamemode == "wave") return GameMode::WAVE;
    if (gamemode == "robot") return GameMode::ROBOT;
    if (gamemode == "spider") return GameMode::SPIDER;
    if (gamemode == "swing") return GameMode::SWING;
    return GameMode::UNKNOWN;
}

void MyUILayer::setGameMode(GameMode mode) {
    PlayerObject* po1 = m_gameLayer->m_player1;

    switch(mode) {
        case GameMode::UNKNOWN:
        case GameMode::CUBE: {
            po1->toggleFlyMode(false, false);
            po1->toggleRollMode(false, false);
            po1->toggleBirdMode(false, false);
            po1->toggleDartMode(false, false);
            po1->toggleRobotMode(false, false);
            po1->toggleSpiderMode(false, false);
            po1->toggleSwingMode(false, false);
            break;
        }
        case GameMode::SHIP: {
            po1->toggleFlyMode(true, true);
            break;
        }
        case GameMode::BALL: {
            po1->toggleRollMode(true, true);
            break;
        }
        case GameMode::UFO: {
            po1->toggleBirdMode(true, true);
            break;
        }
        case GameMode::WAVE: {
            po1->toggleDartMode(true, true);
            break;
        }
        case GameMode::ROBOT: {
            po1->toggleRobotMode(true, true);
            break;
        }
        case GameMode::SPIDER: {
            po1->toggleSpiderMode(true, true);
            break;
        }
        case GameMode::SWING: {
            po1->toggleSwingMode(true, true);
            break;
        }
    }
    auto obj = TeleportPortalObject::create("edit_eGameRotBtn_001.png", true); // yoinked fix from Gamemode Swapper
    obj->m_cameraIsFreeMode = true;
    obj->m_instantCamera = true;
    m_gameLayer->playerWillSwitchMode(po1, obj);
}

void MyUILayer::handleKeypress(cocos2d::enumKeyCodes p0, bool p1) {
    UILayer::handleKeypress(p0, p1);
    auto fields = m_fields.self();

    if ((p0 == enumKeyCodes::KEY_T || p0 == KEY_Slash) && !fields->m_chatboxBG->isVisible()) {
        showChat(true, p0 == KEY_Slash);
    }
}

void MyUILayer::keyDown(enumKeyCodes p0) {
    auto fields = m_fields.self();

    bool hasSprint = Mod::get()->getSettingValue<bool>("sprint");
    bool hasPlatSprint = Mod::get()->getSettingValue<bool>("platformer-sprint");

    bool platformerSprint = hasSprint && (m_gameLayer->m_isPlatformer && hasPlatSprint);
    bool normalSprint = hasSprint && !m_gameLayer->m_isPlatformer;

    if (p0 == enumKeyCodes::KEY_Escape && m_fields->m_chatboxBG->isVisible()) {
        showChat(false);
        return;
    }
    if (p0 == enumKeyCodes::KEY_Enter && m_fields->m_chatboxBG->isVisible()) {
        sendMessage();
        return;
    }

    if (normalSprint || platformerSprint) {
        if (p0 == enumKeyCodes::KEY_LeftShift) {
            fields->m_shiftHeld = true;
            static_cast<MyPlayerObject*>(m_gameLayer->m_player1)->sprint(true);
            static_cast<MyPlayerObject*>(m_gameLayer->m_player2)->sprint(true);
        }
        if (fields->m_shiftHeld) {
            int key = 0;
            switch (p0) {
                case enumKeyCodes::KEY_W:
                case enumKeyCodes::KEY_Up:
                case enumKeyCodes::KEY_Space: {
                    key = 1;
                    break;
                }
                case enumKeyCodes::KEY_A: {
                    key = 2;
                    break;
                }
                case enumKeyCodes::KEY_D: {
                    key = 3;
                    break;
                }
                default: {
                    break;
                }
            }
            m_gameLayer->handleButton(true, key, true);
        }
    }
    handleJumpPress(p0, true);
    UILayer::keyDown(p0);
}

void MyUILayer::keyUp(cocos2d::enumKeyCodes p0) {
    auto fields = m_fields.self();
    
    bool hasSprint = Mod::get()->getSettingValue<bool>("sprint");
    bool hasPlatSprint = Mod::get()->getSettingValue<bool>("platformer-sprint");

    bool platformerSprint = hasSprint && (m_gameLayer->m_isPlatformer && hasPlatSprint);
    bool normalSprint = hasSprint && !m_gameLayer->m_isPlatformer;

    if (normalSprint || platformerSprint) {
        if (p0 == enumKeyCodes::KEY_LeftShift) {
            fields->m_shiftHeld = false;
            static_cast<MyPlayerObject*>(m_gameLayer->m_player1)->sprint(false);
            static_cast<MyPlayerObject*>(m_gameLayer->m_player2)->sprint(false);
        }
        if (fields->m_shiftHeld) {
            int key = -1;
            switch (p0) {
                case enumKeyCodes::KEY_W:
                case enumKeyCodes::KEY_Up:
                case enumKeyCodes::KEY_Space: {
                    key = 1;
                    break;
                }
                case enumKeyCodes::KEY_A: {
                    key = 2;
                    break;
                }
                case enumKeyCodes::KEY_D: {
                    key = 3;
                    break;
                }
                default: {
                    break;
                }
            }
            m_gameLayer->handleButton(false, key, true);
        }
    }
    handleJumpPress(p0, false);
    UILayer::keyUp(p0);
}

void MyUILayer::handleJumpPress(enumKeyCodes key, bool down) {
    if (!Mod::get()->getSettingValue<bool>("keyboard-pro")) return;
    MyGJBaseGameLayer* myGJBGL = static_cast<MyGJBaseGameLayer*>(m_gameLayer);
    auto glbglFields = myGJBGL->m_fields.self();
    if (!myGJBGL->m_isPlatformer) {
        glbglFields->m_skipJump = false;

        if (key == glbglFields->m_currentKey) {
            m_gameLayer->handleButton(down, 1, true);
        }
        else if (key != enumKeyCodes::KEY_LeftShift){
            m_gameLayer->handleButton(false, 1, true);
        }
        glbglFields->m_skipJump = true;
    }
}

void MyUILayer::onPause(cocos2d::CCObject* sender) {
    UILayer::onPause(sender);
    MyPlayerObject* player1 = static_cast<MyPlayerObject*>(m_gameLayer->m_player1);
    MyPlayerObject* player2 = static_cast<MyPlayerObject*>(m_gameLayer->m_player2);
    player1->resetModifiers();
    player2->resetModifiers();
    showChat(false);
}

void MyUILayer::sendMessage() {
    auto fields = m_fields.self();
    std::string message = geode::utils::string::trim(fields->m_input->getString());

    if (message.empty()) return;

    if (geode::utils::string::startsWith(message, "/")) {
        executeCommand(message);
    }
    else {
        postMessage(fmt::format("<{}> {}", fields->m_username, message));
    }
    fields->m_inputHistory.push_back(message);
    showChat(false);
}

void MyUILayer::executeCommand(const std::string& str) {
    auto fields = m_fields.self();

    std::vector<std::string> commandParts = geode::utils::string::split(str, " ");

    auto fn = fields->m_commands[commandParts[0]];
    std::pair<std::string, bool> res = {"", false};
    if (fn) res = fn(commandParts);
    
    if (res.first.empty()) {
        postMessage("Unknown or incomplete command.", true);
    }
    else {
        postMessage(res.first, !res.second);
    }
}

void MyUILayer::postMessage(const std::string& str, bool err) {
    auto fields = m_fields.self();

    std::vector<std::string> newlineSplit = geode::utils::string::split(str, "\n");
    for (const std::string& string : newlineSplit) {
        fields->m_chatWindow->addChild(createChatCell(string, err));
    }
    fields->m_chatWindow->updateLayout();

    fields->m_chatHistory.push_back({str, err});
    fields->m_historyCursor = -1;
}

void MyUILayer::toggleInput(bool toggle) {
    auto fields = m_fields.self();
    fields->m_disableInput = !toggle;
    if (!toggle) {
        fields->m_chatboxBG->setVisible(false);
        fields->m_input->getInputNode()->onClickTrackNode(false);
    }
}

void MyUILayer::fixBlinkLabelPos() {
    auto fields = m_fields.self();
    int pos = fields->m_input->getInputNode()->m_textField->m_uCursorPos;
    CCLabelBMFont* label = fields->m_input->getInputNode()->m_textLabel;
    CCLabelBMFont* cursor = fields->m_input->getInputNode()->m_cursor;
    cursor->setAnchorPoint({0.f, 0.5f});

    bool offset = false;

    int visibleCharacters = 0;
    for (CCNode* node : CCArrayExt<CCNode*>(label->getChildren())) {
        if (!node->isVisible()) break;
        visibleCharacters++;
    }

    if (pos > visibleCharacters) pos = visibleCharacters;
    float x = 0;
    int offsetPos = pos - 1;

    if (offsetPos != -1) {
        CCNode* child = label->getChildByType(pos - 1);
        if (child) {
            x = (child->getPositionX() + child->getContentWidth()/2) * label->getScale();
        }
    }
    if (pos == -1) {
        x = label->getScaledContentWidth();
    }

    cursor->setPositionX(x);

    cursor->setPositionY(label->getPositionY());
}

void MyUILayer::showChat(bool show, bool startWithSlash) {
    auto fields = m_fields.self();
    if (fields->m_disableInput) return;
    queueInMainThread([self = Ref(this), fields, show, startWithSlash] {
        fields->m_chatWindowHistory->removeAllChildren();
        fields->m_historyCursor = -1;
        fields->m_chatboxBG->setVisible(show);
        fields->m_chatWindow->setVisible(!show);
        fields->m_input->setEnabled(false);
        fields->m_input->getInputNode()->onClickTrackNode(false);

        if (show) {
            fields->m_input->setEnabled(true);
            fields->m_input->getInputNode()->onClickTrackNode(true);
            int count = std::min(static_cast<int>(fields->m_chatHistory.size()), 20);
            for (int i = 0; i < count; i++) {
                std::pair<std::string, bool> value = fields->m_chatHistory[fields->m_chatHistory.size() - i - 1];
                std::vector<std::string> newlineSplit = geode::utils::string::split(value.first, "\n");
                std::reverse(newlineSplit.begin(), newlineSplit.end());
                for (const std::string& string : newlineSplit) {
                    fields->m_chatWindowHistory->addChild(self->createChatCell(string, value.second, true));
                }
            }
            fields->m_chatWindowHistory->updateLayout();
        }
        fields->m_chatWindowHistory->setVisible(show);
        
        if (startWithSlash) fields->m_input->getInputNode()->setString("/");
        else fields->m_input->getInputNode()->setString("");
        fields->m_input->getInputNode()->m_textField->m_uCursorPos = fields->m_input->getString().size();
    });
}

CCLayerColor* MyUILayer::createChatCell(const std::string& str, bool error, bool dontFade) {
    CCLayerColor* bg = CCLayerColor::create({0, 0, 0, 100});
    bg->setContentSize({425, 8});
    bg->setAnchorPoint({0, 0});
    bg->setCascadeOpacityEnabled(true);
    bg->setCascadeColorEnabled(true);

    CCNodeRGBA* container = CCNodeRGBA::create();
    container->setContentSize({bg->getContentWidth() - 5, bg->getContentHeight()});
    container->setPosition({bg->getContentWidth()/2, bg->getContentHeight()/2 - 0.6f});
    container->setCascadeOpacityEnabled(true);
    container->setCascadeColorEnabled(true);
    container->setAnchorPoint({0.5f, 0.5f});
    bg->addChild(container);

    RowLayout* layout = RowLayout::create();
    layout->setAxisAlignment(AxisAlignment::Start);
    layout->setCrossAxisAlignment(AxisAlignment::End);
    layout->setGrowCrossAxis(true);
    layout->setAutoScale(false);
    layout->setGap(0);
    container->setLayout(layout);

    std::vector<std::string> spaceSplit = geode::utils::string::split(str, " ");

    CCLabelBMFont* lastSpace;
    for (const std::string& string : spaceSplit) {
        CCLabelBMFont* label = CCLabelBMFont::create(string.c_str(), "minecraft.fnt"_spr);
        label->setScale(0.5f);
        if (error) label->setColor({251, 84, 84});
        lastSpace = CCLabelBMFont::create(" ", "minecraft.fnt"_spr);
        lastSpace->setScale(0.5f);
        container->addChild(label);
        container->addChild(lastSpace);
    }
    lastSpace->removeFromParent();
    container->updateLayout();
    bg->setContentHeight(container->getContentHeight());
    container->setPosition({bg->getContentWidth()/2, bg->getContentHeight()/2 - 1});

    if (!dontFade) {
        CCDelayTime* delay = CCDelayTime::create(10);
        CCFadeTo* fadeOut = CCFadeTo::create(0.75f, 0.f);
        CCFiniteTimeAction* callFunc = CallFuncExt::create([bg = Ref(bg)] {
            CCNode* parent = bg->getParent();
            bg->removeFromParent();
            parent->updateLayout();
        });

        CCSequence* sequence = CCSequence::create(delay, fadeOut, callFunc, nullptr);
        bg->runAction(sequence);
    }

    return bg;
}

class $modify(MyCCEGLView, CCEGLView) {

    void onGLFWKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {

        if (UILayer* layer = UILayer::get()) {
            auto fields = static_cast<MyUILayer*>(layer)->m_fields.self();

            if (key == GLFW_KEY_UP && (action == 1 || action == 2) && fields->m_chatboxBG->isVisible()) {
                fields->m_historyCursor++;
                if (fields->m_inputHistory.empty()) return;
                if (fields->m_historyCursor > fields->m_inputHistory.size() - 1) fields->m_historyCursor = fields->m_inputHistory.size() - 1;
                else fields->m_input->setString(fields->m_inputHistory[fields->m_inputHistory.size() - fields->m_historyCursor - 1]);
                fields->m_input->getInputNode()->m_textField->m_uCursorPos = fields->m_input->getString().size();
            }
            if (key == GLFW_KEY_DOWN && (action == 1 || action == 2) && fields->m_chatboxBG->isVisible()) {
                if (fields->m_inputHistory.empty()) return;
                fields->m_historyCursor--;
                if (fields->m_historyCursor < 0) {
                    fields->m_historyCursor = -1;
                    fields->m_input->setString("");
                }
                else fields->m_input->setString(fields->m_inputHistory[fields->m_inputHistory.size() - fields->m_historyCursor - 1]);
                fields->m_input->getInputNode()->m_textField->m_uCursorPos = fields->m_input->getString().size();
            }
            static_cast<MyUILayer*>(layer)->fixBlinkLabelPos();
        }
        CCEGLView::onGLFWKeyCallback(window, key, scancode, action, mods);
    }
};