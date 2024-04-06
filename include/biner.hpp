/* biner
 * Combine and separate text files
 *
 * Copyright(c) speedie 2024
 * Licensed under the GNU General Public License version 3.0
 */

#pragma once

#include <string>
#include <vector>

namespace biner {
    enum {
        BINER_MODE_COMBINE,
        BINER_MODE_SEPARATE,
        BINER_MODE_UNDEFINED,
    };

    struct Settings {
        bool verbose{false};
        std::string directory{"./"};
        std::string binerBeginMarker{"--!- BINER FILE BEGIN -!--"};
        std::string binerEndMarker{"--!- BINER FILE END -!--"};
    };

    void printHelp(const bool Error);
    template <typename T> std::string combineFiles(const struct Settings& settings, const std::vector<T>& files);
    template <typename T> void separateFiles(const struct Settings& settings, const std::vector<T>& files);
}
