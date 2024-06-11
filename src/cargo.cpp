#include "./cargo.hpp"

namespace st {
namespace cargo {

static const std::array<Product, N_PRODUCTS> PRODUCTS = {
    {{.name = "Provision", .unit_weight = 10, .base_price = 5},
     {.name = "Spices", .unit_weight = 2, .base_price = 50},
     {.name = "Sugar", .unit_weight = 5, .base_price = 20},
     {.name = "Tea", .unit_weight = 1, .base_price = 30},
     {.name = "Tobacco", .unit_weight = 3, .base_price = 40},
     {.name = "Rum", .unit_weight = 8, .base_price = 25},
     {.name = "Cotton", .unit_weight = 4, .base_price = 15},
     {.name = "Silk", .unit_weight = 1, .base_price = 100},
     {.name = "Grain", .unit_weight = 7, .base_price = 10},
     {.name = "Wood", .unit_weight = 20, .base_price = 5},
     {.name = "Silver", .unit_weight = 50, .base_price = 500},
     {.name = "Gold", .unit_weight = 50, .base_price = 1000}}
};

int Product::get_buy_price() {
    return this->buy_price_coeff * this->base_price;
}

int Product::get_sell_price() {
    return this->sell_price_coeff * this->base_price;
}

int Product::get_weight() {
    return this->unit_weight * this->n_units;
}

Cargo::Cargo()
    : products(PRODUCTS) {}

Cargo::Cargo(int capacity)
    : products(PRODUCTS)
    , capacity(capacity) {}

Product &Cargo::get_product(ProductIDX idx) {
    return this->products[(int)idx];
}

void Cargo::empty() {
    for (auto &product : this->products) {
        product.n_units = 0;
    }
}

int Cargo::get_weight() {
    int weight = 0;
    for (auto &product : this->products) {
        weight += product.get_weight();
    }

    return weight;
}

int Cargo::get_free_weight() {
    int weight = this->get_weight();
    int free_weight = this->capacity - weight;
    return free_weight;
}

Cargo create_ship_preset() {
    Cargo cargo(1000);
    cargo.get_product(ProductIDX::PROVISION_IDX).n_units = 30;
    cargo.get_product(ProductIDX::RUM_IDX).n_units = 10;
    cargo.get_product(ProductIDX::WOOD_IDX).n_units = 5;

    return cargo;
}

Cargo create_port_preset() {
    Cargo cargo(1000000);
    cargo.get_product(ProductIDX::PROVISION_IDX).n_units = 1000;
    cargo.get_product(ProductIDX::RUM_IDX).n_units = 1000;
    cargo.get_product(ProductIDX::WOOD_IDX).n_units = 500;
    cargo.get_product(ProductIDX::SILVER_IDX).n_units = 50;
    cargo.get_product(ProductIDX::GOLD_IDX).n_units = 25;

    return cargo;
}

}  // namespace cargo
}  // namespace st
