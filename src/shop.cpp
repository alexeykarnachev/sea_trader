#include "shop.hpp"

#include "cargo.hpp"
#include "components.hpp"
#include "entt/entity/fwd.hpp"
#include "raylib/raylib.h"
#include "registry.hpp"
#include "renderer.hpp"
#include "resources.hpp"
#include "ui.hpp"

namespace st {
namespace shop {

static cargo::Cargo DIFF_CARGO;
static const int N_ROWS = cargo::N_PRODUCTS;
static const int COLUMN_NAME_FONT_SIZE = 32;
static const int PRODUCT_NAME_FONT_SIZE = 27;
static const int PRODUCT_N_UNITS_FONT_SIZE = 27;
static const float PANE_WIDTH = 600.0;
static const float PANE_BORDER = 20.0;
static const float ROW_BORDER = 3.0;
static const float ROW_HEIGHT = 60.0;
static const float PRODUCT_ICON_SIZE_DST = ROW_HEIGHT - 2.0 * ROW_BORDER;
static const float UI_ICON_SIZE_DST = 0.7 * (ROW_HEIGHT - 2.0 * ROW_BORDER);
static const float ROW_WIDTH = PANE_WIDTH - 2.0 * PANE_BORDER;
static const float ROW_GAP = 5.0;
static const float PANE_HEIGHT = 2.0 * PANE_BORDER + (N_ROWS + 1) * ROW_HEIGHT
                                 + N_ROWS * ROW_GAP;
static const float MID_COL_WIDTH = 220.0;
static int SELECTED_PRODUCT_I = -1;

static bool IS_OPENED = false;
static entt::entity PLAYER_ENTITY;
static entt::entity PORT_ENTITY;

void open(entt::entity port_entity) {
    PORT_ENTITY = port_entity;
    PLAYER_ENTITY = registry::registry.view<components::Player>().front();
    DIFF_CARGO.reset();
    IS_OPENED = true;
}

void close() {
    IS_OPENED = false;
}

bool check_if_opened() {
    return IS_OPENED;
}

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

}  // namespace shop
}  // namespace st
