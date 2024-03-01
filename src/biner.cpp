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
#include <biner.hpp>

void biner::printHelp(const bool Error) {
    const std::string_view help{
        "usage: biner [-c] [-s] [-d directory] [-v] [-bm text] [-em text] [-o output] files\n"
    };
    if (Error) {
        std::cerr << help;
    } else {
        std::cout << help;
    }
}

template <typename T>
std::string biner::combineFiles(const std::vector<T>& files) {
    std::string combinedData{};

    for (const auto& it : files) {
        if (!std::filesystem::exists(it)) {
            throw std::runtime_error{"File passed to biner::combineFiles() does not exist."};
        }

        if (biner::verbose) {
            std::cerr << "Adding file '" << it << "' to buffer.\n";
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

        if (biner::verbose) {
            std::cerr << "Added file '" << it << "' to buffer.\n";
        }
    }

    if (biner::verbose) {
        std::cerr << "All done. No problems reported.\n";
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

            if (biner::verbose) {
                std::cerr << "Processing file '" << it << "'.\n";
            }
        } else if (biner::verbose) {
            std::cerr << "'" << it << "' is not a file that exists, so treating it as raw data.\n";
        }

        std::size_t beginning{fileContents.find(biner::binerBeginMarker)};
        if (beginning == std::string::npos || fileContents.find(biner::binerEndMarker) == std::string::npos) {
            std::cerr << "The file or data specified is not valid, because it's missing biner marker data. If needed, try overriding the biner markers.\n";
            std::exit(EXIT_FAILURE);
        }
        while (beginning != std::string::npos) {
            const std::size_t end{fileContents.find(biner::binerEndMarker, beginning)};

            if (biner::verbose) {
                std::cerr << "Parsing file.\n";
            }

            // each section
            if (end != std::string::npos) {
                const std::size_t fileNameBeginning{beginning + biner::binerBeginMarker.size() + 1};
                const std::size_t fileNameEnd{fileContents.find("\n", fileNameBeginning)};
                const std::string fileName{
                    fileContents.substr(fileNameBeginning, fileNameEnd - fileNameBeginning)
                };

                const std::filesystem::path fs{biner::directory + fileName};
                std::string path{ fs.filename().string() };

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

                    if (biner::verbose) {
                        std::cerr << "Duplicate file found, renaming it to '" << path << "'\n";
                    }
                }

                std::ofstream of{biner::directory + path};

                of << fileContents.substr(fileNameEnd + 1, end - fileNameEnd - 1); // 1 is the newline

                of.close();
            }

            fileContents.erase(beginning, end - beginning + biner::binerEndMarker.size() + 1);
            beginning = fileContents.find(biner::binerBeginMarker, beginning); // next file
        }

        if (biner::verbose) {
            std::cerr << "Parsed file.\n";
        }
    }

    if (biner::verbose) {
        std::cerr << "All done. No problems reported.\n";
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
        } else if (!arg.compare("-v") || !arg.compare("--verbose")) {
            biner::verbose = true;
        } else if (!arg.compare("-c") || !arg.compare("--combine")) {
            mode = biner::BINER_MODE_COMBINE;
        } else if (!arg.compare("-s") || !arg.compare("--separate")) {
            mode = biner::BINER_MODE_SEPARATE;
        } else if (!arg.compare("-d") || !arg.compare("--directory")) {
            if (arguments.size() <= it + 1) {
                std::cerr << "-d and --directory parameters require an extra parameter.\n";
                return EXIT_FAILURE;
            }
            biner::directory = arguments.at(++it);
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

    if (biner::verbose) {
        std::cerr << "Verbose mode enabled (-v)\n";
        std::cerr << "Arguments:\n";
        for (const auto& it : arguments) {
            std::cerr << it << "\n";
        }
    }

    std::cin.sync_with_stdio(false);
    if (std::cin.rdbuf()->in_avail()) {
        std::string data{};
        if (biner::verbose) {
            std::cerr << "Reading from standard input.\n";
        }

        while (std::getline(std::cin, data)) {
            files.push_back(data);

            if (biner::verbose) {
                std::cerr << "Added file '" << data << "' to list.\n";
            }
        }
    } else if (biner::verbose) {
        std::cerr << "Not reading from standard input.\n";
    }

    if (mode == biner::BINER_MODE_UNDEFINED) {
        std::cerr << "You must specify a mode.\n";
        return EXIT_FAILURE;
    }

    if (biner::verbose) {
        std::cerr << "Files:\n";
        for (const auto& it : files) {
            std::cerr << it << "\n";
        }
        std::cerr << (biner::BINER_MODE_COMBINE ? "Biner in combine mode.\n" : "Biner in separate mode.\n");
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
                if (biner::verbose) {
                    std::cerr << "Outputting data to standard output (stdout)\n";
                }

                std::cout << biner::combineFiles(files);
            } else {
                if (biner::verbose) {
                    std::cerr << "Writing data to file '" << outputFile << "'\n";
                }

                std::filesystem::path fs = outputFile;
                std::string dirname = fs.remove_filename().string();

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
