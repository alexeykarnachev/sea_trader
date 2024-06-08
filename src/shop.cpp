#include "shop.hpp"

#include "cargo.hpp"
#include "renderer.hpp"
#include "resources.hpp"

namespace rl {
#include "raylib/raylib.h"
}

namespace st {
namespace shop {

static cargo::Cargo diff_cargo;
static const int n_rows = cargo::N_PRODUCTS;
static const int column_name_font_size = 32;
static const int product_name_font_size = 27;
static const int product_n_units_font_size = 27;
static const float pane_width = 600.0;
static const float pane_border = 20.0;
static const float row_border = 3.0;
static const float row_height = 60.0;
static const float product_icon_size_dst = row_height - 2.0 * row_border;
static const float ui_icon_size_dst = 0.7 * (row_height - 2.0 * row_border);
static const float row_width = pane_width - 2.0 * pane_border;
static const float row_gap = 5.0;
static const float pane_height = 2.0 * pane_border + (n_rows + 1) * row_height
                                 + n_rows * row_gap;
static const float mid_col_width = 220.0;
static int selected_product_i = -1;

static bool is_opened = false;

void open() {
    is_opened = true;
}

void close() {
    diff_cargo.reset();
    is_opened = false;
}

bool get_is_opened() {
    return is_opened;
}

void update_and_draw() {
    if (!is_opened) return;

    if (rl::IsKeyPressed(rl::KEY_ESCAPE)) {
        close();
        return;
    }

    renderer::set_screen_camera(resources::sprite_shader);
}

}  // namespace shop
}  // namespace st

#if 0
void update_and_draw_products_shop() {

    int screen_width = rl::GetScreenWidth();
    int screen_height = rl::GetScreenHeight();
    rl::Shader shader = resources::sprite_shader;
    renderer::set_screen_camera(shader);

    auto entity = registry.view<Player>().front();
    auto &ship = registry.get<Ship>(entity);
    auto &port = registry.get<Port>(this->player_moored_port);

    // ---------------------------------------------------------------
    // pane
    float pane_x = 0.5 * (screen_width - pane_width);
    float pane_y = 0.5 * (screen_height - pane_height);
    rl::Rectangle pane_rect = {
        .x = pane_x, .y = pane_y, .width = pane_width, .height = pane_height
    };
    rl::DrawRectangleRec(pane_rect, ui::color::RECT_COLD);

    const float row_x = pane_x + pane_border;
    const float mid_x = pane_x + 0.5 * pane_width;

    // ---------------------------------------------------------------
    // header
    const float header_y = pane_y + pane_border;
    rl::Rectangle header_rect = {
        .x = row_x, .y = header_y, .width = row_width, .height = row_height
    };
    rl::DrawRectangleRec(header_rect, ui::color::RECT_COLD);

    // ---------------------------------------------------------------
    // rows

    // product panes
    const float row_y = header_y + row_height + row_gap;
    for (int i = 0; i < n_rows; ++i) {
        float offset_y = (row_height + row_gap) * i;

        rl::Rectangle dst = {
            .x = row_x, .y = row_y + offset_y, .width = row_width, .height = row_height
        };

        ui::radio_button_rect(dst, &selected_product_i, i);
    }

    // product panes content
    const float icon_x = mid_x - 0.5 * mid_col_width + row_border;
    const float icon_y = row_y + row_border;
    for (int i = 0; i < n_rows; ++i) {
        float offset_y = (row_height + row_gap) * i;
        bool is_selected = selected_product_i == i;
        auto text_color = is_selected ? ui::color::TEXT_DARK : ui::color::TEXT_MILD;
        int *diff_n_units = &diff_cargo.products[i].n_units;

        // ship's n_units
        int ship_n_units = ship.cargo.products[i].n_units + (*diff_n_units);
        auto ship_n_units_str = std::to_string(ship_n_units);
        int ship_n_units_x = row_x + row_border + ui_icon_size_dst + 20.0;
        rl::DrawText(
            ship_n_units_str.c_str(),
            ship_n_units_x,
            icon_y + offset_y,
            product_n_units_font_size,
            text_color
        );

        // port's n_units
        int port_n_units = port.cargo.products[i].n_units - (*diff_n_units);
        auto port_n_units_str = std::to_string(port_n_units);
        int port_n_units_str_width = rl::MeasureText(
            port_n_units_str.c_str(), product_n_units_font_size
        );
        float port_n_units_x = row_x + row_width - row_border - ui_icon_size_dst
                               - port_n_units_str_width - 20.0;
        rl::DrawText(
            port_n_units_str.c_str(),
            port_n_units_x,
            icon_y + offset_y,
            product_n_units_font_size,
            text_color
        );

        // product icon
        rl::Rectangle dst = {
            .x = icon_x,
            .y = icon_y + offset_y,
            .width = product_icon_size_dst,
            .height = product_icon_size_dst
        };
        renderer::draw_product_icon(i, dst);

        // product name
        auto text = ship.cargo.products[i].name;
        float text_y = dst.y;
        float text_x = dst.x + dst.width + 10.0;
        rl::DrawText(text.c_str(), text_x, text_y, product_name_font_size, text_color);

        // buy/sell n_units
        {
            int font_size = 0.8 * product_name_font_size;
            auto text = std::to_string(std::abs(*diff_n_units));
            float offset_y = product_name_font_size + row_border;

            if (*diff_n_units > 0) {
                rl::DrawText(
                    text.c_str(),
                    text_x,
                    text_y + offset_y,
                    font_size,
                    ui::color::TEXT_BUY
                );
            } else if (*diff_n_units < 0) {
                rl::DrawText(
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
            float mid_y = row_y + offset_y + 0.5 * row_height;

            ui::increment_button_sprite(
                ui::SpriteName::LEFT_ARROW_ICON_SRC,
                {.x = row_x + row_border,
                 .y = mid_y - 0.5f * ui_icon_size_dst,
                 .width = ui_icon_size_dst,
                 .height = ui_icon_size_dst},
                diff_n_units,
                +1,
                0,
                100
            );

            ui::increment_button_sprite(
                ui::SpriteName::RIGHT_ARROW_ICON_SRC,
                {.x = row_x + row_width - row_border - ui_icon_size_dst,
                 .y = mid_y - 0.5f * ui_icon_size_dst,
                 .width = ui_icon_size_dst,
                 .height = ui_icon_size_dst},
                diff_n_units,
                -1,
                0,
                100
            );
        }
    }

    // column names
    float text_y = header_y + 0.5 * (row_height - column_name_font_size);

    {
        auto text = "Product";
        int text_width = rl::MeasureText(text, column_name_font_size);
        float text_x = mid_x - 0.5 * text_width;
        rl::DrawText(text, text_x, text_y, column_name_font_size, ui::color::LINE_LIGHT);
    }

    {
        auto text = "Ship";
        int text_width = rl::MeasureText(text, column_name_font_size);
        float col_right_x = mid_x - 0.5 * mid_col_width;
        float col_mid_x = 0.5 * (row_x + col_right_x);
        float text_x = col_mid_x - 0.5 * text_width;
        rl::DrawText(text, text_x, text_y, column_name_font_size, ui::color::LINE_LIGHT);
    }

    {
        auto text = "Port";
        int text_width = rl::MeasureText(text, column_name_font_size);
        float col_left_x = mid_x + 0.5 * mid_col_width;
        float col_mid_x = 0.5 * (row_x + row_width + col_left_x);
        float text_x = col_mid_x - 0.5 * text_width;
        rl::DrawText(text, text_x, text_y, column_name_font_size, ui::color::LINE_LIGHT);
    }

    // column vertical lines
    float line_top_y = pane_y + pane_border;
    float line_bot_y = pane_y + pane_height - pane_border;

    {
        float line_x = mid_x - 0.5 * mid_col_width;
        rl::DrawLine(line_x, line_top_y, line_x, line_bot_y, ui::color::LINE_MILD);
    }

    {
        float line_x = mid_x + 0.5 * mid_col_width;
        rl::DrawLine(line_x, line_top_y, line_x, line_bot_y, ui::color::LINE_MILD);
    }
}
#endif
