#pragma once

#include "dsp/CubicDrive.h"
#include "params.h"

#include <clap/helpers/plugin.hh>
#include <visage/app.h>

namespace stfefane {

using ClapPluginBase =
    clap::helpers::Plugin<clap::helpers::MisbehaviourHandler::Terminate, clap::helpers::CheckingLevel::Maximal>;

class Disstortion : public ClapPluginBase {
public:
    static clap_plugin_descriptor descriptor;

    explicit Disstortion(const clap_host* host);
    ~Disstortion() override = default;

#ifdef __linux__
    bool implementsPosixFdSupport() const noexcept override { return true; }
    void onPosixFd(int fd, clap_posix_fd_flags_t flags) noexcept override;
#endif

protected:
    clap_process_status process(const clap_process* process) noexcept override;

    /**
     * @name params related methods
     * @{
     */
    [[nodiscard]] bool implementsParams() const noexcept override { return true; }
    [[nodiscard]] bool isValidParamId(clap_id paramId) const noexcept override;
    [[nodiscard]] uint32_t paramsCount() const noexcept override { return params::nb_params; }
    bool paramsInfo(uint32_t paramIndex, clap_param_info* info) const noexcept override {
        return params::Parameters::getParamInfo(paramIndex, info);
    }
    bool paramsValue(clap_id paramId, double* value) noexcept override;
    // bool paramsValueToText(clap_id paramId, double value, char *display,
    //                        uint32_t size) noexcept override;
    // bool paramsTextToValue(clap_id paramId, const char *display, double *value) noexcept override;

    /**
     * @name audio ports related methods
     * @{
     */
    [[nodiscard]] bool implementsAudioPorts() const noexcept override { return true; }
    [[nodiscard]] uint32_t audioPortsCount(bool /* isInput */) const noexcept override { return 1; }
    bool audioPortsInfo(uint32_t index, bool isInput, clap_audio_port_info* info) const noexcept override;
    /** @} */

    /**
     * @name state related methods
     * @{
     */
    [[nodiscard]] bool implementsState() const noexcept override { return true; }
    bool stateSave(const clap_ostream* stream) noexcept override;
    bool stateLoad(const clap_istream* stream) noexcept override;
    /** @} */

    /**
     * @name GUI related methods
     * @{
     */
    [[nodiscard]] bool implementsGui() const noexcept override { return true; }

    bool guiIsApiSupported(const char* api, bool is_floating) noexcept override;
    bool guiCreate(const char* api, bool is_floating) noexcept override;
    void guiDestroy() noexcept override;
    bool guiSetParent(const clap_window* window) noexcept override;
    bool guiSetScale(double scale) noexcept override { return false; }
    [[nodiscard]] bool guiCanResize() const noexcept override { return true; }
    bool guiGetResizeHints(clap_gui_resize_hints_t* hints) noexcept override;
    bool guiAdjustSize(uint32_t* width, uint32_t* height) noexcept override;
    bool guiSetSize(uint32_t width, uint32_t height) noexcept override;
    bool guiGetSize(uint32_t* width, uint32_t* height) noexcept override;
    /** @} */

private:
    [[nodiscard]] int pluginWidth() const;
    [[nodiscard]] int pluginHeight() const;
    void setPluginDimensions(int width, int height) const;

    std::unique_ptr<visage::ApplicationWindow> mVisageApp;
    params::Parameters mParameters;

    dsp::CubicDrive mDriveProcessor;
};
} // namespace stfefane