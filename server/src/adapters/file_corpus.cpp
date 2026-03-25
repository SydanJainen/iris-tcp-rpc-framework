#include "file_corpus.h"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <sstream>

namespace iris {

FileCorpus::FileCorpus(const std::string& directory_path) {
    if (!std::filesystem::exists(directory_path) ||
        !std::filesystem::is_directory(directory_path)) {
        return;
    }

    for (const auto& entry : std::filesystem::directory_iterator(directory_path)) {
        if (!entry.is_regular_file()) continue;
        if (entry.path().extension() != ".txt") continue;

        std::string filename = entry.path().filename().string();
        std::ifstream file(entry.path());
        if (!file.is_open()) continue;

        std::string content((std::istreambuf_iterator<char>(file)),
                             std::istreambuf_iterator<char>());

        documents_[filename] = tokenize(content);
    }
}

const std::map<std::string, std::vector<std::string>>& FileCorpus::get_documents() const {
    return documents_;
}

size_t FileCorpus::document_count() const {
    return documents_.size();
}

std::vector<std::string> FileCorpus::tokenize(const std::string& text) {
    std::vector<std::string> tokens;
    std::string current;

    for (char ch : text) {
        if (std::isalnum(static_cast<unsigned char>(ch))) {
            current += static_cast<char>(
                std::tolower(static_cast<unsigned char>(ch)));
        } else {
            if (!current.empty()) {
                tokens.push_back(current);
                current.clear();
            }
        }
    }
    if (!current.empty()) {
        tokens.push_back(current);
    }

    return tokens;
}

}
