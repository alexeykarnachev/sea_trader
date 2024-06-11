#pragma once

#include <array>
#include <string>

namespace st {
namespace cargo {
struct Product {
    const std::string name;
    const int unit_weight;
    const int base_price;

    float buy_price_coeff = 1.0;
    float sell_price_coeff = 1.0;
    int n_units = 0;

    void empty();
    int get_buy_price();
    int get_sell_price();
    int get_weight();
};

static const int N_PRODUCTS = 12;

class Cargo {
public:
    std::array<Product, N_PRODUCTS> products;
    int capacity = 0;

    Cargo();
    Cargo(int capacity);

    void empty();
    int get_weight();
};

}  // namespace cargo
}  // namespace st
