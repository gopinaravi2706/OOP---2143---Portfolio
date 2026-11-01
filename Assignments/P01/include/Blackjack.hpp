#pragma once
#include "Deck.hpp"
#include "Hand.hpp"
#include <optional>
#include <string>

// Rules + state only. No iostream, no FTXUI, no printing or reading.
enum class Phase   { Betting, PlayerTurn, RoundOver, GameOver };
enum class Outcome { None, PlayerBlackjack, PlayerWin, DealerWin, Push,
                     PlayerBust, DealerBust };

class Blackjack {
public:
    explicit Blackjack(int startingChips = 100);

    bool startRound(int bet);   // Betting -> PlayerTurn (or RoundOver on a natural)
    bool hit();                 // PlayerTurn only; may bust -> RoundOver
    bool stand();               // PlayerTurn only; dealer plays, settles -> RoundOver
    bool nextRound();           // RoundOver -> Betting (or GameOver)

    // Tests only: the next round uses this exact deck instead of a shuffled one.
    void setNextDeck(Deck d);

    Phase phase() const;
    int chips() const;
    int bet() const;
    const Hand& playerHand() const;
    const Hand& dealerHand() const;
    bool dealerHoleHidden() const;
    Outcome outcome() const;

    static int handValue(const Hand& h);
    static bool isSoft(const Hand& h);
    static bool isNatural(const Hand& h);
    static std::string outcomeName(Outcome o);

private:
    void checkNaturals();
    void playDealer();
    void settle();
    void endRound(Outcome o, int payout);

    Phase phase_ = Phase::Betting;
    Outcome outcome_ = Outcome::None;
    int chips_;
    int bet_ = 0;
    bool holeHidden_ = false;
    Deck deck_;
    std::optional<Deck> stacked_;
    Hand player_;
    Hand dealer_;
};
