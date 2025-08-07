#pragma once

#include <clap/ext/params.h>
#include <clap/id.h>
#include <cstdint>
#include <unordered_map>

namespace stfefane::params {

enum param_ids : uint32_t {
    eDrive,
    eGain,
    eCutoff,
    eNbParams
};
static constexpr uint32_t nb_params = eNbParams;

class Parameters {
public:
    Parameters() {
        // Init param values map
        mParamToValue[eDrive] = &mDrive;
        mParamToValue[eGain] = &mGain;
        mParamToValue[eCutoff] = &mCutoff;
    }

    [[nodiscard]] bool isValidParamId(const clap_id param_id) const { return mParamToValue.contains(param_id); }
    [[nodiscard]] double* getParamToValue(const clap_id param_id) const { return mParamToValue.at(param_id); }

    static bool getParamInfo(uint32_t paramIndex, clap_param_info* info) {
        if (paramIndex >= nb_params) {
            return false;
        }
        info->flags = CLAP_PARAM_IS_AUTOMATABLE;

        switch (paramIndex) {
        case eDrive:
            info->id = eDrive;
            strncpy(info->name, "Drive", CLAP_NAME_SIZE);
            info->min_value = 0.;
            info->max_value = 1.;
            info->default_value = .5;
            break;
        case eGain:
            info->id = eGain;
            strncpy(info->name, "Gain", CLAP_NAME_SIZE);
            info->min_value = 0.;
            info->max_value = 1.;
            info->default_value = .5;
            break;
        case eCutoff:
            info->id = eCutoff;
            strncpy(info->name, "Cutoff", CLAP_NAME_SIZE);
            info->min_value = 20.;
            info->max_value = 20000.;
            info->default_value = 500.;
            break;
        default:
            return false;
        }
        return true;
    }

private:
    double mDrive{.5};
    double mGain{.5};
    double mCutoff{500.};

    std::unordered_map<clap_id, double*> mParamToValue;
};

} // namespace stfefane::params