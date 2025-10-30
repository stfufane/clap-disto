#include "DisstortionEditor.h"

#include "disstortion.h"
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
, mDriveSelector(d, params::eDriveType)
, mPreFilter(d, "Pre Filter", params::ePreFilterOn, params::ePreFilterFreq, params::ePreFilterQ, params::ePreFilterGain, params::ePreFilterType)
, mPostFilter(d, "Post Filter", params::ePostFilterOn, params::ePostFilterFreq, params::ePostFilterQ, params::ePostFilterGain, params::ePostFilterType)
{
    LOG_INFO("ui", "[DisstortionEditor::createUI]");

    mPalette.initWithDefaults();
    setPalette(&mPalette);

    mPalette.setColor(visage::ToggleButton::ToggleButtonOn, 0xffedae49);
    mPalette.setColor(visage::ToggleButton::ToggleButtonOnHover, 0xffedae49);
    mPalette.setColor(visage::ToggleButton::ToggleButtonOff, 0xffa55555);
    mPalette.setColor(visage::ToggleButton::ToggleButtonOffHover, 0xffedae49);

    addChild(mInputGain);
    addChild(mOutputGain);
    addChild(mDrive);
    addChild(mAsymmetry);
    addChild(mMix);
    addChild(mDriveSelector);

    addChild(mPreFilter);
    addChild(mPostFilter);

    mDrive.setFontSize(42.f);

    mGlitchShader = std::make_unique<visage::ShaderPostEffect>(resources::shaders::vs_custom,
                                                               resources::shaders::fs_glitch);
    mDrive.setPostEffect(mGlitchShader.get());

    // Add a listener on the drive value to make the glitch effect react to the level of drive.
    mDriveAttachment = std::make_unique<params::ParameterAttachment>(d.getParameter(params::eDrive), [&](params::Parameter* param, double new_val) {
        const auto linear_gain = utils::dbToLinear(param->getValueType().denormalizedValue(new_val));
        if (utils::almostEqual(linear_gain, 1.)) {
            mGlitchShader->setUniformValue("u_glitch_amount", 0.f);
        } else {
            mGlitchShader->setUniformValue("u_glitch_amount", linear_gain / 15.f);
        }
    });
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
    mDriveSelector.setBounds(0.f, 345.7f, width(), 64.f);

    mPreFilter.setBounds(2.8f, 409.6f, 214.9f, 228.5f);
    mPostFilter.setBounds(383.2f, 409.6f, 214.9f, 228.5f);
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
