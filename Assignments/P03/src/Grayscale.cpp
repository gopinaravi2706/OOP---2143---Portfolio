#include "Grayscale.h"

void Grayscale::apply(Grid& pixels) {
    for (auto& row : pixels) {
        for (auto& p : row) {
            int gray = static_cast<int>(0.299 * p.r + 0.587 * p.g + 0.114 * p.b);
            p.r = p.g = p.b = gray;
        }
    }
}

std::string Grayscale::name() const {
    return "grayscale";
}
