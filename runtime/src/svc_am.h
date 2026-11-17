#pragma once

namespace nx {

// Preset selected in the Windows quick settings; zero leaves the game's default mode unchanged.
void set_internal_resolution_preset(int preset);
float internal_resolution_scale();
uint32_t internal_resolution_target_width();
void set_native_render_width(uint32_t width);  // internal resolution Native: the presented width
void set_game_language(int language);
const char* game_language_name();  // that selector as "en-US"/"zh-CN"/...
int game_language_index();      // the persisted selector: 0-11
uint64_t game_language_code();  // that selector as a LanguageCode (u64 of "en-US"/"zh-CN"/...)

}  // namespace nx
