#include "Blackjack.hpp"

Blackjack::Blackjack(int startingChips)
    : phase_(startingChips > 0 ? Phase::Betting : Phase::GameOver),
      chips_(startingChips) {}

void Blackjack::setNextDeck(Deck d) { stacked_ = std::move(d); }

bool Blackjack::startRound(int bet) {
    if (phase_ != Phase::Betting) return false;
    if (bet < 1 || bet > chips_) return false;

    chips_ -= bet;
    bet_ = bet;
    outcome_ = Outcome::None;
    player_.clear();
    dealer_.clear();
    if (stacked_) { deck_ = *stacked_; stacked_.reset(); }
    else          { deck_ = Deck(); deck_.shuffle(); deck_.shuffle(); }

    player_.addCard(deck_.deal());
    dealer_.addCard(deck_.deal());
    player_.addCard(deck_.deal());
    dealer_.addCard(deck_.deal());
    holeHidden_ = true;
    phase_ = Phase::PlayerTurn;
    checkNaturals();
    return true;
}

void Blackjack::checkNaturals() {
    bool pn = player_.isNatural();
    bool dn = dealer_.isNatural();
    if (!pn && !dn) return;
    if (pn && dn)       endRound(Outcome::Push, bet_);
    else if (pn)        endRound(Outcome::PlayerBlackjack, bet_ + bet_ * 3 / 2);
    else                endRound(Outcome::DealerWin, 0);
}

bool Blackjack::hit() {
    if (phase_ != Phase::PlayerTurn) return false;
    player_.addCard(deck_.deal());
    if (player_.value() > 21) endRound(Outcome::PlayerBust, 0);
    return true;
}

bool Blackjack::stand() {
    if (phase_ != Phase::PlayerTurn) return false;
    holeHidden_ = false;
    playDealer();
    settle();
    return true;
}

void Blackjack::playDealer() {
    // Hit on 16 or less, stand on 17 or more (including soft 17).
    while (dealer_.value() < 17) dealer_.addCard(deck_.deal());
}

void Blackjack::settle() {
    int pv = player_.value();
    int dv = dealer_.value();
    if (dv > 21)       endRound(Outcome::DealerBust, bet_ * 2);
    else if (pv > dv)  endRound(Outcome::PlayerWin, bet_ * 2);
    else if (pv < dv)  endRound(Outcome::DealerWin, 0);
    else               endRound(Outcome::Push, bet_);
}

void Blackjack::endRound(Outcome o, int payout) {
    outcome_ = o;
    chips_ += payout;
    holeHidden_ = false;
    phase_ = Phase::RoundOver;
}

bool Blackjack::nextRound() {
    if (phase_ != Phase::RoundOver) return false;
    player_.clear();
    dealer_.clear();
    bet_ = 0;
    outcome_ = Outcome::None;
    phase_ = (chips_ > 0) ? Phase::Betting : Phase::GameOver;
    return true;
}

Phase Blackjack::phase() const { return phase_; }
int Blackjack::chips() const { return chips_; }
int Blackjack::bet() const { return bet_; }
const Hand& Blackjack::playerHand() const { return player_; }
const Hand& Blackjack::dealerHand() const { return dealer_; }
bool Blackjack::dealerHoleHidden() const { return holeHidden_; }
Outcome Blackjack::outcome() const { return outcome_; }

int Blackjack::handValue(const Hand& h) { return h.value(); }
bool Blackjack::isSoft(const Hand& h) { return h.isSoft(); }
bool Blackjack::isNatural(const Hand& h) { return h.isNatural(); }

std::string Blackjack::outcomeName(Outcome o) {
    switch (o) {
        case Outcome::PlayerBlackjack: return "BLACKJACK! Pays 3:2";
        case Outcome::PlayerWin:       return "You win";
        case Outcome::DealerWin:       return "Dealer wins";
        case Outcome::Push:            return "Push";
        case Outcome::PlayerBust:      return "Bust! You lose";
        case Outcome::DealerBust:      return "Dealer busts, you win";
        case Outcome::None:            break;
    }
    return "";
}
