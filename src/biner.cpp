/* biner
 * Combine and separate text files
 *
 * Copyright(c) speedie 2024
 * Licensed under the GNU General Public License version 3.0
 */

#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <exception>
#include <string>
#include <string_view>
#include <filesystem>
#include <vector>
#include <algorithm>
#include <unistd.h>
#include <biner.hpp>

void biner::printHelp(const bool Error) {
    if (Error) {
        std::cerr << "usage: biner [-c] [-s] [-bm text] [-em text] [-o output] files\n";
    } else {
        std::cout << "usage: biner [-c] [-s] [-bm text] [-em text] [-o output] files\n";
    }
}

template <typename T>
std::string biner::combineFiles(const std::vector<T>& files) {
    std::string combinedData{};

    for (const auto& it : files) {
        if (!std::filesystem::exists(it)) {
            throw std::runtime_error{"File passed to biner::combineFiles() does not exist."};
        }

        const std::string itstr{it};
        std::ifstream file{itstr};
        std::ostringstream ss{};

        if (!file.is_open()) {
            throw std::runtime_error{"File passed to biner::combineFiles() failed to open."};
        }

        combinedData += biner::binerBeginMarker + " " + itstr + "\n";

        ss << file.rdbuf();
        combinedData += ss.str();

        file.close();

        combinedData += biner::binerEndMarker + " " + itstr + "\n";
    }

    return combinedData;
}

template <typename T>
void biner::separateFiles(const std::vector<T>& files) {
    for (const auto& it : files) {
        std::string fileContents{it};

        if (std::filesystem::exists(it)) {
            const std::string itstr{it};
            std::ifstream file{itstr};
            std::ostringstream ss{};

            if (!file.is_open()) {
                throw std::runtime_error{"File passed to biner::separateFiles() failed to open."};
            }

            ss << file.rdbuf();
            fileContents = ss.str();

            file.close();
        }

        std::size_t beginning{fileContents.find(biner::binerBeginMarker)};
        while (beginning != std::string::npos) {
            std::size_t end{fileContents.find(biner::binerEndMarker, beginning)};

            // each section
            if (end != std::string::npos) {
                std::size_t fileNameBeginning{beginning + biner::binerBeginMarker.size() + 1};
                std::size_t fileNameEnd{fileContents.find("\n", fileNameBeginning)};
                std::string fileName{
                    fileContents.substr(fileNameBeginning, fileNameEnd - fileNameBeginning)
                };

                std::string fileData{
                    fileContents.substr(fileNameEnd + 1, end - fileNameEnd - 1) // 1 is the newline
                };

                std::filesystem::path fs{"./" + fileName};
                std::string path = fs.filename();

                if (std::filesystem::exists(path)) {
                    int i{1};

                    while (i < 100000) {
                        if (!std::filesystem::exists(path + "_" + std::to_string(i))) {
                            path += "_" + std::to_string(i);
                            break;
                        }
                        i++;
                    }

                    if (i == 100000) {
                        throw std::runtime_error{"Too many duplicate files. Because I don't want to kill your SSD, I've decided to stop here."};
                    }
                }

                std::ofstream of{"./" + path};

                of << fileData;

                of.close();
            }

            fileContents.erase(beginning, end - beginning + biner::binerEndMarker.size() + 1);
            beginning = fileContents.find(biner::binerBeginMarker, beginning); // next file
        }
    }
}

int main(int argc, char** argv) {
    std::string outputFile{};
    const std::vector<std::string_view> arguments(argv, argv + argc);
    std::vector<std::string_view> files{};
    int mode{biner::BINER_MODE_UNDEFINED};

    for (std::size_t it{1}; it < arguments.size(); ++it) {
        const std::string_view arg{arguments.at(it)};

        if (!arg.compare("-h") || !arg.compare("--help")) {
            biner::printHelp(false);
            return EXIT_SUCCESS;
        } else if (!arg.compare("-c") || !arg.compare("--combine")) {
            mode = biner::BINER_MODE_COMBINE;
        } else if (!arg.compare("-s") || !arg.compare("--separate")) {
            mode = biner::BINER_MODE_SEPARATE;
        } else if (!arg.compare("-bm") || !arg.compare("--begin-marker")) {
            if (arguments.size() <= it + 1) {
                std::cerr << "-bm and --begin-marker parameters require an extra parameter.\n";
                return EXIT_FAILURE;
            } else {
                biner::binerBeginMarker = arguments.at(++it);
            }
        } else if (!arg.compare("-em") || !arg.compare("--end-marker")) {
            if (arguments.size() <= it + 1) {
                std::cerr << "-em and --end-marker parameters require an extra parameter.\n";
                return EXIT_FAILURE;
            } else {
                biner::binerEndMarker = arguments.at(++it);
            }
        } else if (!arg.compare("-o") || !arg.compare("--output")) {
            if (arguments.size() <= it + 1) {
                std::cerr << "-o and --output parameters require an extra parameter.\n";
                return EXIT_FAILURE;
            } else {
                outputFile = arguments.at(++it);
            }
        } else {
            if (std::filesystem::exists(arg)) {
                files.push_back(arg);
            } else {
                std::cerr << "File '" << arg << "' does not exist, or is an invalid parameter.\n";
            }
        }
    }

    if (!isatty(STDIN_FILENO)) {
        std::string data{};
        while (std::getline(std::cin, data)) {
            files.push_back(data);
        }
    }

    if (mode == biner::BINER_MODE_UNDEFINED) {
        std::cerr << "You must specify a mode.\n";
        return EXIT_FAILURE;
    }

    if (files.empty() == true) {
        if (mode == biner::BINER_MODE_COMBINE) {
            std::cerr << "You must specify at least two files to combine.\n";
        } else {
            std::cerr << "You must specify at least one file to split.\n";
        }

        return EXIT_FAILURE;
    }

    try {
        if (mode == biner::BINER_MODE_SEPARATE) {
            biner::separateFiles(files);
        } else {
            if (!outputFile.compare("")) {
                std::cout << biner::combineFiles(files);
            } else {
                std::filesystem::path fs{};
                std::string dirname = (fs = outputFile).remove_filename();

                if (!std::filesystem::exists(dirname) && !dirname.empty()) {
                    if (!std::filesystem::create_directories(fs)) {
                        throw std::runtime_error{"Failed to create directory."};
                    }
                }

                std::ofstream of{outputFile};

                of << biner::combineFiles(files);

                of.close();
            }

            return EXIT_SUCCESS;
        }
    } catch (const std::exception& e) {
        std::cerr << "biner failed to perform the action you requested.\n" << e.what() << "\n";
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
