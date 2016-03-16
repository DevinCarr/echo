/*
 * Copyright (c) 2015 Devin Carr
 * Licensed under MIT (https://github.com/devincarr/echo/blob/master/LICENSE)
 */

#include "echo/watcher.h"
#include "gtest/gtest.h"

TEST(LevenshteinDistanceTest, MatchUpper) {
    EXPECT_EQ(0, levenshtein_distance("Kappa","Kappa"));

    EXPECT_EQ(0, levenshtein_distance("A","A"));

    EXPECT_EQ(0, levenshtein_distance("AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA",
               "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"));
}

TEST(LevenshteinDistanceTest, MatchLower) {
    EXPECT_EQ(3, levenshtein_distance("AAA","aaa"));
}
