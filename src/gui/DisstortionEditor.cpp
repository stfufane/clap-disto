#include "DisstortionEditor.h"

using namespace visage::dimension;

namespace stfefane::gui {

DisstortionEditor::DisstortionEditor() {
    setWindowDimensions(80_vmin, 60_vmin);
    onDraw() = [this](visage::Canvas &canvas) {
        canvas.setColor(0xdeadbeef);
        canvas.fill(0, 0, width(), height());
    };
}
} // namespace gui::stfefane