#include QMK_KEYBOARD_H
#include "keymap_german.h"
#include "sendstring_german.h"

enum layers {
    L_BASE = 0,
    L_NUM,
    L_NAV,
    L_SYM,
    L_GAME,
    L_EXTRAS,
    L_SLOW,
    L_SCROLL,
};

enum custom_keycodes {
    CK_SPOTLIGHT = SAFE_RANGE,
    CK_CLIP_HIST,
    CK_TMUX_DETACH,
    CK_TMUX_SYNC,
    CK_TMUX_NEXT_PANE,
    CK_TMUX_SHOW_SESS,
    CK_TMUX_NEW_SESS,
    CK_TMUX_H_SPLIT,
    CK_TMUX_V_SPLIT,
    CK_MOVE_LINE_UP,
    CK_MOVE_LINE_DOWN,
    CK_ADD_ABOVE,
    CK_ADD_BELOW,
    CK_DELETE_LINE,
    CK_CODE_BLOCK,
    CK_HOME_DIR,
    CK_COMMA_SCLN,
    CK_DOT_COLN,
    CK_QM_EXLM,
    CK_GRV_TILD,
    CK_GRV,
    CK_CIRC,
    CK_BITWARDEN_OPEN,
};

enum {
    TD_ESC_SCROLL_SLOW = 0,
    TD_CLK_SYM,
    TD_LAYERS,
};

enum unicode_names {
    UC_AE = 0,
    UC_UE,
    UC_OE,
    UC_SS,
};

enum tap_dance_states {
    TD_STATE_NONE = 0,
    TD_STATE_SINGLE_TAP,
    TD_STATE_SINGLE_HOLD,
    TD_STATE_DOUBLE_TAP,
    TD_STATE_DOUBLE_HOLD,
};

static uint8_t td_esc_state    = TD_STATE_NONE;
static uint8_t td_clk_state    = TD_STATE_NONE;
static uint8_t td_layers_state = TD_STATE_NONE;

static bool shifted_active(void) {
    return (get_mods() | get_weak_mods() | get_oneshot_mods()) & MOD_MASK_SHIFT;
}

static uint8_t current_dance(tap_dance_state_t *state) {
    if (state->count == 1) {
        return state->pressed ? TD_STATE_SINGLE_HOLD : TD_STATE_SINGLE_TAP;
    }
    if (state->count == 2) {
        return state->pressed ? TD_STATE_DOUBLE_HOLD : TD_STATE_DOUBLE_TAP;
    }
    return TD_STATE_NONE;
}

static void tap_key_without_shift(uint16_t keycode) {
    const uint8_t mods         = get_mods();
    const uint8_t weak_mods    = get_weak_mods();
    const uint8_t oneshot_mods = get_oneshot_mods();

    del_mods(MOD_MASK_SHIFT);
    del_weak_mods(MOD_MASK_SHIFT);
    set_oneshot_mods(oneshot_mods & ~MOD_MASK_SHIFT);
    send_keyboard_report();

    tap_code16(keycode);

    set_mods(mods);
    set_weak_mods(weak_mods);
    set_oneshot_mods(oneshot_mods);
    send_keyboard_report();
}

static void tap_literal_dead_key(uint16_t keycode) {
    const uint8_t mods         = get_mods();
    const uint8_t weak_mods    = get_weak_mods();
    const uint8_t oneshot_mods = get_oneshot_mods();

    del_mods(MOD_MASK_SHIFT);
    del_weak_mods(MOD_MASK_SHIFT);
    set_oneshot_mods(oneshot_mods & ~MOD_MASK_SHIFT);
    send_keyboard_report();

    tap_code16(keycode);
    tap_code(KC_SPC);

    set_mods(mods);
    set_weak_mods(weak_mods);
    set_oneshot_mods(oneshot_mods);
    send_keyboard_report();
}

static void tap_morph_key(uint16_t base_keycode, uint16_t shifted_keycode) {
    tap_key_without_shift(shifted_active() ? shifted_keycode : base_keycode);
}

static void tap_grave_tilde_key(void) {
    if (shifted_active()) {
        tap_key_without_shift(DE_TILD);
        return;
    }

    tap_literal_dead_key(DE_GRV);
}

static void tap_tmux_prefix(uint16_t keycode) {
    tap_code16(C(KC_B));
    tap_code16(keycode);
}

void esc_scroll_slow_finished(tap_dance_state_t *state, void *user_data) {
    td_esc_state = current_dance(state);

    switch (td_esc_state) {
        case TD_STATE_SINGLE_TAP:
            tap_code(KC_ESC);
            break;
        case TD_STATE_SINGLE_HOLD:
            layer_on(L_SCROLL);
            break;
        case TD_STATE_DOUBLE_TAP:
            tap_code(KC_ESC);
            tap_code(KC_ESC);
            break;
        case TD_STATE_DOUBLE_HOLD:
            layer_on(L_SLOW);
            break;
    }
}

void esc_scroll_slow_reset(tap_dance_state_t *state, void *user_data) {
    switch (td_esc_state) {
        case TD_STATE_SINGLE_HOLD:
            layer_off(L_SCROLL);
            break;
        case TD_STATE_DOUBLE_HOLD:
            layer_off(L_SLOW);
            break;
    }

    td_esc_state = TD_STATE_NONE;
}

void clk_sym_finished(tap_dance_state_t *state, void *user_data) {
    td_clk_state = current_dance(state);

    switch (td_clk_state) {
        case TD_STATE_SINGLE_TAP:
            tap_code(KC_BTN1);
            break;
        case TD_STATE_SINGLE_HOLD:
        case TD_STATE_DOUBLE_HOLD:
            layer_on(L_SYM);
            break;
        case TD_STATE_DOUBLE_TAP:
            tap_code(KC_BTN1);
            tap_code(KC_BTN1);
            break;
    }
}

void clk_sym_reset(tap_dance_state_t *state, void *user_data) {
    switch (td_clk_state) {
        case TD_STATE_SINGLE_HOLD:
        case TD_STATE_DOUBLE_HOLD:
            layer_off(L_SYM);
            break;
    }

    td_clk_state = TD_STATE_NONE;
}

void layers_finished(tap_dance_state_t *state, void *user_data) {
    td_layers_state = current_dance(state);

    switch (td_layers_state) {
        case TD_STATE_SINGLE_TAP:
            layer_move(L_BASE);
            break;
        case TD_STATE_SINGLE_HOLD:
            layer_on(L_EXTRAS);
            break;
        case TD_STATE_DOUBLE_TAP:
        case TD_STATE_DOUBLE_HOLD:
            layer_move(L_GAME);
            break;
    }
}

void layers_reset(tap_dance_state_t *state, void *user_data) {
    if (td_layers_state == TD_STATE_SINGLE_HOLD) {
        layer_off(L_EXTRAS);
    }

    td_layers_state = TD_STATE_NONE;
}

tap_dance_action_t tap_dance_actions[] = {
    [TD_ESC_SCROLL_SLOW] = ACTION_TAP_DANCE_FN_ADVANCED(NULL, esc_scroll_slow_finished, esc_scroll_slow_reset),
    [TD_CLK_SYM]         = ACTION_TAP_DANCE_FN_ADVANCED(NULL, clk_sym_finished, clk_sym_reset),
    [TD_LAYERS]          = ACTION_TAP_DANCE_FN_ADVANCED(NULL, layers_finished, layers_reset),
};

const uint32_t PROGMEM unicode_map[] = {
    [UC_AE] = 0x00E4,
    [UC_UE] = 0x00FC,
    [UC_OE] = 0x00F6,
    [UC_SS] = 0x00DF,
};

#define US_1 DE_1
#define US_2 DE_2
#define US_3 DE_3
#define US_4 DE_4
#define US_5 DE_5
#define US_6 DE_6
#define US_7 DE_7
#define US_8 DE_8
#define US_9 DE_9
#define US_0 DE_0
#define US_Q DE_Q
#define US_W DE_W
#define US_E DE_E
#define US_R DE_R
#define US_T DE_T
#define US_Y DE_Y
#define US_U DE_U
#define US_I DE_I
#define US_O DE_O
#define US_P DE_P
#define US_A DE_A
#define US_S DE_S
#define US_D DE_D
#define US_F DE_F
#define US_G DE_G
#define US_H DE_H
#define US_J DE_J
#define US_K DE_K
#define US_L DE_L
#define US_Z DE_Z
#define US_X DE_X
#define US_C DE_C
#define US_V DE_V
#define US_B DE_B
#define US_N DE_N
#define US_M DE_M
#define US_MINS DE_MINS
#define US_EQL DE_EQL
#define US_LBRC DE_LBRC
#define US_RBRC DE_RBRC
#define US_BSLS DE_BSLS
#define US_SCLN DE_SCLN
#define US_QUOT DE_QUOT
#define US_COMM DE_COMM
#define US_DOT DE_DOT
#define US_SLSH DE_SLSH
#define US_TILD DE_TILD
#define US_EXLM DE_EXLM
#define US_DQUO DE_DQUO
#define US_HASH DE_HASH
#define US_DLR DE_DLR
#define US_PERC DE_PERC
#define US_AMPR DE_AMPR
#define US_ASTR DE_ASTR
#define US_LPRN DE_LPRN
#define US_RPRN DE_RPRN
#define US_AT DE_AT
#define US_UNDS DE_UNDS
#define US_LT DE_LABK
#define US_GT DE_RABK
#define US_PIPE DE_PIPE
#define US_PLUS DE_PLUS
#define US_COLN DE_COLN
#define US_QUES DE_QUES
#define US_LCBR DE_LCBR
#define US_RCBR DE_RCBR

#define HM_A LGUI_T(US_A)
#define HM_S LALT_T(US_S)
#define HM_D LSFT_T(US_D)
#define HM_F LCTL_T(US_F)
#define HM_J RCTL_T(US_J)
#define HM_K RSFT_T(US_K)
#define HM_L RALT_T(US_L)
#define HM_SCLN RGUI_T(KC_SCLN)
#define NUM_BSP LT(L_NUM, KC_BSPC)
#define NAV_SPC LT(L_NAV, KC_SPC)

// clang-format off
const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [L_BASE] = LAYOUT(
        CK_GRV_TILD,     US_1,           US_2,           US_3,           US_4,           US_5,               US_6,           US_7,           US_8,           US_9,           US_0,           KC_BSPC,
        CK_SPOTLIGHT,    US_Q,           US_W,           US_E,           US_R,           US_T,               US_Y,           US_U,           US_I,           US_O,           US_P,           TD(TD_LAYERS),
        KC_TAB,          HM_A,           HM_S,           HM_D,           HM_F,           US_G,               US_H,           HM_J,           HM_K,           HM_L,           HM_SCLN,        KC_DEL,
        CK_CLIP_HIST,    US_Z,           US_X,           US_C,           US_V,           US_B,               US_N,           US_M,           CK_COMMA_SCLN,  CK_DOT_COLN,    CK_QM_EXLM,     US_SLSH,
                                              TD(TD_ESC_SCROLL_SLOW),  TD(TD_CLK_SYM),  NUM_BSP,            KC_ENT,         NAV_SPC,
                                                                       KC_NO,            KC_NO,              KC_NO
    ),

    [L_NUM] = LAYOUT(
        KC_F12,          KC_F1,          KC_F2,          KC_F3,          KC_F4,          KC_F5,              KC_F6,          KC_F7,          KC_F8,          KC_F9,          KC_F10,         KC_F11,
        XXXXXXX,         XXXXXXX,        UM(UC_AE),      UM(UC_UE),      UM(UC_OE),      UM(UC_SS),         US_LBRC,        KC_7,           KC_8,           KC_9,           US_RBRC,        XXXXXXX,
        XXXXXXX,         KC_LGUI,        KC_LALT,        KC_LSFT,        KC_LCTL,        XXXXXXX,            US_PLUS,        KC_4,           KC_5,           KC_6,           US_MINS,        US_EQL,
        XXXXXXX,         XXXXXXX,        XXXXXXX,        CK_BITWARDEN_OPEN, XXXXXXX,     XXXXXXX,            US_ASTR,        KC_1,           KC_2,           KC_3,           US_SLSH,        US_DOT,
                                              _______,                  _______,         _______,            KC_0,           _______,
                                                                       XXXXXXX,          XXXXXXX,            XXXXXXX
    ),

    [L_NAV] = LAYOUT(
        XXXXXXX,         XXXXXXX,        XXXXXXX,        XXXXXXX,        XXXXXXX,        XXXXXXX,            CK_TMUX_DETACH, CK_TMUX_NEW_SESS, CK_TMUX_NEXT_PANE, CK_TMUX_SHOW_SESS, CK_TMUX_SYNC, XXXXXXX,
        KC_TAB,          KC_BTN4,        KC_LALT,        KC_LSFT,        KC_LCTL,        KC_BTN5,            CK_TMUX_H_SPLIT, KC_LEFT,        KC_DOWN,        KC_UP,          KC_RGHT,        XXXXXXX,
        XXXXXXX,         KC_MS_L,        KC_MS_D,        KC_MS_U,        KC_MS_R,        XXXXXXX,            CK_TMUX_V_SPLIT, KC_HOME,        KC_PGDN,        KC_PGUP,        KC_END,         XXXXXXX,
        XXXXXXX,         XXXXXXX,        XXXXXXX,        XXXXXXX,        XXXXXXX,        XXXXXXX,            XXXXXXX,        XXXXXXX,        XXXXXXX,        XXXXXXX,        XXXXXXX,        XXXXXXX,
                                              _______,                  _______,         _______,            XXXXXXX,        _______,
                                                                       XXXXXXX,          XXXXXXX,            XXXXXXX
    ),

    [L_SYM] = LAYOUT(
        US_TILD,         US_EXLM,        US_AT,          US_HASH,        US_DLR,         US_PERC,            CK_CIRC,        US_AMPR,        US_ASTR,        US_LPRN,        US_RPRN,        US_PIPE,
        XXXXXXX,         US_UNDS,        US_LT,          US_GT,          KC_BSPC,        CK_DELETE_LINE,     CK_GRV,         US_MINS,        US_LBRC,        US_RBRC,        US_ASTR,        US_BSLS,
        XXXXXXX,         CK_HOME_DIR,    XXXXXXX,        US_HASH,        CK_MOVE_LINE_DOWN, CK_ADD_BELOW,    CK_CODE_BLOCK,  US_EQL,         US_LCBR,        US_RCBR,        US_AT,          US_SLSH,
        XXXXXXX,         XXXXXXX,        XXXXXXX,        XXXXXXX,        CK_MOVE_LINE_UP, CK_ADD_ABOVE,      XXXXXXX,        XXXXXXX,        XXXXXXX,        XXXXXXX,        XXXXXXX,        XXXXXXX,
                                              _______,                  _______,         _______,            _______,        _______,
                                                                       XXXXXXX,          XXXXXXX,            XXXXXXX
    ),

    [L_GAME] = LAYOUT(
        CK_GRV_TILD,     US_1,           US_2,           US_3,           US_4,           US_5,               US_6,           US_7,           US_8,           US_9,           US_0,           _______,
        _______,         KC_TAB,         US_Q,           US_W,           US_E,           US_R,               XXXXXXX,        XXXXXXX,        XXXXXXX,        XXXXXXX,        XXXXXXX,        _______,
        _______,         KC_LCTL,        US_A,           US_S,           US_D,           US_F,               XXXXXXX,        XXXXXXX,        XXXXXXX,        XXXXXXX,        XXXXXXX,        XXXXXXX,
        _______,         KC_LSFT,        US_Z,           US_X,           US_C,           US_V,               XXXXXXX,        XXXXXXX,        XXXXXXX,        XXXXXXX,        XXXXXXX,        XXXXXXX,
                                              XXXXXXX,                  KC_SPC,          KC_LALT,            XXXXXXX,        XXXXXXX,
                                                                       XXXXXXX,          XXXXXXX,            XXXXXXX
    ),

    [L_EXTRAS] = LAYOUT(
        QK_BOOT,         EE_CLR,         XXXXXXX,        XXXXXXX,        XXXXXXX,        XXXXXXX,            XXXXXXX,        XXXXXXX,        XXXXXXX,        XXXXXXX,        XXXXXXX,        XXXXXXX,
        KC_SLEP,         XXXXXXX,        XXXXXXX,        XXXXXXX,        KC_BRID,        KC_BRIU,            XXXXXXX,        KC_MPRV,        KC_MPLY,        KC_MSTP,        KC_MNXT,        XXXXXXX,
        XXXXXXX,         G(US_Z),        G(US_X),        G(US_C),        G(US_V),        XXXXXXX,            XXXXXXX,        KC_MUTE,        KC_VOLD,        KC_VOLU,        G(S(US_4)),     XXXXXXX,
        XXXXXXX,         CK_CODE_BLOCK,  CK_HOME_DIR,    XXXXXXX,        XXXXXXX,        XXXXXXX,            XXXXXXX,        XXXXXXX,        XXXXXXX,        XXXXXXX,        XXXXXXX,        XXXXXXX,
                                              _______,                  _______,         _______,            _______,        _______,
                                                                       XXXXXXX,          XXXXXXX,            XXXXXXX
    ),

    [L_SLOW] = LAYOUT(
        _______,         _______,        _______,        _______,        _______,        _______,            _______,        _______,        _______,        _______,        _______,        _______,
        _______,         _______,        _______,        _______,        _______,        _______,            _______,        _______,        _______,        _______,        _______,        _______,
        _______,         _______,        _______,        _______,        _______,        _______,            _______,        _______,        _______,        _______,        _______,        _______,
        _______,         _______,        _______,        _______,        _______,        _______,            _______,        _______,        _______,        _______,        _______,        _______,
                                              _______,                  _______,         _______,            _______,        _______,
                                                                       _______,          _______,            _______
    ),

    [L_SCROLL] = LAYOUT(
        _______,         _______,        _______,        _______,        _______,        _______,            _______,        _______,        _______,        _______,        _______,        _______,
        _______,         _______,        _______,        _______,        _______,        _______,            _______,        _______,        _______,        _______,        _______,        _______,
        _______,         _______,        _______,        _______,        _______,        _______,            _______,        _______,        _______,        _______,        _______,        _______,
        _______,         _______,        _______,        _______,        _______,        _______,            _______,        _______,        _______,        _______,        _______,        _______,
                                              _______,                  _______,         _______,            _______,        _______,
                                                                       _______,          _______,            _______
    ),
};
// clang-format on

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    if (!record->event.pressed) {
        return true;
    }

    switch (keycode) {
        case CK_SPOTLIGHT:
            tap_code16(G(KC_SPC));
            return false;

        case CK_CLIP_HIST:
            tap_code16(G(S(US_V)));
            return false;

        case CK_TMUX_DETACH:
            tap_tmux_prefix(US_D);
            return false;

        case CK_TMUX_SYNC:
            tap_tmux_prefix(KC_SPC);
            return false;

        case CK_TMUX_NEXT_PANE:
            tap_tmux_prefix(US_O);
            return false;

        case CK_TMUX_SHOW_SESS:
            tap_tmux_prefix(US_S);
            return false;

        case CK_TMUX_NEW_SESS:
            SEND_STRING("tmux new-session -A -s ");
            return false;

        case CK_TMUX_H_SPLIT:
            tap_tmux_prefix(US_DQUO);
            return false;

        case CK_TMUX_V_SPLIT:
            tap_tmux_prefix(US_PERC);
            return false;

        case CK_MOVE_LINE_UP:
            tap_code16(A(KC_UP));
            return false;

        case CK_MOVE_LINE_DOWN:
            tap_code16(A(KC_DOWN));
            return false;

        case CK_ADD_ABOVE:
            tap_code16(C(A(KC_UP)));
            return false;

        case CK_ADD_BELOW:
            tap_code16(C(A(KC_DOWN)));
            return false;

        case CK_DELETE_LINE:
            tap_code16(C(S(US_K)));
            return false;

        case CK_CODE_BLOCK:
            SEND_STRING("```");
            return false;

        case CK_HOME_DIR:
            SEND_STRING("~/");
            return false;

        case CK_COMMA_SCLN:
            tap_morph_key(US_COMM, US_SCLN);
            return false;

        case CK_DOT_COLN:
            tap_morph_key(US_DOT, US_COLN);
            return false;

        case CK_QM_EXLM:
            tap_morph_key(US_QUES, US_EXLM);
            return false;

        case HM_SCLN:
            if (record->tap.count) {
                tap_key_without_shift(US_SCLN);
                return false;
            }
            break;

        case CK_GRV_TILD:
            tap_grave_tilde_key();
            return false;

        case CK_GRV:
            tap_literal_dead_key(DE_GRV);
            return false;

        case CK_CIRC:
            tap_literal_dead_key(DE_CIRC);
            return false;

        case CK_BITWARDEN_OPEN:
            tap_code16(G(S(US_Y)));
            return false;
    }

    return true;
}

#ifdef POINTING_DEVICE_ENABLE
layer_state_t layer_state_set_user(layer_state_t state) {
    charybdis_set_pointer_dragscroll_enabled(layer_state_cmp(state, L_SCROLL));
    charybdis_set_pointer_sniping_enabled(layer_state_cmp(state, L_SLOW));
    return state;
}
#endif
