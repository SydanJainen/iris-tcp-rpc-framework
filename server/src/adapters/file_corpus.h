#ifndef IRIS_FILE_CORPUS_H
#define IRIS_FILE_CORPUS_H

#include <map>
#include <string>
#include <vector>

#include "ports/i_corpus.h"

namespace iris {

class FileCorpus : public ICorpus {
public:
    explicit FileCorpus(const std::string& directory_path);

    const std::map<std::string, std::vector<std::string>>& get_documents() const override;
    size_t document_count() const override;

private:
    std::map<std::string, std::vector<std::string>> documents_;

    static std::vector<std::string> tokenize(const std::string& text);
};

}

#endif
