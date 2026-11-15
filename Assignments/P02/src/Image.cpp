// Image.cpp — Image implementation (stb_image wrapper).
//
// NOTE: stb's STB_IMAGE_IMPLEMENTATION macros live in stb_impl.cpp ONLY.
// Never define them here (One Definition Rule).

#include "Image.h"

#include "stb_image.h"
#include "stb_image_write.h"

#include <algorithm>
#include <cctype>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace {

// Lowercase file extension including the dot (".PNG" -> ".png"), or "".
std::string lowerExtension(const std::string& filename) {
    const std::size_t dot = filename.find_last_of('.');
    if (dot == std::string::npos) return "";

    std::string ext = filename.substr(dot);
    for (char& c : ext)
        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    return ext;
}

}  // namespace

Image::Image(int width, int height) {
    if (width < 1 || height < 1) {
        throw std::invalid_argument(
            "Image dimensions must be at least 1 x 1 (got " +
            std::to_string(width) + " x " + std::to_string(height) + ")");
    }
    // height rows, each holding width default (black) pixels
    pixels_.assign(static_cast<std::size_t>(height),
                   std::vector<Pixel>(static_cast<std::size_t>(width)));
}

int Image::width() const {
    return pixels_.empty() ? 0 : static_cast<int>(pixels_[0].size());
}

int Image::height() const {
    return static_cast<int>(pixels_.size());
}

const Pixel& Image::at(int row, int col) const {
    if (row < 0 || row >= height() || col < 0 || col >= width()) {
        throw std::out_of_range(
            "pixel (" + std::to_string(row) + ", " + std::to_string(col) +
            ") is outside a " + std::to_string(width()) + " x " +
            std::to_string(height()) + " image (valid: row 0-" +
            std::to_string(height() - 1) + ", col 0-" +
            std::to_string(width() - 1) + ")");
    }
    return pixels_[static_cast<std::size_t>(row)][static_cast<std::size_t>(col)];
}

Pixel& Image::at(int row, int col) {
    // Reuse the checked const version instead of duplicating the logic.
    return const_cast<Pixel&>(std::as_const(*this).at(row, col));
}

Grid& Image::pixels() { return pixels_; }

const Grid& Image::pixels() const { return pixels_; }

std::ostream& operator<<(std::ostream& os, const Image& img) {
    return os << "Image(" << img.width() << " x " << img.height() << ")";
}

Image Image::load(const std::string& filename) {
    int w = 0, h = 0, channelsInFile = 0;

    // RAII: unique_ptr calls stbi_image_free on every path out of this
    // function, including the throws below.
    std::unique_ptr<unsigned char, decltype(&stbi_image_free)> data(
        stbi_load(filename.c_str(), &w, &h, &channelsInFile, 3),  // 3 = force RGB
        &stbi_image_free);

    if (!data) {
        const char* reason = stbi_failure_reason();
        throw std::runtime_error("cannot read image '" + filename + "'" +
                                 (reason ? std::string(": ") + reason : ""));
    }

    Image img(w, h);
    const unsigned char* bytes = data.get();
    for (int row = 0; row < h; ++row) {
        for (int col = 0; col < w; ++col) {
            const std::size_t i =
                (static_cast<std::size_t>(row) * static_cast<std::size_t>(w) +
                 static_cast<std::size_t>(col)) * 3;
            Pixel& p = img.at(row, col);
            p.r = bytes[i];
            p.g = bytes[i + 1];
            p.b = bytes[i + 2];
        }
    }
    return img;
}

void Image::save(const std::string& filename, int jpegQuality) const {
    const int w = width();
    const int h = height();

    // Flatten the grid to r,g,b,r,g,b,... Channels are int, so filters can
    // overshoot; this is where they get pulled back into [0, 255].
    std::vector<unsigned char> bytes;
    bytes.reserve(static_cast<std::size_t>(w) * static_cast<std::size_t>(h) * 3);
    for (const std::vector<Pixel>& row : pixels_) {
        for (const Pixel& p : row) {
            bytes.push_back(static_cast<unsigned char>(std::clamp(p.r, 0, 255)));
            bytes.push_back(static_cast<unsigned char>(std::clamp(p.g, 0, 255)));
            bytes.push_back(static_cast<unsigned char>(std::clamp(p.b, 0, 255)));
        }
    }

    // Pick the writer from the (lowercase) extension. Each returns 0 on failure.
    const std::string ext = lowerExtension(filename);
    int ok = 0;
    if (ext == ".jpg" || ext == ".jpeg")
        ok = stbi_write_jpg(filename.c_str(), w, h, 3, bytes.data(), jpegQuality);
    else if (ext == ".bmp")
        ok = stbi_write_bmp(filename.c_str(), w, h, 3, bytes.data());
    else
        ok = stbi_write_png(filename.c_str(), w, h, 3, bytes.data(), w * 3);

    if (ok == 0)
        throw std::runtime_error("cannot write image '" + filename + "'");
}

void Image::toGrayscale() {
    for (std::vector<Pixel>& row : pixels_) {
        for (Pixel& p : row) {
            const int gray = static_cast<int>(0.299 * p.r + 0.587 * p.g + 0.114 * p.b);
            p.r = p.g = p.b = gray;
        }
    }
}
