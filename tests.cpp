#include "microtest.h"
#include "grep_utils.h"
#include "grep_interface_functions.h"

namespace grep_tests
{
  TEST(ReadGrepArgumentsTest_ValidInput)
  {
    const char *argv[] = {"program", "arg1 arg1", "arg2"};
    int argc = 3;

    auto result = grep::read_grep_arguments(argc, const_cast<char **>(argv));

    ASSERT_EQ(result.first, "arg1 arg1");
    ASSERT_EQ(result.second, "arg2");
  }

  TEST(ReadGrepArgumentsTest_LessThanTwoArguments)
  {
    const char *argv[] = {"program", "arg1"};
    int argc = 2;

    auto result = grep::read_grep_arguments(argc, const_cast<char **>(argv));

    ASSERT_EQ(result.first, "");
    ASSERT_EQ(result.second, "");
  }

  TEST(ReadGrepArgumentsTest_MoreThanTwoArguments)
  {
    const char *argv[] = {"program", "arg1", "arg2", "arg3"};
    int argc = 4;

    auto result = grep::read_grep_arguments(argc, const_cast<char **>(argv));

    ASSERT_EQ(result.first, "");
    ASSERT_EQ(result.second, "");
  }

  // todo move
  namespace fs = std::filesystem;

  auto file_collector = [](std::vector<fs::path> &collected_files)
  {
    return [&collected_files](fs::path file)
    {
      collected_files.push_back(file);
    };
  };

  TEST(FindFilesTest_PathDoesNotExist)
  {
    std::vector<fs::path> files;
    std::string non_existent_path = "./test/non_existent_directory";
    grep::find_files(non_existent_path, file_collector(files));
    ASSERT_TRUE(files.empty());
  }

  TEST(FindFilesTest_SingleRegularFile)
  {
    std::vector<fs::path> files;
    fs::path single_file = "./test/single_file.txt";
    grep::find_files(single_file, file_collector(files));
    ASSERT_EQ(files.size(), 1);
    ASSERT_EQ(files.front(), single_file);
  }

  TEST(FindFilesTest_DirectoryWithFiles)
  {
    std::vector<fs::path> files;
    fs::path directory = "./test/test_directory";
    grep::find_files(directory, file_collector(files));
    ASSERT_EQ(files.size(), 2);
    ASSERT_EQ(files[0], "./test/test_directory/test.txt");
    ASSERT_EQ(files[1], "./test/test_directory/subdir/test.txt");
  }

  TEST(OutputColoredResultTest_WithMatched)
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

    ASSERT_EQ(outputBuffer.str(), expectedOutput);
  }

  TEST(SearchInFileTest)
  {
    grep::Result::LineMatchResults line_match_results = {
        {"matchmatchgmatch", {grep::MatchPosition(0, 4), grep::MatchPosition(5, 9), grep::MatchPosition(11, 15)}},
        {"match", {grep::MatchPosition(0, 4)}}};

    grep::Result expectedResult("./test/single_file.txt", std::move(line_match_results));
    const auto actualResult = grep::search_in_file("match", "./test/single_file.txt");

    ASSERT_TRUE(expectedResult == actualResult);
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
  TEST(GrepTest_WithMatched)
  {
    std::stringstream outputBuffer;
    const auto oldCoutStreamBuf = std::cout.rdbuf();
    std::cout.rdbuf(outputBuffer.rdbuf());

    grep::grep("match", "./test/");

    std::cout.rdbuf(oldCoutStreamBuf);

    std::vector<std::string> expectedLines = {
        "./test/test_directory/subdir/test.txt: iii" + grep::ColorStart + "match" + grep::ColorEnd + "iii" + "\n",
        "./test/single_file.txt: " + grep::ColorStart + "match" + grep::ColorEnd + grep::ColorStart + "match" + grep::ColorEnd + "g" + grep::ColorStart + "match" + grep::ColorEnd + "\n",
        "./test/single_file.txt: " + grep::ColorStart + "match" + grep::ColorEnd + "\n"};

    // Split the actual output into lines
    std::vector<std::string> actualLines = splitLines(outputBuffer.str());

    // Sort both the expected and actual output, as order doesn't matter
    std::sort(expectedLines.begin(), expectedLines.end());
    std::sort(actualLines.begin(), actualLines.end());

    ASSERT_EQ(actualLines.size(), expectedLines.size());
    for (size_t i = 0; i < expectedLines.size(); ++i)
    {
      ASSERT_EQ(actualLines[i], expectedLines[i]);
    }
  }

  TEST(GrepTest_NoMatches)
  {
    std::stringstream outputBuffer;
    const auto oldCoutStreamBuf = std::cout.rdbuf();
    std::cout.rdbuf(outputBuffer.rdbuf());
    grep::grep("notfound", "./test/");
    std::cout.rdbuf(oldCoutStreamBuf);

    ASSERT_TRUE(outputBuffer.str().empty());
  }

  TEST(GrepTest_WrongPath)
  {
    std::stringstream outputBuffer;
    const auto oldCoutStreamBuf = std::cout.rdbuf();
    std::cout.rdbuf(outputBuffer.rdbuf());
    grep::grep("notfound", "./test_wrong/");
    std::cout.rdbuf(oldCoutStreamBuf);

    ASSERT_TRUE(outputBuffer.str().empty());
  }
}

TEST_MAIN();