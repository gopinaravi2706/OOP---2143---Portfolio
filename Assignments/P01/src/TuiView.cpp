#include "TuiView.hpp"

#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>

#include <algorithm>
#include <string>

using namespace ftxui;

namespace {

Element faceUpCard(const Card& c) {
    int suit = c.getValue() / 13;                  // 1 = diamonds, 2 = hearts
    Color col = (suit == 1 || suit == 2) ? Color::Red : Color::White;
    return text(c.rankSymbol() + c.suitSymbol()) | bold | color(col) | center |
           size(WIDTH, EQUAL, 6) | size(HEIGHT, EQUAL, 3) | border;
}

Element faceDownCard() {
    return vbox({text("░░░░"), text("░░░░"), text("░░░░")}) | color(Color::Blue) |
           size(WIDTH, EQUAL, 6) | size(HEIGHT, EQUAL, 3) | border;
}

Element handRow(const Hand& h, bool hideSecond) {
    Elements cards;
    for (int i = 0; i < h.size(); ++i)
        cards.push_back((hideSecond && i == 1) ? faceDownCard() : faceUpCard(h.at(i)));
    if (cards.empty()) cards.push_back(text(" ") | size(HEIGHT, EQUAL, 5));
    return hbox(std::move(cards));
}

std::string keysFor(Phase p) {
    switch (p) {
        case Phase::Betting:    return "[←/→] change bet   [Enter] deal   [q] quit";
        case Phase::PlayerTurn: return "[h] hit   [s] stand   [q] quit";
        case Phase::RoundOver:  return "[Enter] next round   [q] quit";
        case Phase::GameOver:   return "[q] quit";
    }
    return "";
}

}  // namespace

void TuiView::run() {
    auto screen = ScreenInteractive::Fullscreen();
    int selectedBet = 5;          // UI-only: what the bet selector currently shows
    std::string message = "Place your bet.";

    auto renderer = Renderer([&] {
        bool hidden = game_.dealerHoleHidden();
        std::string dealerTotal =
            game_.dealerHand().size() == 0 ? "" :
            hidden ? "Total: ?" : "Total: " + std::to_string(game_.dealerHand().value());
        std::string playerTotal =
            game_.playerHand().size() == 0 ? "" :
            "Total: " + std::to_string(game_.playerHand().value()) +
            (game_.playerHand().isSoft() ? " (soft)" : "");

        int shownBet = (game_.phase() == Phase::Betting) ? selectedBet : game_.bet();

        return vbox({
            text(" BLACKJACK ") | bold | center,
            separator(),
            hbox({text(" Chips: " + std::to_string(game_.chips())) | bold,
                  filler(),
                  text("Bet: " + std::to_string(shownBet) + " ")}),
            separator(),
            text(" Dealer  " + dealerTotal),
            handRow(game_.dealerHand(), hidden),
            text(" You  " + playerTotal),
            handRow(game_.playerHand(), false),
            separator(),
            text(" " + message) | bold | color(Color::Yellow),
            text(" " + keysFor(game_.phase())) | dim,
        }) | border | size(WIDTH, GREATER_THAN, 50);
    });

    auto component = CatchEvent(renderer, [&](Event e) {
        if (e == Event::Character('q') || e == Event::Character('Q')) {
            screen.Exit();
            return true;
        }
        switch (game_.phase()) {
            case Phase::Betting:
                if (e == Event::ArrowRight) selectedBet = std::min(selectedBet + 1, std::max(1, game_.chips()));
                else if (e == Event::ArrowLeft) selectedBet = std::max(selectedBet - 1, 1);
                else if (e == Event::Return) {
                    if (game_.startRound(selectedBet)) {
                        message = game_.phase() == Phase::RoundOver
                            ? Blackjack::outcomeName(game_.outcome()) + "."
                            : "Hit or stand?";
                    } else {
                        message = "Not enough chips for that bet.";
                    }
                } else return false;
                return true;
            case Phase::PlayerTurn:
                if (e == Event::Character('h') || e == Event::Character('H')) game_.hit();
                else if (e == Event::Character('s') || e == Event::Character('S')) game_.stand();
                else return false;
                if (game_.phase() == Phase::RoundOver)
                    message = Blackjack::outcomeName(game_.outcome()) + ".";
                return true;
            case Phase::RoundOver:
                if (e != Event::Return) return false;
                game_.nextRound();
                selectedBet = std::min(selectedBet, std::max(1, game_.chips()));
                message = game_.phase() == Phase::GameOver ? "Out of chips. Game over!"
                                                           : "Place your bet.";
                return true;
            case Phase::GameOver:
                return false;
        }
        return false;
    });

    screen.Loop(component);
}
