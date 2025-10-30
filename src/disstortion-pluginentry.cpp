#include "disstortion.h"

#include "utils/Logger.h"
#include "utils/Folders.h"
#include <cstring>

namespace stfefane::plugin_entry {

uint32_t getPluginCount(const clap_plugin_factory *f) {
    return 1;
}

const clap_plugin_descriptor *getPluginDescriptor(const clap_plugin_factory *f, uint32_t w) {
    return &Disstortion::descriptor;
}

static const clap_plugin *createPlugin(const clap_plugin_factory *factory, const clap_host *host,
                                       const char *plugin_id) {
    if (strcmp(plugin_id, Disstortion::descriptor.id) != 0) {
        return nullptr;
    }

#if DEBUG
    utils::initLoggers();
#endif
    utils::folders::setupPluginFolder();

    auto *p = new Disstortion(host);
    return p->clapPlugin();
}

static const void *get_factory(const char *factory_id) {
    static constexpr clap_plugin_factory va_clap_plugin_factory = {
        getPluginCount,
        getPluginDescriptor,
        createPlugin,
    };
    return (!strcmp(factory_id, CLAP_PLUGIN_FACTORY_ID)) ? &va_clap_plugin_factory : nullptr;
}

bool clap_init(const char *p) {
    return true;
}
void clap_deinit() {}
} // namespace stfefane::plugin_entry

extern "C" {
extern const CLAP_EXPORT clap_plugin_entry clap_entry = {CLAP_VERSION, stfefane::plugin_entry::clap_init,
                                                         stfefane::plugin_entry::clap_deinit,
                                                         stfefane::plugin_entry::get_factory};
}