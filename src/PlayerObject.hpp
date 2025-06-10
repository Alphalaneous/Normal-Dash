#pragma once

#include <Geode/Geode.hpp>
#include <Geode/modify/PlayerObject.hpp>
#include "KeyNode.hpp"

using namespace geode::prelude;

struct JumpType {
	std::string name;
	std::string path;
};

static const std::vector<JumpType> s_audioFiles = {
	{"Amazing!", "amazing.ogg"_spr},
	{"Nice Jump!", "nice_jump.ogg"_spr},
	{"Critical Hit!", "critical_hit.ogg"_spr},
	{"Insane!", "insane.ogg"_spr}
};

class $modify(MyPlayerObject, PlayerObject) {

	struct Fields {
		bool m_down = false;
		bool m_intersected = false;
		bool m_sprinting = false;
		bool m_sprintLock = false;
		bool m_justCrit = false;
		bool m_hasCriticalJumps = true;
		bool m_hasDragShip = true;
		int m_criticalJumpsOdds = 15;
		float m_origGravity = 0;
		float m_yOffset = 0;
		float m_origSpeed = 0;
		float m_currentSpeed = 0;
		float m_stamina = 100;
		float m_sprintModifier = 1.5f;

	};

    void resetObject() {
		PlayerObject::resetObject();
		if (!GJBaseGameLayer::get()) return;

		auto fields = m_fields.self();
		fields->m_justCrit = false;
		fields->m_stamina = 100;
		fields->m_down = false;
		fields->m_sprintModifier = Mod::get()->getSettingValue<float>("sprint-modifier");
		fields->m_hasCriticalJumps = Mod::get()->getSettingValue<bool>("critical-jumps");
		fields->m_hasDragShip = Mod::get()->getSettingValue<bool>("drag-ship");
		fields->m_criticalJumpsOdds = Mod::get()->getSettingValue<int>("critical-jump-odds");
	}

	float getStamina() {
		return m_fields->m_stamina;
	}

	void setDown(bool down) {
		auto fields = m_fields.self();

		fields->m_down = down;
		fields->m_intersected = false;
		fields->m_yOffset = 0;
	}

	void handleShipDrag(CCPoint mouse) {
		auto fields = m_fields.self();

		if (!fields->m_hasDragShip) return;

		if (m_isShip && !m_isDead) {
			CCSize iconSize = {30, 30};
			setContentSize(iconSize);

			releaseButton(PlayerButton::Jump);
			CCRect pRect = boundingBox();
			pRect.origin.y = m_position.y - pRect.size.height/2;

			if (!fields->m_intersected) {
				fields->m_intersected = pRect.containsPoint(mouse);
				fields->m_yOffset = (pRect.origin.y + pRect.size.height/2) - mouse.y;
			}

			if (fields->m_intersected && fields->m_down) {
				m_position = CCPoint{m_position.x, mouse.y + fields->m_yOffset};
			}
			setContentSize({0, 0});
		}
	}

    void update(float p0) {
		auto fields = m_fields.self();
		if (!GJBaseGameLayer::get()) return PlayerObject::update(p0);

		if (m_isOnGround && fields->m_justCrit) {
			m_gravityMod = fields->m_origGravity;
			fields->m_justCrit = false;
		}

		if (m_wasJumpBuffered) {
			tryCriticalJump();
		}

		PlayerObject::update(p0);
		fields->m_stamina += 0.35 * p0;

		if (fields->m_stamina <= 0) {
			fields->m_stamina = 0;
			fields->m_sprinting = false;
			fields->m_sprintLock = false;
		}
		if (fields->m_stamina > 100) {
			fields->m_stamina = 100;
		}

		if (m_playerSpeed != fields->m_currentSpeed) {
			fields->m_origSpeed = m_playerSpeed;
			fields->m_currentSpeed = m_playerSpeed;
			fields->m_sprintLock = false;
		}

		if (fields->m_sprinting) {
			fields->m_stamina -= 1 * p0;
			if (!fields->m_sprintLock) {
				fields->m_origSpeed = m_playerSpeed;
				fields->m_currentSpeed = m_playerSpeed * fields->m_sprintModifier;
				m_playerSpeed = fields->m_currentSpeed;
				fields->m_sprintLock = true;
			}
		}
		else {
			m_playerSpeed = fields->m_origSpeed;
		}
	}

	void sprint(bool enable) {
		auto fields = m_fields.self();
		fields->m_sprinting = enable;
		fields->m_sprintLock = false;
	}

	void tryCriticalJump() {
		auto fields = m_fields.self();
		if (!GJBaseGameLayer::get()) return;
		if (!fields->m_hasCriticalJumps) return;
		if (fields->m_criticalJumpsOdds < 1) return;
		if (!getRandomNumber(0, fields->m_criticalJumpsOdds - 1) && m_isOnGround && isCube()) {
			fields->m_justCrit = true;
			fields->m_origGravity = m_gravityMod;
			m_gravityMod *= 0.6;

			CCCircleWave* wave = CCCircleWave::create(50, 0, 0.6, false);

			wave->setPosition(getPosition() - 15.f);

			m_gameLayer->m_objectLayer->addChild(wave);

			CCSize winSize = CCDirector::get()->getWinSize();

			JumpType j = s_audioFiles[getRandomNumber(0, s_audioFiles.size()-1)];

			CCLayerColor* textBG = CCLayerColor::create({0, 0, 0, 100});
			textBG->ignoreAnchorPointForPosition(false);
			textBG->setAnchorPoint({0, 1});
			textBG->setContentSize({winSize.width, 30});
			textBG->setPosition({winSize.width, winSize.height - 40});

			CCNode* textContainer = CCNode::create();
			textContainer->setAnchorPoint({1, 1});
			textContainer->setContentSize({winSize.width, 30});
			textContainer->setPosition({0, winSize.height - 38});

			CCLabelBMFont* text = CCLabelBMFont::create(j.name.c_str(), "goldFont.fnt");
			text->setPosition(textContainer->getContentSize()/2);

			textContainer->addChild(text);

			m_gameLayer->addChild(textBG);
			m_gameLayer->addChild(textContainer);

			CCFiniteTimeAction* bgAction1 = CCEaseIn::create(CCMoveTo::create(0.3f, {0, textBG->getPositionY()}), 2);
			CCFiniteTimeAction* bgAction2 = CCEaseIn::create(CCMoveTo::create(0.3f, {-winSize.width, textBG->getPositionY()}), 2);

			CCFiniteTimeAction* textAction1 = CCEaseIn::create(CCMoveTo::create(0.3f, {winSize.width, textContainer->getPositionY()}), 2);
			CCFiniteTimeAction* textAction2 = CCEaseIn::create(CCMoveTo::create(0.3f, {winSize.width * 2, textContainer->getPositionY()}), 2);

			CCSequence* bgSeq = CCSequence::create(bgAction1, CCDelayTime::create(1), bgAction2, CallFuncExt::create([textBG] {
				textBG->removeFromParent();
			}), nullptr);

			CCSequence* textSeq = CCSequence::create(textAction1, CCDelayTime::create(1), textAction2, CallFuncExt::create([textContainer] {
				textContainer->removeFromParent();
			}), nullptr);

			textBG->runAction(bgSeq);
			textContainer->runAction(textSeq);

            FMODAudioEngine::sharedEngine()->playEffect(j.path);
		}
	}

	bool isCube() {
		return !m_isShip && !m_isBird && !m_isBall && !m_isDart && !m_isRobot && !m_isSpider && !m_isSwing;
	}

	void resetModifiers() {
		auto fields = m_fields.self();
		fields->m_origGravity = m_gravityMod;
		fields->m_justCrit = false;
		fields->m_sprinting = false;
		fields->m_sprintLock = false;
		fields->m_down = false;
	}
};