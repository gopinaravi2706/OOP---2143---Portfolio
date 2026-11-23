#include "Rotate.h"

#include <cstddef>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

Rotate::Rotate(int degrees) : degrees_(degrees) {
    if (degrees != 0 && degrees != 90 && degrees != 180 && degrees != 270)
        throw std::invalid_argument("rotate: degrees must be 0, 90, 180, or 270");
}

void Rotate::apply(Grid& pixels) {
    if (degrees_ == 0 || pixels.empty()) return;

    const std::size_t H = pixels.size();
    const std::size_t W = pixels[0].size();

    if (degrees_ == 180) {
        Grid result(H, std::vector<Pixel>(W));
        for (std::size_t row = 0; row < H; ++row)
            for (std::size_t col = 0; col < W; ++col)
                result[H - 1 - row][W - 1 - col] = pixels[row][col];
        pixels = std::move(result);
        return;
    }

    // 90 and 270: the result is W rows x H cols
    Grid result(W, std::vector<Pixel>(H));
    for (std::size_t row = 0; row < H; ++row) {
        for (std::size_t col = 0; col < W; ++col) {
            if (degrees_ == 90)
                result[col][H - 1 - row] = pixels[row][col];
            else  // 270
                result[W - 1 - col][row] = pixels[row][col];
        }
    }
    pixels = std::move(result);
}

std::string Rotate::name() const {
    return "rotate(" + std::to_string(degrees_) + ")";
}
