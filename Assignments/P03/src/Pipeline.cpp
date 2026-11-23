#include "Pipeline.h"

#include <iostream>
#include <utility>

void Pipeline::add(std::unique_ptr<Filter> f) {
    if (!f) return;                       // ignore nullptr
    filters_.push_back(std::move(f));     // unique_ptr can only be moved
}

void Pipeline::run(Grid& pixels) const {
    for (const auto& f : filters_)        // & so we don't try to copy a unique_ptr
        f->apply(pixels);
}

void Pipeline::printSteps() const {
    std::cout << "FILTERS : ";
    if (filters_.empty()) {
        std::cout << "(none)\n";
        return;
    }
    bool first = true;
    for (const auto& f : filters_) {
        if (!first) std::cout << " -> ";
        std::cout << f->name();
        first = false;
    }
    std::cout << "\n";
}

int Pipeline::size() const {
    return static_cast<int>(filters_.size());
}
