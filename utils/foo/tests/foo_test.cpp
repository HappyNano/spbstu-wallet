#include <gtest/gtest.h>

#include <utils/foo/foo.h>

TEST(FooTest, FooTestOne) {
    EXPECT_STREQ(util::foo().data(), "Hello World!\n");
}

TEST(FooTest, FooIntTest) {
    EXPECT_EQ(util::fooInt(), 1);
}
