#pragma once
#include "Card.hpp"
#include <vector>

// A hand HAS cards (composition); it is not a vector or a deck.
class Hand {
public:
    void addCard(const Card& c);
    void clear();
    int size() const;
    const Card& at(int i) const;   // throws std::out_of_range on a bad index

    // Blackjack scoring lives here because Hand owns the cards it reads.
    int value() const;             // best total <= 21 if possible
    bool isSoft() const;           // an Ace is still counted as 11
    bool isNatural() const;        // exactly two cards totaling 21

private:
    int hardTotal() const;         // every Ace counted as 1
    bool hasAce() const;
    std::vector<Card> cards_;
};
