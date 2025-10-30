#include "Parameters.h"

#include "dsp/MultiDisto.h"

namespace stfefane::params {

Parameters::Parameters() {
    addParameter(eMix, "Mix", std::make_unique<ParamValueType>(0., 100., 50., " %"));
    addParameter(eDriveType, "Drive Type", std::make_unique<SteppedValueType>(dsp::MultiDisto::types(), 0.));
    addParameter(eDrive, "Drive", std::make_unique<ParamValueType>(0., dsp::kMaxDriveDb, 6., " dB", MappingType::Logarithmic));
    addParameter(eAsymmetry, "Asymmetry",
             std::make_unique<ParamValueType>(-0.5, 0.5, 0., std::string(), MappingType::BipolarSCurve));
    addParameter(eInGain, "Input Gain", std::make_unique<ParamValueType>(-12., 24., 0., " dB", MappingType::Logarithmic));
    addParameter(eOutGain, "Output Gain", std::make_unique<ParamValueType>(-24., 6., 0., " dB", MappingType::Logarithmic));

    addParameter(ePreFilterOn, "Pre Filter On", std::make_unique<BooleanValueType>(true));
    addParameter(ePreFilterType, "Pre Filter Type", std::make_unique<SteppedValueType>(dsp::BiquadFilter::types(), 0.));
    addParameter(ePreFilterFreq, "Pre Filter Freq",
                 std::make_unique<ParamValueType>(20., 20000., 10000., " Hz", MappingType::Logarithmic));
    addParameter(ePreFilterQ, "Pre Filter Q", std::make_unique<ParamValueType>(0.1, 35., 0.707, "", MappingType::Logarithmic));
    addParameter(ePreFilterGain, "Pre Filter Gain", std::make_unique<ParamValueType>(-12., 12., 0., " dB"));

    addParameter(ePostFilterOn, "Post Filter On", std::make_unique<BooleanValueType>(true));
    addParameter(ePostFilterType, "Post Filter Type", std::make_unique<SteppedValueType>(dsp::BiquadFilter::types(), 1.));
    addParameter(ePostFilterFreq, "Post Filter Freq",
                 std::make_unique<ParamValueType>(20., 20000., 80., " Hz", MappingType::Logarithmic));
    addParameter(ePostFilterQ, "Post Filter Q", std::make_unique<ParamValueType>(0.1, 35., 0.707, "", MappingType::Logarithmic));
    addParameter(ePostFilterGain, "Post Filter Gain", std::make_unique<ParamValueType>(-12., 12., 0., " dB"));
}

void Parameters::addParameter(clap_id id, const std::string& name, std::unique_ptr<ParamValueType> value_type) {
    auto new_param = std::make_unique<Parameter>(id, name, std::move(value_type), mParameters.size());
    auto inserted = mIdToParameter.insert_or_assign(id, new_param.get());
    if (!inserted.second) {
        throw std::logic_error("same parameter id was inserted twice -> " + std::to_string(id));
    }
    mParameters.emplace_back(std::move(new_param));
}

[[nodiscard]] Parameter* Parameters::getParamById(clap_id id) const noexcept {
    if (!mIdToParameter.contains(id)) {
        return nullptr;
    }
    return mIdToParameter.at(id);
}

[[nodiscard]] Parameter* Parameters::getParamByIndex(size_t index) const noexcept {
    if (index >= mParameters.size()) {
        return nullptr;
    }
    return mParameters[index].get();
}

} // namespace stfefane::params