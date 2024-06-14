#pragma once

#include <array>
#include <climits>

namespace st {
namespace cargo {

enum class ProductID {
    PROVISION_ID = 0,
    SPICES_ID,
    SUGAR_ID,
    TEA_ID,
    TOBACCO_ID,
    RUM_ID,
    COTTON_ID,
    SILK_ID,
    GRAIN_ID,
    WOOD_ID,
    SILVER_ID,
    GOLD_ID,
    _N_PRODUCTS,
};

static const int N_PRODUCTS = (int)ProductID::_N_PRODUCTS;

class Product {
public:
    ProductID id;
    int unit_weight;
    int base_price;

    float buy_price_coeff = 1.0;
    float sell_price_coeff = 1.0;
    int n_units = 0;

    std::string get_name();
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
    Cargo(const Cargo &other);

    void empty();
    Product &get_product(ProductID idx);
    int get_weight();
    int get_free_weight();
};

Cargo create_ship_preset();
Cargo create_port_preset();

}  // namespace cargo
}  // namespace st
