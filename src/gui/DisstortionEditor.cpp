#include "DisstortionEditor.h"

#include "../disstortion.h"
#include "../params/Parameters.h"

namespace stfefane::gui {

using namespace visage::dimension;

DisstortionEditor::DisstortionEditor(Disstortion& disstortion)
    : mDisstortion(disstortion)
    , mDriveKnob(disstortion, params::eDrive)
    , mGainKnob(disstortion, params::eGain)
    , mCutoffKnob(disstortion, params::eCutoff) {

    setWindowDimensions(300.f, 450.f);
    setFixedAspectRatio(true);

    addChild(mDriveKnob);
    addChild(mGainKnob);
    addChild(mCutoffKnob);
}

void DisstortionEditor::draw(visage::Canvas& canvas) {
    canvas.setColor(0xffdeadbb);
    canvas.fill(0.f, 0.f, width(), height());
}

void DisstortionEditor::resized() {
    const auto third_height = height() / 3.f;
    const auto two_thirds_height = third_height * 2.f;
    const auto half_width = width() / 2.f;

    mDriveKnob.setBounds(visage::Bounds(0.f, 0.f, width(), two_thirds_height));
    mGainKnob.setBounds(visage::Bounds(0.f, two_thirds_height, half_width, third_height));
    mCutoffKnob.setBounds(visage::Bounds(half_width, two_thirds_height, half_width, third_height));
}

int DisstortionEditor::pluginWidth() const {
#if __APPLE__
    return width();
#else
    return nativeWidth();
#endif
}

int DisstortionEditor::pluginHeight() const {
#if __APPLE__
    return height();
#else
    return nativeHeight();
#endif
}

void DisstortionEditor::setPluginDimensions(int width, int height) {
#if __APPLE__
    setWindowDimensions(width, height);
#else
    setNativeWindowDimensions(width, height);
#endif
}

} // namespace stfefane::gui
