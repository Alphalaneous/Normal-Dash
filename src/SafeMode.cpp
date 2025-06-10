#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/GJGameLevel.hpp>

using namespace geode::prelude;

class $modify(PlayLayer) {

    void showNewBest(bool p0, int p1, int p2, bool p3, bool p4, bool p5) {
        // do nothing
    }

    void levelComplete() {
        bool testMode = m_isTestMode;
        m_isTestMode = true;
        PlayLayer::levelComplete();
        m_isTestMode = testMode;
    }

};

class $modify(GJGameLevel) {

    void savePercentage(int p0, bool p1, int p2, int p3, bool p4) {
        // do nothing
    }
};