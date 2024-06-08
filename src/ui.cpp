#include "./ui.hpp"

#include "raylib/raylib.h"
#include <cstdio>

namespace st {
namespace ui {

// sprite src
Rectangle get_sprite_src(SpriteName sprite_name) {
    switch (sprite_name) {
        case SpriteName::LEFT_ARROW_ICON_SRC:
            return {.x = 0.0, .y = 0.0, .width = -32.0, .height = 32.0};
        case SpriteName::RIGHT_ARROW_ICON_SRC:
            return {.x = 0.0, .y = 0.0, .width = 32.0, .height = 32.0};
        default: return {.x = 0.0, .y = 0.0, .width = -32.0, .height = 32.0};
    }
}

Texture texture;

Vector2 mouse_position;
bool is_lmb_down;
bool is_lmb_pressed;
bool is_lmb_released;

enum class ButtonState {
    COLD,
    HOVER,
    DOWN,
    RELEASE,
};

enum class RadioButtonState {
    COLD,
    HOVER,
    DOWN,
    RELEASE,
    SELECTED,
};

enum class IncrementButtonState {
    COLD,
    HOVER,
    DOWN,
};

struct Button {
    ButtonState state;

    Color color;
    Color tint;

    Button(Rectangle dst) {
        bool is_hover = CheckCollisionPointRec(mouse_position, dst);

        if (is_hover && is_lmb_released) {
            this->state = ButtonState::RELEASE;
            this->color = color::BUTTON_COLD;
            this->tint = color::SPRITE_COLD;
        } else if (is_hover && is_lmb_down) {
            this->state = ButtonState::DOWN;
            this->color = color::BUTTON_DOWN;
            this->tint = color::SPRITE_DOWN;
        } else if (is_hover) {
            this->state = ButtonState::HOVER;
            this->color = color::BUTTON_HOVER;
            this->tint = color::SPRITE_HOVER;
        } else {
            this->state = ButtonState::COLD;
            this->color = color::BUTTON_COLD;
            this->tint = color::SPRITE_COLD;
        }
    }

    bool as_bool() {
        return this->state == ButtonState::RELEASE;
    }
};

struct RadioButton {
    RadioButtonState state;

    Color color;
    Color tint;

    RadioButton(Rectangle dst, int *store, int value) {
        bool is_hover = CheckCollisionPointRec(mouse_position, dst);

        if (*store == value) {
            this->state = RadioButtonState::SELECTED;
            this->color = color::RADIO_BUTTON_SELECTED;
            this->tint = color::SPRITE_SELECTED;
        } else if (is_hover && is_lmb_released) {
            this->state = RadioButtonState::RELEASE;
            this->color = color::RADIO_BUTTON_SELECTED;
            this->tint = color::SPRITE_SELECTED;

            *store = value;
        } else if (is_hover && is_lmb_down) {
            this->state = RadioButtonState::DOWN;
            this->color = color::RADIO_BUTTON_DOWN;
            this->tint = color::SPRITE_DOWN;
        } else if (is_hover) {
            this->state = RadioButtonState::HOVER;
            this->color = color::RADIO_BUTTON_HOVER;
            this->tint = color::SPRITE_HOVER;
        } else {
            this->state = RadioButtonState::COLD;
            this->color = color::BUTTON_COLD;
            this->tint = color::SPRITE_COLD;
        }
    }

    bool as_bool() {
        return this->state == RadioButtonState::RELEASE;
    }
};

struct IncrementButton {
public:
    IncrementButtonState state;

    Color color;
    Color tint;

    IncrementButton(Rectangle dst, int *value, int speed, int min, int max) {
        static const double long_increment_period = 0.5;
        static const double short_increment_period = 0.033;
        static double last_increment_time = 0.0;

        bool is_hover = CheckCollisionPointRec(mouse_position, dst);

        if (is_hover && is_lmb_down) {
            this->state = IncrementButtonState::DOWN;
            this->color = color::INCREMENT_BUTTON_DOWN;
            this->tint = color::SPRITE_DOWN;

            double time = GetTime();
            if (last_increment_time <= 0.0) {
                last_increment_time = time + long_increment_period;
                *value += speed;
            } else if (time - last_increment_time >= 0.0) {
                last_increment_time = time + short_increment_period;
                *value += speed;
            }
        } else if (is_hover) {
            this->state = IncrementButtonState::HOVER;
            this->color = color::INCREMENT_BUTTON_HOVER;
            this->tint = color::SPRITE_HOVER;
            last_increment_time = 0.0;
        } else {
            this->state = IncrementButtonState::COLD;
            this->color = color::INCREMENT_BUTTON_COLD;
            this->tint = color::SPRITE_COLD;
        }
    }

    bool as_bool() {
        return this->state == IncrementButtonState::DOWN;
    }
};

void load() {
    texture = LoadTexture("./resources/sprites/ui_icons_32.png");
    SetTextureFilter(texture, TEXTURE_FILTER_BILINEAR);
}

void unload() {
    UnloadTexture(texture);
}

void begin() {
    mouse_position = GetMousePosition();
    is_lmb_down = IsMouseButtonDown(MOUSE_BUTTON_LEFT);
    is_lmb_pressed = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
    is_lmb_released = IsMouseButtonReleased(MOUSE_BUTTON_LEFT);
}

// -----------------------------------------------------------------------
// sprite
void sprite(Texture texture, Rectangle src, Rectangle dst) {
    DrawTexturePro(texture, src, dst, {0.0, 0.0}, 0.0, WHITE);
}
void sprite(SpriteName sprite_name, Rectangle dst) {
    Rectangle src = get_sprite_src(sprite_name);
    return sprite(texture, src, dst);
}

// -----------------------------------------------------------------------
// button
bool button_sprite(Texture texture, Rectangle src, Rectangle dst) {
    Button btn(dst);

    DrawTexturePro(texture, src, dst, {0.0, 0.0}, 0.0, btn.tint);
    return btn.as_bool();
}

bool button_sprite(SpriteName sprite_name, Rectangle dst) {
    Rectangle src = get_sprite_src(sprite_name);
    return button_sprite(texture, src, dst);
}

bool button_rect(Rectangle dst) {
    Button btn(dst);

    DrawRectangleRec(dst, btn.color);
    return btn.as_bool();
}

// -----------------------------------------------------------------------
// radio button
bool radio_button_sprite(
    Texture texture, Rectangle src, Rectangle dst, int *store, int value
) {
    RadioButton btn(dst, store, value);

    DrawTexturePro(texture, src, dst, {0.0, 0.0}, 0.0, btn.tint);
    return btn.as_bool();
}
bool radio_button_sprite(SpriteName sprite_name, Rectangle dst, int *store, int value) {
    Rectangle src = get_sprite_src(sprite_name);
    return radio_button_sprite(texture, src, dst, store, value);
}
bool radio_button_rect(Rectangle dst, int *store, int value) {
    RadioButton btn(dst, store, value);

    DrawRectangleRec(dst, btn.color);
    return btn.as_bool();
}

// -----------------------------------------------------------------------
// increment button
bool increment_button_sprite(
    Texture texture, Rectangle src, Rectangle dst, int *value, int speed, int min, int max
) {
    IncrementButton btn(dst, value, speed, min, max);

    DrawTexturePro(texture, src, dst, {0.0, 0.0}, 0.0, btn.tint);
    return btn.as_bool();
}
bool increment_button_sprite(
    SpriteName sprite_name, Rectangle dst, int *value, int speed, int min, int max
) {
    Rectangle src = get_sprite_src(sprite_name);
    return increment_button_sprite(texture, src, dst, value, speed, min, max);
}

bool increment_button_rect(Rectangle dst, int *value, int speed, int min, int max) {
    IncrementButton btn(dst, value, speed, min, max);

    DrawRectangleRec(dst, btn.color);
    return btn.as_bool();
}

}  // namespace ui

}  // namespace st
