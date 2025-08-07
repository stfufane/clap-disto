#include "DisstortionEditor.h"
#include "../params.h"

namespace stfefane::gui {

using namespace visage::dimension;

DisstortionEditor::DisstortionEditor()
    : mGainKnob(params::eGain) {

    setWindowDimensions(80_vmin, 60_vmin);
    addChild(mGainKnob);
}

void DisstortionEditor::draw(visage::Canvas& canvas) {
    canvas.setColor(0xdeadbeef);
    canvas.fill(0.f, 0.f, width(), height());
}

void DisstortionEditor::resized() {
    mGainKnob.setBounds(visage::Bounds(10.f, 10.f, 100.f, 100.f));
}
} // namespace stfefane::gui
