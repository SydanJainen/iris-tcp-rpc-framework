#include <gtest/gtest.h>

#include <any>
#include <cmath>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>

#include "ports/i_corpus.h"
#include "domain/tfidf_func.h"

namespace {

class MockCorpus : public iris::ICorpus {
public:
    MockCorpus() = default;

    explicit MockCorpus(std::map<std::string, std::vector<std::string>> docs)
        : docs_(std::move(docs)) {}

    const std::map<std::string, std::vector<std::string>>& get_documents() const override {
        return docs_;
    }

    size_t document_count() const override {
        return docs_.size();
    }

private:
    std::map<std::string, std::vector<std::string>> docs_;
};

} // namespace

TEST(TfidfFuncTest, Name) {
    MockCorpus corpus;
    iris::TfidfFunc func(corpus);
    EXPECT_EQ(func.name(), "tfidf");
}

TEST(TfidfFuncTest, EmptyCorpus) {
    MockCorpus corpus;
    iris::TfidfFunc func(corpus);
    auto result = func.execute({std::any(std::string("hello"))});
    EXPECT_EQ(std::any_cast<std::string>(result), "");
}

TEST(TfidfFuncTest, TermInOneDoc) {
    MockCorpus corpus({
        {"doc1.txt", {"apple", "banana", "cherry"}},
        {"doc2.txt", {"dog", "cat", "bird"}},
        {"doc3.txt", {"red", "green", "blue"}}
    });
    iris::TfidfFunc func(corpus);
    auto result = func.execute({std::any(std::string("apple"))});
    std::string output = std::any_cast<std::string>(result);

    EXPECT_NE(output.find("doc1.txt:0.366"), std::string::npos);
    EXPECT_NE(output.find("doc2.txt:0.000"), std::string::npos);
    EXPECT_NE(output.find("doc3.txt:0.000"), std::string::npos);

    // doc1 should come first (highest score)
    EXPECT_EQ(output.substr(0, 8), "doc1.txt");
}

TEST(TfidfFuncTest, TermInAllDocs) {
    MockCorpus corpus({
        {"a.txt", {"the", "cat"}},
        {"b.txt", {"the", "dog"}},
        {"c.txt", {"the", "bird"}}
    });
    iris::TfidfFunc func(corpus);
    auto result = func.execute({std::any(std::string("the"))});
    std::string output = std::any_cast<std::string>(result);

    EXPECT_NE(output.find("0.000"), std::string::npos);
    EXPECT_EQ(output, "a.txt:0.000,b.txt:0.000,c.txt:0.000");
}

TEST(TfidfFuncTest, AbsentTerm) {
    MockCorpus corpus({
        {"doc1.txt", {"hello", "world"}},
        {"doc2.txt", {"foo", "bar"}}
    });
    iris::TfidfFunc func(corpus);
    auto result = func.execute({std::any(std::string("missing"))});
    std::string output = std::any_cast<std::string>(result);

    EXPECT_NE(output.find("0.000"), std::string::npos);
}

TEST(TfidfFuncTest, OutputFormatSortedDescending) {
    MockCorpus corpus({
        {"doc_a.txt", {"alpha", "alpha", "beta", "gamma"}},
        {"doc_b.txt", {"alpha", "delta", "epsilon", "zeta"}},
        {"doc_c.txt", {"beta", "gamma", "delta", "epsilon"}}
    });
    iris::TfidfFunc func(corpus);
    auto result = func.execute({std::any(std::string("alpha"))});
    std::string output = std::any_cast<std::string>(result);

    // Should be sorted descending: doc_a first, then doc_b, then doc_c
    auto pos_a = output.find("doc_a.txt");
    auto pos_b = output.find("doc_b.txt");
    auto pos_c = output.find("doc_c.txt");
    EXPECT_LT(pos_a, pos_b);
    EXPECT_LT(pos_b, pos_c);
    EXPECT_NE(output.find("doc_c.txt:0.000"), std::string::npos);
}

TEST(TfidfFuncTest, CaseInsensitiveTerm) {
    MockCorpus corpus({
        {"doc1.txt", {"hello", "world"}}
    });
    iris::TfidfFunc func(corpus);
    auto result = func.execute({std::any(std::string("HELLO"))});
    std::string output = std::any_cast<std::string>(result);
    EXPECT_EQ(output, "doc1.txt:0.000");
}

TEST(TfidfFuncTest, WrongArgCountThrows) {
    MockCorpus corpus;
    iris::TfidfFunc func(corpus);
    EXPECT_THROW(func.execute({}), std::invalid_argument);
}

TEST(TfidfFuncTest, WrongArgTypeThrows) {
    MockCorpus corpus;
    iris::TfidfFunc func(corpus);
    EXPECT_THROW(func.execute({std::any(42)}), std::invalid_argument);
}
