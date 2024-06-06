#include "./ui.hpp"

#include <cstdio>
#include <string>

namespace rl {
#include "raylib/raylib.h"
}  // namespace rl

namespace st {
namespace ui {

// line
const rl::Color LINE_LIGHT_COLOR = {245, 245, 245, 255};
const rl::Color LINE_MILD_COLOR = {150, 150, 150, 255};
const rl::Color LINE_DARK_COLOR = {10, 10, 10, 255};

// rect
const rl::Color RECT_COLD_COLOR = {70, 50, 30, 255};
const rl::Color RECT_HOVER_COLOR = {110, 90, 70, 255};

// sprite
const rl::Color SPRITE_COLD_TINT = {90, 70, 50, 255};
const rl::Color SPRITE_HOVER_TINT = {110, 90, 70, 255};
const rl::Color SPRITE_DOWN_TINT = {245, 245, 245, 255};
const rl::Color SPRITE_SELECTED_TINT = {245, 245, 245, 255};

// text
const rl::Color TEXT_LIGHT_COLOR = {245, 245, 245, 255};
const rl::Color TEXT_MILD_COLOR = {150, 150, 150, 255};
const rl::Color TEXT_DARK_COLOR = {10, 10, 10, 255};
const rl::Color TEXT_INFO_COLOR = {50, 255, 50, 255};
const rl::Color TEXT_ERROR_COLOR = {255, 50, 50, 255};

// button
const rl::Color BUTTON_COLD_COLOR = {90, 70, 50, 255};
const rl::Color BUTTON_HOVER_COLOR = {110, 90, 70, 255};
const rl::Color BUTTON_DOWN_COLOR = {140, 120, 100, 255};

// radio button
const rl::Color RADIO_BUTTON_COLD_COLOR = {80, 60, 40, 255};
const rl::Color RADIO_BUTTON_HOVER_COLOR = {100, 80, 60, 255};
const rl::Color RADIO_BUTTON_DOWN_COLOR = {140, 120, 100, 255};
const rl::Color RADIO_BUTTON_SELECTED_COLOR = {140, 120, 100, 255};

// increment button
const rl::Color INCREMENT_BUTTON_COLD_COLOR = {80, 60, 40, 255};
const rl::Color INCREMENT_BUTTON_HOVER_COLOR = {100, 80, 60, 255};
const rl::Color INCREMENT_BUTTON_DOWN_COLOR = {140, 120, 100, 255};

// sprite src
rl::Rectangle get_sprite_src(SpriteName sprite_name) {
    switch (sprite_name) {
        case SpriteName::LEFT_ARROW_ICON_SRC:
            return {.x = 0.0, .y = 0.0, .width = -32.0, .height = 32.0};
        case SpriteName::RIGHT_ARROW_ICON_SRC:
            return {.x = 0.0, .y = 0.0, .width = 32.0, .height = 32.0};
        default: return {.x = 0.0, .y = 0.0, .width = -32.0, .height = 32.0};
    }
}

rl::Texture texture;

rl::Vector2 mouse_position;
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

    rl::Color color;
    rl::Color tint;

    Button(rl::Rectangle dst) {
        bool is_hover = rl::CheckCollisionPointRec(mouse_position, dst);

        if (is_hover && is_lmb_released) {
            this->state = ButtonState::RELEASE;
            this->color = BUTTON_COLD_COLOR;
            this->tint = SPRITE_COLD_TINT;
        } else if (is_hover && is_lmb_down) {
            this->state = ButtonState::DOWN;
            this->color = BUTTON_DOWN_COLOR;
            this->tint = SPRITE_DOWN_TINT;
        } else if (is_hover) {
            this->state = ButtonState::HOVER;
            this->color = BUTTON_HOVER_COLOR;
            this->tint = SPRITE_HOVER_TINT;
        } else {
            this->state = ButtonState::COLD;
            this->color = BUTTON_COLD_COLOR;
            this->tint = SPRITE_COLD_TINT;
        }
    }

    bool as_bool() {
        return this->state == ButtonState::RELEASE;
    }
};

struct RadioButton {
    RadioButtonState state;

    rl::Color color;
    rl::Color tint;

    RadioButton(rl::Rectangle dst, int *store, int value) {
        bool is_hover = rl::CheckCollisionPointRec(mouse_position, dst);

        if (*store == value) {
            this->state = RadioButtonState::SELECTED;
            this->color = RADIO_BUTTON_SELECTED_COLOR;
            this->tint = SPRITE_SELECTED_TINT;
        } else if (is_hover && is_lmb_released) {
            this->state = RadioButtonState::RELEASE;
            this->color = RADIO_BUTTON_SELECTED_COLOR;
            this->tint = SPRITE_SELECTED_TINT;

            *store = value;
        } else if (is_hover && is_lmb_down) {
            this->state = RadioButtonState::DOWN;
            this->color = RADIO_BUTTON_DOWN_COLOR;
            this->tint = SPRITE_DOWN_TINT;
        } else if (is_hover) {
            this->state = RadioButtonState::HOVER;
            this->color = RADIO_BUTTON_HOVER_COLOR;
            this->tint = SPRITE_HOVER_TINT;
        } else {
            this->state = RadioButtonState::COLD;
            this->color = BUTTON_COLD_COLOR;
            this->tint = SPRITE_COLD_TINT;
        }
    }

    bool as_bool() {
        return this->state == RadioButtonState::RELEASE;
    }
};

struct IncrementButton {
    const double first_increment_period = 0.5;
    const double next_increments_period = 0.033;

    IncrementButtonState state;

    rl::Color color;
    rl::Color tint;

    IncrementButton(
        rl::Rectangle dst,
        double *last_increment_time,
        int *value,
        int speed,
        int min,
        int max
    ) {
        bool is_hover = rl::CheckCollisionPointRec(mouse_position, dst);

        if (is_hover && is_lmb_down) {
            this->state = IncrementButtonState::DOWN;
            this->color = INCREMENT_BUTTON_DOWN_COLOR;
            this->tint = SPRITE_DOWN_TINT;

            double time = rl::GetTime();
            double dt = time - (*last_increment_time);

            if (*last_increment_time <= 0.0) {
                *last_increment_time = time + this->first_increment_period;
                *value += speed;
            } else if (dt >= 0.0) {
                *last_increment_time = time + this->next_increments_period;
                *value += speed;
            }
        } else if (is_hover) {
            this->state = IncrementButtonState::HOVER;
            this->color = INCREMENT_BUTTON_HOVER_COLOR;
            this->tint = SPRITE_HOVER_TINT;
            *last_increment_time = 0.0;
        } else {
            this->state = IncrementButtonState::COLD;
            this->color = INCREMENT_BUTTON_COLD_COLOR;
            this->tint = SPRITE_COLD_TINT;
            *last_increment_time = 0.0;
        }
    }

    bool as_bool() {
        return this->state == IncrementButtonState::DOWN;
    }
};

void load() {
    texture = rl::LoadTexture("./resources/sprites/ui_icons_32.png");
    SetTextureFilter(texture, rl::TEXTURE_FILTER_BILINEAR);
}

void unload() {
    rl::UnloadTexture(texture);
}

void begin() {
    mouse_position = rl::GetMousePosition();
    is_lmb_down = rl::IsMouseButtonDown(rl::MOUSE_BUTTON_LEFT);
    is_lmb_pressed = rl::IsMouseButtonPressed(rl::MOUSE_BUTTON_LEFT);
    is_lmb_released = rl::IsMouseButtonReleased(rl::MOUSE_BUTTON_LEFT);
}

// -----------------------------------------------------------------------
// line
void line(float x0, float y0, float x1, float y1, rl::Color color) {
    rl::DrawLine(x0, y0, x1, y1, color);
}

void line_light(float x0, float y0, float x1, float y1) {
    return line(x0, y0, x1, y1, LINE_LIGHT_COLOR);
}

void line_mild(float x0, float y0, float x1, float y1) {
    return line(x0, y0, x1, y1, LINE_MILD_COLOR);
}

void line_dark(float x0, float y0, float x1, float y1) {
    return line(x0, y0, x1, y1, LINE_LIGHT_COLOR);
}

// -----------------------------------------------------------------------
// rect
void rect(rl::Rectangle dst, rl::Color color) {
    rl::DrawRectangleRec(dst, color);
}

void rect_cold(rl::Rectangle dst) {
    return rect(dst, RECT_COLD_COLOR);
}

void rect_hover(rl::Rectangle dst) {
    return rect(dst, RECT_HOVER_COLOR);
}

// -----------------------------------------------------------------------
// sprite
void sprite(rl::Texture texture, rl::Rectangle src, rl::Rectangle dst) {
    rl::DrawTexturePro(texture, src, dst, {0.0, 0.0}, 0.0, rl::WHITE);
}
void sprite(SpriteName sprite_name, rl::Rectangle dst) {
    rl::Rectangle src = get_sprite_src(sprite_name);
    return sprite(texture, src, dst);
}

// -----------------------------------------------------------------------
// text
void text(std::string str, float x, float y, int size, rl::Color color) {
    rl::DrawText(str.c_str(), x, y, size, color);
}

void text_light(std::string str, float x, float y, int size) {
    return text(str, x, y, size, TEXT_LIGHT_COLOR);
}

void text_mild(std::string str, float x, float y, int size) {
    return text(str, x, y, size, TEXT_MILD_COLOR);
}

void text_dark(std::string str, float x, float y, int size) {
    return text(str, x, y, size, TEXT_DARK_COLOR);
}

void text_info(std::string str, float x, float y, int size) {
    return text(str, x, y, size, TEXT_INFO_COLOR);
}

void text_error(std::string str, float x, float y, int size) {
    return text(str, x, y, size, TEXT_ERROR_COLOR);
}

// -----------------------------------------------------------------------
// button
bool button_sprite(rl::Texture texture, rl::Rectangle src, rl::Rectangle dst) {
    Button btn(dst);

    rl::DrawTexturePro(texture, src, dst, {0.0, 0.0}, 0.0, btn.tint);
    return btn.as_bool();
}

bool button_sprite(SpriteName sprite_name, rl::Rectangle dst) {
    rl::Rectangle src = get_sprite_src(sprite_name);
    return button_sprite(texture, src, dst);
}

bool button_rect(rl::Rectangle dst) {
    Button btn(dst);

    rl::DrawRectangleRec(dst, btn.color);
    return btn.as_bool();
}

// -----------------------------------------------------------------------
// radio button
bool radio_button_sprite(
    rl::Texture texture, rl::Rectangle src, rl::Rectangle dst, int *store, int value
) {
    RadioButton btn(dst, store, value);

    rl::DrawTexturePro(texture, src, dst, {0.0, 0.0}, 0.0, btn.tint);
    return btn.as_bool();
}
bool radio_button_sprite(
    SpriteName sprite_name, rl::Rectangle dst, int *store, int value
) {
    rl::Rectangle src = get_sprite_src(sprite_name);
    return radio_button_sprite(texture, src, dst, store, value);
}
bool radio_button_rect(rl::Rectangle dst, int *store, int value) {
    RadioButton btn(dst, store, value);

    rl::DrawRectangleRec(dst, btn.color);
    return btn.as_bool();
}

// -----------------------------------------------------------------------
// increment button
bool increment_button_sprite(
    rl::Texture texture,
    rl::Rectangle src,
    rl::Rectangle dst,
    double *last_increment_time,
    int *value,
    int speed,
    int min,
    int max
) {
    IncrementButton btn(dst, last_increment_time, value, speed, min, max);

    rl::DrawTexturePro(texture, src, dst, {0.0, 0.0}, 0.0, btn.tint);
    return btn.as_bool();
}
bool increment_button_sprite(
    SpriteName sprite_name,
    rl::Rectangle dst,
    double *last_increment_time,
    int *value,
    int speed,
    int min,
    int max
) {
    rl::Rectangle src = get_sprite_src(sprite_name);
    return increment_button_sprite(
        texture, src, dst, last_increment_time, value, speed, min, max
    );
}

bool increment_button_rect(
    rl::Rectangle dst,
    double *last_increment_time,
    int *value,
    int speed,
    int min,
    int max
) {
    IncrementButton btn(dst, last_increment_time, value, speed, min, max);

    rl::DrawRectangleRec(dst, btn.color);
    return btn.as_bool();
}

}  // namespace ui

}  // namespace st
