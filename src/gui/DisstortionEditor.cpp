#include "DisstortionEditor.h"

#include "../disstortion.h"
#include "../params/Parameters.h"

namespace stfefane::gui {

using namespace visage::dimension;

DisstortionEditor::DisstortionEditor(Disstortion& disstortion)
    : mDisstortion(disstortion)
    , mDriveType(disstortion, params::eDriveType)
    , mDrive(disstortion, params::eDrive)
    , mGain(disstortion, params::eInGain)
    , mPreFilter(disstortion, params::ePreFilterFreq)
    , mPostFilter(disstortion, params::ePostFilterFreq) {

    setFixedAspectRatio(true);

    addChild(&mDriveType);
    addChild(&mDrive);
    addChild(&mGain);
    addChild(&mPreFilter);
    addChild(&mPostFilter);

    setWindowDimensions(450.f, 600.f);
}

void DisstortionEditor::draw(visage::Canvas& canvas) {
    canvas.setColor(0xffdeadbb);
    canvas.fill(0.f, 0.f, width(), height());
}

void DisstortionEditor::resized() {
    const auto third_height = height() / 3.f;
    const auto two_thirds_height = third_height * 2.f;
    const auto half_width = width() / 2.f;

    mDriveType.setBounds(visage::Bounds(0.f, 0.f, width(), third_height));
    mDrive.setBounds(visage::Bounds(0.f, third_height, half_width, third_height));
    mGain.setBounds(visage::Bounds(half_width, third_height, half_width, third_height));
    mPreFilter.setBounds(visage::Bounds(0.f, two_thirds_height, half_width, third_height));
    mPostFilter.setBounds(visage::Bounds(half_width, two_thirds_height, half_width, third_height));
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
