#include "disstortion.h"

#include "helpers/Logger.h"

#include <clap/helpers/host-proxy.hh>
#include <clap/helpers/plugin.hh>
#include <clap/helpers/plugin.hxx>
#include <nlohmann/json.hpp>

namespace stfefane {

static const char* kClapFeatures[] = {CLAP_PLUGIN_FEATURE_AUDIO_EFFECT, nullptr};

clap_plugin_descriptor Disstortion::descriptor = {CLAP_VERSION,
                                                  "dev.stephanealbanese.disstortion" PLUGIN_ID_SUFFIX,
                                                  "Disstortion" PLUGIN_ID_SUFFIX,
                                                  "Stfefane",
                                                  "stephanealbanese.dev",
                                                  "stephanealbanese.dev",
                                                  "stephanealbanese.dev",
                                                  PROJECT_VERSION,
                                                  "Disstortion Plugin",
                                                  kClapFeatures};

Disstortion::Disstortion(const clap_host* host) : ClapPluginBase(&descriptor, host) {
    LOG_INFO("dsp", "[Disstortion::constructor]");

    mParameters.addParameter(params::eDrive, "Drive", std::make_unique<params::DecibelValueType>(.3, 0., dsp::kMaxDriveDb));
    mParameters.addParameter(params::eDriveType, "Drive Type",
                             std::make_unique<params::SteppedValueType>(
                                 std::vector<std::string>({"Cubic Saturation", "Tube Saturation", "Asymmetric Clip", "Foldback",
                                                           "Bitcrush", "Waveshaper", "Tube Screamer", "Fuzz"}),
                                 0.));
    mParameters.addParameter(params::eInGain, "Input Gain",
                             std::make_unique<params::DecibelValueType>(0.3333333333333333, -12., 24.));
    mParameters.addParameter(params::eOutGain, "Output Gain", std::make_unique<params::DecibelValueType>(.8, -24., 6.));
    mParameters.addParameter(params::ePreFilterOn, "Pre Filter On", std::make_unique<params::BooleanValueType>(true));
    mParameters.addParameter(params::ePreFilterFreq, "Pre Filter",
                             std::make_unique<params::ParamValueType>(20., 20000., 10000., " Hz"));
    mParameters.addParameter(params::ePostFilterOn, "Post Filter On", std::make_unique<params::BooleanValueType>(true));
    mParameters.addParameter(params::ePostFilterFreq, "Post Filter",
                             std::make_unique<params::ParamValueType>(20., 20000., 80., " Hz"));
    mParameters.addParameter(params::eAsymmetry, "Asymmetry",
                             std::make_unique<params::ParamValueType>(-0.5, 0.5, 0., std::string()));
    mParameters.addParameter(params::eBias, "Bias", std::make_unique<params::ParamValueType>(-0.3, 0.3, 0., std::string()));
    mParameters.addParameter(params::eMix, "Mix", std::make_unique<params::ParamPercentValueType>(.5));

    std::ranges::for_each(mDistoProcessors, [&](auto& proc) { proc.initParameterAttachments(*this); });
}

bool Disstortion::activate(double sampleRate, uint32_t, uint32_t) noexcept {
    LOG_INFO("dsp", "[Disstortion::activate]");
    std::ranges::for_each(mDistoProcessors, [&](auto& proc) { proc.setSampleRate(sampleRate); });
    return true;
}

void Disstortion::reset() noexcept {
    LOG_INFO("dsp", "[Disstortion::reset]");
    std::ranges::for_each(mDistoProcessors, [&](auto& proc) { proc.reset(); });
}

clap_process_status Disstortion::process(const clap_process* process) noexcept {
    // process audio
    if (process->audio_outputs_count <= 0) {
        return CLAP_PROCESS_CONTINUE;
    }

    handleEventsFromUIQueue(process->out_events);
    processEvents(process->in_events);

    const auto* in = process->audio_inputs_count > 0 ? process->audio_inputs : nullptr;
    auto* out = process->audio_outputs;

    const uint32_t in_channels = in ? in->channel_count : 0;
    const uint32_t out_channels = out->channel_count;
    const uint32_t frames = process->frames_count;

    // If no input, output silence
    if (in_channels == 0) {
        for (uint32_t ch = 0; ch < out_channels; ++ch) {
            std::fill_n(out->data32[ch], frames, 0.0f);
        }
        return CLAP_PROCESS_CONTINUE;
    }

    const uint32_t proc_channels = std::min({in_channels, out_channels, static_cast<uint32_t>(mDistoProcessors.size())});

    // Process available channel pairs
    for (uint32_t ch = 0; ch < proc_channels; ++ch) {
        auto &proc = mDistoProcessors[ch];
        float* outCh = out->data32[ch];
        const float* inCh = in->data32[ch];
        for (uint32_t f = 0; f < frames; ++f) {
            outCh[f] = static_cast<float>(proc.process(static_cast<double>(inCh[f])));
        }
    }

    // If mono-in and more outputs, duplicate left to others
    if (in_channels == 1 && out_channels > 1) {
        const float* left = out->data32[0];
        for (uint32_t ch = 1; ch < out_channels; ++ch) {
            std::copy_n(left, frames, out->data32[ch]);
        }
    } else if (out_channels > proc_channels) {
        // Zero any remaining output channels to avoid garbage/noise
        for (uint32_t ch = proc_channels; ch < out_channels; ++ch) {
            std::fill_n(out->data32[ch], frames, 0.0f);
        }
    }

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
                LOG_INFO("param", "Processing event for param {}", getParameter(param_event->param_id)->getInfo().name);
                mParameters.getParamById(param_event->param_id)->setValue(param_event->value);
            }
        }
    }
}

void Disstortion::handleEventsFromUIQueue(const clap_output_events_t* ov) {
    // --- Handle incoming UI events
    UIEvent f{};
    while (mEventsQueue.try_dequeue(f))
    {
        switch (f.type)
        {
        case UIEvent::GESTURE_BEGIN:
        case UIEvent::GESTURE_END:
        {
            auto [header, param_id] = clap_event_param_gesture();
            header.space_id = CLAP_CORE_EVENT_SPACE_ID;
            header.flags = 0;
            header.time = 0;
            header.type = (f.type == UIEvent::GESTURE_BEGIN ? CLAP_EVENT_PARAM_GESTURE_BEGIN : CLAP_EVENT_PARAM_GESTURE_END);
            header.size = sizeof(clap_event_param_gesture);
            param_id = f.id;
            ov->try_push(ov, &header);
            break;
        }
        case UIEvent::ADJUST_VALUE:
        {
            auto evt = clap_event_param_value();
            evt.header.size = sizeof(clap_event_param_value);
            evt.header.type = static_cast<uint16_t>(CLAP_EVENT_PARAM_VALUE);
            evt.header.time = 0; // for now
            evt.header.space_id = CLAP_CORE_EVENT_SPACE_ID;
            evt.header.flags = 0;
            evt.param_id = f.id;
            evt.value = f.value;
            LOG_INFO("param", "Set param {} value from UI event", getParameter(evt.param_id)->getInfo().name);
            mParameters.getParamById(evt.param_id)->setValue(evt.value);
            ov->try_push(ov, &evt.header);
            break;
        }
        }
    }
}

bool Disstortion::isValidParamId(clap_id paramId) const noexcept {
    return mParameters.isValidParamId(paramId);
}

void Disstortion::paramsFlush(const clap_input_events* in, const clap_output_events* out) noexcept {
    processEvents(in);
    handleEventsFromUIQueue(out);
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

bool Disstortion::paramsValueToText(clap_id paramId, double value, char* display, uint32_t size) noexcept {
    const auto& value_type = mParameters.getParamValueType(paramId);
    snprintf(display, size, "%s", value_type.toText(value).c_str());
    return true;
}

bool Disstortion::paramsTextToValue(clap_id paramId, const char* display, double* value) noexcept {
    const auto& value_type = mParameters.getParamValueType(paramId);
    *value = value_type.toValue(display);
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
        info->channel_count = kNbInChannels;
        info->port_type = CLAP_PORT_STEREO;
        info->in_place_pair = 0; // Paired with output port 0 for in-place processing
    } else {
        // Output port configuration
        info->id = 0;
        snprintf(info->name, sizeof(info->name), "%s", "Audio Output");
        info->flags = CLAP_AUDIO_PORT_IS_MAIN;
        info->channel_count = kNbOutChannels;
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
    LOG_DEBUG("param", "[stateSave] -> {}", j.dump(4));

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
        LOG_DEBUG("param", "[stateLoad] -> {}", j.dump(4));

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
    LOG_INFO("ui", "[guiCreate]");
    if (is_floating) {
        return false;
    }
    if (mEditor) {
        return true;
    }
    mEditor = std::make_unique<gui::DisstortionEditor>(*this);
    mEditor->setWindowDimensions(500.f, 300.f);
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
    hints->can_resize_horizontally = true;
    hints->can_resize_vertically = true;
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
    if (mEditor->isFixedAspectRatio()) {
        mEditor->adjustWindowDimensions(width, height, false, false);
    } else {
        mEditor->setPluginDimensions(*width, *height);
    }
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

void Disstortion::editorParamsFlush() const {
    if (_host.canUseParams()) {
        _host.paramsRequestFlush();
    }
}

void Disstortion::beginParameterChange(clap_id param_id) {
    mEventsQueue.emplace(UIEvent{UIEvent::GESTURE_BEGIN, param_id, 0.0});
    editorParamsFlush();
}

void Disstortion::updateParameterChange(clap_id param_id, double value) {
    mEventsQueue.emplace(UIEvent{UIEvent::ADJUST_VALUE, param_id, value});
    editorParamsFlush();
}

void Disstortion::endParameterChange(clap_id param_id) {
    mEventsQueue.emplace(UIEvent{UIEvent::GESTURE_END, param_id, 0.0});
    editorParamsFlush();
}

} // namespace stfefane
