#include <gtest/gtest.h>

#include <utils/foo/foo.h>

TEST(FooIntTest, FooTest) {
    EXPECT_STREQ(util::foo().data(), "Hello World!\n");
}
