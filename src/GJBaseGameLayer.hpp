#pragma once

#include <Geode/Geode.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>
#include "Geode/binding/UILayer.hpp"
#include "KeyNode.hpp"
#include "PlayerObject.hpp"
#include "UILayer.hpp"

using namespace geode::prelude;

static const ccColor3B s_red = {255, 20, 20};
static const ccColor3B s_yellow = {225, 255, 0};
static const ccColor3B s_green = {0, 255, 55};

class $modify(MyGJBaseGameLayer, GJBaseGameLayer) {
	
	struct Fields {
		Ref<CCSprite> m_staminaBar;
		CCSprite* m_staminaBarInner;
		CCLayer* m_keyNodeLayer;
		enumKeyCodes m_currentKey = enumKeyCodes::KEY_F;
		std::vector<Ref<KeyNode>> m_keyNodes = {};
		float m_lastMilestone = 0.0f;
		bool m_hasSprint;
		bool m_hasPlatformerSprint;
		bool m_hasKeyboardPro;
		bool m_hasDragShip;
		bool m_hasCriticalJump;
        bool m_skipJump = true;
		int m_keyDistance = 0;
		int m_keyPreload = 0;
	};

    void resetPlayer() {
		auto fields = m_fields.self();

		if (fields->m_hasKeyboardPro) {
			fields->m_lastMilestone = 0.0f;
			for (KeyNode* node : fields->m_keyNodes) {
				node->removeFromParent();
			}
			fields->m_keyNodes.clear();
			
			if (!m_isPlatformer) {
				KeyNode* keyNode = KeyNode::create(this, 60);
				fields->m_currentKey = keyNode->m_key;
				fields->m_keyNodes.push_back(keyNode);
				fields->m_keyNodeLayer->addChild(keyNode);

				for (int i = 0; i < fields->m_keyPreload - 1; i++) {
					KeyNode* keyNodeI = KeyNode::create(this, 60 + fields->m_keyDistance + fields->m_keyDistance * i);
					fields->m_keyNodes.push_back(keyNodeI);
					fields->m_keyNodeLayer->addChild(keyNodeI);
				}
			}
		}
        static_cast<MyUILayer*>(UILayer::get())->toggleInput(true);

		GJBaseGameLayer::resetPlayer();
	}

    bool init() {
		if (!GJBaseGameLayer::init()) return false;
		auto fields = m_fields.self();

		fields->m_hasSprint = Mod::get()->getSettingValue<bool>("sprint");
		fields->m_hasPlatformerSprint = Mod::get()->getSettingValue<bool>("platformer-sprint");
		fields->m_hasKeyboardPro = Mod::get()->getSettingValue<bool>("keyboard-pro");
		fields->m_hasDragShip = Mod::get()->getSettingValue<bool>("drag-ship");
		fields->m_hasCriticalJump = Mod::get()->getSettingValue<bool>("critical-jumps");
		fields->m_keyDistance = Mod::get()->getSettingValue<int>("keyboard-pro-interval");
		fields->m_keyPreload = Mod::get()->getSettingValue<int>("keyboard-pro-preload");

		CCSize winSize = CCDirector::get()->getWinSize();

		fields->m_keyNodeLayer = CCLayer::create();
		fields->m_keyNodeLayer->setContentSize(winSize);
		fields->m_keyNodeLayer->setAnchorPoint({0.5f, 0.5f});
		fields->m_keyNodeLayer->ignoreAnchorPointForPosition(false);
		fields->m_keyNodeLayer->setPosition(winSize/2);
		fields->m_keyNodeLayer->setID("key-node-layer"_spr);
		fields->m_keyNodeLayer->setZOrder(999);

		addChild(fields->m_keyNodeLayer);

		fields->m_staminaBar = CCSprite::create("slidergroove2.png");
		fields->m_staminaBar->setPosition({winSize.width/2, 10});
		fields->m_staminaBar->setZOrder(999);
		fields->m_staminaBar->setID("stamina-bar"_spr);

		fields->m_staminaBarInner = CCSprite::create("sliderBar2.png");
		fields->m_staminaBarInner->setAnchorPoint({0, 0});
		fields->m_staminaBarInner->setPosition({2, 4});
		fields->m_staminaBarInner->setZOrder(-1);
		fields->m_staminaBarInner->setID("stamina-bar-inner"_spr);

		fields->m_staminaBarInner->setTextureRect({0, 0, 0, 8});

		fields->m_staminaBar->addChild(fields->m_staminaBarInner);

		queueInMainThread([self = Ref(this), fields] {
			bool platformerSprint = fields->m_hasSprint && (self->m_isPlatformer && fields->m_hasPlatformerSprint);
			bool normalSprint = fields->m_hasSprint && !self->m_isPlatformer;

			if ((normalSprint || platformerSprint) && !EditorUI::get()) {
				self->addChild(fields->m_staminaBar);
			}
		});

		return true;
	}

    void handleButton(bool down, int button, bool isPlayer1) {
		bool skip = false;
		auto fields = m_fields.self();

		if (fields->m_hasDragShip) {
			if (button == 1) {
				if (isPlayer1 && m_player1->m_isShip) skip = true;
				if (!isPlayer1 && m_player2->m_isShip) skip = true;
			}
		}

		MyPlayerObject* player1 = static_cast<MyPlayerObject*>(m_player1);
		MyPlayerObject* player2 = static_cast<MyPlayerObject*>(m_player2);

		if (button == 1) {
			if (fields->m_hasDragShip) {
				player1->setDown(down && isPlayer1);
				player2->setDown(down && !isPlayer1);
			}

			if (fields->m_skipJump && !m_isPlatformer && fields->m_hasKeyboardPro) return;

			if (down && fields->m_hasCriticalJump) {
				player1->tryCriticalJump();
				player2->tryCriticalJump();
			}
			
			return GJBaseGameLayer::handleButton(!skip && down, button, isPlayer1);
		}

		GJBaseGameLayer::handleButton(down, button, isPlayer1);
	}

    void update(float p0) {
		auto fields = m_fields.self();

		CCPoint mouseToObj = m_objectLayer->convertToNodeSpaceAR(getMousePos());

		MyPlayerObject* player1 = static_cast<MyPlayerObject*>(m_player1);
		player1->handleShipDrag(mouseToObj);
		MyPlayerObject* player2 = static_cast<MyPlayerObject*>(m_player2);
		player2->handleShipDrag(mouseToObj);

		if (fields->m_hasSprint) {
			float maxWidth = fields->m_staminaBar->getContentSize().width - 4;
			float stamina = player1->getStamina();
			float barWidth = maxWidth * (stamina/100);
			ccColor3B color;
			float threshold = 70.f;
			if (stamina > threshold) {
				float t = (stamina - threshold) / threshold;
				color = lerpColor(s_yellow, s_green, t);
			}
			if (stamina <= threshold) {
				float t = stamina / threshold;
				color = lerpColor(s_red, s_yellow, t);
			}
			
			fields->m_staminaBarInner->setColor(color);
			fields->m_staminaBarInner->setTextureRect({0, 0, barWidth, 8});
		}

		if (!m_isPlatformer && fields->m_hasKeyboardPro) {
			float currentX = m_player1->m_position.x;
			float interval = fields->m_keyDistance * fields->m_keyPreload;
			float preCurrentX = m_player1->m_position.x + interval;
			float milestone = std::floor(preCurrentX / interval);

			if (milestone > std::floor(fields->m_lastMilestone / interval)) {
				fields->m_lastMilestone = preCurrentX;
				for (int i = 0; i < fields->m_keyPreload; i++) {
					KeyNode* keyNode = KeyNode::create(this, 60 + preCurrentX + fields->m_keyDistance * i);
					fields->m_keyNodes.push_back(keyNode);
					fields->m_keyNodeLayer->addChild(keyNode);
				}
			}

			for (KeyNode* node : fields->m_keyNodes) {
				enumKeyCodes key = node->tryActivate(currentX * m_objectLayer->getScale());
				if (key != enumKeyCodes::KEY_Unknown) {
					if (!m_player1->m_isShip) {
						GJBaseGameLayer::get()->handleButton(false, 1, true);
					} 
					fields->m_currentKey = key;
					fields->m_keyNodes.erase(std::remove(fields->m_keyNodes.begin(), fields->m_keyNodes.end(), node), fields->m_keyNodes.end());
					break;
				}
			}
		}

		GJBaseGameLayer::update(p0);
	}

	ccColor3B lerpColor(const ccColor3B& a, const ccColor3B& b, float t) {
		return {
			static_cast<GLubyte>(a.r + (b.r - a.r) * t),
			static_cast<GLubyte>(a.g + (b.g - a.g) * t),
			static_cast<GLubyte>(a.b + (b.b - a.b) * t)
		};
	}
};