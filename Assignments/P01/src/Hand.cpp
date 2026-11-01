#include "Hand.hpp"
#include <stdexcept>

void Hand::addCard(const Card& c) { cards_.push_back(c); }
void Hand::clear() { cards_.clear(); }
int Hand::size() const { return static_cast<int>(cards_.size()); }

const Card& Hand::at(int i) const {
    if (i < 0 || i >= size()) throw std::out_of_range("Hand::at index out of range");
    return cards_[i];
}

int Hand::hardTotal() const {
    int total = 0;
    for (const Card& c : cards_) {
        int rank = c.getValue() % 13;      // Ace = 0
        if (rank == 0) total += 1;
        else if (rank >= 9) total += 10;   // 10, J, Q, K
        else total += rank + 1;
    }
    return total;
}

bool Hand::hasAce() const {
    for (const Card& c : cards_)
        if (c.getValue() % 13 == 0) return true;
    return false;
}

bool Hand::isSoft() const { return hasAce() && hardTotal() + 10 <= 21; }

int Hand::value() const {
    int total = hardTotal();
    return isSoft() ? total + 10 : total;
}

bool Hand::isNatural() const { return size() == 2 && value() == 21; }
