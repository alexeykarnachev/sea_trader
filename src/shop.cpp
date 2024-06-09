#include "shop.hpp"

#include "cargo.hpp"
#include "components.hpp"
#include "entt/entity/fwd.hpp"
#include "raylib/raylib.h"
#include "registry.hpp"
#include "renderer.hpp"
#include "resources.hpp"
#include "ui.hpp"
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

const static float BORDER = 5.0;
const static float PAD = 5.0;
const static float GAP = 5.0;
const static int LARGE_FONT_SIZE = 30;
const static int MEDIUM_FONT_SIZE = 25;

const static float WINDOW_WIDTH = 1200.0;
const static float WINDOW_HEIGHT = 900.0;

const static float HEADER_HEIGHT = 50;
const static float FOOTER_HEIGHT = 100;

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

void draw_header(Rectangle rect) {
    static const int n_cols = 5;
    static std::array<std::string, n_cols> col_names = {
        "Ship",
        "Sell Price",
        "Product",
        "Buy Price",
        "Port",
    };

    float col_width = (rect.width - BORDER * (n_cols - 1)) / n_cols;
    int font_size = LARGE_FONT_SIZE;

    for (auto text : col_names) {
        RectangleSplit2 split = split_left_and_draw_mid(
            rect, col_width, BORDER, ui::color::BORDER
        );
        rect = split.rect1;

        DrawRectangleRec(split.rect0, ui::color::RECT_COLD);
        draw_text_in_rect(split.rect0, text, LARGE_FONT_SIZE, ui::color::TEXT_LIGHT);
    }
}

void draw_row(Rectangle rect, int row_idx) {
    static const int n_cols = 5;
    float col_width = (rect.width - BORDER * (n_cols - 1)) / n_cols;

    int font_size = MEDIUM_FONT_SIZE;

    Color text_color;
    if (SELECTED_PRODUCT_IDX == row_idx) {
        text_color = ui::color::TEXT_DARK;
    } else {
        text_color = ui::color::TEXT_MILD;
    }

    for (int i = 0; i < n_cols; ++i) {
        RectangleSplit2 split = split_left_and_draw_mid(
            rect, col_width, BORDER, ui::color::BORDER
        );
        Rectangle cell_rect = split.rect0;
        rect = split.rect1;

        std::string text;
        auto ship_product = &SHIP_CARGO->products[row_idx];
        auto port_product = &PORT_CARGO->products[row_idx];
        switch (i) {
            case 0: {  // ship amount
                text = std::to_string(ship_product->n_units);
                break;
            }
            case 1:  // sell price
                text = "sell price";
                break;
            case 2: {  // product
                text = ship_product->name;
                break;
            }
            case 3:  // buy price
                text = "buy price";
                break;
            case 4: {  // port amount
                text = std::to_string(port_product->n_units);
                break;
            }
        }

        draw_text_in_rect(cell_rect, text, font_size, text_color);
    }
}

void draw_rows(Rectangle rect) {
    static const int n_rows = cargo::N_PRODUCTS;

    float row_height = (rect.height - BORDER * (n_rows - 1)) / n_rows;
    int font_size = MEDIUM_FONT_SIZE;

    for (int i = 0; i < n_rows; ++i) {
        RectangleSplit2 split = split_top_and_draw_mid(
            rect, row_height, BORDER, ui::color::BORDER
        );
        ui::radio_button_rect(split.rect0, &SELECTED_PRODUCT_IDX, i);

        draw_row(split.rect0, i);
        rect = split.rect1;
    }
}

void draw_footer(Rectangle rect) {
    DrawRectangleRec(rect, BLUE);
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

#if 0
void update_and_draw() {
    if (!IS_OPENED) return;

    if (IsKeyPressed(KEY_ESCAPE)) {
        close();
        return;
    }

    renderer::set_screen_camera(resources::SPRITE_SHADER);
    auto &ship = registry::registry.get<components::Ship>(PLAYER_ENTITY);
    auto &port = registry::registry.get<components::Port>(PORT_ENTITY);

    int screen_width = GetScreenWidth();
    int screen_height = GetScreenHeight();

    // ---------------------------------------------------------------
    // pane
    float pane_x = 0.5 * (screen_width - PANE_WIDTH);
    float pane_y = 0.5 * (screen_height - PANE_HEIGHT);
    Rectangle pane_rect = {
        .x = pane_x, .y = pane_y, .width = PANE_WIDTH, .height = PANE_HEIGHT
    };
    DrawRectangleRec(pane_rect, ui::color::RECT_COLD);

    const float row_x = pane_x + PANE_BORDER;
    const float mid_x = pane_x + 0.5 * PANE_WIDTH;

    // ---------------------------------------------------------------
    // header
    const float header_y = pane_y + PANE_BORDER;
    Rectangle header_rect = {
        .x = row_x, .y = header_y, .width = ROW_WIDTH, .height = ROW_HEIGHT
    };
    DrawRectangleRec(header_rect, ui::color::RECT_COLD);

    // ---------------------------------------------------------------
    // rows
    // product panes
    const float row_y = header_y + ROW_HEIGHT + ROW_GAP;
    for (int i = 0; i < N_ROWS; ++i) {
        float offset_y = (ROW_HEIGHT + ROW_GAP) * i;

        Rectangle dst = {
            .x = row_x, .y = row_y + offset_y, .width = ROW_WIDTH, .height = ROW_HEIGHT
        };

        ui::radio_button_rect(dst, &SELECTED_PRODUCT_I, i);
    }

    // product panes content
    const float icon_x = mid_x - 0.5 * MID_COL_WIDTH + ROW_BORDER;
    const float icon_y = row_y + ROW_BORDER;
    for (int i = 0; i < N_ROWS; ++i) {
        float offset_y = (ROW_HEIGHT + ROW_GAP) * i;
        bool is_selected = SELECTED_PRODUCT_I == i;
        auto text_color = is_selected ? ui::color::TEXT_DARK : ui::color::TEXT_MILD;
        int *diff_n_units = &DIFF_CARGO.products[i].n_units;

        // ship's n_units
        int ship_n_units = ship.cargo.products[i].n_units + (*diff_n_units);
        auto ship_n_units_str = std::to_string(ship_n_units);
        int ship_n_units_x = row_x + ROW_BORDER + UI_ICON_SIZE_DST + 20.0;
        DrawText(
            ship_n_units_str.c_str(),
            ship_n_units_x,
            icon_y + offset_y,
            PRODUCT_N_UNITS_FONT_SIZE,
            text_color
        );

        // port's n_units
        int port_n_units = port.cargo.products[i].n_units - (*diff_n_units);
        auto port_n_units_str = std::to_string(port_n_units);
        int port_n_units_str_width = MeasureText(
            port_n_units_str.c_str(), PRODUCT_N_UNITS_FONT_SIZE
        );
        float port_n_units_x = row_x + ROW_WIDTH - ROW_BORDER - UI_ICON_SIZE_DST
                               - port_n_units_str_width - 20.0;
        DrawText(
            port_n_units_str.c_str(),
            port_n_units_x,
            icon_y + offset_y,
            PRODUCT_N_UNITS_FONT_SIZE,
            text_color
        );

        // product icon
        Rectangle dst = {
            .x = icon_x,
            .y = icon_y + offset_y,
            .width = PRODUCT_ICON_SIZE_DST,
            .height = PRODUCT_ICON_SIZE_DST
        };
        renderer::draw_product_icon(i, dst);

        // product name
        auto text = ship.cargo.products[i].name;
        float text_y = dst.y;
        float text_x = dst.x + dst.width + 10.0;
        DrawText(text.c_str(), text_x, text_y, PRODUCT_NAME_FONT_SIZE, text_color);

        // buy/sell n_units
        {
            int font_size = 0.8 * PRODUCT_NAME_FONT_SIZE;
            auto text = std::to_string(std::abs(*diff_n_units));
            float offset_y = PRODUCT_NAME_FONT_SIZE + ROW_BORDER;

            if (*diff_n_units > 0) {
                DrawText(
                    text.c_str(),
                    text_x,
                    text_y + offset_y,
                    font_size,
                    ui::color::TEXT_BUY
                );
            } else if (*diff_n_units < 0) {
                DrawText(
                    text.c_str(),
                    text_x,
                    text_y + offset_y,
                    font_size,
                    ui::color::TEXT_SELL
                );
            }
        }

        // arrows
        if (is_selected) {
            float mid_y = row_y + offset_y + 0.5 * ROW_HEIGHT;

            ui::increment_button_sprite(
                ui::SpriteName::LEFT_ARROW_ICON_SRC,
                {.x = row_x + ROW_BORDER,
                 .y = mid_y - 0.5f * UI_ICON_SIZE_DST,
                 .width = UI_ICON_SIZE_DST,
                 .height = UI_ICON_SIZE_DST},
                diff_n_units,
                +1,
                0,
                100
            );

            ui::increment_button_sprite(
                ui::SpriteName::RIGHT_ARROW_ICON_SRC,
                {.x = row_x + ROW_WIDTH - ROW_BORDER - UI_ICON_SIZE_DST,
                 .y = mid_y - 0.5f * UI_ICON_SIZE_DST,
                 .width = UI_ICON_SIZE_DST,
                 .height = UI_ICON_SIZE_DST},
                diff_n_units,
                -1,
                0,
                100
            );
        }
    }

    // column names
    float text_y = header_y + 0.5 * (ROW_HEIGHT - COLUMN_NAME_FONT_SIZE);

    {
        auto text = "Product";
        int text_width = MeasureText(text, COLUMN_NAME_FONT_SIZE);
        float text_x = mid_x - 0.5 * text_width;
        DrawText(text, text_x, text_y, COLUMN_NAME_FONT_SIZE, ui::color::LINE_LIGHT);
    }

    {
        auto text = "Ship";
        int text_width = MeasureText(text, COLUMN_NAME_FONT_SIZE);
        float col_right_x = mid_x - 0.5 * MID_COL_WIDTH;
        float col_mid_x = 0.5 * (row_x + col_right_x);
        float text_x = col_mid_x - 0.5 * text_width;
        DrawText(text, text_x, text_y, COLUMN_NAME_FONT_SIZE, ui::color::LINE_LIGHT);
    }

    {
        auto text = "Port";
        int text_width = MeasureText(text, COLUMN_NAME_FONT_SIZE);
        float col_left_x = mid_x + 0.5 * MID_COL_WIDTH;
        float col_mid_x = 0.5 * (row_x + ROW_WIDTH + col_left_x);
        float text_x = col_mid_x - 0.5 * text_width;
        DrawText(text, text_x, text_y, COLUMN_NAME_FONT_SIZE, ui::color::LINE_LIGHT);
    }

    // column vertical lines
    float line_top_y = pane_y + PANE_BORDER;
    float line_bot_y = pane_y + PANE_HEIGHT - PANE_BORDER;

    {
        float line_x = mid_x - 0.5 * MID_COL_WIDTH;
        DrawLine(line_x, line_top_y, line_x, line_bot_y, ui::color::LINE_MILD);
    }

    {
        float line_x = mid_x + 0.5 * MID_COL_WIDTH;
        DrawLine(line_x, line_top_y, line_x, line_bot_y, ui::color::LINE_MILD);
    }
}
#endif

}  // namespace shop
}  // namespace st
