#include <gtest/gtest.h>
#include "kv/Store.hpp"

TEST(Store, StartsEmpty) {
    kv::Store s;
    EXPECT_EQ(s.size(), 0u);
}