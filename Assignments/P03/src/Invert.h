#pragma once

#include <string>

#include "Filter.h"

// Photographic negative: every channel c becomes 255 - c.
class Invert : public Filter {
public:
    void apply(Grid& pixels) override;
    std::string name() const override;
};
