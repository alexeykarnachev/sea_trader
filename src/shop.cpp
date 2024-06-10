#include "shop.hpp"

#include "cargo.hpp"
#include "components.hpp"
#include "entt/entity/fwd.hpp"
#include "raylib/raylib.h"
#include "registry.hpp"
#include "ui.hpp"
#include <algorithm>
#include <array>
#include <cstdio>
#include <string>

namespace st {
namespace shop {

static int SELECTED_PRODUCT_IDX = -1;
static bool IS_OPENED = false;
static cargo::Cargo DIFF_CARGO;
static cargo::Cargo *SHIP_CARGO;
static cargo::Cargo *PORT_CARGO;

void open(entt::entity port_entity) {
    auto player_entity = registry::registry.view<components::Player>().front();
    auto &ship = registry::registry.get<components::Ship>(player_entity);
    auto &port = registry::registry.get<components::Port>(port_entity);

    SHIP_CARGO = &ship.cargo;
    PORT_CARGO = &port.cargo;
    DIFF_CARGO.reset();
    IS_OPENED = true;
}

void close() {
    IS_OPENED = false;
}

bool check_if_opened() {
    return IS_OPENED;
}

const static float BORDER = 3.0;
const static float PAD = 5.0;
const static float GAP = 5.0;
const static int LARGE_FONT_SIZE = 30;
const static int MEDIUM_FONT_SIZE = 25;
const static int SMALL_FONT_SIZE = 20;

const static float WINDOW_WIDTH = 1200.0;
const static float WINDOW_HEIGHT = 900.0;

const static float HEADER_HEIGHT = 50;
const static float FOOTER_HEIGHT = 100;

const static float TRADE_BUTTON_WIDTH = 150.0;
const static float TRADE_BUTTON_HEIGHT = 50.0;

const static float FINAL_BUTTONS_CLOTHENESS = 0.2;

enum class Pivot {
    MID_MID,
    MID_TOP,
};

Rectangle normalize_rect(Rectangle rect, Pivot pivot) {
    switch (pivot) {
        case Pivot::MID_MID: {
            rect.x -= 0.5 * rect.width;
            rect.y -= 0.5 * rect.height;
            break;
        }
        case Pivot::MID_TOP: {
            rect.x -= 0.5 * rect.width;
            break;
        }
    }

    return rect;
}

Rectangle get_middle_square(Rectangle rect, float pad) {
    float size = std::min(rect.width, rect.height) - 2.0 * pad;

    Vector2 center = {
        .x = rect.x + 0.5f * rect.width,
        .y = rect.y + 0.5f * rect.height,
    };

    Rectangle square = {
        .x = center.x - 0.5f * size,
        .y = center.y - 0.5f * size,
        .width = size,
        .height = size,
    };

    return square;
}

Vector2 get_text_top_left(
    std::string text, int font_size, Vector2 position, Pivot pivot
) {
    float width = MeasureText(text.c_str(), font_size);
    Rectangle rect = {
        .x = position.x,
        .y = position.y,
        .width = width,
        .height = (float)font_size,
    };

    rect = normalize_rect(rect, pivot);
    return {rect.x, rect.y};
}

void draw_text_in_rect(Rectangle rect, std::string text, int font_size, Color color) {
    Vector2 center = {
        .x = rect.x + 0.5f * rect.width,
        .y = rect.y + 0.5f * rect.height,
    };
    Vector2 top_left = get_text_top_left(text, font_size, center, Pivot::MID_MID);
    DrawText(text.c_str(), top_left.x, top_left.y, font_size, color);
}

Rectangle split_and_draw_outer_border(Rectangle rect) {
    // top
    Rectangle border = rect;
    border.height = BORDER;
    DrawRectangleRec(border, ui::color::BORDER);

    // right
    border = rect;
    border.x += border.width - BORDER;
    border.width = BORDER;
    DrawRectangleRec(border, ui::color::BORDER);

    // bot
    border = rect;
    border.y += border.height - BORDER;
    border.height = BORDER;
    DrawRectangleRec(border, ui::color::BORDER);

    // left
    border = rect;
    border.width = BORDER;
    DrawRectangleRec(border, ui::color::BORDER);

    // erode rect
    rect = {
        .x = rect.x + BORDER,
        .y = rect.y + BORDER,
        .width = rect.width - 2.0f * BORDER,
        .height = rect.height - 2.0f * BORDER,
    };

    return rect;
}

struct RectangleSplit2 {
    Rectangle rect0;
    Rectangle rect1;
};

struct RectangleSplit3 {
    Rectangle rect0;
    Rectangle rect1;
    Rectangle rect2;
};

RectangleSplit3 split_top(Rectangle rect, float top_size, float mid_size) {
    Rectangle rect0 = rect;
    rect0.height = top_size;

    Rectangle rect1 = rect;
    rect1.height = mid_size;
    rect1.y = rect0.y + rect0.height;

    Rectangle rect2 = rect;
    rect2.y = rect1.y + rect1.height;
    rect2.height = rect.height - rect2.y + rect.y;

    RectangleSplit3 split = {
        .rect0 = rect0,
        .rect1 = rect1,
        .rect2 = rect2,
    };
    return split;
}

RectangleSplit3 split_bot(Rectangle rect, float bot_size, float mid_size) {
    float top_size = rect.height - bot_size - mid_size;
    return split_top(rect, top_size, mid_size);
}

RectangleSplit3 split_left(Rectangle rect, float left_size, float mid_size) {
    Rectangle rect0 = rect;
    rect0.width = left_size;

    Rectangle rect1 = rect;
    rect1.width = mid_size;
    rect1.x = rect0.x + rect0.width;

    Rectangle rect2 = rect;
    rect2.x = rect1.x + rect1.width;
    rect2.width = rect.width - rect2.x + rect.x;

    RectangleSplit3 split = {
        .rect0 = rect0,
        .rect1 = rect1,
        .rect2 = rect2,
    };
    return split;
}

RectangleSplit2 split_top_and_draw_mid(
    Rectangle rect, float top_size, float mid_size, Color mid_color
) {
    RectangleSplit3 split = split_top(rect, top_size, mid_size);
    DrawRectangleRec(split.rect1, mid_color);
    return {.rect0 = split.rect0, .rect1 = split.rect2};
}

RectangleSplit2 split_bot_and_draw_mid(
    Rectangle rect, float bot_size, float mid_size, Color mid_color
) {
    RectangleSplit3 split = split_bot(rect, bot_size, mid_size);
    DrawRectangleRec(split.rect1, mid_color);
    return {.rect0 = split.rect0, .rect1 = split.rect2};
}

RectangleSplit2 split_left_and_draw_mid(
    Rectangle rect, float left_size, float mid_size, Color mid_color
) {
    RectangleSplit3 split = split_left(rect, left_size, mid_size);
    DrawRectangleRec(split.rect1, mid_color);
    return {.rect0 = split.rect0, .rect1 = split.rect2};
}

RectangleSplit2 split_left_and_draw_border(Rectangle rect, float left_size) {
    return split_left_and_draw_mid(rect, left_size, BORDER, ui::color::BORDER);
}

float get_col_width(int col_idx, float full_width) {
    static const int n_cols = 5;
    static const int product_col_idx = 2;
    static const float product_col_scale = 1.8;

    float col_width = (full_width - BORDER * (n_cols - 1)) / n_cols;

    if (col_idx == product_col_idx) {
        col_width *= product_col_scale;
    } else {
        col_width -= col_width * (product_col_scale - 1.0) / (n_cols - 1);
    }

    return col_width;
}

void draw_header(Rectangle rect) {
    static const int n_cols = 5;
    static std::array<std::string, n_cols> col_names = {
        "Ship",
        "Sell Price",
        "Product",
        "Buy Price",
        "Port",
    };

    float full_width = rect.width;
    for (int i = 0; i < n_cols; ++i) {
        float col_width = get_col_width(i, full_width);
        auto text = col_names[i];
        RectangleSplit2 split = split_left_and_draw_border(rect, col_width);
        rect = split.rect1;

        DrawRectangleRec(split.rect0, ui::color::RECT_COLD);
        draw_text_in_rect(split.rect0, text, LARGE_FONT_SIZE, ui::color::TEXT_LIGHT);
    }
}

void draw_row(Rectangle rect, int row_idx) {
    static const int n_cols = 5;

    int font_size = MEDIUM_FONT_SIZE;

    Color text_color;
    if (SELECTED_PRODUCT_IDX == row_idx) {
        text_color = ui::color::TEXT_DARK;
    } else {
        text_color = ui::color::TEXT_MILD;
    }

    float full_width = rect.width;
    for (int i = 0; i < n_cols; ++i) {
        float col_width = get_col_width(i, full_width);
        RectangleSplit2 split = split_left_and_draw_border(rect, col_width);
        Rectangle cell_rect = split.rect0;
        rect = split.rect1;

        std::string text;
        auto ship_product = &SHIP_CARGO->products[row_idx];
        auto port_product = &PORT_CARGO->products[row_idx];
        int *diff_n_units = &DIFF_CARGO.products[row_idx].n_units;
        switch (i) {
            case 0: {  // ship amount
                int ship_n_units = ship_product->n_units + (*diff_n_units);
                text = std::to_string(ship_n_units);
                draw_text_in_rect(cell_rect, text, font_size, text_color);
                break;
            }
            case 1:  // sell price
                text = std::to_string(row_idx);
                draw_text_in_rect(cell_rect, text, font_size, text_color);
                break;
            case 2: {  // product
                if (SELECTED_PRODUCT_IDX == row_idx) {
                    float icon_size = cell_rect.height - 2.0 * PAD;
                    Rectangle dst = {
                        .x = cell_rect.x + PAD,
                        .y = cell_rect.y + PAD,
                        .width = icon_size,
                        .height = icon_size,
                    };

                    auto sprite = ui::SpriteName::LEFT_ARROW_ICON;
                    ui::increment_button_sprite(sprite, dst, diff_n_units, +1, 0, 100);

                    dst.x = cell_rect.x + cell_rect.width - PAD - icon_size;
                    sprite = ui::SpriteName::RIGHT_ARROW_ICON;
                    ui::increment_button_sprite(sprite, dst, diff_n_units, -1, 0, 100);
                }

                text = ship_product->name;
                draw_text_in_rect(cell_rect, text, font_size, text_color);
                break;
            }
            case 3:  // buy price
                text = std::to_string(row_idx);
                draw_text_in_rect(cell_rect, text, font_size, text_color);
                break;
            case 4: {  // port amount
                int port_n_units = port_product->n_units - (*diff_n_units);
                text = std::to_string(port_n_units);
                draw_text_in_rect(cell_rect, text, font_size, text_color);
                break;
            }
        }
    }
}

void draw_rows(Rectangle rect) {
    static const int n_rows = cargo::N_PRODUCTS;

    float row_height = (rect.height - BORDER * (n_rows - 1)) / n_rows;

    for (int i = 0; i < n_rows; ++i) {
        RectangleSplit2 split = split_top_and_draw_mid(
            rect, row_height, BORDER, ui::color::BORDER
        );
        ui::radio_button_rect(split.rect0, &SELECTED_PRODUCT_IDX, i);

        draw_row(split.rect0, i);
        rect = split.rect1;
    }
}

void draw_stats_cell(Rectangle rect, bool is_ship) {
    auto cargo = is_ship ? SHIP_CARGO : PORT_CARGO;
    auto who = is_ship ? "Ship" : "Port";

    float row_height = rect.height / 3.0;
    DrawRectangleRec(rect, ui::color::RECT_COLD);

    RectangleSplit3 split = split_top(rect, row_height, 0.0);
    Rectangle who_rect = split.rect0;
    rect = split.rect2;

    split = split_top(rect, row_height, 0.0);
    Rectangle cap_rect = split.rect0;
    Rectangle gold_rect = split.rect2;

    draw_text_in_rect(who_rect, who, MEDIUM_FONT_SIZE, ui::color::TEXT_LIGHT);
    draw_text_in_rect(
        cap_rect, "Capacity: 228 / 1488", SMALL_FONT_SIZE, ui::color::TEXT_MILD
    );
    draw_text_in_rect(gold_rect, "Gold: 1422888", SMALL_FONT_SIZE, ui::color::TEXT_MILD);
}

void draw_buttons_cell(Rectangle rect) {
    DrawRectangleRec(rect, ui::color::RECT_COLD);

    float row_height = rect.height / 2.0;
    RectangleSplit3 split = split_top(rect, row_height, 0.0);

    Rectangle summary_rect = split.rect0;
    Rectangle buttons_rect = split.rect2;
    buttons_rect.x += buttons_rect.width * FINAL_BUTTONS_CLOTHENESS;
    buttons_rect.width -= buttons_rect.width * 2.0 * FINAL_BUTTONS_CLOTHENESS;

    split = split_left(buttons_rect, 0.5 * buttons_rect.width, 0.0);
    Rectangle accept_button_rect = get_middle_square(split.rect0, PAD);
    Rectangle cancel_button_rect = get_middle_square(split.rect2, PAD);

    draw_text_in_rect(
        summary_rect, "Total: -6969 GOLD", LARGE_FONT_SIZE, ui::color::TEXT_BUY
    );
    ui::button_sprite(ui::SpriteName::ACCEPT_ICON, accept_button_rect);
    ui::button_sprite(ui::SpriteName::CANCEL_ICON, cancel_button_rect);
}

void draw_footer(Rectangle rect) {
    float full_width = rect.width;

    float col_width = get_col_width(0, full_width);
    RectangleSplit2 split = split_left_and_draw_border(rect, 2.0 * col_width + BORDER);
    Rectangle ship_rect = split.rect0;
    rect = split.rect1;

    col_width = get_col_width(2, full_width);
    split = split_left_and_draw_border(rect, col_width);
    Rectangle buttons_rect = split.rect0;
    Rectangle port_rect = split.rect1;

    draw_stats_cell(ship_rect, true);
    draw_stats_cell(port_rect, false);
    draw_buttons_cell(buttons_rect);
}

void update_and_draw() {
    if (!IS_OPENED) return;

    if (IsKeyPressed(KEY_ESCAPE)) {
        close();
        return;
    }

    int screen_width = GetScreenWidth();
    int screen_height = GetScreenHeight();
    Rectangle rect = {
        .x = 0.5f * (screen_width - WINDOW_WIDTH),
        .y = 0.5f * (screen_height - WINDOW_HEIGHT),
        .width = WINDOW_WIDTH,
        .height = WINDOW_HEIGHT,
    };

    RectangleSplit2 split;

    rect = split_and_draw_outer_border(rect);
    split = split_top_and_draw_mid(rect, HEADER_HEIGHT, BORDER, ui::color::BORDER);
    Rectangle header_rect = split.rect0;

    split = split_bot_and_draw_mid(split.rect1, FOOTER_HEIGHT, BORDER, ui::color::BORDER);
    Rectangle rows_rect = split.rect0;
    Rectangle footer_rect = split.rect1;

    draw_header(header_rect);
    draw_rows(rows_rect);
    draw_footer(footer_rect);
}

}  // namespace shop
}  // namespace st
