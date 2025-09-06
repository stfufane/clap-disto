#include "DisstortionEditor.h"

#include "../disstortion.h"
#include "../params/Parameters.h"

namespace stfefane::gui {

using namespace visage::dimension;

DisstortionEditor::DisstortionEditor(Disstortion& disstortion)
: mDisstortion(disstortion)
, mScrollable("ScrollContainer")
, mDriveType(disstortion, params::eDriveType)
, mDrive(disstortion, params::eDrive)
, mInputGain(disstortion, params::eInGain)
, mOutputGain(disstortion, params::eOutGain)
, mPreFilter(disstortion, params::ePreFilterFreq)
, mPostFilter(disstortion, params::ePostFilterFreq) {

    setFlexLayout(true);
    layout().setFlexItemAlignment(visage::Layout::ItemAlignment::Center);

    addChild(mScrollable);

    mScrollable.scrollableLayout().setFlex(true);
    mScrollable.scrollableLayout().setPadding(10_px);
    mScrollable.scrollableLayout().setFlexGap(10_px);
    mScrollable.scrollableLayout().setFlexWrap(true);
    mScrollable.scrollableLayout().setFlexRows(false);
    mScrollable.scrollableLayout().setFlexWrapAlignment(visage::Layout::WrapAlignment::Start);

    setupKnob(mDriveType);
    setupKnob(mDrive);
    setupKnob(mInputGain);
    setupKnob(mOutputGain);
    setupKnob(mPreFilter);
    setupKnob(mPostFilter);
}

void DisstortionEditor::draw(visage::Canvas& canvas) {
    canvas.setColor(0xffdeadbb);
    canvas.fill(0.f, 0.f, width(), height());
}

void DisstortionEditor::resized() {
    mScrollable.setBounds(0, 0, width(), height());
    mScrollable.setScrollableHeight(mPostFilter.bottom() + 10.f);
}

void DisstortionEditor::setupKnob(RotaryKnob& knob) {
    knob.layout().setFlex(true);
    knob.layout().setHeight(120);
    knob.layout().setWidth(120);
    knob.layout().setFlexGrow(1.0f);
    mScrollable.addScrolledChild(&knob);
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
