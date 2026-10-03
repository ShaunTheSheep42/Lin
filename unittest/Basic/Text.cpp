#include <catch2/catch_all.hpp>
#include <string>

import Basic.Geometry;
import Basic.Char;
import Basic.Text;

namespace Lin {

TEST_CASE("Text starts empty and hit-tests only occupied positions",
          "[text][query]") {
  Text text({10, 20});

  REQUIRE(text.isEmpty());
  REQUIRE(text.toString().empty());
  REQUIRE_FALSE(text.hitTest({10, 20}));

  text.insertChar({12, 21}, MakeChar(U'a'));

  REQUIRE_FALSE(text.isEmpty());
  REQUIRE(text.hitTest({12, 21}));
  REQUIRE_FALSE(text.hitTest({11, 21}));
  REQUIRE_FALSE(text.hitTest({12, 20}));
}

TEST_CASE("Text stores relative coordinates and exposes raw characters",
          "[text][query]") {
  Text text({10, 20});
  text.insertChar({12, 21}, MakeChar(U'a'));

  REQUIRE(text.getRawChar({12, 21}) == MakeChar(U'a'));
  REQUIRE(text.getChar({12, 21}) == MakeChar(U'a'));
  REQUIRE(text.getPayload().size() == 2);
  REQUIRE(text.getPayload()[1][2] == MakeChar(U'a'));
}

TEST_CASE("Text preserves gaps between independently positioned characters",
          "[text][sparse][string]") {
  Text text({0, 0});
  text.insertChar({0, 0}, MakeChar(U'a'));
  text.insertChar({4, 0}, MakeChar(U'x'));

  REQUIRE(text.hitTest({0, 0}));
  REQUIRE_FALSE(text.hitTest({1, 0}));
  REQUIRE_FALSE(text.hitTest({2, 0}));
  REQUIRE_FALSE(text.hitTest({3, 0}));
  REQUIRE(text.getRawChar({1, 0}) == WhiteSpace);
  REQUIRE(text.hitTest({4, 0}));
  REQUIRE(text.toString() == U"a   x");
}

TEST_CASE("Forward insertion at an occupied position shifts existing text",
          "[text][insert]") {
  Text text({0, 0});
  text.insertChar({0, 0}, MakeChar(U'b'));

  const auto result = text.insertChar({0, 0}, MakeChar(U'a'), Forward);

  REQUIRE(text.getRawChar({0, 0}) == U'a');
  REQUIRE(text.getRawChar({1, 0}) == U'b');
  REQUIRE(result.nextCursorPos.x == 0);
  REQUIRE(result.nextCursorPos.y == 0);
  REQUIRE(result.ep == Backward);
}

TEST_CASE("Backward insertion occurs after the current character",
          "[text][insert]") {
  Text text({0, 0});
  text.insertChar({0, 0}, MakeChar(U'a'));
  text.insertChar({1, 0}, MakeChar(U'b'));

  text.insertChar({0, 0}, MakeChar(U'x'), Backward);

  REQUIRE(text.getRawChar({0, 0}) == U'a');
  REQUIRE(text.getRawChar({1, 0}) == U'x');
  REQUIRE(text.getRawChar({2, 0}) == U'b');
}

TEST_CASE("Insertion into a sparse segment consumes only its adjacent gap",
          "[text][insert][sparse]") {
  Text text({0, 0});
  text.insertChar({0, 0}, MakeChar(U'a'));
  text.insertChar({1, 0}, MakeChar(U'b'));
  text.insertChar({2, 0}, MakeChar(U'c'));
  text.insertChar({5, 0}, MakeChar(U'x'));
  text.insertChar({6, 0}, MakeChar(U'y'));
  text.insertChar({7, 0}, MakeChar(U'z'));
  text.insertChar({2, 0}, MakeChar(U'd'), Backward);

  REQUIRE(text.getRawChar({3, 0}) == MakeChar(U'd'));
  REQUIRE(text.getRawChar({5, 0}) == MakeChar(U'x'));
  REQUIRE(text.getRawChar({6, 0}) == MakeChar(U'y'));
  REQUIRE(text.getRawChar({7, 0}) == MakeChar(U'z'));
  REQUIRE_FALSE(text.hitTest({4, 0}));

  text.insertChar({3, 0}, MakeChar(U'e'), Backward);

  REQUIRE(text.getRawChar({3, 0}) == MakeChar(U'd'));
  REQUIRE(text.getRawChar({4, 0}) == MakeChar(U'e'));
  REQUIRE(text.getRawChar({5, 0}) == MakeChar(U'x'));
  REQUIRE(text.toString() == U"abcdexyz");
}

TEST_CASE("Full-width characters occupy a placeholder column",
          "[text][wide-character]") {
  Text text({3, 4});
  Char wide = MakeChar(U'界');
  text.insertChar({3, 4}, wide, Forward);

  REQUIRE(text.getRawChar({3, 4}) == wide);
  REQUIRE(text.getRawChar({4, 4}) == Lin::PlaceHolder);
  REQUIRE(text.getChar({4, 4}) == wide);
  REQUIRE(text.toString() == U"界");
}

TEST_CASE("Inserting a full-width character preserves the shifted character",
          "[text][insert][wide-character]") {
  Text text({0, 0});
  text.insertChar({0, 0}, MakeChar(U'a'));
  text.insertChar({0, 0}, MakeChar(U'界'), Forward);
  REQUIRE(text.hitTest({0, 0}));
  REQUIRE(text.hitTest({1, 0}));
  REQUIRE(text.hitTest({2, 0}));
  REQUIRE(text.getRawChar({0, 0}) == MakeChar(U'界'));
  REQUIRE(text.getRawChar({1, 0}) == Lin::PlaceHolder);
  REQUIRE(text.getRawChar({2, 0}) == U'a');
}

TEST_CASE("Updating a character maintains full-width placeholder state",
          "[text][update][wide-character]") {
  Text text({0, 0});
  text.insertChar({0, 0}, MakeChar(U'a'));

  Char wide = MakeChar(U'界');
  text.updateChar({0, 0}, wide);
  REQUIRE(text.getRawChar({0, 0}) == wide);
  REQUIRE(text.getRawChar({1, 0}) == Lin::PlaceHolder);
  REQUIRE(text.getChar({1, 0}) == wide);

  text.updateChar({1, 0}, MakeChar(U'b'));
  REQUIRE(text.getRawChar({0, 0}) == U'b');
  REQUIRE_FALSE(text.hitTest({1, 0}));
  REQUIRE(text.toString() == U"b");
}

TEST_CASE("Updating a point in a gap creates a sparse character",
          "[text][update][sparse]") {
  Text text({0, 0});
  text.insertChar({0, 0}, MakeChar(U'a'));
  text.insertChar({4, 0}, MakeChar(U'x'));

  text.updateChar({2, 0}, MakeChar(U'b'));

  REQUIRE(text.getRawChar({0, 0}) == U'a');
  REQUIRE(text.getRawChar({2, 0}) == U'b');
  REQUIRE(text.getRawChar({4, 0}) == U'x');
  REQUIRE(text.toString() == U"a b x");
}

TEST_CASE("Newline insertion splits the contiguous segment and aligns it",
          "[text][insert][newline]") {
  Text text({10, 20});
  text.insertChar({10, 20}, MakeChar(U'a'));
  text.insertChar({11, 20}, MakeChar(U'b'));
  text.insertChar({12, 20}, MakeChar(U'c'));

  const auto result = text.insertChar({11, 20}, MakeChar(U'\n'), Backward);

  REQUIRE(text.getRawChar({10, 20}) == U'a');
  REQUIRE(text.getRawChar({11, 20}) == U'b');
  REQUIRE_FALSE(text.hitTest({12, 20}));
  REQUIRE(text.getRawChar({10, 21}) == U'c');
  REQUIRE(result.nextCursorPos.x == 10);
  REQUIRE(result.nextCursorPos.y == 21);
  REQUIRE(result.ep == Forward);
}

TEST_CASE("Newline insertion before a character moves that character",
          "[text][insert][newline]") {
  Text text({0, 0});
  text.insertChar({0, 0}, MakeChar(U'a'));
  text.insertChar({1, 0}, MakeChar(U'b'));
  text.insertChar({2, 0}, MakeChar(U'c'));

  text.insertChar({2, 0}, MakeChar(U'\n'), Forward);

  REQUIRE(text.hitTest({0, 0}));
  REQUIRE(text.hitTest({1, 0}));
  REQUIRE(text.getRawChar({0, 0}) == U'a');
  REQUIRE(text.getRawChar({1, 0}) == U'b');
  REQUIRE_FALSE(text.hitTest({2, 0}));
  REQUIRE(text.hitTest({0, 1}));
  if (text.hitTest({0, 1}))
    REQUIRE(text.getRawChar({0, 1}) == U'c');
}

TEST_CASE("Newline does not move a separate segment after the current segment",
          "[text][insert][newline][sparse]") {
  Text text({0, 0});
  text.insertChar({0, 0}, MakeChar(U'a'));
  text.insertChar({1, 0}, MakeChar(U'b'));
  text.insertChar({2, 0}, MakeChar(U'c'));
  text.insertChar({5, 0}, MakeChar(U'x'));
  text.insertChar({6, 0}, MakeChar(U'y'));

  const auto result = text.insertChar({2, 0}, MakeChar(U'\n'), Backward);

  REQUIRE(text.getRawChar({0, 0}) == U'a');
  REQUIRE(text.getRawChar({1, 0}) == U'b');
  REQUIRE(text.getRawChar({2, 0}) == U'c');
  REQUIRE(text.getRawChar({5, 0}) == U'x');
  REQUIRE(text.getRawChar({6, 0}) == U'y');
  REQUIRE(result.nextCursorPos.x == 0);
  REQUIRE(result.nextCursorPos.y == 1);
}

TEST_CASE("Newline insertion keeps spaces in a moved contiguous suffix",
          "[text][insert][newline]") {
  Text text({0, 0});
  text.insertChar({0, 0}, MakeChar(U'a'));
  text.insertChar({1, 0}, MakeChar(U' '));
  text.insertChar({2, 0}, MakeChar(U'b'));

  text.insertChar({0, 0}, MakeChar(U'\n'), Backward);

  REQUIRE(text.getRawChar({0, 0}) == U'a');
  REQUIRE_FALSE(text.hitTest({1, 0}));
  REQUIRE_FALSE(text.hitTest({2, 0}));
  REQUIRE(text.getRawChar({0, 1}) == U' ');
  REQUIRE(text.getRawChar({1, 1}) == U'b');
  REQUIRE(text.toString() == U"a\n b");
}

TEST_CASE("insertString inserts a sequence at the requested position",
          "[text][insert][string]") {
  Text text({5, 7});

  text.insertString({5, 7}, U"abc");

  REQUIRE(text.getRawChar({5, 7}) == U'a');
  REQUIRE(text.getRawChar({6, 7}) == U'b');
  REQUIRE(text.getRawChar({7, 7}) == U'c');
  REQUIRE(text.toString() == U"abc");
}

TEST_CASE("insertString inserts before an existing suffix",
          "[text][insert][string]") {
  Text text({0, 0});
  text.insertString({0, 0}, U"ab");

  text.insertString({1, 0}, U"XY", Forward);

  REQUIRE(text.toString() == U"aXYb");
}

TEST_CASE("Inserting an empty string leaves text and cursor unchanged",
          "[text][insert][string]") {
  Text text({2, 3});
  text.insertChar({2, 3}, MakeChar(U'a'));

  const auto result = text.insertString({2, 3}, U"", Backward);

  REQUIRE(text.getRawChar({2, 3}) == U'a');
  REQUIRE(result.nextCursorPos.x == 2);
  REQUIRE(result.nextCursorPos.y == 3);
  REQUIRE(result.ep == Backward);
}

TEST_CASE("toString includes empty rows between sparse y coordinates",
          "[text][string][sparse]") {
  Text text({0, 0});
  text.insertChar({0, 0}, MakeChar(U'a'));
  text.insertChar({0, 2}, MakeChar(U'a'));

  REQUIRE(text.toString() == U"a\n\na");
}

TEST_CASE("Backward deletion removes the character at the cursor",
          "[text][delete]") {
  Text text({0, 0});
  text.insertChar({0, 0}, MakeChar(U'a'));
  text.insertChar({1, 0}, MakeChar(U'b'));
  text.insertChar({2, 0}, MakeChar(U'c'));

  const auto result = text.deleteChar({1, 0}, Backward);

  REQUIRE(text.hitTest({0, 0}));
  REQUIRE(text.getRawChar({0, 0}) == U'a');
  REQUIRE(text.hitTest({1, 0}));
  if (text.hitTest({1, 0}))
    REQUIRE(text.getRawChar({1, 0}) == U'c');
  REQUIRE_FALSE(text.hitTest({2, 0}));
  REQUIRE(result.nextCursorPos.x == 0);
  REQUIRE(result.nextCursorPos.y == 0);
  REQUIRE(result.ep == Backward);
}

TEST_CASE("Backward deletion at the beginning flips to forward editing",
          "[text][delete][boundary]") {
  Text text({0, 0});
  text.insertChar({0, 0}, MakeChar(U'a'));
  text.insertChar({1, 0}, MakeChar(U'b'));

  const auto result = text.deleteChar({0, 0}, Backward);

  REQUIRE(result.ep == Forward);
  REQUIRE(result.nextCursorPos.x == 0);
  REQUIRE(result.nextCursorPos.y == 0);
  REQUIRE(text.hitTest({0, 0}));
  REQUIRE(text.getRawChar({0, 0}) == MakeChar(U'b'));
}

TEST_CASE("Forward deletion removes the character before the cursor",
          "[text][delete]") {
  Text text({0, 0});
  text.insertChar({0, 0}, MakeChar(U'a'));
  text.insertChar({1, 0}, MakeChar(U'b'));
  text.insertChar({2, 0}, MakeChar(U'c'));

  const auto result = text.deleteChar({1, 0}, Forward);

  REQUIRE(text.hitTest({0, 0}));
  REQUIRE(text.getRawChar({0, 0}) == MakeChar(U'b'));
  REQUIRE(text.hitTest({1, 0}));
  REQUIRE(text.getRawChar({1, 0}) == MakeChar(U'c'));
  REQUIRE_FALSE(text.hitTest({2, 0}));
  REQUIRE(result.nextCursorPos.x == 0);
  REQUIRE(result.ep == Forward);
}

TEST_CASE("Newline insertion moves only the suffix of the current segment",
          "[text][insert][newline][sparse]") {
  Text text({0, 0});
  text.insertChar({0, 0}, MakeChar(U'a'));
  text.insertChar({1, 0}, MakeChar(U'b'));
  text.insertChar({2, 0}, MakeChar(U'c'));
  text.insertChar({5, 0}, MakeChar(U'd'));
  text.insertChar({6, 0}, MakeChar(U'e'));
  text.insertChar({7, 0}, MakeChar(U'f'));
  text.insertChar({10, 0}, MakeChar(U'x'));

  text.insertChar({5, 0}, MakeChar(U'\n'), Forward);

  REQUIRE(text.hitTest({0, 0}));
  REQUIRE(text.hitTest({1, 0}));
  REQUIRE(text.hitTest({2, 0}));
  REQUIRE_FALSE(text.hitTest({5, 0}));
  REQUIRE_FALSE(text.hitTest({6, 0}));
  REQUIRE_FALSE(text.hitTest({7, 0}));
  REQUIRE(text.hitTest({5, 1}));
  REQUIRE(text.hitTest({6, 1}));
  REQUIRE(text.hitTest({7, 1}));
  REQUIRE(text.hitTest({10, 0}));
  if (text.hitTest({5, 1}))
    REQUIRE(text.getRawChar({5, 1}) == MakeChar(U'd'));
  REQUIRE(text.getRawChar({10, 0}) == U'x');
}

TEST_CASE("Forward newline moves f but preserves the later hjk segment",
          "[text][requirements][newline][sparse]") {
  Text text({0, 0});
  text.insertString({0, 0}, U"abc");
  text.insertString({5, 0}, U"xyz");
  text.insertString({0, 1}, U"def");
  text.insertString({5, 1}, U"hjk");

  text.insertChar({2, 1}, MakeChar(U'\n'), Forward);

  REQUIRE(text.getPayload().size() == 3);
  REQUIRE(text.toString() == U"abc  xyz\nde   hjk\nf");
  REQUIRE(text.getRawChar({5, 1}) == MakeChar(U'h'));
  REQUIRE(text.getRawChar({0, 2}) == MakeChar(U'f'));
}

TEST_CASE("Backward newline at b preserves xyz and shifts later rows down",
          "[text][requirements][newline][sparse]") {
  Text text({0, 0});
  text.insertString({0, 0}, U"abc");
  text.insertString({5, 0}, U"xyz");
  text.insertString({0, 1}, U"def");
  text.insertString({5, 1}, U"hjk");

  text.insertChar({1, 0}, MakeChar(U'\n'), Backward);

  REQUIRE(text.getPayload().size() == 3);
  REQUIRE(text.toString() == U"ab   xyz\nc\ndef  hjk");
  REQUIRE(text.getRawChar({5, 0}) == MakeChar(U'x'));
  REQUIRE(text.getRawChar({0, 1}) == MakeChar(U'c'));
  REQUIRE(text.getRawChar({5, 2}) == MakeChar(U'h'));
}

TEST_CASE(
    "Newline after the last character keeps text in place and moves cursor",
    "[text][insert][newline]") {
  Text text({4, 8});
  text.insertChar({4, 8}, MakeChar(U'a'));
  text.insertChar({5, 8}, MakeChar(U'b'));

  const auto result = text.insertChar({5, 8}, MakeChar(U'\n'), Backward);

  REQUIRE(text.hitTest({4, 8}));
  REQUIRE(text.hitTest({5, 8}));
  REQUIRE(result.nextCursorPos.x == 4);
  REQUIRE(result.nextCursorPos.y == 9);
  REQUIRE(result.ep == Forward);
}

TEST_CASE("Updating an occupied point preserves distant sparse segments",
          "[text][update][sparse]") {
  Text text({0, 0});
  text.insertChar({0, 0}, MakeChar(U'a'));
  text.insertChar({4, 0}, MakeChar(U'x'));

  text.updateChar({0, 0}, MakeChar(U'b'));

  REQUIRE(text.getRawChar({0, 0}) == U'b');
  REQUIRE(text.getRawChar({4, 0}) == U'x');
  REQUIRE_FALSE(text.hitTest({1, 0}));
  REQUIRE(text.toString() == U"b   x");
}

TEST_CASE("Updating a character to newline splits its contiguous suffix",
          "[text][update][newline]") {
  Text text({0, 0});
  text.insertChar({0, 0}, MakeChar(U'a'));
  text.insertChar({1, 0}, MakeChar(U'b'));
  text.insertChar({2, 0}, MakeChar(U'c'));
  text.insertChar({5, 0}, MakeChar(U'x'));

  text.updateChar({1, 0}, MakeChar(U'\n'));

  REQUIRE(text.hitTest({0, 0}));
  REQUIRE_FALSE(text.hitTest({1, 0}));
  REQUIRE_FALSE(text.hitTest({2, 0}));
  REQUIRE(text.hitTest({0, 1}));
  REQUIRE(text.hitTest({5, 0}));
  REQUIRE(text.getRawChar({0, 0}) == U'a');
  REQUIRE(text.getRawChar({0, 1}) == U'c');
  REQUIRE(text.getRawChar({5, 0}) == U'x');
}

TEST_CASE(
    "Backward deletion of a full-width character preserves other segments",
    "[text][delete][wide-character][sparse]") {
  Text text({0, 0});
  Char wide = MakeChar(U'界');
  text.insertChar({0, 0}, wide, Forward);
  text.insertChar({4, 0}, MakeChar(U'x'));

  const auto result = text.deleteChar({0, 0}, Backward);

  REQUIRE_FALSE(text.hitTest({0, 0}));
  REQUIRE_FALSE(text.hitTest({1, 0}));
  REQUIRE(text.hitTest({4, 0}));
  REQUIRE(text.getRawChar({4, 0}) == U'x');
  REQUIRE(result.ep == Forward);
}

TEST_CASE("Deleting a full-width character removes its placeholder",
          "[text][delete][wide-character]") {
  Text text({0, 0});
  Char wide = MakeChar(U'界');
  text.insertChar({0, 0}, wide, Forward);
  text.insertChar({2, 0}, MakeChar(U'x'));

  text.deleteChar({0, 0}, Backward);

  REQUIRE(text.hitTest({0, 0}));
  REQUIRE_FALSE(text.hitTest({1, 0}));
  REQUIRE(text.getRawChar({0, 0}) == MakeChar(U'x'));
}

TEST_CASE("Deleting the only character makes text empty",
          "[text][delete][empty]") {
  Text text({0, 0});
  text.insertChar({0, 0}, MakeChar(U'a'));

  text.deleteChar({0, 0}, Backward);

  REQUIRE(text.isEmpty());
  REQUIRE(text.toString().empty());
  REQUIRE_FALSE(text.hitTest({0, 0}));
}

TEST_CASE("Deleting beyond either end is safe and updates edit direction",
          "[text][delete][boundary]") {
  Text text({0, 0});
  text.insertChar({0, 0}, MakeChar(U'a'));

  const auto backward = text.deleteChar({0, 0}, Backward);
  REQUIRE(backward.ep == Forward);
  REQUIRE(text.isEmpty());

  const auto forward = text.deleteChar({1, 0}, Forward);
  REQUIRE(forward.ep == Backward);
}

TEST_CASE("Backward insertion consumes one WhiteSpace from a sparse gap",
          "[text][requirements][whitespace]") {
  Text text({0, 0});
  text.insertString({0, 0}, U"abc");
  text.insertString({5, 0}, U"xyz");

  const auto result = text.insertChar({2, 0}, MakeChar(U'f'), Backward);

  REQUIRE(text.toString() == U"abcf xyz");
  REQUIRE(text.getRawChar({3, 0}) == MakeChar(U'f'));
  REQUIRE(text.getRawChar({4, 0}) == WhiteSpace);
  REQUIRE(text.getRawChar({5, 0}) == MakeChar(U'x'));
  REQUIRE(result.nextCursorPos.x == 3);
  REQUIRE(result.ep == Backward);
}

TEST_CASE("Backward characters inserted consecutively follow each other",
          "[text][requirements][insert]") {
  Text text({0, 0});
  text.insertString({0, 0}, U"abc");
  text.insertString({5, 0}, U"xyz");

  auto result = text.insertChar({2, 0}, MakeChar(U'd'), Backward);
  result = text.insertChar(result.nextCursorPos, MakeChar(U'e'), result.ep);

  REQUIRE(text.toString() == U"abcdexyz");
}

TEST_CASE("Forward characters inserted consecutively remain before current",
          "[text][requirements][insert]") {
  Text text({0, 0});
  text.insertString({0, 0}, U"abc");
  text.insertString({5, 0}, U"xyz");

  auto result = text.insertChar({2, 0}, MakeChar(U'd'), Forward);
  result = text.insertChar(result.nextCursorPos, MakeChar(U'e'), result.ep);

  REQUIRE(text.toString() == U"abdecxyz");
}

TEST_CASE(
    "Forward WhiteSpace insertions expand gaps before the current segment",
    "[text][requirements][whitespace]") {
  Text text({0, 0});
  text.insertString({0, 0}, U"abc");
  text.insertString({5, 0}, U"xyz");

  auto result = text.insertChar({2, 0}, WhiteSpace, Forward);
  result = text.insertChar(result.nextCursorPos, WhiteSpace, result.ep);

  REQUIRE(text.toString() == U"ab  c  xyz");
  REQUIRE(text.getRawChar({2, 0}) == WhiteSpace);
  REQUIRE(text.getRawChar({3, 0}) == WhiteSpace);
  REQUIRE(text.getRawChar({4, 0}) == MakeChar(U'c'));
  REQUIRE(text.getRawChar({5, 0}) == WhiteSpace);
  REQUIRE(text.getRawChar({6, 0}) == WhiteSpace);
}

TEST_CASE(
    "Forward WhiteSpace insertion expands the gap before the next segment",
    "[text][requirements][whitespace]") {
  Text text({0, 0});
  text.insertString({0, 0}, U"abc");
  text.insertString({5, 0}, U"xyz");

  auto result = text.insertChar({5, 0}, WhiteSpace, Forward);
  result = text.insertChar(result.nextCursorPos, WhiteSpace, result.ep);

  REQUIRE(text.toString() == U"abc    xyz");
  REQUIRE(text.getRawChar({2, 0}) == MakeChar(U'c'));
  REQUIRE(text.getRawChar({3, 0}) == WhiteSpace);
  REQUIRE(text.getRawChar({4, 0}) == WhiteSpace);
  REQUIRE(text.getRawChar({5, 0}) == WhiteSpace);
  REQUIRE(text.getRawChar({6, 0}) == WhiteSpace);
  REQUIRE(text.getRawChar({7, 0}) == MakeChar(U'x'));
}

TEST_CASE("Tab inserts one WhiteSpace column",
          "[text][requirements][whitespace][tab]") {
  Text text({0, 0});
  text.insertString({0, 0}, U"abc");
  text.insertString({5, 0}, U"xyz");

  const auto result = text.insertChar({2, 0}, MakeChar(U'\t'), Forward);

  REQUIRE(text.getRawChar({2, 0}) == WhiteSpace);
  REQUIRE(text.getRawChar({3, 0}) == MakeChar(U'c'));
  REQUIRE(text.getRawChar({6, 0}) == MakeChar(U'x'));
  REQUIRE(text.toString() == U"ab c  xyz");
  REQUIRE(result.ep == Forward);
}

TEST_CASE("Tab in insertString inserts a WhiteSpace column",
          "[text][requirements][whitespace][tab][string]") {
  Text text({0, 0});

  text.insertString({0, 0}, U"a\tb");

  REQUIRE(text.getRawChar({0, 0}) == MakeChar(U'a'));
  REQUIRE(text.getRawChar({1, 0}) == WhiteSpace);
  REQUIRE(text.getRawChar({2, 0}) == MakeChar(U'b'));
  REQUIRE(text.toString() == U"a b");
}

TEST_CASE("Ordinary spaces are content, distinct from WhiteSpace gaps",
          "[text][requirements][whitespace]") {
  Text text({0, 0});
  text.insertString({0, 0}, U"abc");
  text.insertString({5, 0}, U"xyz");

  auto result = text.insertChar({2, 0}, MakeChar(U' '), Backward);
  result = text.insertChar(result.nextCursorPos, MakeChar(U' '), result.ep);

  REQUIRE(text.getRawChar({3, 0}) == MakeChar(U' '));
  REQUIRE(text.getRawChar({4, 0}) == MakeChar(U' '));
  REQUIRE(text.getRawChar({5, 0}) == MakeChar(U'x'));
  REQUIRE(text.toString() == U"abc  xyz");

  Text singleSpace({0, 0});
  singleSpace.insertString({0, 0}, U"abc");
  singleSpace.insertString({5, 0}, U"xyz");
  singleSpace.insertChar({2, 0}, MakeChar(U' '), Backward);
  REQUIRE(singleSpace.getRawChar({3, 0}) == MakeChar(U' '));
  REQUIRE(singleSpace.getRawChar({4, 0}) == WhiteSpace);
  REQUIRE(singleSpace.toString() == U"abc  xyz");
}

TEST_CASE("Forward backspace removes the whitespace gap before the cursor",
          "[text][requirements][delete][whitespace]") {
  Text text({0, 0});
  text.insertString({0, 0}, U"abc");
  text.insertString({5, 0}, U"xyz");

  REQUIRE(text.toString() == U"abc  xyz");

  const auto result = text.deleteChar({5, 0}, Forward);

  REQUIRE(text.toString() == U"abc xyz");
  REQUIRE(text.getRawChar({3, 0}) == WhiteSpace);
  REQUIRE(text.getRawChar({4, 0}) == MakeChar(U'x'));
  REQUIRE(result.nextCursorPos.x == 4);
  REQUIRE(result.nextCursorPos.y == 0);
  REQUIRE(result.ep == Forward);
}

TEST_CASE("Forward backspace before c shifts c left and adds a WhiteSpace",
          "[text][requirements][delete][whitespace]") {
  Text text({0, 0});
  text.insertString({0, 0}, U"abc");
  text.insertString({5, 0}, U"def");

  REQUIRE(text.toString() == U"abc  def");

  const auto result = text.deleteChar({2, 0}, Forward);

  REQUIRE(text.toString() == U"ac   def");
  REQUIRE(text.getRawChar({0, 0}) == MakeChar(U'a'));
  REQUIRE(text.getRawChar({1, 0}) == MakeChar(U'c'));
  REQUIRE(text.getRawChar({2, 0}) == WhiteSpace);
  REQUIRE(text.getRawChar({3, 0}) == WhiteSpace);
  REQUIRE(text.getRawChar({4, 0}) == WhiteSpace);
  REQUIRE(text.getRawChar({5, 0}) == MakeChar(U'd'));
  REQUIRE(result.nextCursorPos == Position{1, 0});
  REQUIRE(result.ep == Forward);
}

TEST_CASE("Backward backspace at c removes c and preserves the next segment",
          "[text][requirements][delete][whitespace]") {
  Text text({0, 0});
  text.insertString({0, 0}, U"abc");
  text.insertString({5, 0}, U"def");

  const auto result = text.deleteChar({2, 0}, Backward);

  REQUIRE(text.toString() == U"ab   def");
  REQUIRE(text.getRawChar({0, 0}) == MakeChar(U'a'));
  REQUIRE(text.getRawChar({1, 0}) == MakeChar(U'b'));
  REQUIRE(text.getRawChar({2, 0}) == WhiteSpace);
  REQUIRE(text.getRawChar({3, 0}) == WhiteSpace);
  REQUIRE(text.getRawChar({4, 0}) == WhiteSpace);
  REQUIRE(text.getRawChar({5, 0}) == MakeChar(U'd'));
  REQUIRE(result.nextCursorPos == Position{1, 0});
  REQUIRE(result.ep == Backward);
}

TEST_CASE("Forward backspace at d removes one WhiteSpace before it",
          "[text][requirements][delete][whitespace]") {
  Text text({0, 0});
  text.insertString({0, 0}, U"abc");
  text.insertString({5, 0}, U"def");

  REQUIRE(text.getRawChar({3, 0}) == WhiteSpace);
  REQUIRE(text.getRawChar({4, 0}) == WhiteSpace);
  REQUIRE(text.getRawChar({5, 0}) == MakeChar(U'd'));
  REQUIRE(text.toString() == U"abc  def");

  const auto result = text.deleteChar({5, 0}, Forward);

  REQUIRE(text.getRawChar({2, 0}) == MakeChar(U'c'));
  REQUIRE(text.getRawChar({3, 0}) == WhiteSpace);
  REQUIRE(text.getRawChar({4, 0}) == MakeChar(U'd'));
  REQUIRE(text.toString() == U"abc def");
  REQUIRE(result.nextCursorPos == Position{4, 0});
  REQUIRE(result.ep == Forward);
}

TEST_CASE("Backward newline after a segment inserts an empty row",
          "[text][requirements][newline]") {
  Text text({0, 0});
  text.insertString({0, 0}, U"abc");
  text.insertString({5, 0}, U"xyz");

  const auto result = text.insertChar({2, 0}, MakeChar(U'\n'), Backward);

  REQUIRE(text.getPayload().size() == 2);
  REQUIRE(text.toString() == U"abc  xyz\n");
  REQUIRE(result.nextCursorPos.x == 0);
  REQUIRE(result.nextCursorPos.y == 1);
  REQUIRE(result.ep == Forward);
}

TEST_CASE("Backward newline after b moves c and preserves xyz coordinates",
          "[text][requirements][newline]") {
  Text text({0, 0});
  text.insertString({0, 0}, U"abc");
  text.insertString({5, 0}, U"xyz");

  text.insertChar({1, 0}, MakeChar(U'\n'), Backward);

  REQUIRE(text.getPayload().size() == 2);
  REQUIRE(text.toString() == U"ab   xyz\nc");
  REQUIRE(text.getRawChar({5, 0}) == MakeChar(U'x'));
  REQUIRE(text.getRawChar({0, 1}) == MakeChar(U'c'));
}

TEST_CASE(
    "Forward newline splits within a later segment and preserves alignment",
    "[text][requirements][newline]") {
  Text text({0, 0});
  text.insertString({0, 0}, U"abc");
  text.insertString({5, 0}, U"xyz");

  text.insertChar({6, 0}, MakeChar(U'\n'), Forward);

  REQUIRE(text.getPayload().size() == 2);
  REQUIRE(text.toString() == U"abc  x\n     yz");
  REQUIRE(text.getRawChar({5, 1}) == MakeChar(U'y'));
}

TEST_CASE("Backward WhiteSpace then newline leaves separated suffix in place",
          "[text][requirements][newline][whitespace]") {
  Text text({0, 0});
  text.insertString({0, 0}, U"abc");
  text.insertString({5, 0}, U"xyz");

  auto result = text.insertChar({6, 0}, WhiteSpace, Backward);
  result = text.insertChar(result.nextCursorPos, MakeChar(U'\n'), result.ep);

  REQUIRE(text.getPayload().size() == 2);
  REQUIRE(text.getRawChar({7, 0}) == WhiteSpace);
  REQUIRE(text.getRawChar({8, 0}) == MakeChar(U'z'));
  REQUIRE(text.getPayload()[1].empty());
  REQUIRE(result.nextCursorPos.x == 5);
  REQUIRE(result.nextCursorPos.y == 1);
}

TEST_CASE("Backward ordinary space then newline moves the following suffix",
          "[text][requirements][newline]") {
  Text text({0, 0});
  text.insertString({0, 0}, U"abc");
  text.insertString({5, 0}, U"xyz");

  auto result = text.insertChar({6, 0}, MakeChar(U' '), Backward);
  result = text.insertChar(result.nextCursorPos, MakeChar(U'\n'), result.ep);

  REQUIRE(text.getPayload().size() == 2);
  REQUIRE(text.toString() == U"abc  xy \n     z");
  REQUIRE(text.getRawChar({7, 0}) == MakeChar(U' '));
  REQUIRE(text.getRawChar({5, 1}) == MakeChar(U'z'));
}

TEST_CASE("Backward newline at the last separated character aligns next input",
          "[text][requirements][newline]") {
  Text text({0, 0});
  text.insertString({0, 0}, U"abcdefgh");
  text.insertString({0, 1}, U"i");
  text.insertString({7, 1}, U"j");

  auto result = text.insertChar({7, 1}, MakeChar(U'\n'), Backward);
  text.insertChar(result.nextCursorPos, MakeChar(U'u'), result.ep);

  REQUIRE(text.toString() == U"abcdefgh\ni      j\n       u");
}

TEST_CASE("Forward newline at the start of a separated segment moves it",
          "[text][requirements][newline]") {
  Text text({0, 0});
  text.insertString({0, 0}, U"abcdefgh");
  text.insertString({0, 1}, U"i");
  text.insertString({7, 1}, U"j");

  auto result = text.insertChar({0, 1}, MakeChar(U'\n'), Forward);
  text.insertChar(result.nextCursorPos, MakeChar(U'w'), result.ep);

  REQUIRE(text.toString() == U"abcdefgh\n      j\nwi");
}

TEST_CASE("Forward newline at i splits one contiguous i-space-j segment",
          "[text][requirements][newline]") {
  Text text({0, 0});
  text.insertString({0, 0}, U"abcdefgh");
  text.insertString({0, 1}, U"i j");

  auto result = text.insertChar({0, 1}, MakeChar(U'\n'), Forward);
  text.insertChar(result.nextCursorPos, MakeChar(U'w'), result.ep);

  REQUIRE(text.toString() == U"abcdefgh\n\nwi j");
}

} // namespace Lin
