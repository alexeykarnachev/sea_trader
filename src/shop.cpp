#include "shop.hpp"

#include "cargo.hpp"
#include "components.hpp"
#include "entt/entity/fwd.hpp"
#include "raylib/raylib.h"
#include "registry.hpp"
#include "ship.hpp"
#include "ui.hpp"
#include <algorithm>
#include <array>
#include <climits>
#include <cstdio>
#include <string>

namespace st {
namespace shop {

static int SELECTED_PRODUCT_IDX = -1;
static bool IS_OPENED = false;
static entt::entity PORT_ENTITY;
static cargo::Cargo *SHIP_CARGO_P;
static cargo::Cargo *PORT_CARGO_P;
static cargo::Cargo SHIP_CARGO_ORIG;
static components::Money *SHIP_MONEY_P;
static components::Money *PORT_MONEY_P;
static components::Money SHIP_MONEY_ORIG;
static components::Money PORT_MONEY_ORIG;

void open(entt::entity port_entity) {
    PORT_ENTITY = port_entity;

    auto player_entity = registry::registry.view<components::Player>().front();
    auto &ship = registry::registry.get<ship::Ship>(player_entity);
    auto &port = registry::registry.get<components::Port>(port_entity);

    SHIP_MONEY_P = &registry::registry.get<components::Money>(player_entity);
    PORT_MONEY_P = &registry::registry.get<components::Money>(port_entity);

    SHIP_CARGO_ORIG = ship.cargo;
    SHIP_MONEY_ORIG = *SHIP_MONEY_P;
    PORT_MONEY_ORIG = *PORT_MONEY_P;

    SHIP_CARGO_P = &ship.cargo;
    PORT_CARGO_P = &port.cargo;

    IS_OPENED = true;
}

void reset_deal() {
    *SHIP_CARGO_P = SHIP_CARGO_ORIG;
    *SHIP_MONEY_P = SHIP_MONEY_ORIG;
    *PORT_MONEY_P = PORT_MONEY_ORIG;
}

void accept_deal() {
    open(PORT_ENTITY);
}

void close_and_resed_deal() {
    reset_deal();
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
        auto ship_product = &SHIP_CARGO_P->products[row_idx];
        auto port_product = &PORT_CARGO_P->products[row_idx];
        switch (i) {
            case 0: {  // ship amount
                text = std::to_string(ship_product->n_units);
                draw_text_in_rect(cell_rect, text, font_size, text_color);
                break;
            }
            case 1: {  // ship sell price (port buy price)
                int port_buy_price = port_product->get_sell_price();
                text = std::to_string(port_buy_price);
                draw_text_in_rect(cell_rect, text, font_size, text_color);
                break;
            }
            case 2: {  // product
                if (SELECTED_PRODUCT_IDX == row_idx) {
                    float icon_size = cell_rect.height - 2.0 * PAD;
                    Rectangle dst = {
                        .x = cell_rect.x + PAD,
                        .y = cell_rect.y + PAD,
                        .width = icon_size,
                        .height = icon_size,
                    };

                    int speed;
                    if (IsKeyDown(KEY_LEFT_CONTROL)) {
                        speed = 10;
                    } else if (IsKeyDown(KEY_LEFT_SHIFT)) {
                        speed = port_product->n_units + ship_product->n_units;
                    } else {
                        speed = 1;
                    }

                    // buy
                    int ship_increment_n = ui::increment_button_sprite(
                        ui::SpriteName::LEFT_ARROW_ICON, dst, speed
                    );

                    int ship_has_cargo_for_n = SHIP_CARGO_P->get_free_weight()
                                               / port_product->unit_weight;
                    int ship_has_money_for_n = SHIP_MONEY_P->value
                                               / port_product->get_sell_price();
                    int ship_max_n_buy = std::min(
                        ship_has_cargo_for_n, port_product->n_units
                    );
                    ship_max_n_buy = std::min(ship_has_money_for_n, ship_max_n_buy);
                    ship_increment_n = std::min(ship_increment_n, ship_max_n_buy);

                    // sell
                    dst.x = cell_rect.x + cell_rect.width - PAD - icon_size;
                    int port_increment_n = ui::increment_button_sprite(
                        ui::SpriteName::RIGHT_ARROW_ICON, dst, speed
                    );

                    int port_has_cargo_for_n = PORT_CARGO_P->get_free_weight()
                                               / port_product->unit_weight;
                    int port_has_money_for_n = PORT_MONEY_P->value
                                               / port_product->get_buy_price();
                    int port_max_n_buy = std::min(
                        port_has_cargo_for_n, ship_product->n_units
                    );
                    port_max_n_buy = std::min(port_has_money_for_n, port_max_n_buy);
                    port_increment_n = std::min(port_increment_n, port_max_n_buy);

                    // apply changes
                    int ship_money_spent = ship_increment_n
                                           * port_product->get_sell_price();
                    int ship_money_received = port_increment_n
                                              * port_product->get_buy_price();
                    int port_money_spent = port_increment_n
                                           * port_product->get_buy_price();
                    int port_money_received = ship_increment_n
                                              * port_product->get_sell_price();

                    SHIP_MONEY_P->value += ship_money_received - ship_money_spent;
                    PORT_MONEY_P->value += port_money_received - port_money_spent;

                    ship_product->n_units += ship_increment_n - port_increment_n;
                    port_product->n_units += port_increment_n - ship_increment_n;
                }

                text = ship_product->get_name();
                draw_text_in_rect(cell_rect, text, font_size, text_color);
                break;
            }
            case 3: {  // ship buy price (port sell price)
                int port_sell_price = port_product->get_sell_price();
                text = std::to_string(port_sell_price);
                draw_text_in_rect(cell_rect, text, font_size, text_color);
                break;
            }
            case 4: {  // port amount
                text = std::to_string(port_product->n_units);
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
    auto who = is_ship ? "Ship" : "Port";
    auto cargo = is_ship ? SHIP_CARGO_P : PORT_CARGO_P;
    auto money = is_ship ? SHIP_MONEY_P : PORT_MONEY_P;

    float row_height = rect.height / 3.0;
    DrawRectangleRec(rect, ui::color::RECT_COLD);

    RectangleSplit3 split = split_top(rect, row_height, 0.0);
    Rectangle who_rect = split.rect0;
    rect = split.rect2;

    split = split_top(rect, row_height, 0.0);
    Rectangle cap_rect = split.rect0;
    Rectangle gold_rect = split.rect2;

    // who
    draw_text_in_rect(who_rect, who, MEDIUM_FONT_SIZE, ui::color::TEXT_LIGHT);

    // capacity
    auto weight_str = std::to_string(cargo->get_weight());
    auto capacity_str = std::to_string(cargo->capacity);
    capacity_str = "Capacity: " + weight_str + " / " + capacity_str;
    draw_text_in_rect(cap_rect, capacity_str, SMALL_FONT_SIZE, ui::color::TEXT_MILD);

    // money
    auto money_str = std::to_string(money->value) + " $";
    draw_text_in_rect(gold_rect, money_str, SMALL_FONT_SIZE, ui::color::TEXT_MILD);
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

    int money_diff = SHIP_MONEY_P->value - SHIP_MONEY_ORIG.value;
    Color money_diff_color;
    std::string sign_str;
    if (money_diff > 0) {
        money_diff_color = ui::color::TEXT_POSITIVE;
        sign_str = "+";
    } else if (money_diff < 0) {
        money_diff_color = ui::color::TEXT_NEGATIVE;
        sign_str = "";
    } else {
        money_diff_color = ui::color::TEXT_MILD;
        sign_str = "";
    }
    auto money_diff_str = sign_str + std::to_string(money_diff) + "$";
    draw_text_in_rect(summary_rect, money_diff_str, LARGE_FONT_SIZE, money_diff_color);

    bool is_accept = ui::button_sprite(ui::SpriteName::ACCEPT_ICON, accept_button_rect);
    bool is_cancel = ui::button_sprite(ui::SpriteName::CANCEL_ICON, cancel_button_rect);
    if (is_accept) accept_deal();
    else if (is_cancel) close_and_resed_deal();
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
        close_and_resed_deal();
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
