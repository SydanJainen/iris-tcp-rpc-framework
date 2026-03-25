#ifndef IRIS_TFIDF_FUNC_H
#define IRIS_TFIDF_FUNC_H

#include <algorithm>
#include <any>
#include <cctype>
#include <cmath>
#include <format>
#include <ranges>
#include <stdexcept>
#include <string>
#include <vector>

#include "ports/i_api_function.h"
#include "ports/i_corpus.h"

namespace iris {

class TfidfFunc : public IApiFunction {
public:
    explicit TfidfFunc(ICorpus& corpus) : corpus_(corpus) {}

    std::string name() const override { return "tfidf"; }

    std::any execute(const std::vector<std::any>& args) override {
        if (args.size() != 1) {
            throw std::invalid_argument(
                "BAD_ARGUMENTS: tfidf expects 1 argument, got " +
                std::to_string(args.size()));
        }

        std::string term;
        try {
            term = std::any_cast<std::string>(args[0]);
        } catch (const std::bad_any_cast&) {
            throw std::invalid_argument(
                "BAD_ARGUMENTS: tfidf expects a string argument");
        }

        const auto& docs = corpus_.get_documents();
        size_t N = corpus_.document_count();

        if (N == 0) {
            return std::string("");
        }

        // Convert term to lowercase
        std::string lower_term;
        lower_term.reserve(term.size());
        for (char ch : term) {
            lower_term += static_cast<char>(
                std::tolower(static_cast<unsigned char>(ch)));
        }

        size_t df = 0;
        for (const auto& [doc_name, tokens] : docs) {
            if (std::ranges::any_of(tokens, [&](const auto& t) { return t == lower_term; }))
                ++df;
        }

        double idf = (df == 0) ? 0.0 : std::log(static_cast<double>(N) / static_cast<double>(df));

        struct DocScore {
            std::string name;
            double score;
        };

        std::vector<DocScore> scores;
        for (const auto& [doc_name, tokens] : docs) {
            if (tokens.empty()) {
                scores.push_back({doc_name, 0.0});
                continue;
            }

            auto count = std::ranges::count(tokens, lower_term);
            double tf = static_cast<double>(count) / static_cast<double>(tokens.size());
            scores.push_back({doc_name, tf * idf});
        }

        std::ranges::sort(scores, [](const DocScore& a, const DocScore& b) {
            return std::tie(b.score, a.name) < std::tie(a.score, b.name);
        });

        std::string result;
        for (const auto& [name, score] : scores) {
            if (!result.empty()) result += ',';
            result += std::format("{}:{:.3f}", name, score);
        }

        return result;
    }

private:
    ICorpus& corpus_;
};

}

#endif
