#include "Blackjack.hpp"
#include "ConsoleView.hpp"
#include "GameView.hpp"
#include "Tests.hpp"
#include "TuiView.hpp"

#include <memory>
#include <string>

int main(int argc, char** argv) {
    std::string mode = (argc > 1) ? argv[1] : "";
    if (mode == "--test") return runTests();

    Blackjack game;
    std::unique_ptr<GameView> view;
    if (mode == "--text") view = std::make_unique<ConsoleView>(game);
    else                  view = std::make_unique<TuiView>(game);
    view->run();
    return 0;
}
