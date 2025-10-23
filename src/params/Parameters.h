#pragma once

#include "Parameter.h"

#include <clap/id.h>
#include <vector>
#include <unordered_map>

namespace stfefane::params {

enum param_ids : clap_id {
    eDrive,
    eDriveType,
    eInGain,
    eOutGain,
    ePreFilterFreq,
    ePreFilterOn,
    ePostFilterFreq,
    ePostFilterOn,
    eAsymmetry,
    eMix,
};

class Parameters {
public:
    Parameters() = default;

    void addParameter(clap_id id, const std::string& name, std::unique_ptr<ParamValueType> value_type);

    [[nodiscard]] size_t count() const noexcept { return mParameters.size(); }

    [[nodiscard]] bool isValidParamId(const clap_id param_id) const noexcept { return mIdToParameter.contains(param_id); }
    [[nodiscard]] double getParamValue(const clap_id param_id) const { return mIdToParameter.at(param_id)->getValue(); }
    [[nodiscard]] const ParamValueType& getParamValueType(const clap_id param_id) const { return mIdToParameter.at(param_id)->getValueType(); }

    [[nodiscard]] Parameter* getParamById(clap_id id) const noexcept;
    [[nodiscard]] Parameter* getParamByIndex(size_t index) const noexcept;

    [[nodiscard]] const std::vector<std::unique_ptr<Parameter>>& getParams() const { return mParameters; }

private:
    std::vector<std::unique_ptr<Parameter>> mParameters;
    std::unordered_map<clap_id, Parameter*> mIdToParameter;
};

} // namespace stfefane::params