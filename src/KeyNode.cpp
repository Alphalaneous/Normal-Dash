#include "KeyNode.hpp"

KeyNode* KeyNode::create(GJBaseGameLayer* gameLayer, float xOffset) {
    KeyNode* node = new KeyNode();
    if (node->init(gameLayer, xOffset)) {
        node->autorelease();
        return node;
    }
    delete node;
    return nullptr;
}

bool KeyNode::init(GJBaseGameLayer* gameLayer, float xOffset) {
    m_gameLayer = gameLayer;
    m_xOffset = xOffset;
    m_key = s_validKeys[getRandomNumber(0, s_validKeys.size()-1)];

    setID(fmt::format("key-node-{}", static_cast<char>(m_key)));

    CCSize winSize = CCDirector::get()->getWinSize();
    
    m_line = CCSprite::createWithSpriteFrameName("floorLine_01_001.png");
    m_line->setRotation(90);
    m_line->setPositionY(winSize.height/2);
    m_line->setScale(.675f);
    m_line->setID("line"_spr);

    setContentSize({30, winSize.height});
    setAnchorPoint({0, 0});
    setPosition({m_gameLayer->m_objectLayer->getPosition().x + m_xOffset, 0});

    addChild(m_line);

    m_keycap = CCSprite::create("keycap.png"_spr);
    m_keycap->setPosition({-30, 00});
    m_keycap->setCascadeOpacityEnabled(true);
    m_keycap->setCascadeColorEnabled(true);
    m_keycap->setScale(0.2f);
    m_keycap->setID("key"_spr);

    CCLabelBMFont* keyText = CCLabelBMFont::create(keyForEnum(m_key).c_str(), "chatFont.fnt");
    keyText->setPosition(m_keycap->getContentSize()/2);
    keyText->setScale(4.f);
    keyText->setID("key-label"_spr);

    m_keycap->addChild(keyText);

    m_pivot = CCNode::create();
    m_pivot->setAnchorPoint({1, 0});
    m_pivot->setContentSize({0, 0});
    m_pivot->setPosition({60, winSize.height/2});
    m_pivot->setID("pivot"_spr);

    m_pivot->addChild(m_keycap);

    addChild(m_pivot);

    scheduleUpdate();
    return true;
}

std::string KeyNode::keyForEnum(enumKeyCodes key) {
    if (key >= 0x41 && key <= 0x5A) {
        return std::string(1, static_cast<char>(key));
    }
    return "";
}

void KeyNode::update(float dt) {
    if (m_gameLayer) {
        
        CCSize winSize = CCDirector::get()->getWinSize();
        float widthToAdd = winSize.width * m_gameLayer->m_gameState.m_levelFlipping;
        int flip = m_gameLayer->m_gameState.m_levelFlipping == 1 ? -1 : 1;
        float result = 1.0f - 2.0f * m_gameLayer->m_gameState.m_levelFlipping;
        setPosition({(flip * m_gameLayer->m_objectLayer->getPosition().x + m_xOffset) + widthToAdd * m_gameLayer->m_gameState.m_levelFlipping, 0});
        getParent()->setScaleX(result);
        // cheaty but I am lazy
        if (EditorUI* editorUI = EditorUI::get()) {
            getParent()->setVisible(!editorUI->m_positionSlider->isVisible());
        }
    }
}

enumKeyCodes KeyNode::tryActivate(float x) {

    float result = 1.0f - 2.0f * m_gameLayer->m_gameState.m_levelFlipping;
    x *= result;

    if (!m_activated && x > m_xOffset) {
        m_activated = true;

        m_line->runAction(CCFadeOut::create(0.35));

        CCRotateBy* rotatePivot = CCRotateBy::create(0.5, 180);
        CCRotateBy* rotate = CCRotateBy::create(0.5, 360);
        CCRotateBy* rotate2 = CCRotateBy::create(5, 5400);
        CCFadeOut* fadeOut = CCFadeOut::create(0.5);

        CCMoveBy* movePivot = CCMoveBy::create(5, {200, -1000});

        CCSequence* pivotSeq = CCSequence::create(rotatePivot, CCEaseSineOut::create(movePivot), nullptr);
        CCSequence* keySeq = CCSequence::create(rotate, rotate2, fadeOut, CallFuncExt::create([this] {
            removeFromParent();
        }), nullptr);

        m_pivot->runAction(pivotSeq);
        m_keycap->runAction(keySeq);

        FMODAudioEngine::sharedEngine()->playEffect("collect_key.ogg"_spr);

        return m_key;
    }
    return enumKeyCodes::KEY_Unknown;
}