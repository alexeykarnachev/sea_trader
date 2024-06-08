#pragma once

#include <array>
#include <string>

namespace st {
namespace cargo {
struct Product {
    const std::string name;
    const int unit_weight;
    const int unit_price;

    int n_units = 0;
};

static const int N_PRODUCTS = 12;

class Cargo {
public:
    std::array<Product, N_PRODUCTS> products;

    Cargo();
    void reset();
};

}  // namespace cargo
}  // namespace st

