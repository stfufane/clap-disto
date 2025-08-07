#pragma once

#include "clap/ext/params.h"

#include <visage/ui.h>

class RotaryKnob final : public visage::Frame
{
public:
    explicit RotaryKnob(uint32_t param_id);

    void draw(visage::Canvas& canvas) override;
    void mouseDown(const visage::MouseEvent& e) override;
    void mouseUp(const visage::MouseEvent& e) override;
    void mouseDrag(const visage::MouseEvent& e) override;

private:
    double min_value = 0.;
    double max_value = 0.;
    double value = 0.;
    bool is_dragging = false;
    int drag_start_y = 0;
};