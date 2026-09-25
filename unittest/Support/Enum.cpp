#include <catch2/catch_all.hpp>

import Support;

// Test enum for testing purposes
enum class TestMode { None, Read, Write, Append };

// Test 1: Enum to string conversion
TEST_CASE("Enum to string conversion works", "[enum]") {
  REQUIRE(Lin::Enum(TestMode::Read) == "Read");
  REQUIRE(Lin::Enum(TestMode::Write) == "Write");
  REQUIRE(Lin::Enum(TestMode::Append) == "Append");
  REQUIRE(Lin::Enum(TestMode::None) == "None");
}

// Test 2: String to enum conversion with valid values
TEST_CASE("String to enum conversion with valid values", "[enum]") {
  REQUIRE(Lin::Enum<TestMode>("Read") == TestMode::Read);
  REQUIRE(Lin::Enum<TestMode>("Write") == TestMode::Write);
  REQUIRE(Lin::Enum<TestMode>("Append") == TestMode::Append);
  REQUIRE(Lin::Enum<TestMode>("None") == TestMode::None);
}

// Test 3: String to enum conversion with invalid values falls back to None
TEST_CASE("String to enum conversion with invalid values returns None",
          "[enum]") {
  REQUIRE(Lin::Enum<TestMode>("InvalidValue") == TestMode::None);
  REQUIRE(Lin::Enum<TestMode>("") == TestMode::None);
  REQUIRE(Lin::Enum<TestMode>("read") == TestMode::None); // case sensitive
}

// Test 4: Round-trip conversion
TEST_CASE("Round-trip enum conversion", "[enum]") {
  auto modes = {TestMode::Read, TestMode::Write, TestMode::Append};
  for (auto mode : modes) {
    auto str = Lin::Enum(mode);
    auto back = Lin::Enum<TestMode>(str);
    REQUIRE(back == mode);
  }
}
