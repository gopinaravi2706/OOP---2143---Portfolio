#include "Tests.hpp"
#include "Blackjack.hpp"
#include <initializer_list>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

namespace {

int failures = 0;

void check(const std::string& name, bool ok) {
    std::cout << (ok ? "PASS  " : "FAIL  ") << name << "\n";
    if (!ok) ++failures;
}

// rank: 0=A, 1..8 = 2..9, 9=10, 10=J, 11=Q, 12=K.  suit: 0 C, 1 D, 2 H, 3 S.
Card mk(int rank, int suit = 0) { return Card(suit * 13 + rank); }
const int A = 0, T = 9, J = 10, Q = 11, K = 12;

Hand handOf(std::initializer_list<int> ranks) {
    Hand h;
    for (int r : ranks) h.addCard(mk(r));
    return h;
}

// Deck order is: player, dealer, player, dealer, then draws.
Deck stacked(std::vector<int> ranks) {
    std::vector<Card> cs;
    int suit = 0;
    for (int r : ranks) cs.push_back(mk(r, suit++ % 4));
    return Deck(cs);
}

void testHandValues() {
    Hand h1 = handOf({A, K});
    check("A + K = 21", Blackjack::handValue(h1) == 21);
    check("A + K is a natural", Blackjack::isNatural(h1));
    Hand h2 = handOf({A, 5});
    check("A + 6 = 17 and soft", Blackjack::handValue(h2) == 17 && Blackjack::isSoft(h2));
    Hand h3 = handOf({A, 5, T});
    check("A + 6 + 10 = 17 and hard", Blackjack::handValue(h3) == 17 && !Blackjack::isSoft(h3));
    Hand h4 = handOf({A, A, 8});
    check("A + A + 9 = 21", Blackjack::handValue(h4) == 21);
    Hand h5 = handOf({K, Q, 4});
    check("K + Q + 5 busts", Blackjack::handValue(h5) > 21);
    Hand h6 = handOf({T, 5, 5});
    check("10 + 6 + 6 is not a natural", !Blackjack::isNatural(h6));
    check("21 with three cards is not a natural", !Blackjack::isNatural(handOf({7, 6, 6})));
}

void testDealer() {
    {   // P: 10, 8 (18)   D: A, 6 (soft 17)   spare: 5 must NOT be drawn
        Blackjack g(100);
        g.setNextDeck(stacked({T, A, 7, 5, 4}));
        g.startRound(10);
        g.stand();
        check("dealer stands on soft 17", g.dealerHand().size() == 2);
        check("player 18 beats dealer 17", g.outcome() == Outcome::PlayerWin);
    }
    {   // P: 10, 8 (18)   D: 10, 6 (16) hits 5 -> 21
        Blackjack g(100);
        g.setNextDeck(stacked({T, T, 7, 5, 4}));
        g.startRound(10);
        g.stand();
        check("dealer hits on 16", g.dealerHand().size() == 3);
        check("dealer reaching 21 beats player 18", g.outcome() == Outcome::DealerWin);
    }
    {   // dealer 10,6 hits K -> bust
        Blackjack g(100);
        g.setNextDeck(stacked({T, T, 7, 5, K}));
        g.startRound(10);
        g.stand();
        check("dealer bust pays 1:1", g.outcome() == Outcome::DealerBust && g.chips() == 110);
    }
}

void testNaturals() {
    {
        Blackjack g(100);
        g.setNextDeck(stacked({A, 8, K, 6, 2}));   // P: A,K  D: 9,7
        g.startRound(10);
        check("natural ends the round immediately", g.phase() == Phase::RoundOver);
        check("natural pays 3:2 (bet 10 -> +15)",
              g.outcome() == Outcome::PlayerBlackjack && g.chips() == 115);
    }
    {
        Blackjack g(100);
        g.setNextDeck(stacked({A, A, K, Q, 2}));   // both naturals
        g.startRound(10);
        check("natural vs natural pushes", g.outcome() == Outcome::Push && g.chips() == 100);
    }
    {
        Blackjack g(100);
        g.setNextDeck(stacked({8, A, 7, K, 2}));   // dealer natural only
        g.startRound(10);
        check("dealer natural only: player loses",
              g.outcome() == Outcome::DealerWin && g.chips() == 90);
    }
    {
        Blackjack g(100);
        g.setNextDeck(stacked({A, 8, K, 6}));
        g.startRound(5);
        check("odd bet natural uses integer division (bet 5 -> +7)", g.chips() == 107);
    }
}

void testBustAndPush() {
    Blackjack g(100);
    g.setNextDeck(stacked({T, 6, 5, 7, K}));      // P: 10,6 hits K -> 26
    g.startRound(20);
    g.hit();
    check("player bust loses bet and ends round",
          g.outcome() == Outcome::PlayerBust && g.chips() == 80 && g.phase() == Phase::RoundOver);
    check("dealer hole revealed after round", !g.dealerHoleHidden());

    Blackjack p(100);
    p.setNextDeck(stacked({T, T, 7, 7, 4}));      // 17 vs 17
    p.startRound(10);
    p.stand();
    check("equal totals push", p.outcome() == Outcome::Push && p.chips() == 100);
}

void testPhases() {
    Blackjack g(100);
    bool r = g.hit();
    check("hit() during Betting returns false", !r);
    check("hit() during Betting changes nothing",
          g.phase() == Phase::Betting && g.chips() == 100 && g.playerHand().size() == 0);
    check("stand() during Betting returns false", !g.stand());
    check("nextRound() during Betting returns false", !g.nextRound());
    check("bet of 0 rejected", !g.startRound(0));
    check("bet above chips rejected", !g.startRound(101));
    check("negative bet rejected", !g.startRound(-5));
    check("rejected bets change nothing", g.chips() == 100 && g.phase() == Phase::Betting);

    g.setNextDeck(stacked({T, T, 7, 7, 4}));
    g.startRound(10);
    check("bet is subtracted right away", g.chips() == 90);
    check("startRound() during PlayerTurn returns false", !g.startRound(5));
    check("hole card hidden during player turn", g.dealerHoleHidden());
    g.stand();
    check("hit() after round over returns false", !g.hit());
    check("stand() after round over returns false", !g.stand());
}

void testGameOver() {
    Blackjack g(10);
    g.setNextDeck(stacked({T, T, 5, 8, K}));      // P: 10,6 -> bust on K
    g.startRound(10);
    g.hit();
    g.nextRound();
    check("0 chips -> GameOver", g.phase() == Phase::GameOver);
    check("startRound() in GameOver returns false", !g.startRound(1));
}

}  // namespace

int runTests() {
    failures = 0;
    testHandValues();
    testDealer();
    testNaturals();
    testBustAndPush();
    testPhases();
    testGameOver();
    std::cout << (failures == 0 ? "\nAll tests passed.\n"
                                : "\nSome tests FAILED.\n");
    return failures == 0 ? 0 : 1;
}
