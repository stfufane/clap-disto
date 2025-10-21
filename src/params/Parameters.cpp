#include "Parameters.h"

#include <clap/ext/params.h>

namespace stfefane::params {

void Parameters::addParameter(clap_id id, const std::string& name, std::unique_ptr<ParamValueType> value_type) {
    auto new_param = std::make_unique<Parameter>(id, name, std::move(value_type), mParameters.size());
    auto* param_ptr = new_param.get();
    auto inserted = mIdToParameter.insert_or_assign(id, param_ptr);
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

}