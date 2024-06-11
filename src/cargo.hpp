#pragma once

#include <array>
#include <climits>
#include <string>

namespace st {
namespace cargo {

enum class ProductIDX {
    PROVISION_IDX = 0,
    SPICES_IDX,
    SUGAR_IDX,
    TEA_IDX,
    TOBACCO_IDX,
    RUM_IDX,
    COTTON_IDX,
    SILK_IDX,
    GRAIN_IDX,
    WOOD_IDX,
    SILVER_IDX,
    GOLD_IDX,
    _N_PRODUCTS,
};

static const int N_PRODUCTS = (int)ProductIDX::_N_PRODUCTS;

struct Product {
    const std::string name;
    const int unit_weight;
    const int base_price;

    float buy_price_coeff = 1.0;
    float sell_price_coeff = 1.0;
    int n_units = 0;

    int get_buy_price();
    int get_sell_price();
    int get_weight();
};

class Cargo {
public:
    std::array<Product, N_PRODUCTS> products;
    int capacity = INT_MAX;

    Cargo();
    Cargo(int capacity);

    void empty();
    Product &get_product(ProductIDX idx);
    int get_weight();
    int get_free_weight();
};

Cargo create_ship_preset();
Cargo create_port_preset();

}  // namespace cargo
}  // namespace st
