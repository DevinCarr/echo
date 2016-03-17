/*
 * Copyright (c) 2015 Devin Carr
 * Licensed under MIT (https://github.com/devincarr/echo/blob/master/LICENSE)
 */

#include "echo/watcher.h"
#include "gtest/gtest.h"

TEST(LCSSTest, GenericChecks) {
    ASSERT_EQ("A", longest_common_substr("A","A"));
    ASSERT_EQ("", longest_common_substr("",""));
    ASSERT_EQ("", longest_common_substr("A","B"));
}

TEST(LCSSTest, MatchWord) {
    ASSERT_EQ("ABC", longest_common_substr("ABCD","ABCE"));
    ASSERT_EQ("CDE", longest_common_substr("ACDE","BCDE"));
}

TEST(LCSSTest, MatchWords) {
    ASSERT_EQ("ABC DEF ", longest_common_substr("ABC DEF FGH","ABC DEF IJK"));
    ASSERT_EQ("The cat", longest_common_substr("The cat .","The cats."));
}

TEST(LCSSTest, SubsequenceTooShort) {
    // LCSS is shorter than 60% of the max(S,T)
    ASSERT_EQ("", longest_common_substr("ABC DEF FG", "ABC DFG HI"));
    ASSERT_EQ("", longest_common_substr("A","BCDEFGHIJKLMNOP"));
}

TEST(LCSSTest, MatchingCommonSpam) {
    ASSERT_EQ("Kappa", longest_common_substr("Kappa .","Kappa"));
    ASSERT_EQ("Kappa 123", longest_common_substr("Kappa 123 .","Kappa 123"));
    ASSERT_EQ("Keepo Keepo Keepo Keepo Keepo Keepo Keepo Keepo Keepo",
            longest_common_substr("Keepo Keepo Keepo Keepo Keepo Keepo Keepo Keepo Keepo",
                "Keepo Keepo Keepo Keepo Keepo Keepo Keepo Keepo Keepo 1"));
}

TEST(LCSSTest, LongMessages) {
    ASSERT_EQ("PAJLADA forsenFajita PAJLADA forsenFajita PAJLADA forsenFajita PAJLADA forsenFajita PAJLADA forsenFajita PAJLADA forsenFajita PAJLADA forsenFajita PAJLADA forsenFajita PAJLADA forsenFajita PAJLADA forsenFajita PAJLADA forsenFajita PAJLADA",
            longest_common_substr("PAJLADA forsenFajita PAJLADA forsenFajita PAJLADA forsenFajita PAJLADA forsenFajita PAJLADA forsenFajita PAJLADA forsenFajita PAJLADA forsenFajita PAJLADA forsenFajita PAJLADA forsenFajita PAJLADA forsenFajita PAJLADA forsenFajita PAJLADA",
                "PAJLADA forsenFajita PAJLADA forsenFajita PAJLADA forsenFajita PAJLADA forsenFajita PAJLADA forsenFajita PAJLADA forsenFajita PAJLADA forsenFajita PAJLADA forsenFajita PAJLADA forsenFajita PAJLADA forsenFajita PAJLADA forsenFajita PAJLADA"));
}
