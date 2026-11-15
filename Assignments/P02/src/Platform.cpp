// Platform.cpp — OS-specific code lives here and nowhere else.

#include "Platform.h"

#include <cstdlib>
#include <string>

bool openInDefaultViewer(const std::string& path) {
    // The preprocessor keeps only the branch for the OS being compiled.
#if defined(_WIN32)
    std::string cmd = "start \"\" \"" + path + "\"";
#elif defined(__APPLE__)
    std::string cmd = "open \"" + path + "\"";
#else
    std::string cmd = "xdg-open \"" + path + "\" >/dev/null 2>&1";
#endif
    return std::system(cmd.c_str()) == 0;
}
