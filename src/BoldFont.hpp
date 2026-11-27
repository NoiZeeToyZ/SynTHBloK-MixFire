#pragma once
// FONTS (Nov 26 2026, Nick): section headers on code-drawn panels: 1.22x, real bold (DejaVu Sans Bold, shipped in
// res/fonts with its licence), colour #ececec. Knob / jack labels stay as they are. Gemini review: a real bold face, loaded
// with APP->window->loadFont() in draw() (Rack caches it by path; never keep the handle), looks crisp at 100% and in
// Rack -t; drawing the regular face twice smears. Falls back to that only if the font file is missing.
#include "plugin.hpp"
// shipped with every plugin that includes this (tools/make_plugins.py lists it in RES): the font and its licence
static const char* const BOLDFONT_LICENCE = "res/fonts/DejaVu-LICENSE.txt";

inline std::shared_ptr<window::Font> boldFont() {
    return APP->window->loadFont(asset::plugin(pluginInstance, "res/fonts/DejaVuSans-Bold.ttf"));
}
// Draws one section header at (x, y) px with the caller's text align; `size` = the old header size (drawn 1.22x).
// nvgSave / nvgRestore keep the caller's font, size and colour for the labels drawn next (Copilot review).
inline void drawSectionHeader(NVGcontext* vg, float x, float y, const char* s, float size) {
    nvgSave(vg);
    std::shared_ptr<window::Font> b = boldFont();
    nvgFontSize(vg, size * 1.22f);
    nvgFillColor(vg, nvgRGB(0xec, 0xec, 0xec));
    if (b && b->handle >= 0) {
        nvgFontFaceId(vg, b->handle);
        nvgText(vg, x, y, s, NULL);
    } else {   // font file missing: the caller's face, drawn twice
        nvgText(vg, x - 0.125f, y, s, NULL);
        nvgText(vg, x + 0.125f, y, s, NULL);
    }
    nvgRestore(vg);
}
