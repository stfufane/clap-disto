#include "disstortion.h"

#include <clap/helpers/host-proxy.hh>
#include <clap/helpers/plugin.hh>
#include <clap/helpers/plugin.hxx>

using namespace visage::dimension;

namespace stfefane {

static const char *kClapFeatures[] = {CLAP_PLUGIN_FEATURE_AUDIO_EFFECT, nullptr};

clap_plugin_descriptor Disstortion::descriptor = {CLAP_VERSION,           "dev.stephanealbanese.disstortion",
                                                  "Disstortion",          "Stfefane",
                                                  "stephanealbanese.dev", "stephanealbanese.dev",
                                                  "stephanealbanese.dev", "0.0.1",
                                                  "Disstortion Plugin",   kClapFeatures};

Disstortion::Disstortion(const clap_host *host) : ClapPluginBase(&descriptor, host) {
}

Disstortion::~Disstortion() = default;

#ifdef __linux__
void ClapPlugin::onPosixFd(int fd, clap_posix_fd_flags_t flags) noexcept {
    if (app_ && app_->window()) {
        app_->window()->processPluginFdEvents();
    }
}
#endif

bool Disstortion::guiIsApiSupported(const char *api, bool is_floating) noexcept {
    if (is_floating) {
        return false;
    }
#ifdef _WIN32
    if (strcmp(api, CLAP_WINDOW_API_WIN32) == 0) {
        return true;
    }
#elif __APPLE__
    if (strcmp(api, CLAP_WINDOW_API_COCOA) == 0) {
        return true;
    }
#elif __linux__
    if (strcmp(api, CLAP_WINDOW_API_X11) == 0) {
        return true;
    }
#endif
    return false;
}

bool Disstortion::guiCreate(const char *api, bool is_floating) noexcept {
    if (is_floating) {
        return false;
    }
    if (mVisageApp) {
        return true;
    }
    mVisageApp = std::make_unique<visage::ApplicationWindow>();
    mVisageApp->setWindowDimensions(80_vmin, 60_vmin);
    mVisageApp->onDraw() = [this](visage::Canvas &canvas) {
        canvas.setColor(0xff000066);
        canvas.fill(0, 0, mVisageApp->width(), mVisageApp->height());
        float circle_radius = mVisageApp->height() * 0.1f;
        float x = mVisageApp->width() * 0.5f - circle_radius;
        float y = mVisageApp->height() * 0.5f - circle_radius;
        canvas.setColor(0xff00ffff);
        canvas.circle(x, y, 2.0f * circle_radius);
    };
    mVisageApp->onWindowContentsResized() = [this] { _host.guiRequestResize(pluginWidth(), pluginHeight()); };
    return true;
}

void Disstortion::guiDestroy() noexcept {
#if __linux__
    if (app_ && app_->window() && _host.canUsePosixFdSupport()) {
        _host.posixFdSupportUnregister(app_->window()->posixFd());
    }
#endif
    mVisageApp->close();
}

bool Disstortion::guiSetParent(const clap_window *window) noexcept {
    if (mVisageApp == nullptr) {
        return false;
    }
    mVisageApp->show(window->ptr);
#if __linux__
    if (_host.canUsePosixFdSupport() && app_->window()) {
        int fd_flags = CLAP_POSIX_FD_READ | CLAP_POSIX_FD_WRITE | CLAP_POSIX_FD_ERROR;
        return _host.posixFdSupportRegister(app_->window()->posixFd(), fd_flags);
    }
#endif
    return true;
}

bool Disstortion::guiGetResizeHints(clap_gui_resize_hints_t *hints) noexcept {
    if (mVisageApp == nullptr) {
        return false;
    }
    bool fixed_aspect_ratio = mVisageApp->isFixedAspectRatio();
    hints->can_resize_horizontally = true;
    hints->can_resize_vertically = true;
    hints->preserve_aspect_ratio = fixed_aspect_ratio;
    if (fixed_aspect_ratio) {
        hints->aspect_ratio_width = mVisageApp->height() * mVisageApp->aspectRatio();
        hints->aspect_ratio_height = mVisageApp->width();
    }
    return true;
}

bool Disstortion::guiAdjustSize(uint32_t *width, uint32_t *height) noexcept {
    if (mVisageApp == nullptr) {
        return false;
    }
    mVisageApp->adjustWindowDimensions(width, height, true, true);
    return true;
}

bool Disstortion::guiSetSize(uint32_t width, uint32_t height) noexcept {
    if (mVisageApp == nullptr) {
        return false;
    }
    setPluginDimensions(width, height);
    return true;
}

bool Disstortion::guiGetSize(uint32_t *width, uint32_t *height) noexcept {
    if (mVisageApp == nullptr) {
        return false;
    }
    *width = pluginWidth();
    *height = pluginHeight();
    return true;
}

int Disstortion::pluginWidth() const {
    if (mVisageApp == nullptr) {
        return 0;
    }
#if __APPLE__
    return app_->width();
#else
    return mVisageApp->nativeWidth();
#endif
}

int Disstortion::pluginHeight() const {
    if (mVisageApp == nullptr) {
        return 0;
    }
#if __APPLE__
    return app_->height();
#else
    return mVisageApp->nativeHeight();
#endif
}

void Disstortion::setPluginDimensions(int width, int height) const {
    if (mVisageApp == nullptr) {
        return;
    }
#if __APPLE__
    app_->setWindowDimensions(width, height);
#else
    mVisageApp->setNativeWindowDimensions(width, height);
#endif
}
} // namespace stfefane
