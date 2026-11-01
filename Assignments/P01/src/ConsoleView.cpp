#include "ConsoleView.hpp"
#include <cctype>
#include <iostream>
#include <sstream>

bool ConsoleView::readLine(std::string& line) const {
    return static_cast<bool>(std::getline(std::cin, line));
}

std::string ConsoleView::handText(const Hand& h, bool hideSecond) const {
    std::string s;
    for (int i = 0; i < h.size(); ++i) {
        if (i > 0) s += "  ";
        s += (hideSecond && i == 1) ? "??" : h.at(i).toString();
    }
    return s;
}

void ConsoleView::showTable() const {
    bool hidden = game_.dealerHoleHidden();
    std::cout << "\n  Dealer: " << handText(game_.dealerHand(), hidden);
    if (!hidden) std::cout << "  (" << game_.dealerHand().value() << ")";
    std::cout << "\n  You:    " << handText(game_.playerHand(), false)
              << "  (" << game_.playerHand().value()
              << (game_.playerHand().isSoft() ? " soft" : "") << ")\n";
}

void ConsoleView::bettingStep(bool& quit) {
    std::cout << "\nChips: " << game_.chips()
              << "   Bet (1-" << game_.chips() << ", q to quit): ";
    std::string line;
    if (!readLine(line)) { quit = true; return; }

    std::istringstream in(line);
    std::string token, extra;
    if (!(in >> token)) return;                       // blank line: re-prompt
    if (token == "q" || token == "Q") { quit = true; return; }

    int bet = 0;
    bool numeric = !token.empty();
    for (char c : token)
        if (!std::isdigit(static_cast<unsigned char>(c))) numeric = false;
    if (numeric && token.size() <= 9) bet = std::stoi(token);

    if (!numeric || (in >> extra) || !game_.startRound(bet))
        std::cout << "Please enter a whole number from 1 to " << game_.chips() << ".\n";
}

void ConsoleView::playerStep(bool& quit) {
    showTable();
    std::cout << "(h)it, (s)tand, or q to quit: ";
    std::string line;
    if (!readLine(line)) { quit = true; return; }
    if (line.empty()) return;
    char c = static_cast<char>(std::tolower(static_cast<unsigned char>(line[0])));
    if (line.size() == 1 && c == 'h') game_.hit();
    else if (line.size() == 1 && c == 's') game_.stand();
    else if (line.size() == 1 && c == 'q') quit = true;
    else std::cout << "Please type h, s, or q.\n";
}

void ConsoleView::roundOverStep() {
    showTable();
    std::cout << Blackjack::outcomeName(game_.outcome())
              << ".   Chips: " << game_.chips() << "\n";
    game_.nextRound();
}

void ConsoleView::run() {
    std::cout << "=== BLACKJACK ===  (dealer stands on all 17s, natural pays 3:2)\n";
    bool quit = false;
    while (!quit) {
        switch (game_.phase()) {
            case Phase::Betting:    bettingStep(quit); break;
            case Phase::PlayerTurn: playerStep(quit);  break;
            case Phase::RoundOver:  roundOverStep();   break;
            case Phase::GameOver:
                std::cout << "\nYou're out of chips. Game over!\n";
                return;
        }
    }
    std::cout << "\nThanks for playing. Final chips: " << game_.chips() << "\n";
}
