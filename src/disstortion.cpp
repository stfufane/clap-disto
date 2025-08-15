#include "disstortion.h"

#include <clap/helpers/host-proxy.hh>
#include <clap/helpers/plugin.hh>
#include <clap/helpers/plugin.hxx>
#include <iostream>
#include <nlohmann/json.hpp>

namespace stfefane {

static const char* kClapFeatures[] = {CLAP_PLUGIN_FEATURE_AUDIO_EFFECT, nullptr};

clap_plugin_descriptor Disstortion::descriptor = {CLAP_VERSION,           "dev.stephanealbanese.disstortion",
                                                  "Disstortion",          "Stfefane",
                                                  "stephanealbanese.dev", "stephanealbanese.dev",
                                                  "stephanealbanese.dev", "0.0.1",
                                                  "Disstortion Plugin",   kClapFeatures};

Disstortion::Disstortion(const clap_host* host) : ClapPluginBase(&descriptor, host) {
    mParameters.addParameter(params::eDrive, "Drive", CLAP_PARAM_IS_AUTOMATABLE, { 0., 1., .1});
    mParameters.addParameter(params::eGain, "Gain", CLAP_PARAM_IS_AUTOMATABLE, { 0., 1., .5});
    mParameters.addParameter(params::eCutoff, "Cutoff", CLAP_PARAM_IS_AUTOMATABLE, { 20., 20000., 4000. });
}

clap_process_status Disstortion::process(const clap_process* process) noexcept {
    processEvents(process->in_events);
    updateParameters();

    // process audio
    if (process->audio_outputs_count <= 0) {
        return CLAP_PROCESS_CONTINUE;
    }

    auto in = choc::buffer::createChannelArrayView(process->audio_inputs->data32, process->audio_inputs->channel_count,
                                                   process->frames_count);
    auto out = choc::buffer::createChannelArrayView(process->audio_outputs->data32,
                                                    process->audio_outputs->channel_count, process->frames_count);
    choc::buffer::copy(out, in);
    mDriveProcessor.process(out);

    return CLAP_PROCESS_CONTINUE;
}

void Disstortion::processEvents(const clap_input_events* in_events) const {
    // process parameters
    const auto event_count = in_events->size(in_events);
    for (uint32_t i = 0; i < event_count; ++i) {
        const auto* event = in_events->get(in_events, i);
        if (event->space_id == CLAP_CORE_EVENT_SPACE_ID && event->type == CLAP_EVENT_PARAM_VALUE) {
            auto* param_event = reinterpret_cast<const clap_event_param_value*>(event);
            if (mParameters.isValidParamId(param_event->param_id)) {
                mParameters.getParamById(param_event->param_id)->setValue(param_event->value);
            }
        }
    }
}

void Disstortion::updateParameters() {
    mDriveProcessor.setDrive(mParameters.getParamValue(params::eDrive));
}

bool Disstortion::isValidParamId(clap_id paramId) const noexcept {
    return mParameters.isValidParamId(paramId);
}

void Disstortion::paramsFlush(const clap_input_events* in, const clap_output_events* out) noexcept {
    processEvents(in);
    // TODO: handle out for UI.
}

bool Disstortion::paramsInfo(uint32_t paramIndex, clap_param_info* info) const noexcept {
    const auto* param = mParameters.getParamByIndex(paramIndex);
    if (param == nullptr) {
        return false;
    }
    *info = param->getInfo();
    return true;
}

bool Disstortion::paramsValue(clap_id paramId, double* value) noexcept {
    *value = mParameters.getParamValue(paramId);
    return true;
}

bool Disstortion::audioPortsInfo(uint32_t index, bool isInput, clap_audio_port_info* info) const noexcept {
    if (index != 0) {
        return false;
    }

    if (isInput) {
        // Input port configuration
        info->id = 0;
        snprintf(info->name, sizeof(info->name), "%s", "Audio Input");
        info->flags = CLAP_AUDIO_PORT_IS_MAIN;
        info->channel_count = 2;
        info->port_type = CLAP_PORT_STEREO;
        info->in_place_pair = 0; // Paired with output port 0 for in-place processing
    } else {
        // Output port configuration
        info->id = 0;
        snprintf(info->name, sizeof(info->name), "%s", "Audio Output");
        info->flags = CLAP_AUDIO_PORT_IS_MAIN;
        info->channel_count = 2;
        info->port_type = CLAP_PORT_STEREO;
        info->in_place_pair = 0; // Paired with input port 0 for in-place processing
    }

    return true;
}

bool Disstortion::stateSave(const clap_ostream* stream) noexcept {
    if (!stream || !stream->write) {
        return false;
    }

    nlohmann::json j;

    // Store all parameters in the JSON object
    j["state_version"] = PROJECT_VERSION;
    for (const auto& param: mParameters.getParams()) {
        j[param->getInfo().name] = param->getValue();
    }

    const auto jsonStr = j.dump();

    // CLAP streams may have size limitations, so we need to write in chunks
    const auto* buffer = jsonStr.data();

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

bool Disstortion::stateLoad(const clap_istream* stream) noexcept {
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

    // No state to load but I guess that's ok
    if (buffer.empty()) {
        return false;
    }

    buffer.push_back('\0'); // Ensure buffer is null-terminated

    try {
        nlohmann::json j = nlohmann::json::parse(buffer.data());

        const auto state_version = j["state_version"].get<std::string>();
        if (state_version == PROJECT_VERSION) {
            for (const auto& param: mParameters.getParams()) {
                param->setValue(j[param->getInfo().name].get<double>());
            }
        } else {
            // TODO: handle changes between versions.
        }

        return true;
    } catch (std::exception& e) {
        std::cerr << "Disstortion: Failed to load state: " << e.what() << std::endl;
        return false;
    }
}

#ifdef __linux__
void Disstortion::onPosixFd(int fd, clap_posix_fd_flags_t flags) noexcept {
    if (mEditor && mEditor->window()) {
        mEditor->window()->processPluginFdEvents();
    }
}
#endif

bool Disstortion::guiIsApiSupported(const char* api, bool is_floating) noexcept {
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

bool Disstortion::guiCreate(const char* api, bool is_floating) noexcept {
    if (is_floating) {
        return false;
    }
    if (mEditor) {
        return true;
    }
    mEditor = std::make_unique<gui::DisstortionEditor>(*this);
    mEditor->onWindowContentsResized() = [this] {
        _host.guiRequestResize(mEditor->pluginWidth(), mEditor->pluginHeight());
    };
    return true;
}

void Disstortion::guiDestroy() noexcept {
#if __linux__
    if (mEditor && mEditor->window() && _host.canUsePosixFdSupport()) {
        _host.posixFdSupportUnregister(mEditor->window()->posixFd());
    }
#endif
    mEditor->close();
}

bool Disstortion::guiSetParent(const clap_window* window) noexcept {
    if (mEditor == nullptr) {
        return false;
    }
    mEditor->show(window->ptr);
#if __linux__
    if (_host.canUsePosixFdSupport() && mEditor->window()) {
        int fd_flags = CLAP_POSIX_FD_READ | CLAP_POSIX_FD_WRITE | CLAP_POSIX_FD_ERROR;
        return _host.posixFdSupportRegister(mEditor->window()->posixFd(), fd_flags);
    }
#endif
    return true;
}

bool Disstortion::guiGetResizeHints(clap_gui_resize_hints_t* hints) noexcept {
    if (mEditor == nullptr) {
        return false;
    }
    bool fixed_aspect_ratio = mEditor->isFixedAspectRatio();
    hints->can_resize_horizontally = false;
    hints->can_resize_vertically = false;
    hints->preserve_aspect_ratio = fixed_aspect_ratio;
    if (fixed_aspect_ratio) {
        hints->aspect_ratio_width = mEditor->height() * mEditor->aspectRatio();
        hints->aspect_ratio_height = mEditor->width();
    }
    return true;
}

bool Disstortion::guiAdjustSize(uint32_t* width, uint32_t* height) noexcept {
    if (mEditor == nullptr) {
        return false;
    }
    mEditor->adjustWindowDimensions(width, height, false, false);
    return true;
}

bool Disstortion::guiSetSize(uint32_t width, uint32_t height) noexcept {
    if (mEditor == nullptr) {
        return false;
    }
    mEditor->setPluginDimensions(width, height);
    return true;
}

bool Disstortion::guiGetSize(uint32_t* width, uint32_t* height) noexcept {
    if (mEditor == nullptr) {
        return false;
    }
    *width = mEditor->pluginWidth();
    *height = mEditor->pluginHeight();
    return true;
}

} // namespace stfefane
