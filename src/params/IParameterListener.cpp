#include "IParameterListener.h"

#include "Parameter.h"

namespace stfefane::params {

IParameterListener::IParameterListener(Parameter* param) : mParam(param) {
    if (mParam) {
        mParam->addListener(this);
    }
}

IParameterListener::~IParameterListener() {
    if (mParam) {
        mParam->removeListener(this);
    }
}

IParameterListener::IParameterListener(const IParameterListener& other) : mParam(other.mParam) {
    if (mParam) {
        mParam->addListener(this);
    }
}

IParameterListener::IParameterListener(IParameterListener&& other) noexcept : mParam(other.mParam) {
    if (mParam) {
        // Replace other's registration with this one
        mParam->removeListener(&other);
        mParam->addListener(this);
    }
    other.mParam = nullptr;
}

IParameterListener& IParameterListener::operator=(const IParameterListener& other) {
    if (this == &other) {
        return *this;
    }
    if (mParam) {
        mParam->removeListener(this);
    }
    mParam = other.mParam;
    if (mParam) {
        mParam->addListener(this);
    }
    return *this;
}

IParameterListener& IParameterListener::operator=(IParameterListener&& other) noexcept {
    if (this == &other) {
        return *this;
    }
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


void ParameterAttachment::onParameterUpdated(double new_value) {
    if (mUpdateCallback) {
        mUpdateCallback(mParam, new_value);
    }
}

} // namespace stfefane::params
