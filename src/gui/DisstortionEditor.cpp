#include "DisstortionEditor.h"
#include "../params.h"

namespace stfefane::gui {

using namespace visage::dimension;

DisstortionEditor::DisstortionEditor()
    : mGainKnob(params::eGain) {

    setWindowDimensions(400.f, 300.f);
    setFixedAspectRatio(true);

    addChild(mGainKnob);
}

void DisstortionEditor::draw(visage::Canvas& canvas) {
    canvas.setColor(0xffdeadbb);
    canvas.fill(0.f, 0.f, width(), height());
}

void DisstortionEditor::resized() {
    mGainKnob.setBounds(visage::Bounds(0.f, 0.f, width(), height()));
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
