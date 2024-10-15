#include <gtest/gtest.h>  // Google Test
#include "grep_utils.h"
#include "grep_interface_functions.h"
#include <sstream>
#include <filesystem>

namespace grep_tests
{
  TEST(ReadGrepArgumentsTest, ValidInput)
  {
    const char *argv[] = {"program", "arg1 arg1", "arg2"};
    int argc = 3;

    auto result = grep::read_grep_arguments(argc, const_cast<char **>(argv));

    EXPECT_EQ(result.first, "arg1 arg1");
    EXPECT_EQ(result.second, "arg2");
  }

  TEST(ReadGrepArgumentsTest, LessThanTwoArguments)
  {
    const char *argv[] = {"program", "arg1"};
    int argc = 2;

    auto result = grep::read_grep_arguments(argc, const_cast<char **>(argv));

    EXPECT_EQ(result.first, "");
    EXPECT_EQ(result.second, "");
  }

  TEST(ReadGrepArgumentsTest, MoreThanTwoArguments)
  {
    const char *argv[] = {"program", "arg1", "arg2", "arg3"};
    int argc = 4;

    auto result = grep::read_grep_arguments(argc, const_cast<char **>(argv));

    EXPECT_EQ(result.first, "");
    EXPECT_EQ(result.second, "");
  }

  namespace fs = std::filesystem;

  auto file_collector = [](std::vector<fs::path> &collected_files)
  {
    return [&collected_files](fs::path file)
    {
      collected_files.push_back(file);
    };
  };

  TEST(FindFilesTest, PathDoesNotExist)
  {
    std::vector<fs::path> files;
    std::string non_existent_path = "./test_assets/non_existent_directory";
    grep::find_files(non_existent_path, file_collector(files));
    EXPECT_TRUE(files.empty());
  }

  TEST(FindFilesTest, SingleRegularFile)
  {
    std::vector<fs::path> files;
    fs::path single_file = "./test_assets/single_file.txt";
    grep::find_files(single_file, file_collector(files));
    EXPECT_EQ(files.size(), 1);
    EXPECT_EQ(files.front(), single_file);
  }

  TEST(FindFilesTest, DirectoryWithFiles)
  {
    std::vector<fs::path> files;
    fs::path directory = "./test_assets/test_directory";
    grep::find_files(directory, file_collector(files));
    EXPECT_EQ(files.size(), 2);
    EXPECT_EQ(files[0], "./test_assets/test_directory/test.txt");
    EXPECT_EQ(files[1], "./test_assets/test_directory/subdir/test.txt");
  }

  TEST(OutputColoredResultTest, WithMatched)
  {
    grep::Result::LineMatchResults line_match_results = {
        {"This is a test line with a match", {grep::MatchPosition(27, 31)}},
        {"Another line with multiple matches", {grep::MatchPosition(8, 11), grep::MatchPosition(18, 25)}}};
    grep::Result result("test_file.txt", std::move(line_match_results));

    std::stringstream outputBuffer;
    const auto oldCoutStreamBuf = std::cout.rdbuf();
    std::cout.rdbuf(outputBuffer.rdbuf());

    output_colored_result(result);

    std::cout.rdbuf(oldCoutStreamBuf);

    std::string expectedOutput =
        "test_file.txt: This is a test line with a " + grep::ColorStart + "match" + grep::ColorEnd + "\n"
                                                                                                     "test_file.txt: Another " +
        grep::ColorStart + "line" + grep::ColorEnd + " with " + grep::ColorStart + "multiple" + grep::ColorEnd + " matches\n";

    EXPECT_EQ(outputBuffer.str(), expectedOutput);
  }

  TEST(SearchInFileTest, SearchMatchInFile)
  {
    grep::Result::LineMatchResults line_match_results = {
        {"matchmatchgmatch", {grep::MatchPosition(0, 4), grep::MatchPosition(5, 9), grep::MatchPosition(11, 15)}},
        {"match", {grep::MatchPosition(0, 4)}}};

    grep::Result expectedResult("./test_assets/single_file.txt", std::move(line_match_results));
    const auto actualResult = grep::search_in_file("match", "./test_assets/single_file.txt");

    EXPECT_TRUE(expectedResult == actualResult);
  }

  // Helper function to split string by lines
  std::vector<std::string> splitLines(const std::string &output)
  {
    std::vector<std::string> lines;
    std::istringstream stream(output);
    std::string line;
    while (std::getline(stream, line))
    {
      lines.push_back(line + "\n"); // Preserve the newline at the end
    }
    return lines;
  }

  TEST(GrepTest, WithMatched)
  {
    std::stringstream outputBuffer;
    const auto oldCoutStreamBuf = std::cout.rdbuf();
    std::cout.rdbuf(outputBuffer.rdbuf());

    grep::grep("match", "./test_assets/");

    std::cout.rdbuf(oldCoutStreamBuf);

    std::vector<std::string> expectedLines = {
        "./test_assets/test_directory/subdir/test.txt: iii" + grep::ColorStart + "match" + grep::ColorEnd + "iii" + "\n",
        "./test_assets/single_file.txt: " + grep::ColorStart + "match" + grep::ColorEnd + grep::ColorStart + "match" + grep::ColorEnd + "g" + grep::ColorStart + "match" + grep::ColorEnd + "\n",
        "./test_assets/single_file.txt: " + grep::ColorStart + "match" + grep::ColorEnd + "\n"};

    std::vector<std::string> actualLines = splitLines(outputBuffer.str());

    std::sort(expectedLines.begin(), expectedLines.end());
    std::sort(actualLines.begin(), actualLines.end());

    EXPECT_EQ(actualLines.size(), expectedLines.size());
    for (size_t i = 0; i < expectedLines.size(); ++i)
    {
      EXPECT_EQ(actualLines[i], expectedLines[i]);
    }
  }

  TEST(GrepTest, NoMatches)
  {
    std::stringstream outputBuffer;
    const auto oldCoutStreamBuf = std::cout.rdbuf();
    std::cout.rdbuf(outputBuffer.rdbuf());
    grep::grep("notfound", "./test_assets/");
    std::cout.rdbuf(oldCoutStreamBuf);

    EXPECT_TRUE(outputBuffer.str().empty());
  }

  TEST(GrepTest, WrongPath)
  {
    std::stringstream outputBuffer;
    const auto oldCoutStreamBuf = std::cout.rdbuf();
    std::cout.rdbuf(outputBuffer.rdbuf());
    grep::grep("notfound", "./test_wrong/");
    std::cout.rdbuf(oldCoutStreamBuf);

    EXPECT_TRUE(outputBuffer.str().empty());
  }
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
