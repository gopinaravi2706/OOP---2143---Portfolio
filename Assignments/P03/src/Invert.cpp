#include "Invert.h"

void Invert::apply(Grid& pixels) {
    for (auto& row : pixels) {
        for (auto& p : row) {
            p.r = 255 - p.r;
            p.g = 255 - p.g;
            p.b = 255 - p.b;
        }
    }
}

std::string Invert::name() const {
    return "invert";
}
