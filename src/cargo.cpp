#include "./cargo.hpp"

namespace st {
namespace cargo {

static const std::array<Product, N_PRODUCTS> PRODUCTS = {
    {{.name = "Provision", .unit_weight = 10, .unit_price = 5},
     {.name = "Spices", .unit_weight = 2, .unit_price = 50},
     {.name = "Sugar", .unit_weight = 5, .unit_price = 20},
     {.name = "Tea", .unit_weight = 1, .unit_price = 30},
     {.name = "Tobacco", .unit_weight = 3, .unit_price = 40},
     {.name = "Rum", .unit_weight = 8, .unit_price = 25},
     {.name = "Cotton", .unit_weight = 4, .unit_price = 15},
     {.name = "Silk", .unit_weight = 1, .unit_price = 100},
     {.name = "Grain", .unit_weight = 7, .unit_price = 10},
     {.name = "Wood", .unit_weight = 20, .unit_price = 5},
     {.name = "Silver", .unit_weight = 50, .unit_price = 500},
     {.name = "Gold", .unit_weight = 50, .unit_price = 1000}}
};

Cargo::Cargo()
    : products(PRODUCTS) {}

void Cargo::reset() {
    for (auto &product : this->products) {
        product.n_units = 0;
    }
}

}  // namespace cargo
}  // namespace st
