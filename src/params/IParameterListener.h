#pragma once

#include <functional>

namespace stfefane::params {
class Parameter;

class IParameterListener {
public:
    explicit IParameterListener(Parameter* param);
    virtual ~IParameterListener();

    // Copy constructor: register this new listener with the same parameter
    IParameterListener(const IParameterListener& other);

    // Move constructor: transfer the registration from 'other' to 'this'
    IParameterListener(IParameterListener&& other) noexcept;

    // Copy assignment: rewire this listener to other's parameter
    IParameterListener& operator=(const IParameterListener& other);

    // Move assignment: transfer registration from other to this
    IParameterListener& operator=(IParameterListener&& other) noexcept;

    virtual void onParameterUpdated(double new_value) = 0;
protected:
    Parameter* mParam = nullptr;
};

class ParameterAttachment : public IParameterListener {
    using UpdateCallback = std::function<void(Parameter*, double)>;
public:
    explicit ParameterAttachment(Parameter* param, UpdateCallback update_callback)
        : IParameterListener(param), mUpdateCallback(std::move(update_callback)) {}

    void onParameterUpdated(double new_value) override;

private:
    UpdateCallback mUpdateCallback;
};

}