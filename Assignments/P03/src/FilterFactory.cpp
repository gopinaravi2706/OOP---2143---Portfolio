// FilterFactory.cpp — the name -> Creator registry.

#include "FilterFactory.h"

#include "Blur.h"
#include "Brighten.h"
#include "FlipH.h"
#include "FlipV.h"
#include "Grayscale.h"
#include "Invert.h"
#include "Rotate.h"

#include <stdexcept>

const std::map<std::string, FilterFactory::Creator>& FilterFactory::registry() {
    static const std::map<std::string, Creator> table = {
        {"blur",      [](const Operation&)    { return std::make_unique<Blur>(); }},
        {"brighten",  [](const Operation& op) { return std::make_unique<Brighten>(op.value); }},
        {"grayscale", [](const Operation&)    { return std::make_unique<Grayscale>(); }},
        {"flipH",     [](const Operation&)    { return std::make_unique<FlipH>(); }},
        {"flipV",     [](const Operation&)    { return std::make_unique<FlipV>(); }},
        {"rotate",    [](const Operation& op) { return std::make_unique<Rotate>(op.value); }},
        {"invert",    [](const Operation&)    { return std::make_unique<Invert>(); }},
    };
    return table;
}

std::unique_ptr<Filter> FilterFactory::create(const Operation& op) {
    const auto& table = registry();
    auto it = table.find(op.name);
    if (it == table.end())
        throw std::invalid_argument("no filter named '" + op.name + "'");
    return it->second(op);
}

std::vector<std::string> FilterFactory::names() {
    std::vector<std::string> result;
    for (const auto& entry : registry())
        result.push_back(entry.first);
    return result;
}
