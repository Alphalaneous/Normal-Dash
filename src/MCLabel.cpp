#include "MCLabel.hpp"

MCLabel* MCLabel::create(std::string text, std::string font){

    MCLabel *ret = new (std::nothrow) MCLabel();

    if (ret && ret->init(text, font)) {
        ret->autorelease();
    } else {
        delete ret;
        ret = nullptr;
    }
    return ret;
}

bool MCLabel::init(std::string text, std::string font){
    return CCLabelBMFont::initWithString(text.c_str(), font.c_str());
}
