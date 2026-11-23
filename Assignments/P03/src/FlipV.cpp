#include "FlipV.h"

#include <cstddef>
#include <utility>

void FlipV::apply(Grid& pixels) {
    const std::size_t height = pixels.size();
    for (std::size_t row = 0; row < height / 2; ++row)      // stop at the middle
        std::swap(pixels[row], pixels[height - 1 - row]);
}

std::string FlipV::name() const {
    return "flipV";
}
