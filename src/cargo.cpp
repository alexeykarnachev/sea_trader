#include "./cargo.hpp"

#include <stdexcept>

namespace st {
namespace cargo {

static const std::array<Product, N_PRODUCTS> PRODUCTS = {
    {{.id = ProductID::PROVISION_ID, .unit_weight = 10, .base_price = 5},
     {.id = ProductID::SPICES_ID, .unit_weight = 2, .base_price = 50},
     {.id = ProductID::SUGAR_ID, .unit_weight = 5, .base_price = 20},
     {.id = ProductID::TEA_ID, .unit_weight = 1, .base_price = 30},
     {.id = ProductID::TOBACCO_ID, .unit_weight = 3, .base_price = 40},
     {.id = ProductID::RUM_ID, .unit_weight = 8, .base_price = 25},
     {.id = ProductID::COTTON_ID, .unit_weight = 4, .base_price = 15},
     {.id = ProductID::SILK_ID, .unit_weight = 1, .base_price = 100},
     {.id = ProductID::GRAIN_ID, .unit_weight = 7, .base_price = 10},
     {.id = ProductID::WOOD_ID, .unit_weight = 20, .base_price = 5},
     {.id = ProductID::SILVER_ID, .unit_weight = 50, .base_price = 500},
     {.id = ProductID::GOLD_ID, .unit_weight = 50, .base_price = 1000}}
};

std::string Product::get_name() {
    switch (this->id) {
        case ProductID::PROVISION_ID: return "Provision";
        case ProductID::SPICES_ID: return "Spices";
        case ProductID::SUGAR_ID: return "Sugar";
        case ProductID::TEA_ID: return "Tea";
        case ProductID::TOBACCO_ID: return "Tobacco";
        case ProductID::RUM_ID: return "Rum";
        case ProductID::COTTON_ID: return "Cotton";
        case ProductID::SILK_ID: return "Silk";
        case ProductID::GRAIN_ID: return "Grain";
        case ProductID::WOOD_ID: return "Wood";
        case ProductID::SILVER_ID: return "Silver";
        case ProductID::GOLD_ID: return "Gold";
        default: throw std::runtime_error("Unhandled product id");
    }
}

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

Cargo::Cargo(const Cargo &other)
    : products(other.products)
    , capacity(other.capacity) {}

Product &Cargo::get_product(ProductID idx) {
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

}  // namespace cargo
}  // namespace st
