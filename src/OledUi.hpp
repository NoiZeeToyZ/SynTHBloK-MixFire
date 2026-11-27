#pragma once
// SynTHBloK OLED look (first used on MATRIX-DUO): amber text on dark brown, thin amber outline,
// faint divider lines, lit cells orange (RUN: green) with dark ink. Reuse these so every module's
// displays and display buttons match.
//
// Design rules:
//  - a display is oled::box() (2 px corners, 0.8 px outline at amber alpha 80), inset 1.5 mm from a
//    section's edge; it draws in the lit layer (drawLayer 1), so it glows in a dark room
//  - info OLED: one line per item, rows kRow (4.4 mm) tall, divided by oled::divider(); dim 6 px names,
//    amber 7.5 px values (8 px for the headline, e.g. BPM); text inset 4 px; clickable values step
//    forward on click / scroll up, back on right-click / scroll down
//  - MIDI rows: oled::midiLine() ("IN" / "OUT" + device), click = oled::midiMenu()
//  - buttons inside a display: oled::Cell ParamWidgets laid over the box (so tooltips and param menus
//    still work), 4.4 mm tall, 0.9 mm row gap, 1 mm column gap, 2 mm side padding, 6.5 px labels
//    (6 px in tight stacks). A button-only panel is oled::Box with Cells on it; size it to the
//    controls beside it (e.g. as tall as a knob's ring)
#include "plugin.hpp"
#include <string>
#include <cmath>
#include <algorithm>

namespace oled {

static const float kRow = 4.4f;      // info line height, mm
static const float kCellH = 4.4f;    // button cell height, mm

// BACKDROP RECOLOUR step 2 (Nov 20 2026): BACKDROP can hand every NoiZee ToyZ screen its theme colour (off: the usual amber)
struct Theme { bool on = false; float r = 1.f, g = 0.745f, b = 0.353f; float rot = 0.f, sat = 1.f; };   // (BLOX THEMES: rot / sat)
inline Theme& theme() { static Theme t; return t; }
inline NVGcolor accent(int a) { const Theme& t = theme(); return nvgRGBA((int) (t.r * 255.f), (int) (t.g * 255.f), (int) (t.b * 255.f), a); }
// BLOX THEMES (Nov 20 2026): BLOX's colour turn (themes.js nzShiftRGB): hue + rot, saturation x sat; near-greys stay
inline NVGcolor shift(NVGcolor c) {
    const Theme& t = theme();
    if (t.rot == 0.f && t.sat == 1.f) return c;
    float R = c.r, G = c.g, B = c.b, mx = std::max(R, std::max(G, B)), mn = std::min(R, std::min(G, B)), l = (mx + mn) * 0.5f, d = mx - mn;
    if (d < 0.04f) return c;
    float sv = l > 0.5f ? d / (2.f - mx - mn) : d / (mx + mn), h;
    if (mx == R) h = ((G - B) / d + (G < B ? 6.f : 0.f)) * 60.f; else if (mx == G) h = ((B - R) / d + 2.f) * 60.f; else h = ((R - G) / d + 4.f) * 60.f;
    h = std::fmod(std::fmod(h + t.rot, 360.f) + 360.f, 360.f); sv = std::min(1.f, sv * t.sat);
    float q = l < 0.5f ? l * (1.f + sv) : l + sv - l * sv, p = 2.f * l - q;
    auto f = [&](float k) { k = k - std::floor(k); return k < 1.f / 6.f ? p + (q - p) * 6.f * k : k < 0.5f ? q : k < 2.f / 3.f ? p + (q - p) * (2.f / 3.f - k) * 6.f : p; };
    return nvgRGBAf(f(h / 360.f + 1.f / 3.f), f(h / 360.f), f(h / 360.f - 1.f / 3.f), c.a);
}
// (BLOX THEMES on the MTX parts) colour makers that follow the BACKDROP theme
inline NVGcolor tRGB(unsigned char r, unsigned char g, unsigned char b) { return shift(nvgRGB(r, g, b)); }
inline NVGcolor tRGBA(unsigned char r, unsigned char g, unsigned char b, unsigned char a) { return shift(nvgRGBA(r, g, b, a)); }
inline NVGcolor amber(int a = 255) { return shift(nvgRGBA(255, 190, 90, a)); }
inline NVGcolor dim() { return shift(nvgRGBA(255, 190, 90, 130)); }
inline NVGcolor green() { return shift(nvgRGBA(120, 230, 90, 255)); }
inline NVGcolor outline(int a = 80) { return shift(nvgRGBA(255, 180, 90, a)); }
inline NVGcolor litFill() { return shift(nvgRGBA(255, 112, 15, 235)); }
inline NVGcolor offFill() { return shift(nvgRGBA(40, 25, 8, 255)); }
inline NVGcolor ink() { return nvgRGBA(20, 10, 2, 255); }

// The display body.
inline void box(const widget::Widget::DrawArgs& args, float W, float H, NVGcolor stroke = outline(), float strokeW = 0.8f) {
    nvgBeginPath(args.vg);
    nvgRoundedRect(args.vg, 0.f, 0.f, W, H, 2.f);
    nvgFillColor(args.vg, nvgRGBA(14, 9, 3, 255));
    nvgFill(args.vg);
    nvgStrokeWidth(args.vg, strokeW);
    nvgStrokeColor(args.vg, stroke);
    nvgStroke(args.vg);
}

// Faint divider line (add to the current path, stroke with dividerStroke()).
inline void divider(const widget::Widget::DrawArgs& args, float x0, float y0, float x1, float y1) {
    nvgMoveTo(args.vg, x0, y0);
    nvgLineTo(args.vg, x1, y1);
}
inline void dividerStroke(const widget::Widget::DrawArgs& args) {
    nvgStrokeColor(args.vg, outline(40));
    nvgStrokeWidth(args.vg, 0.6f);
    nvgStroke(args.vg);
}

// One MIDI line: dim title, then the device (the preview name in the module browser). Font must be set.
inline void midiLine(const widget::Widget::DrawArgs& args, midi::Port* port, const char* title, float y,
                     const char* preview = "Maschine Jam - 1") {
    std::string dev = "(no device)";
    if (port) { if (port->getDeviceId() >= 0) dev = port->getDeviceName(port->getDeviceId()); }
    else dev = preview;
    float yc = y + mm2px(kRow * 0.5f);
    nvgFontSize(args.vg, 6.f);
    nvgTextAlign(args.vg, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE);
    nvgFillColor(args.vg, dim());
    nvgText(args.vg, 4.f, yc, title, NULL);
    nvgFontSize(args.vg, 7.5f);
    nvgFillColor(args.vg, amber());
    nvgText(args.vg, mm2px(6.5f), yc, dev.c_str(), NULL);
}

// The port menu a MIDI line opens.
// The MTX header title, as on MTX-ECHO / MTX-DRUM: "SynTHBloK" in white, the module's name in spaced orange under it,
// left at 8.5 mm (Nick, Nov 12 2026: every new module's faceplate title matches the others)
inline void mtxTitle(NVGcontext* vg, const char* name) {
    std::shared_ptr<window::Font> font = APP->window->uiFont;
    if (!font) return;
    nvgFontFaceId(vg, font->handle);
    nvgTextAlign(vg, NVG_ALIGN_LEFT | NVG_ALIGN_BASELINE);
    nvgFontSize(vg, 11.f);
    nvgTextLetterSpacing(vg, 0.f);
    nvgFillColor(vg, nvgRGB(0xff, 0xff, 0xff));
    nvgText(vg, mm2px(8.5f), mm2px(3.5f), "SynTHBloK", NULL);
    nvgFontSize(vg, 10.f);
    nvgTextLetterSpacing(vg, 0.8f);
    nvgFillColor(vg, shift(nvgRGB(0xff, 0x7a, 0x00)));
    nvgText(vg, mm2px(8.5f), mm2px(6.4f), name, NULL);
    nvgTextLetterSpacing(vg, 0.f);
}
inline void midiMenu(midi::Port* port, const char* title) {
    if (!port) return;
    ui::Menu* menu = createMenu();
    menu->addChild(createMenuLabel(title));
    appendMidiMenu(menu, port);
}

// A bare display for Cells to sit on.
struct Box : TransparentWidget {
    void drawLayer(const DrawArgs& args, int layer) override {
        if (layer == 1) oled::box(args, box.size.x, box.size.y);
        Widget::drawLayer(args, layer);
    }
};

// A button cell drawn in the lit layer. setTo >= 0: a press sets that value (lit while it holds it:
// view / mode selectors); else latch toggles, or momentary (1 while held, so SchmittTriggers see a
// press). green = lit green (RUN). jack: a patched input shows a dot; flash: lit while *flash > 0.
struct Cell : app::ParamWidget {
    Module* module = NULL;
    const char* label = "";
    int setTo = -1;
    bool latch = false, green = false;
    int jack = -1;
    const float* flash = NULL;
    float fontSize = 6.5f;
    bool lit() {
        ParamQuantity* pq = getParamQuantity();
        if (!module || !pq) return setTo == 0 || green;
        if (flash && *flash > 0.f) return true;
        if (setTo >= 0) return (int)std::round(pq->getValue()) == setTo;
        return pq->getValue() > 0.5f;
    }
    void drawLayer(const DrawArgs& args, int layer) override {
        ParamWidget::drawLayer(args, layer);
        if (layer != 1) return;
        bool on = lit();
        float W = box.size.x, H = box.size.y;
        nvgBeginPath(args.vg);
        nvgRoundedRect(args.vg, 0.f, 0.f, W, H, 1.2f);
        nvgFillColor(args.vg, on ? (green ? oled::green() : litFill()) : offFill());
        nvgFill(args.vg);
        nvgStrokeColor(args.vg, outline(on ? 255 : 110));
        nvgStrokeWidth(args.vg, 0.7f);
        nvgStroke(args.vg);
        if (module && jack >= 0 && module->inputs[jack].isConnected()) {
            nvgBeginPath(args.vg);
            nvgCircle(args.vg, W - 2.4f, 2.4f, 1.1f);
            nvgFillColor(args.vg, on ? ink() : amber());
            nvgFill(args.vg);
        }
        std::shared_ptr<window::Font> font = APP->window->uiFont;
        if (!font) return;
        nvgFontFaceId(args.vg, font->handle);
        nvgFontSize(args.vg, fontSize);
        nvgTextAlign(args.vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
        nvgFillColor(args.vg, on ? ink() : amber());
        nvgText(args.vg, W * 0.5f, H * 0.5f, label, NULL);
    }
    void onButton(const ButtonEvent& e) override {
        if (e.button == GLFW_MOUSE_BUTTON_LEFT && e.action == GLFW_PRESS) {
            if (ParamQuantity* pq = getParamQuantity()) {
                if (setTo >= 0) pq->setValue((float)setTo);
                else pq->setValue(latch ? (pq->getValue() > 0.5f ? 0.f : 1.f) : 1.f);
            }
            e.consume(this);
            return;
        }
        ParamWidget::onButton(e);
    }
    void onDragStart(const DragStartEvent& e) override {}
    void onDragEnd(const DragEndEvent& e) override {
        if (setTo < 0 && !latch && e.button == GLFW_MOUSE_BUTTON_LEFT)
            if (ParamQuantity* pq = getParamQuantity()) pq->setValue(0.f);
    }
};

} // namespace oled
