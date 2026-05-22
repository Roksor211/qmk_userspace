#include QMK_KEYBOARD_H

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
    CK_TART_LOGIN,
};

enum {
    TD_ESC_SCROLL_SLOW = 0,
    TD_CLK_SYM,
    TD_LAYERS,
};

enum unicode_names {
    DE_AE = 0,
    DE_UE,
    DE_OE,
    DE_SS,
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

static uint8_t current_dance(qk_tap_dance_state_t *state) {
    if (state->count == 1) {
        return state->pressed ? TD_STATE_SINGLE_HOLD : TD_STATE_SINGLE_TAP;
    }
    if (state->count == 2) {
        return state->pressed ? TD_STATE_DOUBLE_HOLD : TD_STATE_DOUBLE_TAP;
    }
    return TD_STATE_NONE;
}

static void tap_morph_key(uint16_t base_keycode, uint16_t shifted_keycode) {
    const uint8_t mods        = get_mods();
    const uint8_t weak_mods   = get_weak_mods();
    const uint8_t oneshot_mods = get_oneshot_mods();
    const bool shifted        = (mods | weak_mods | oneshot_mods) & MOD_MASK_SHIFT;

    del_mods(MOD_MASK_SHIFT);
    del_weak_mods(MOD_MASK_SHIFT);
    set_oneshot_mods(oneshot_mods & ~MOD_MASK_SHIFT);
    send_keyboard_report();

    tap_code16(shifted ? shifted_keycode : base_keycode);

    set_mods(mods);
    set_weak_mods(weak_mods);
    set_oneshot_mods(oneshot_mods);
    send_keyboard_report();
}

static void tap_tmux_prefix(uint16_t keycode) {
    tap_code16(C(KC_B));
    tap_code16(keycode);
}

void esc_scroll_slow_finished(qk_tap_dance_state_t *state, void *user_data) {
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

void esc_scroll_slow_reset(qk_tap_dance_state_t *state, void *user_data) {
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

void clk_sym_finished(qk_tap_dance_state_t *state, void *user_data) {
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

void clk_sym_reset(qk_tap_dance_state_t *state, void *user_data) {
    switch (td_clk_state) {
        case TD_STATE_SINGLE_HOLD:
        case TD_STATE_DOUBLE_HOLD:
            layer_off(L_SYM);
            break;
    }

    td_clk_state = TD_STATE_NONE;
}

void layers_finished(qk_tap_dance_state_t *state, void *user_data) {
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

void layers_reset(qk_tap_dance_state_t *state, void *user_data) {
    if (td_layers_state == TD_STATE_SINGLE_HOLD) {
        layer_off(L_EXTRAS);
    }

    td_layers_state = TD_STATE_NONE;
}

qk_tap_dance_action_t tap_dance_actions[] = {
    [TD_ESC_SCROLL_SLOW] = ACTION_TAP_DANCE_FN_ADVANCED(NULL, esc_scroll_slow_finished, esc_scroll_slow_reset),
    [TD_CLK_SYM]         = ACTION_TAP_DANCE_FN_ADVANCED(NULL, clk_sym_finished, clk_sym_reset),
    [TD_LAYERS]          = ACTION_TAP_DANCE_FN_ADVANCED(NULL, layers_finished, layers_reset),
};

const uint32_t PROGMEM unicode_map[] = {
    [DE_AE] = 0x00E4,
    [DE_UE] = 0x00FC,
    [DE_OE] = 0x00F6,
    [DE_SS] = 0x00DF,
};

#define HM_A LGUI_T(KC_A)
#define HM_S LALT_T(KC_S)
#define HM_D LSFT_T(KC_D)
#define HM_F LCTL_T(KC_F)
#define HM_J RCTL_T(KC_J)
#define HM_K RSFT_T(KC_K)
#define HM_L RALT_T(KC_L)
#define HM_SCLN RGUI_T(KC_SCLN)
#define NUM_BSP LT(L_NUM, KC_BSPC)
#define NAV_SPC LT(L_NAV, KC_SPC)

// clang-format off
const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [L_BASE] = LAYOUT(
        KC_GRV,          KC_1,           KC_2,           KC_3,           KC_4,           KC_5,               KC_6,           KC_7,           KC_8,           KC_9,           KC_0,           KC_BSPC,
        CK_SPOTLIGHT,    KC_Q,           KC_W,           KC_E,           KC_R,           KC_T,               KC_Y,           KC_U,           KC_I,           KC_O,           KC_P,           TD(TD_LAYERS),
        KC_TAB,          HM_A,           HM_S,           HM_D,           HM_F,           KC_G,               KC_H,           HM_J,           HM_K,           HM_L,           HM_SCLN,        KC_DEL,
        CK_CLIP_HIST,    KC_Z,           KC_X,           KC_C,           KC_V,           KC_B,               KC_N,           KC_M,           CK_COMMA_SCLN,  CK_DOT_COLN,    CK_QM_EXLM,     KC_SLSH,
                                              TD(TD_ESC_SCROLL_SLOW),  TD(TD_CLK_SYM),  NUM_BSP,            KC_ENT,         NAV_SPC,
                                                                       KC_NO,            KC_NO,              KC_NO
    ),

    [L_NUM] = LAYOUT(
        KC_F12,          KC_F1,          KC_F2,          KC_F3,          KC_F4,          KC_F5,              KC_F6,          KC_F7,          KC_F8,          KC_F9,          KC_F10,         KC_F11,
        XXXXXXX,         XXXXXXX,        X(DE_AE),       X(DE_UE),       X(DE_OE),       X(DE_SS),          KC_LBRC,        KC_7,           KC_8,           KC_9,           KC_RBRC,        XXXXXXX,
        XXXXXXX,         KC_LGUI,        KC_LALT,        KC_LSFT,        KC_LCTL,        XXXXXXX,            KC_PLUS,        KC_4,           KC_5,           KC_6,           KC_MINS,        KC_EQL,
        XXXXXXX,         XXXXXXX,        XXXXXXX,        CK_TART_LOGIN,  XXXXXXX,        XXXXXXX,            KC_ASTR,        KC_1,           KC_2,           KC_3,           KC_SLSH,        KC_DOT,
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
        KC_TILD,         KC_EXLM,        KC_AT,          KC_HASH,        KC_DLR,         KC_PERC,            KC_CIRC,        KC_AMPR,        KC_ASTR,        KC_LPRN,        KC_RPRN,        KC_PIPE,
        XXXXXXX,         KC_UNDS,        KC_LT,          KC_GT,          KC_BSPC,        CK_DELETE_LINE,     KC_GRV,         KC_MINS,        KC_LBRC,        KC_RBRC,        KC_ASTR,        KC_BSLS,
        XXXXXXX,         CK_HOME_DIR,    XXXXXXX,        KC_HASH,        CK_MOVE_LINE_DOWN, CK_ADD_BELOW,    CK_CODE_BLOCK,  KC_EQL,         KC_LCBR,        KC_RCBR,        KC_AT,          KC_SLSH,
        XXXXXXX,         XXXXXXX,        XXXXXXX,        XXXXXXX,        CK_MOVE_LINE_UP, CK_ADD_ABOVE,      XXXXXXX,        XXXXXXX,        XXXXXXX,        XXXXXXX,        XXXXXXX,        XXXXXXX,
                                              _______,                  _______,         _______,            _______,        _______,
                                                                       XXXXXXX,          XXXXXXX,            XXXXXXX
    ),

    [L_GAME] = LAYOUT(
        KC_GRV,          KC_1,           KC_2,           KC_3,           KC_4,           KC_5,               KC_6,           KC_7,           KC_8,           KC_9,           KC_0,           _______,
        _______,         KC_TAB,         KC_Q,           KC_W,           KC_E,           KC_R,               XXXXXXX,        XXXXXXX,        XXXXXXX,        XXXXXXX,        XXXXXXX,        _______,
        _______,         KC_LCTL,        KC_A,           KC_S,           KC_D,           KC_F,               XXXXXXX,        XXXXXXX,        XXXXXXX,        XXXXXXX,        XXXXXXX,        XXXXXXX,
        _______,         KC_LSFT,        KC_Z,           KC_X,           KC_C,           KC_V,               XXXXXXX,        XXXXXXX,        XXXXXXX,        XXXXXXX,        XXXXXXX,        XXXXXXX,
                                              XXXXXXX,                  KC_SPC,          KC_LALT,            XXXXXXX,        XXXXXXX,
                                                                       XXXXXXX,          XXXXXXX,            XXXXXXX
    ),

    [L_EXTRAS] = LAYOUT(
        QK_BOOT,         EE_CLR,         XXXXXXX,        XXXXXXX,        XXXXXXX,        XXXXXXX,            XXXXXXX,        XXXXXXX,        XXXXXXX,        XXXXXXX,        XXXXXXX,        XXXXXXX,
        KC_SLEP,         XXXXXXX,        XXXXXXX,        XXXXXXX,        KC_BRID,        KC_BRIU,            XXXXXXX,        KC_MPRV,        KC_MPLY,        KC_MSTP,        KC_MNXT,        XXXXXXX,
        XXXXXXX,         G(KC_Z),        G(KC_X),        G(KC_C),        G(KC_V),        XXXXXXX,            XXXXXXX,        KC_MUTE,        KC_VOLD,        KC_VOLU,        G(S(KC_4)),     XXXXXXX,
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
            tap_code16(G(S(KC_V)));
            return false;

        case CK_TMUX_DETACH:
            tap_tmux_prefix(KC_D);
            return false;

        case CK_TMUX_SYNC:
            tap_tmux_prefix(KC_SPC);
            return false;

        case CK_TMUX_NEXT_PANE:
            tap_tmux_prefix(KC_O);
            return false;

        case CK_TMUX_SHOW_SESS:
            tap_tmux_prefix(KC_S);
            return false;

        case CK_TMUX_NEW_SESS:
            SEND_STRING("tmux new-session -A -s ");
            return false;

        case CK_TMUX_H_SPLIT:
            tap_tmux_prefix(S(KC_QUOT));
            return false;

        case CK_TMUX_V_SPLIT:
            tap_tmux_prefix(S(KC_5));
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
            tap_code16(C(S(KC_K)));
            return false;

        case CK_CODE_BLOCK:
            SEND_STRING("```");
            return false;

        case CK_HOME_DIR:
            SEND_STRING("~/");
            return false;

        case CK_COMMA_SCLN:
            tap_morph_key(KC_COMM, KC_SCLN);
            return false;

        case CK_DOT_COLN:
            tap_morph_key(KC_DOT, KC_COLN);
            return false;

        case CK_QM_EXLM:
            tap_morph_key(KC_QUES, KC_EXLM);
            return false;

        case CK_TART_LOGIN:
            SEND_STRING("$Tart1976!26:05");
            tap_code(KC_ENT);
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
