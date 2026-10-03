#include <catch2/catch_all.hpp>

#include <string>

import Basic.Char;
import Basic.String;

namespace Lin {

TEST_CASE("MakeString constructs a single narrow character", "[string][make]") {
  const Char c = MakeChar(U'a');
  const String string = MakeString(c);

  REQUIRE(string.size() == 1);
  REQUIRE(string[0] == std::vector<Char>{c});
  REQUIRE(ToStdU32String(string) == std::u32string{U'a'});
}

TEST_CASE("MakeString adds a placeholder after a full-width character",
          "[string][make][wide-character]") {
  const Char c = MakeChar(U'界');
  const String string = MakeString(c);

  REQUIRE(string.size() == 1);
  REQUIRE(string[0].size() == 2);
  REQUIRE(string[0][0] == c);
  REQUIRE(string[0][1] == PlaceHolder);
  REQUIRE(ToStdU32String(string) == std::u32string{U'界'});
}

TEST_CASE("MakeString represents one newline as two empty lines",
          "[string][make][newline]") {
  const String string = MakeString(MakeChar(U'\n'));

  REQUIRE(string.size() == 2);
  REQUIRE(string[0].empty());
  REQUIRE(string[1].empty());
  REQUIRE(ToStdU32String(string) == std::u32string{U'\n'});
}

TEST_CASE("MakeString parses regular text and stores full-width placeholders",
          "[string][make]") {
  const String string = MakeString(std::u32string{U'a', U'界', U'b'});

  REQUIRE(string.size() == 1);
  REQUIRE(string[0].size() == 4);
  REQUIRE(string[0][0] == MakeChar(U'a'));
  REQUIRE(string[0][1] == MakeChar(U'界'));
  REQUIRE(string[0][2] == PlaceHolder);
  REQUIRE(string[0][3] == MakeChar(U'b'));
}

TEST_CASE("MakeString preserves consecutive and trailing newlines",
          "[string][make][newline]") {
  const String consecutive =
      MakeString(std::u32string{U'a', U'\n', U'\n', U'b'});
  REQUIRE(consecutive.size() == 3);
  REQUIRE(consecutive[0] == std::vector<Char>{MakeChar(U'a')});
  REQUIRE(consecutive[1].empty());
  REQUIRE(consecutive[2] == std::vector<Char>{MakeChar(U'b')});
  REQUIRE(ToStdU32String(consecutive) == std::u32string{U"a\n\nb"});

  const String trailing = MakeString(std::u32string{U'a', U'\n'});
  REQUIRE(trailing.size() == 2);
  REQUIRE(trailing[0] == std::vector<Char>{MakeChar(U'a')});
  REQUIRE(trailing[1].empty());
  REQUIRE(ToStdU32String(trailing) == std::u32string{U"a\n"});
}

TEST_CASE("MakeString of empty text produces an empty String",
          "[string][make]") {
  REQUIRE(MakeString(std::u32string{}).empty());
  REQUIRE(ToStdU32String(String{}).empty());
}

TEST_CASE("ToStdU32String distinguishes placeholders from whitespace",
          "[string][convert]") {
  const String string{
      {MakeChar(U'a'), PlaceHolder, WhiteSpace, MakeChar(U'b')}};

  REQUIRE(ToStdU32String(string) == std::u32string{U"a b"});
}

TEST_CASE("ToStdU32String joins rows, including empty rows",
          "[string][convert][newline]") {
  const String string{{MakeChar(U'a')}, {}, {MakeChar(U'b')}};

  REQUIRE(ToStdU32String(string) == std::u32string{U"a\n\nb"});
}

TEST_CASE("MakeString and ToStdU32String round-trip mixed-width text",
          "[string][round-trip]") {
  const std::u32string source{U'L', U'i', U'n', U'界', U' ', U'🧑'};

  REQUIRE(ToStdU32String(MakeString(source)) == source);
}

} // namespace Lin
