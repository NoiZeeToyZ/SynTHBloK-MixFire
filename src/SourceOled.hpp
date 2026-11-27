#pragma once
// A mixer channel's source OLED (Nov 11 2026): a small amber screen at the top of the strip with the channel number
// and the name of the module plugged into its L / R inputs ("1 TOYBOX"), or "UNPLUGGED" when nothing is. L and R
// from two different modules show "L-NAME + R-NAME". Read from the patch's cables on the UI thread, every frame.
// Used by MIX-8 / MIX-12 / MIX24 (SynTHBloK-MixFire).
#include "plugin.hpp"
#include "OledUi.hpp"   // (BLOX THEMES: MTX parts themed)
#include <cctype>

// (Nov 12 2026, Nick: "more visible and below MUTE and SOLO") createBelow(): a bigger screen in the gap between MUTE /
// SOLO and the L jack, name only (the panel prints the channel number at the top again), larger and brighter text.
struct SourceOled : TransparentWidget {
    app::PortWidget* inL = NULL;
    app::PortWidget* inR = NULL;
    int number = 1;
    bool big = false;

    static std::string sourceOf(app::PortWidget* pw) {
        if (!pw || !APP->scene || !APP->scene->rack) return "";
        app::CableWidget* cw = APP->scene->rack->getTopCable(pw);
        if (!cw || !cw->cable || !cw->cable->outputModule || !cw->cable->outputModule->model) return "";
        std::string n = cw->cable->outputModule->model->name;
        for (char& ch : n) ch = (char) std::toupper((unsigned char) ch);
        return n;
    }

    void drawLayer(const DrawArgs& args, int layer) override {
        if (layer != 1) return;
        float W = box.size.x, H = box.size.y;
        std::string l = sourceOf(inL), r = sourceOf(inR);
        std::string name = l.empty() ? r : (r.empty() || r == l) ? l : l + " + " + r;
        bool on = !name.empty();
        nvgBeginPath(args.vg);
        nvgRoundedRect(args.vg, 0.f, 0.f, W, H, 1.2f);
        nvgFillColor(args.vg, big ? oled::tRGBA(6, 4, 1, 255) : oled::tRGBA(14, 9, 3, 255));
        nvgFill(args.vg);
        nvgStrokeColor(args.vg, oled::tRGBA(255, 122, 0, on ? (big ? 235 : 150) : (big ? 90 : 60)));
        nvgStrokeWidth(args.vg, big ? 0.9f : 0.6f);
        nvgStroke(args.vg);
        std::shared_ptr<window::Font> font = APP->window->uiFont;
        if (!font) return;
        nvgFontFaceId(args.vg, font->handle);
        if (big) {   // the name only, as large as fits (two lines for "L + R" pairs), bright
            float b[4], room = W - 3.f;
            std::string s = on ? name : "UNPLUGGED";
            std::string l1 = s, l2;
            size_t plus = s.find(" + ");
            if (plus != std::string::npos) { l1 = s.substr(0, plus) + " +"; l2 = s.substr(plus + 3); }
            auto fit = [&](std::string& t, float maxFs, float minFs) {
                float fs = maxFs;
                nvgFontSize(args.vg, fs);
                while (fs > minFs && nvgTextBounds(args.vg, 0.f, 0.f, t.c_str(), NULL, b) > room) { fs -= 0.2f; nvgFontSize(args.vg, fs); }
                if (nvgTextBounds(args.vg, 0.f, 0.f, t.c_str(), NULL, b) > room) {
                    while (t.size() > 2 && nvgTextBounds(args.vg, 0.f, 0.f, (t + ".").c_str(), NULL, b) > room) t.pop_back();
                    t += ".";
                }
                return fs;
            };
            nvgTextAlign(args.vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
            NVGcolor ink = on ? oled::tRGB(255, 214, 140) : oled::tRGBA(255, 190, 90, 120);
            if (l2.empty()) {
                fit(l1, 8.4f, 5.f);
                nvgFillColor(args.vg, ink);
                nvgText(args.vg, W * .5f, H * .5f, l1.c_str(), NULL);
            } else {
                float f1 = fit(l1, 6.4f, 4.6f), f2 = fit(l2, 6.4f, 4.6f), fs = std::min(f1, f2);
                nvgFontSize(args.vg, fs);
                nvgFillColor(args.vg, ink);
                nvgText(args.vg, W * .5f, H * .29f, l1.c_str(), NULL);
                nvgText(args.vg, W * .5f, H * .71f, l2.c_str(), NULL);
            }
            return;
        }
        // the channel number, then the source, shrunk (then cut short) to fit
        char num[4];
        snprintf(num, sizeof num, "%d", number);
        nvgFontSize(args.vg, 6.5f);
        nvgTextAlign(args.vg, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE);
        nvgFillColor(args.vg, oled::tRGB(0xff, 0x7a, 0x00));
        float nx = 2.f;
        nvgText(args.vg, nx, H * .5f, num, NULL);
        float b[4];
        float x0 = nx + nvgTextBounds(args.vg, 0.f, 0.f, num, NULL, b) + 1.8f, room = W - x0 - 1.5f;
        std::string s = on ? name : "UNPLUGGED";
        float fs = 5.6f;
        nvgFontSize(args.vg, fs);
        while (fs > 4.2f && nvgTextBounds(args.vg, 0.f, 0.f, s.c_str(), NULL, b) > room) { fs -= 0.2f; nvgFontSize(args.vg, fs); }
        if (nvgTextBounds(args.vg, 0.f, 0.f, s.c_str(), NULL, b) > room) {
            while (s.size() > 2 && nvgTextBounds(args.vg, 0.f, 0.f, (s + ".").c_str(), NULL, b) > room) s.pop_back();
            s += ".";
        }
        nvgFillColor(args.vg, on ? oled::tRGB(255, 190, 90) : oled::tRGBA(255, 190, 90, 70));
        nvgText(args.vg, x0, H * .5f, s.c_str(), NULL);
    }

    static SourceOled* create(Vec centreMm, int number, app::PortWidget* l, app::PortWidget* r) {
        SourceOled* o = new SourceOled;
        o->box.size = mm2px(Vec(13.2f, 4.6f));
        o->box.pos = mm2px(centreMm).minus(o->box.size.div(2.f));
        o->number = number; o->inL = l; o->inR = r;
        return o;
    }
    // under MUTE / SOLO (centre x of the strip, y between the switches at 98 mm and the L jack at 111.4 mm)
    static SourceOled* createBelow(float xMm, int number, app::PortWidget* l, app::PortWidget* r) {
        SourceOled* o = new SourceOled;
        o->big = true;
        o->box.size = mm2px(Vec(12.8f, 6.2f));   // inside the strip's plate (strips are 14.5 mm apart)
        o->box.pos = mm2px(Vec(xMm, 104.6f)).minus(o->box.size.div(2.f));
        o->number = number; o->inL = l; o->inR = r;
        return o;
    }
};
