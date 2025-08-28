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

    // setFixedAspectRatio(false);
    setFlexLayout(true);
    layout().setPadding(10_px);
    layout().setFlexGap(10_px);
    layout().setFlexWrap(true);
    layout().setFlexRows(false);
    layout().setFlexWrapAlignment(visage::Layout::WrapAlignment::Start);

    addChild(mDriveType);
    addChild(mDrive);
    addChild(mGain);
    addChild(mPreFilter);
    addChild(mPostFilter);

    for (auto* child: children()) {
        child->layout().setHeight(120);
        child->layout().setWidth(120);
        child->layout().setFlexGrow(1.0f);
    }
}

void DisstortionEditor::draw(visage::Canvas& canvas) {
    canvas.setColor(0xffdeadbb);
    canvas.fill(0.f, 0.f, width(), height());
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
