#pragma once

#include <cppunit/extensions/HelperMacros.h>

#include "CloneDetector.h"

class CloneDetectorTest : public CppUnit::TestFixture  {
    CPPUNIT_TEST_SUITE(CloneDetectorTest);
    CPPUNIT_TEST(test_size);
    CPPUNIT_TEST(test_insert);
    CPPUNIT_TEST(test_prune_non_clones);
    CPPUNIT_TEST_SUITE_END();
public:
    void test_size() {
        CPPUNIT_ASSERT_EQUAL(size_t(8), sizeof(CloneLocation));
    }

    void test_insert() {
        std::istringstream iss("Fname\n12 42 4\n\n7\n12 42 9\n7\n5 10\n5 10\n5 10\n");
        TokenContainer tc(iss);
        CloneDetector cd(tc, 2);
        CPPUNIT_ASSERT_EQUAL(4, cd.get_number_of_sites());
        CPPUNIT_ASSERT_EQUAL(5, cd.get_number_of_clones());
    }

    void test_prune_non_clones() {
        std::istringstream iss("Fname\n12 42 4\n\n7\n12 42 9\n7\n5 10\n5 10\n5 10\n");
        TokenContainer tc(iss);
        CloneDetector cd(tc, 2);
        cd.prune_non_clones();
        CPPUNIT_ASSERT_EQUAL(2, cd.get_number_of_sites());
    }
};
