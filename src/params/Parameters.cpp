#include "Parameters.h"

#include <clap/ext/params.h>

namespace stfefane::params {

void Parameters::addParameter(clap_id id, const std::string& name, uint32_t flags, std::unique_ptr<ParamValueType> value_type) {
    clap_param_info param_info {
        .id = id,
        .flags = flags,
        .min_value = value_type->mMin,
        .max_value = value_type->mMax,
        .default_value = value_type->mDefault,
    };
    snprintf(param_info.name, sizeof(param_info.name), "%s", name.c_str());

    auto new_param = std::make_unique<Parameter>(param_info, std::move(value_type), mParameters.size());
    auto* param_ptr = new_param.get();
    auto inserted = mIdToParameter.insert_or_assign(param_info.id, param_ptr);
    if (!inserted.second) {
        throw std::logic_error("same parameter id was inserted twice -> " + std::to_string(param_info.id));
    }
    param_info.cookie = param_ptr;
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

}