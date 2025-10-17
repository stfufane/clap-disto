#pragma once

#include "clap/ext/params.h"
#include "ParamValueType.h"

#include <atomic>
#include <functional>
#include <memory>
#include <vector>

namespace stfefane::params {

class IParameterListener;

class Parameter {
public:
    explicit Parameter(const clap_param_info& info, std::unique_ptr<ParamValueType> value_type, size_t index) :
         mIndex(index), mInfo(info), mValueType(std::move(value_type)), mValue(mInfo.default_value) {}
    Parameter() = delete;
    Parameter(const Parameter &) = delete;
    Parameter(Parameter &&) = delete;

    [[nodiscard]] double getValue() const noexcept { return mValue.load(std::memory_order_relaxed); }
    void setValue(double value);

    void notifyAllParamListeners() const noexcept;

    [[nodiscard]] const clap_param_info& getInfo() const noexcept { return mInfo; }
    [[nodiscard]] const ParamValueType& getValueType() const noexcept { return *mValueType; }

    [[nodiscard]] bool isStepped() const noexcept { return mInfo.flags & CLAP_PARAM_IS_STEPPED; }
    [[nodiscard]] size_t nbSteps() const noexcept;

    void addListener(IParameterListener* listener);
    void removeListener(IParameterListener* listener);

private:
    size_t mIndex = -1;
    clap_param_info mInfo;

    std::unique_ptr<ParamValueType> mValueType;
    std::atomic<double> mValue = 0.;

    std::vector<IParameterListener*> mListeners;
};

class IParameterListener {
public:
    explicit IParameterListener(Parameter* param) : mParam(param) {
        if (mParam) {
            mParam->addListener(this);
        }
    }

    // Copy constructor: register this new listener with the same parameter
    IParameterListener(const IParameterListener& other) : mParam(other.mParam) {
        if (mParam) {
            mParam->addListener(this);
        }
    }

    // Move constructor: transfer the registration from 'other' to 'this'
    IParameterListener(IParameterListener&& other) noexcept : mParam(other.mParam) {
        if (mParam) {
            // Replace other's registration with this one
            mParam->removeListener(&other);
            mParam->addListener(this);
        }
        other.mParam = nullptr;
    }

    // Copy assignment: rewire this listener to other's parameter
    IParameterListener& operator=(const IParameterListener& other) {
        if (this == &other) return *this;
        if (mParam) {
            mParam->removeListener(this);
        }
        mParam = other.mParam;
        if (mParam) {
            mParam->addListener(this);
        }
        return *this;
    }

    // Move assignment: transfer registration from other to this
    IParameterListener& operator=(IParameterListener&& other) noexcept {
        if (this == &other) return *this;
        if (mParam) {
            mParam->removeListener(this);
        }
        mParam = other.mParam;
        if (mParam) {
            mParam->removeListener(&other);
            mParam->addListener(this);
        }
        other.mParam = nullptr;
        return *this;
    }

    virtual ~IParameterListener() {
        if (mParam) {
            mParam->removeListener(this);
        }
    }

    virtual void onParameterUpdated(double new_value) = 0;
protected:
    Parameter* mParam = nullptr;
};

class ParameterAttachment : public IParameterListener {
    using UpdateCallback = std::function<void(Parameter*, double)>;
public:
    explicit ParameterAttachment(Parameter* param, UpdateCallback update_callback)
        : IParameterListener(param), mUpdateCallback(std::move(update_callback)) {}
    void onParameterUpdated(double new_value) override {
        if (mUpdateCallback) {
            mUpdateCallback(mParam, new_value);
        }
    }
private:
    UpdateCallback mUpdateCallback;
};

}