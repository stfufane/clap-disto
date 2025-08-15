#pragma once

#include "Parameter.h"

#include <clap/ext/params.h>
#include <clap/id.h>
#include <cstdint>
#include <vector>
#include <unordered_map>

namespace stfefane::params {

enum param_ids : clap_id {
    eDrive,
    eGain,
    eCutoff,
};

struct ParamValueBounds {
    double mMin = std::numeric_limits<double>::lowest();
    double mMax = std::numeric_limits<double>::max();
    double mDefault = 0.;
};

class Parameters {
public:
    Parameters() = default;

    void addParameter(clap_id id, const std::string& name, uint32_t flags, const ParamValueBounds& value_bounds) {
        clap_param_info param_info;
        param_info.id = id;
        param_info.flags = flags;
        snprintf(param_info.name, sizeof(param_info.name), "%s", name.c_str());
        param_info.min_value = value_bounds.mMin;
        param_info.max_value = value_bounds.mMax;
        param_info.default_value = value_bounds.mDefault;

        auto new_param = std::make_unique<Parameter>(param_info, mParameters.size());
        auto* param_ptr = new_param.get();
        auto inserted = mIdToParameter.insert_or_assign(param_info.id, param_ptr);
        if (!inserted.second) {
            throw std::logic_error("same parameter id was inserted twice -> " + std::to_string(param_info.id));
        }
        mParameters.emplace_back(std::move(new_param));
    }

    size_t count() const noexcept { return mParameters.size(); }

    [[nodiscard]] bool isValidParamId(const clap_id param_id) const noexcept { return mIdToParameter.contains(param_id); }
    [[nodiscard]] double getParamValue(const clap_id param_id) const {
        return mIdToParameter.at(param_id)->getValue();
    }

    Parameter* getParamById(clap_id id) const {
        if (!mIdToParameter.contains(id)) {
            return nullptr;
        }
        return mIdToParameter.at(id);
    }

    Parameter* getParamByIndex(size_t index) const noexcept {
        if (index >= mParameters.size()) {
            return nullptr;
        }
        return mParameters[index].get();
    }

    const std::vector<std::unique_ptr<Parameter>>& getParams() const { return mParameters; }

private:
    std::vector<std::unique_ptr<Parameter>> mParameters;
    std::unordered_map<clap_id, Parameter*> mIdToParameter;
};

} // namespace stfefane::params