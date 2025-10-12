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
, mPostFilter(disstortion, params::ePostFilterFreq)
, mMix(disstortion, params::eMix)
, mPreFilterOn(disstortion, params::ePreFilterOn)
, mPostFilterOn(disstortion, params::ePostFilterOn) {

    setFlexLayout(true);
    layout().setFlexItemAlignment(visage::Layout::ItemAlignment::Center);

    addChild(mScrollable);

    mScrollable.scrollableLayout().setFlex(true);
    mScrollable.scrollableLayout().setPadding(10_px);
    mScrollable.scrollableLayout().setFlexGap(10_px);
    mScrollable.scrollableLayout().setFlexWrap(true);
    mScrollable.scrollableLayout().setFlexRows(false);
    mScrollable.scrollableLayout().setFlexWrapAlignment(visage::Layout::WrapAlignment::Start);

    setupElement(mDriveType);
    setupElement(mDrive);
    setupElement(mInputGain);
    setupElement(mOutputGain);
    setupElement(mPreFilterOn);
    setupElement(mPreFilter);
    setupElement(mPostFilterOn);
    setupElement(mPostFilter);
    setupElement(mMix);
}

void DisstortionEditor::draw(visage::Canvas& canvas) {
    canvas.setColor(0xffdeadbb);
    canvas.fill(0.f, 0.f, width(), height());
}

void DisstortionEditor::resized() {
    mScrollable.setBounds(0, 0, width(), height());
}

void DisstortionEditor::setupElement(visage::Frame& element) {
    element.layout().setFlex(true);
    element.layout().setHeight(120);
    element.layout().setWidth(120);
    element.layout().setFlexGrow(1.0f);
    mScrollable.addScrolledChild(&element);
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
    setNativeWindowDimensions(std::clamp(width, kMinWidth, kMaxWidth), std::clamp(height, kMinHeight, kMaxHeight));
#endif
}

} // namespace stfefane::gui
