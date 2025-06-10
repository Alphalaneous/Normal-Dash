#pragma once

#include <Geode/Geode.hpp>

using namespace geode::prelude;

class MCLabel : public CCLabelBMFont {
public:

    static MCLabel* create(std::string text, std::string font);
    bool init(std::string text, std::string font);

};