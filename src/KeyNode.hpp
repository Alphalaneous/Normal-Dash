#pragma once

#include <Geode/Geode.hpp>
#include "Utils.hpp"

using namespace geode::prelude;

static const std::vector<enumKeyCodes> s_validKeys = {
    KEY_B,
    KEY_C,
    KEY_E,
    KEY_F,
    KEY_G,
    KEY_H,
    KEY_I,
    KEY_J,
    KEY_K,
    KEY_L,
    KEY_M,
    KEY_N,
    KEY_O,
    KEY_Q,
    KEY_U,
    KEY_V,
    KEY_Y
};

class KeyNode : public CCNode {
public:
	enumKeyCodes m_key;
	bool m_activated = false;
	GJBaseGameLayer* m_gameLayer;
	float m_xOffset;
	CCSprite* m_line;
	CCSprite* m_keycap;
	CCNode* m_pivot;

	static KeyNode* create(GJBaseGameLayer* gameLayer, float xOffset);
	bool init(GJBaseGameLayer* gameLayer, float xOffset);
	std::string keyForEnum(enumKeyCodes key);
	void update(float dt);
	enumKeyCodes tryActivate(float x);
};