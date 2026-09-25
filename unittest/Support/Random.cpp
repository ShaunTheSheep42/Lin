#include <catch2/catch_all.hpp>

#include <cstdint>
#include <limits>
#include <mutex>
#include <thread>

import Support;

// Test 1: Verify generated numbers are within type limits
TEST_CASE("Random number is within type limits", "[random]") {
  for (int i = 0; i < 1000; ++i) {
    int val = lin::Random::Number<int>();
    REQUIRE(val >= std::numeric_limits<int>::min());
    REQUIRE(val <= std::numeric_limits<int>::max());
  }

  for (int i = 0; i < 1000; ++i) {
    short val = lin::Random::Number<short>();
    REQUIRE(val >= std::numeric_limits<short>::min());
    REQUIRE(val <= std::numeric_limits<short>::max());
  }
}

// Test 2: Verify randomness (distribution uniformity)
TEST_CASE("Random numbers have good distribution", "[random]") {
  std::set<int> unique_values;
  for (int i = 0; i < 10000; ++i) {
    unique_values.insert(lin::Random::Number<int>());
  }
  // Among 10000 generations, unique values should be very high
  REQUIRE(unique_values.size() > 9500);
}

// Test 3: Multi-threaded concurrency test
TEST_CASE("Random is thread-safe", "[random][thread]") {
  const int num_threads = 10;
  const int nums_per_thread = 1000;
  std::vector<std::thread> threads;
  std::vector<int> results;
  std::mutex mtx;

  for (int i = 0; i < num_threads; ++i) {
    threads.emplace_back([&results, &mtx] {
      for (int j = 0; j < nums_per_thread; ++j) {
        int val = lin::Random::Number<int>();
        std::lock_guard<std::mutex> lock(mtx);
        results.push_back(val);
      }
    });
  }

  for (auto &t : threads) {
    t.join();
  }

  // Verify total count
  REQUIRE(results.size() == num_threads * nums_per_thread);

  // Verify uniqueness
  std::set<int> unique_results(results.begin(), results.end());
  REQUIRE(unique_results.size() > num_threads * nums_per_thread * 0.9);
}

// Test 4: Template type test
TEST_CASE("Random works with different integral types", "[random][template]") {
  auto char_val = lin::Random::Number<char>();
  auto long_val = lin::Random::Number<long>();
  auto uint32_val = lin::Random::Number<uint32_t>();

  REQUIRE(char_val >= std::numeric_limits<char>::min());
  REQUIRE(long_val >= std::numeric_limits<long>::min());
  REQUIRE(uint32_val >= std::numeric_limits<uint32_t>::min());
}
