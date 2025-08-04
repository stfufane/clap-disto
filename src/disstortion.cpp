#include "disstortion.h"

#include <iostream>
#include <clap/helpers/host-proxy.hh>
#include <clap/helpers/plugin.hh>
#include <clap/helpers/plugin.hxx>
#include <nlohmann/json.hpp>

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

clap_process_status Disstortion::process(const clap_process* process) noexcept {
    // process parameters
    const auto* in_events = process->in_events;
    const auto event_count = in_events->size(in_events);
    for (uint32_t i = 0; i < event_count; ++i) {
        const auto* event = in_events->get(in_events, i);
        if (event->space_id == CLAP_CORE_EVENT_SPACE_ID && event->type == CLAP_EVENT_PARAM_VALUE) {
            auto* param_event = reinterpret_cast<const clap_event_param_value*>(event);
            if (mParameters.isValidParamId(param_event->param_id)) {
                *mParameters.getParamToValue(param_event->param_id) = param_event->value;
            }
        }
    }

    mDriveProcessor.setDrive(*mParameters.getParamToValue(params::eDrive));

    // process audio
    if (process->audio_outputs_count <= 0) {
        return CLAP_PROCESS_CONTINUE;
    }

    auto in = choc::buffer::createChannelArrayView(process->audio_inputs->data32,
                                         process->audio_inputs->channel_count, process->frames_count);
    auto out = choc::buffer::createChannelArrayView(process->audio_outputs->data32,
                                         process->audio_outputs->channel_count, process->frames_count);
    choc::buffer::copy(out, in);
    mDriveProcessor.process(out);

    return CLAP_PROCESS_CONTINUE;
}

bool Disstortion::isValidParamId(clap_id paramId) const noexcept {
    return mParameters.isValidParamId(paramId);
}

bool Disstortion::paramsValue(clap_id paramId, double *value) noexcept {
    *value = *mParameters.getParamToValue(paramId);
    return true;
}

bool Disstortion::audioPortsInfo(uint32_t index, bool isInput, clap_audio_port_info *info) const noexcept {
    if (index != 0) return false;

    if (isInput) {
        // Input port configuration
        info->id = 0;
        strcpy_s(info->name, "Audio Input");
        info->flags = CLAP_AUDIO_PORT_IS_MAIN;
        info->channel_count = 2;
        info->port_type = CLAP_PORT_STEREO;
        info->in_place_pair = 0; // Paired with output port 0 for in-place processing
    } else {
        // Output port configuration
        info->id = 0;
        strcpy_s(info->name, "Audio Output");
        info->flags = CLAP_AUDIO_PORT_IS_MAIN;
        info->channel_count = 2;
        info->port_type = CLAP_PORT_STEREO;
        info->in_place_pair = 0; // Paired with input port 0 for in-place processing
    }

    return true;
}

bool Disstortion::stateSave(const clap_ostream *stream) noexcept {
    if (!stream || !stream->write) {
        return false;
    }

    nlohmann::json j;

    // Store all parameters in the JSON object
    j["state_version"] = PROJECT_VERSION;

    const auto jsonStr = j.dump();

    // CLAP streams may have size limitations, so we need to write in chunks
    const auto *buffer = jsonStr.data();

    auto remaining = jsonStr.size();
    while (remaining > 0) {
        // Try to write remaining bytes
        const auto written = stream->write(stream, buffer + (jsonStr.size() - remaining), remaining);

        if (written < 0) {
            return false; // Write error occurred
        }

        remaining -= written;
    }

    return true;
}

bool Disstortion::stateLoad(const clap_istream *stream) noexcept {
    if (!stream || !stream->read) {
        return false;
    }

    // Read the JSON string from the stream in chunks
    constexpr auto chunkSize = 4096;
    std::vector<char> buffer;
    char chunk[chunkSize];

    while (true) {
        int64_t bytesRead = stream->read(stream, chunk, chunkSize);
        if (bytesRead < 0) {
            return false; // Read error
        }
        if (bytesRead == 0) {
            break; // End of stream
        }
        buffer.insert(buffer.end(), chunk, chunk + bytesRead);
    }

    buffer.push_back('\0'); // Ensure buffer is null-terminated

    try {
        nlohmann::json j = nlohmann::json::parse(buffer.data());

        auto state_version = j["state_version"].get<std::string>();

        return state_version == PROJECT_VERSION;
    } catch (std::exception &e) {
        std::cerr << "Disstortion: Failed to load state: " << e.what() << std::endl;
        return false;
    }
}

#ifdef __linux__
void ClapPlugin::onPosixFd(int fd, clap_posix_fd_flags_t flags) noexcept {
    if (mVisageApp && mVisageApp->window()) {
        mVisageApp->window()->processPluginFdEvents();
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
    if (mVisageApp && mVisageApp->window() && _host.canUsePosixFdSupport()) {
        _host.posixFdSupportUnregister(mVisageApp->window()->posixFd());
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
    if (_host.canUsePosixFdSupport() && mVisageApp->window()) {
        int fd_flags = CLAP_POSIX_FD_READ | CLAP_POSIX_FD_WRITE | CLAP_POSIX_FD_ERROR;
        return _host.posixFdSupportRegister(mVisageApp->window()->posixFd(), fd_flags);
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
    return mVisageApp->width();
#else
    return mVisageApp->nativeWidth();
#endif
}

int Disstortion::pluginHeight() const {
    if (mVisageApp == nullptr) {
        return 0;
    }
#if __APPLE__
    return mVisageApp->height();
#else
    return mVisageApp->nativeHeight();
#endif
}

void Disstortion::setPluginDimensions(int width, int height) const {
    if (mVisageApp == nullptr) {
        return;
    }
#if __APPLE__
    mVisageApp->setWindowDimensions(width, height);
#else
    mVisageApp->setNativeWindowDimensions(width, height);
#endif
}

} // namespace stfefane
