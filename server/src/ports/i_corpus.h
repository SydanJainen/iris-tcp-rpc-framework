#ifndef IRIS_I_CORPUS_H
#define IRIS_I_CORPUS_H

#include <map>
#include <string>
#include <vector>

namespace iris {

class ICorpus {
public:
    virtual ~ICorpus() = default;

    virtual const std::map<std::string, std::vector<std::string>>& get_documents() const = 0;
    virtual size_t document_count() const = 0;
};

}

#endif
