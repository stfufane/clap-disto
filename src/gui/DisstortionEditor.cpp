#include "DisstortionEditor.h"

namespace stfefane::gui {

using namespace visage::dimension;

DisstortionEditor::DisstortionEditor() {
    setWindowDimensions(80_vmin, 60_vmin);
    onDraw() = [this](visage::Canvas& canvas) {
        canvas.setColor(0xdeadbeef);
        canvas.fill(0, 0, width(), height());
    };
}
} // namespace stfefane::gui