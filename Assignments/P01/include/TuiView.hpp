#pragma once
#include "Blackjack.hpp"
#include "GameView.hpp"

// FTXUI front end. FTXUI headers are only included in TuiView.cpp.
class TuiView : public GameView {
public:
    explicit TuiView(Blackjack& game) : game_(game) {}
    void run() override;

private:
    Blackjack& game_;
};
