#include <gtest/gtest.h>
#include "kv/Store.hpp"

#include <atomic>
#include <chrono>
#include <string>
#include <thread>
#include <vector>

using namespace std::chrono_literals;

namespace {
kv::TimePoint now() { return kv::Clock::now(); }

bool wellFormed(const std::string& v) {
    return v.size() >= 3 && v[0] == 't' && v.find('-') != std::string::npos;
}
}  // namespace

// ---------- basics ----------

TEST(Store, StartsEmpty) {
    kv::Store s;
    EXPECT_EQ(s.size(), 0u);
}

TEST(Store, SetThenGet) {
    kv::Store s;
    s.set("a", "1");
    EXPECT_EQ(s.get("a"), "1");
    EXPECT_EQ(s.size(), 1u);
}

TEST(Store, GetMissing) {
    kv::Store s;
    EXPECT_EQ(s.get("nope"), std::nullopt);
}

TEST(Store, Overwrite) {
    kv::Store s;
    s.set("a", "1");
    s.set("a", "2");
    EXPECT_EQ(s.get("a"), "2");
    EXPECT_EQ(s.size(), 1u);
}

TEST(Store, BinarySafeValues) {
    kv::Store s;
    std::string v("a\0b", 3);  // RESP values can contain NUL bytes
    s.set("", v);
    EXPECT_EQ(s.get(""), v);
}

TEST(Store, DelTwice) {
    kv::Store s;
    s.set("a", "1");
    EXPECT_TRUE(s.del("a"));
    EXPECT_FALSE(s.del("a"));
    EXPECT_EQ(s.get("a"), std::nullopt);
    EXPECT_EQ(s.size(), 0u);
}

TEST(Store, DelMissing) {
    kv::Store s;
    EXPECT_FALSE(s.del("nope"));
}

TEST(Store, ClearEmpties) {
    kv::Store s;
    s.set("a", "1");
    s.set("b", "2");
    s.clear();
    EXPECT_EQ(s.size(), 0u);
    EXPECT_EQ(s.get("a"), std::nullopt);
}

// ---------- expiry ----------

TEST(Store, ExpiredIsInvisible) {
    kv::Store s;
    s.set("a", "1", now() - 1s);
    EXPECT_EQ(s.get("a"), std::nullopt);
}

TEST(Store, FutureIsVisibleUntilDeadline) {
    kv::Store s;
    auto t = now();
    s.set("a", "1", t + 1h);
    EXPECT_EQ(s.get("a", t), "1");
    EXPECT_EQ(s.get("a", t + 2h), std::nullopt);
}

TEST(Store, ExactDeadlineIsExpired) {
    kv::Store s;
    auto t = now();
    s.set("a", "1", t + 1h);
    EXPECT_EQ(s.get("a", t + 1h), std::nullopt);  // <= now means expired
}

TEST(Store, GetDoesNotEraseExpired) {
    kv::Store s;
    s.set("a", "1", now() - 1s);
    EXPECT_EQ(s.get("a"), std::nullopt);
    EXPECT_EQ(s.size(), 1u);  // still in the map until del/sweep
}

TEST(Store, PlainSetClearsTtl) {
    kv::Store s;
    auto t = now();
    s.set("a", "1", t + 1s);
    s.set("a", "2");  // no expiry: lives forever
    EXPECT_EQ(s.get("a", t + 1h), "2");
}

TEST(Store, DelExpiredReturnsFalseButErases) {
    kv::Store s;
    s.set("a", "1", now() - 1s);
    EXPECT_FALSE(s.del("a"));
    EXPECT_EQ(s.size(), 0u);
}

// ---------- concurrency (run under KV_TSAN too) ----------

TEST(Store, DisjointThreads) {
    kv::Store s;
    constexpr int kThreads = 8, kOps = 10'000, kKeys = 100;
    std::atomic<int> mismatches{0};

    std::vector<std::thread> threads;
    for (int t = 0; t < kThreads; ++t) {
        threads.emplace_back([&, t] {
            for (int i = 0; i < kOps; ++i) {
                std::string key = std::to_string(t) + ":" + std::to_string(i % kKeys);
                if (i % 7 == 0) {
                    s.del(key);
                    if (s.get(key)) ++mismatches;
                } else {
                    std::string value = std::to_string(i);
                    s.set(key, value);
                    if (s.get(key) != value) ++mismatches;
                }
            }
        });
    }
    for (auto& th : threads) th.join();
    EXPECT_EQ(mismatches.load(), 0);

    // Final state is deterministic: each key's last op is i = kOps - kKeys + k.
    std::size_t expectedSize = 0;
    for (int t = 0; t < kThreads; ++t) {
        for (int k = 0; k < kKeys; ++k) {
            int last = kOps - kKeys + k;
            std::string key = std::to_string(t) + ":" + std::to_string(k);
            if (last % 7 == 0) {
                EXPECT_EQ(s.get(key), std::nullopt) << key;
            } else {
                EXPECT_EQ(s.get(key), std::to_string(last)) << key;
                ++expectedSize;
            }
        }
    }
    EXPECT_EQ(s.size(), expectedSize);
}

TEST(Store, ContendedKeys) {
    kv::Store s;
    constexpr int kThreads = 8, kOps = 10'000;
    std::atomic<int> bad{0};

    std::vector<std::thread> threads;
    for (int t = 0; t < kThreads; ++t) {
        threads.emplace_back([&, t] {
            for (int i = 0; i < kOps; ++i) {
                std::string key = "k" + std::to_string(i % 4);
                switch (i % 3) {
                    case 0:
                        s.set(key, "t" + std::to_string(t) + "-" + std::to_string(i));
                        break;
                    case 1:
                        if (auto v = s.get(key); v && !wellFormed(*v)) ++bad;
                        break;
                    case 2:
                        s.del(key);
                        break;
                }
            }
        });
    }
    for (auto& th : threads) th.join();
    EXPECT_EQ(bad.load(), 0);
    EXPECT_LE(s.size(), 4u);
}