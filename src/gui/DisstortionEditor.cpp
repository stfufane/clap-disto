#include "DisstortionEditor.h"

#include "embedded/disto_images.h"
#include "embedded/disto_shaders.h"
#include "params/Parameters.h"
#include "utils/Logger.h"

namespace stfefane::gui {

using namespace visage::dimension;

DisstortionEditor::DisstortionEditor(Disstortion& d)
: mDisstortion(d)
, mInputGain(d, params::eInGain)
, mOutputGain(d, params::eOutGain)
, mDrive(d, params::eDrive)
, mAsymmetry(d, params::eAsymmetry)
, mMix(d, params::eMix)
, mPreFreq(d, params::ePreFilterFreq)
, mPreRes(d, params::ePreFilterQ)
, mPreGain(d, params::ePreFilterGain)
, mPostFreq(d, params::ePostFilterFreq)
, mPostRes(d, params::ePostFilterQ)
, mPostGain(d, params::ePostFilterGain){
    LOG_INFO("ui", "[DisstortionEditor::createUI]");

    addChild(mInputGain);
    addChild(mOutputGain);
    addChild(mDrive);
    addChild(mAsymmetry);
    addChild(mMix);

    addChild(mPreFreq);
    addChild(mPreRes);
    addChild(mPreGain);

    addChild(mPostFreq);
    addChild(mPostRes);
    addChild(mPostGain);

    mDrive.setFontSize(42.f);

    mPreFreq.setFontSize(10.f);
    mPreRes.setFontSize(10.f);
    mPreGain.setFontSize(10.f);

    mPostFreq.setFontSize(10.f);
    mPostRes.setFontSize(10.f);
    mPostGain.setFontSize(10.f);

    mPreGain.setDisplayUnit(false);
    mPostGain.setDisplayUnit(false);

    mGlitchShader = std::make_unique<visage::ShaderPostEffect>(resources::shaders::vs_custom,
                                                               resources::shaders::fs_glitch);
    mDrive.setPostEffect(mGlitchShader.get());
}

void DisstortionEditor::draw(visage::Canvas& canvas) {
    canvas.setColor(0xffffffff);
    canvas.fill(0, 0, width(), height());
    canvas.image(resources::images::disstortion_png.data, resources::images::disstortion_png.size, 0.f, 0.f, width(), height());
}

void DisstortionEditor::resized() {
    ApplicationWindow::resized();

    mInputGain.setBounds(40.f, 80.f, 78.f, 78.f);
    mOutputGain.setBounds(472.f, 80.f, 78.f, 78.f);
    mDrive.setBounds(194.f, 80.f, 212.f, 212.f);
    mAsymmetry.setBounds(270.f, 440.f, 60.f, 60.f);
    mMix.setBounds(270.f, 545.f, 60.f, 60.f);

    // TODO: put in a specific panel
    mPreFreq.setBounds(22.f, 530.f, 75.f, 75.f);
    mPreRes.setBounds(110.f, 550.f, 34.f, 34.f);
    mPreGain.setBounds(160.f, 550.f, 34.f, 34.f);

    mPostFreq.setBounds(404.f, 530.f, 75.f, 75.f);
    mPostRes.setBounds(490.f, 550.f, 34.f, 34.f);
    mPostGain.setBounds(540.f, 550.f, 34.f, 34.f);
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
