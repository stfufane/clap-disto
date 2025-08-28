#pragma once

#include "clap/ext/params.h"
#include "ParamValueType.h"

#include <memory>
#include <vector>

namespace stfefane::params {

class IParameterUIListener;

class Parameter {
public:
    explicit Parameter(const clap_param_info& info, std::unique_ptr<ParamValueType> value_type, size_t index) :
         mIndex(index), mInfo(info), mValueType(std::move(value_type)), mValue(mInfo.default_value) {}
    Parameter() = delete;
    Parameter(const Parameter &) = delete;
    Parameter(Parameter &&) = delete;

    [[nodiscard]] double getValue() const noexcept { return mValue; }
    void setValue(double value);

    [[nodiscard]] const clap_param_info& getInfo() const noexcept { return mInfo; }
    [[nodiscard]] const ParamValueType& getValueType() const noexcept { return *mValueType; }

    [[nodiscard]] bool isStepped() const noexcept { return mInfo.flags & CLAP_PARAM_IS_STEPPED; }
    [[nodiscard]] size_t nbSteps() const noexcept;

    void addUIListener(IParameterUIListener* listener);
    void removeUIListener(IParameterUIListener* listener);

private:
    size_t mIndex = -1;
    clap_param_info mInfo;

    std::unique_ptr<ParamValueType> mValueType;
    double mValue = 0.;

    std::vector<IParameterUIListener*> mUIListeners;
};

class IParameterUIListener {
public:
    explicit IParameterUIListener(Parameter* param) : mParam(param) {
        if (mParam) {
            mParam->addUIListener(this);
        }
    }

    virtual ~IParameterUIListener() {
        if (mParam) {
            mParam->removeUIListener(this);
        }
    }

    virtual void onParameterUpdated(double new_value) = 0;
protected:
    Parameter* mParam = nullptr;
};

}