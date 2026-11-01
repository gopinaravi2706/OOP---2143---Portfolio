#pragma once
#include "Blackjack.hpp"
#include "GameView.hpp"
#include <string>

class ConsoleView : public GameView {
public:
    explicit ConsoleView(Blackjack& game) : game_(game) {}
    void run() override;

private:
    bool readLine(std::string& line) const;      // false on EOF
    void showTable() const;
    std::string handText(const Hand& h, bool hideSecond) const;
    void bettingStep(bool& quit);
    void playerStep(bool& quit);
    void roundOverStep();

    Blackjack& game_;
};
