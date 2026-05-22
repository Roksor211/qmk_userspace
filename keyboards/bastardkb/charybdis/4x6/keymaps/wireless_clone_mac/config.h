#pragma once

#ifdef VIA_ENABLE
#    define DYNAMIC_KEYMAP_LAYER_COUNT 8
#endif

#ifndef __arm__
#    define NO_ACTION_ONESHOT
#endif

#define UNICODE_SELECTED_MODES UNICODE_MODE_MACOS
#define TAPPING_TERM 200
#define QUICK_TAP_TERM 200
