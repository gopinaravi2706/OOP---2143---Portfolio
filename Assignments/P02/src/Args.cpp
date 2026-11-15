// Args.cpp — command-line parsing for imgtool.
//
//   ./imgtool <input_image> <output_image> [options]
//
// Args::parse is a static factory: it validates everything and either
// returns a fully valid Args or prints an error + usage and exits(1).

#include "Args.h"
#include "Console.h"

#include <cctype>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace {

const std::string kUsage =
    "Usage: ./imgtool <input_image> <output_image> [options]\n"
    "Options: -g/--grayscale  -l/--blur  -h/--flipH  -v/--flipV  "
    "-b/--brighten N  -r/--rotate N  --open";

const std::string kImageExtensions = ".png, .jpg, .jpeg, .bmp";

// What one option looks like to the parser.
struct OptionSpec {
    const char* longName;   // "--brighten"
    const char* shortName;  // "-b"   (nullptr if there is no short form)
    const char* opName;     // name stored in Operation ("brighten"); nullptr for --open
    bool takesValue;
};

const OptionSpec kOptions[] = {
    {"--grayscale", "-g",    "grayscale", false},
    {"--blur",      "-l",    "blur",      false},
    {"--flipH",     "-h",    "flipH",     false},
    {"--flipV",     "-v",    "flipV",     false},
    {"--brighten",  "-b",    "brighten",  true},
    {"--rotate",    "-r",    "rotate",    true},
    {"--open",      nullptr, nullptr,     false},
};

// Print a red error line plus the usage text to std::cerr, then exit(1).
[[noreturn]] void fail(const std::string& message) {
    printError(message);
    std::cerr << kUsage << "\n";
    std::exit(1);
}

// True if the filename ends in .png/.jpg/.jpeg/.bmp (any capitalization).
bool isImageFile(const std::string& filename) {
    const std::size_t dot = filename.find_last_of('.');
    if (dot == std::string::npos) return false;

    std::string ext = filename.substr(dot);
    for (char& c : ext)
        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));

    return ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".bmp";
}

// "--b=20" -> ("--b", "20").  No '=' -> (token, "").
std::pair<std::string, std::string> splitEq(const std::string& token) {
    const std::size_t eq = token.find('=');
    if (eq == std::string::npos) return {token, ""};
    return {token.substr(0, eq), token.substr(eq + 1)};
}

// "-g" and "--blur" are options; "-20" is a number (dash + digit), and
// anything not starting with '-' is a plain word.
bool looksLikeOption(const std::string& s) {
    return s.size() >= 2 && s[0] == '-' &&
           !std::isdigit(static_cast<unsigned char>(s[1]));
}

// Finds the spec for "-b" or "--brighten"; nullptr if unknown.
const OptionSpec* findOption(const std::string& flag) {
    for (const OptionSpec& spec : kOptions) {
        if (flag == spec.longName) return &spec;
        if (spec.shortName && flag == spec.shortName) return &spec;
    }
    return nullptr;
}

// The value for an option that needs one: either the text after '=', or
// the next token (which is consumed by advancing i).
std::string takeValue(const std::string& flag, bool hasInline,
                      const std::string& inlineValue,
                      const std::vector<std::string>& tokens, std::size_t& i) {
    if (hasInline) {
        if (inlineValue.empty())
            fail("'" + flag + "' requires an integer value");
        return inlineValue;
    }
    if (i + 1 >= tokens.size())
        fail("'" + flag + "' requires an integer value");

    const std::string& next = tokens[i + 1];
    if (looksLikeOption(next))
        fail("'" + flag + "' expected a value but got option '" + next + "'");

    ++i;
    return next;
}

// The whole string must be an integer: "twenty" and "20abc" both fail.
int toInt(const std::string& flag, const std::string& text) {
    std::size_t used = 0;
    int value = 0;
    try {
        value = std::stoi(text, &used);
    } catch (const std::invalid_argument&) {
        fail("invalid integer for '" + flag + "': '" + text + "'");
    } catch (const std::out_of_range&) {
        fail("integer out of range for '" + flag + "': '" + text + "'");
    }
    if (used != text.size())
        fail("invalid integer for '" + flag + "': '" + text + "'");
    return value;
}

}  // namespace

Args Args::parse(int argc, char* argv[]) {
    // Copy argv into a vector so indexing is always bounds-checkable.
    std::vector<std::string> tokens;
    for (int i = 1; i < argc; ++i) tokens.emplace_back(argv[i]);

    Args args;
    std::vector<std::string> positionals;

    for (std::size_t i = 0; i < tokens.size(); ++i) {
        const std::string& token = tokens[i];

        if (!looksLikeOption(token)) {
            positionals.push_back(token);
            continue;
        }

        const auto [flag, inlineValue] = splitEq(token);
        const bool hasInline = token.find('=') != std::string::npos;

        const OptionSpec* spec = findOption(flag);
        if (!spec) fail("unknown option '" + flag + "'");

        if (!spec->takesValue) {
            if (hasInline)
                fail("'" + std::string(spec->longName) + "' does not take a value");
            if (spec->opName)
                args.order.push_back({spec->opName, 0});
            else
                args.open = true;   // --open
            continue;
        }

        // brighten / rotate: need an integer, validated before it is stored.
        const std::string longName = spec->longName;
        const std::string text = takeValue(longName, hasInline, inlineValue, tokens, i);
        const int value = toInt(longName, text);

        if (longName == "--brighten") {
            if (value < -255 || value > 255)
                fail("'--brighten' must be in [-255, 255]");
        } else {  // --rotate
            if (value != 0 && value != 90 && value != 180 && value != 270)
                fail("'--rotate' must be one of {0, 90, 180, 270}");
        }
        args.order.push_back({spec->opName, value});
    }

    // Positional arguments: exactly two, both image files.
    if (positionals.empty())    fail("input and output files are required");
    if (positionals.size() == 1) fail("output file is required");
    if (positionals.size() > 2)
        fail("unexpected extra argument '" + positionals[2] + "'");

    if (!isImageFile(positionals[0]))
        fail("input must be an image file (" + kImageExtensions + "): '" + positionals[0] + "'");
    if (!isImageFile(positionals[1]))
        fail("output must be an image file (" + kImageExtensions + "): '" + positionals[1] + "'");

    args.input = positionals[0];
    args.output = positionals[1];
    return args;
}

void Args::print() const {
    std::cout << "INPUT   : " << input << "\n";
    std::cout << "OUTPUT  : " << output << "\n";
    std::cout << "ORDER   :";
    for (const Operation& op : order) std::cout << ' ' << op.label();
    std::cout << "\n";
}
