#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "recipe.hh"


using namespace testing;
using namespace std;


TEST(RecipeTest, DefaultValues) {
  EXPECT_EQ(0, Recipe().servings());
}