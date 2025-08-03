#pragma once

#include <clap/helpers/plugin.hh>
#include <visage/app.h>

namespace stfefane {

using ClapPluginBase =
    clap::helpers::Plugin<clap::helpers::MisbehaviourHandler::Terminate, clap::helpers::CheckingLevel::Maximal>;

class Disstortion : public ClapPluginBase {
public:
    static clap_plugin_descriptor descriptor;

    explicit Disstortion(const clap_host *host);
    ~Disstortion() override;

#ifdef __linux__
    bool implementsPosixFdSupport() const noexcept override { return true; }
    void onPosixFd(int fd, clap_posix_fd_flags_t flags) noexcept override;
#endif

protected:
    [[nodiscard]] bool implementsGui() const noexcept override { return true; }

    bool guiIsApiSupported(const char *api, bool is_floating) noexcept override;
    bool guiCreate(const char *api, bool is_floating) noexcept override;
    void guiDestroy() noexcept override;
    bool guiSetParent(const clap_window *window) noexcept override;
    bool guiSetScale(double scale) noexcept override { return false; }
    [[nodiscard]] bool guiCanResize() const noexcept override { return true; }
    bool guiGetResizeHints(clap_gui_resize_hints_t *hints) noexcept override;
    bool guiAdjustSize(uint32_t *width, uint32_t *height) noexcept override;
    bool guiSetSize(uint32_t width, uint32_t height) noexcept override;
    bool guiGetSize(uint32_t *width, uint32_t *height) noexcept override;

private:
    [[nodiscard]] int pluginWidth() const;
    [[nodiscard]] int pluginHeight() const;
    void setPluginDimensions(int width, int height) const;

    std::unique_ptr<visage::ApplicationWindow> mVisageApp;
};
} // namespace stfefane