#include "FlipH.h"

#include <cstddef>
#include <utility>

void FlipH::apply(Grid& pixels) {
    for (auto& row : pixels) {
        const std::size_t width = row.size();
        for (std::size_t col = 0; col < width / 2; ++col)   // stop at the middle
            std::swap(row[col], row[width - 1 - col]);
    }
}

std::string FlipH::name() const {
    return "flipH";
}
