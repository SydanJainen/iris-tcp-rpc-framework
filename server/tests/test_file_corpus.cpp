#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include "adapters/file_corpus.h"

namespace {

class FileCorpusTest : public ::testing::Test {
protected:
    std::filesystem::path temp_dir_;

    void SetUp() override {
        temp_dir_ = std::filesystem::temp_directory_path() / "iris_test_corpus";
        std::filesystem::create_directories(temp_dir_);
    }

    void TearDown() override {
        std::filesystem::remove_all(temp_dir_);
    }

    void write_file(const std::string& name, const std::string& content) {
        std::ofstream f(temp_dir_ / name);
        f << content;
    }
};

} // namespace

TEST_F(FileCorpusTest, LoadsTextFiles) {
    write_file("hello.txt", "Hello World");
    write_file("test.txt", "Foo Bar Baz");

    iris::FileCorpus corpus(temp_dir_.string());
    EXPECT_EQ(corpus.document_count(), 2u);

    const auto& docs = corpus.get_documents();
    EXPECT_TRUE(docs.count("hello.txt"));
    EXPECT_TRUE(docs.count("test.txt"));
}

TEST_F(FileCorpusTest, TokenizesLowercase) {
    write_file("mixed.txt", "Hello WORLD");

    iris::FileCorpus corpus(temp_dir_.string());
    const auto& docs = corpus.get_documents();
    auto it = docs.find("mixed.txt");
    ASSERT_NE(it, docs.end());

    const auto& tokens = it->second;
    ASSERT_EQ(tokens.size(), 2u);
    EXPECT_EQ(tokens[0], "hello");
    EXPECT_EQ(tokens[1], "world");
}

TEST_F(FileCorpusTest, RemovesPunctuation) {
    write_file("punct.txt", "Hello, world! How's it going?");

    iris::FileCorpus corpus(temp_dir_.string());
    const auto& tokens = corpus.get_documents().at("punct.txt");

    for (const auto& t : tokens) {
        for (char c : t) {
            EXPECT_TRUE(std::isalnum(static_cast<unsigned char>(c))) << "Token '" << t << "' contains non-alphanumeric char";
        }
        EXPECT_FALSE(t.empty());
    }
}

TEST_F(FileCorpusTest, IgnoresNonTxtFiles) {
    write_file("data.txt", "valid content");
    write_file("notes.md", "markdown content");
    write_file("image.png", "binary junk");

    iris::FileCorpus corpus(temp_dir_.string());
    EXPECT_EQ(corpus.document_count(), 1u);
    EXPECT_TRUE(corpus.get_documents().count("data.txt"));
}

TEST_F(FileCorpusTest, EmptyDirectory) {
    // temp_dir_ exists but has no files
    iris::FileCorpus corpus(temp_dir_.string());
    EXPECT_EQ(corpus.document_count(), 0u);
    EXPECT_TRUE(corpus.get_documents().empty());
}

TEST_F(FileCorpusTest, NonExistentDirectory) {
    iris::FileCorpus corpus("nonexistent_dir_that_does_not_exist_12345");
    EXPECT_EQ(corpus.document_count(), 0u);
    EXPECT_TRUE(corpus.get_documents().empty());
}

TEST_F(FileCorpusTest, DocumentCount) {
    write_file("a.txt", "one");
    write_file("b.txt", "two");
    write_file("c.txt", "three");

    iris::FileCorpus corpus(temp_dir_.string());
    EXPECT_EQ(corpus.document_count(), 3u);
}
