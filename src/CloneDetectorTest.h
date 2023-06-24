#pragma once

#include <cppunit/extensions/HelperMacros.h>

#include "CloneDetector.h"

class CloneDetectorTest : public CppUnit::TestFixture  {
    CPPUNIT_TEST_SUITE(CloneDetectorTest);
    CPPUNIT_TEST(test_size);
    CPPUNIT_TEST(test_seen_compare);
    CPPUNIT_TEST(test_seen_container);
    CPPUNIT_TEST(test_insert);
    CPPUNIT_TEST(test_prune_non_clones);
    CPPUNIT_TEST(test_create_line_region_clones);
    CPPUNIT_TEST(test_create_line_extended_region_clones);
    CPPUNIT_TEST(test_extend_clones_same);
    CPPUNIT_TEST(test_extend_clones_different);
    CPPUNIT_TEST(test_extend_clones_two_lines);
    CPPUNIT_TEST_SUITE_END();
public:
    void test_size() {
        CPPUNIT_ASSERT_EQUAL(size_t(8), sizeof(CloneLocation));
    }

    void test_seen_compare() {
        //                             0  1  2    3  4  5  6  7  8 9 10 11 12 13
        std::istringstream iss("Fname\n12 42 4\n\n7\n12 42 9\n7\n5 9\n5 9\n5 9\n");
        TokenContainer tc(iss);
        CloneDetector cd(tc, 2);

        SeenTokens s1(0, 0);
        SeenTokens s2(0, 4);
        CPPUNIT_ASSERT(!(s1 < s2));
        CPPUNIT_ASSERT(!(s2 < s1));

        SeenTokens s3(0, 8);
        CPPUNIT_ASSERT(s3 < s1);
        CPPUNIT_ASSERT(!(s1 < s3));
    }

    void test_seen_container() {
        //                             0  1  2    3  4  5  6  7  8 9 10 11 12 13
        std::istringstream iss("Fname\n12 42 4\n\n7\n12 42 9\n7\n5 9\n5 9\n5 9\n");
        TokenContainer tc(iss);
        CloneDetector cd(tc, 2);

        std::map<SeenTokens, bool> m;
        m.insert(std::make_pair(SeenTokens(0, 7), true));
        CPPUNIT_ASSERT(m.find(SeenTokens(0, 7)) != m.end());

        m.insert(std::make_pair(SeenTokens(0, 8), true));
        CPPUNIT_ASSERT(m.find(SeenTokens(0, 8)) != m.end());
    }

    void test_insert() {
        std::istringstream iss("Fname\n12 42 4\n\n7\n12 42 9\n7\n5 10\n5 10\n5 10\n");
        TokenContainer tc(iss);
        CloneDetector cd(tc, 2);
        CPPUNIT_ASSERT_EQUAL(4, cd.get_number_of_seen_sites());
        CPPUNIT_ASSERT_EQUAL(5, cd.get_number_of_seen_clones());
    }

    void test_prune_non_clones() {
        std::istringstream iss("Fname\n12 42 4\n\n7\n12 42 9\n7\n5 10\n5 10\n5 10\n");
        TokenContainer tc(iss);
        CloneDetector cd(tc, 2);
        cd.prune_non_clones();
        CPPUNIT_ASSERT_EQUAL(2, cd.get_number_of_seen_sites());
        CPPUNIT_ASSERT_EQUAL(5, cd.get_number_of_seen_clones());
    }

    void test_create_line_region_clones() {
        std::istringstream iss(
        //              0  1  2    3  4  5  6  7  8  9   10 11 12  13 14
                "Fname\n12 42 4\n\n7\n12 42 9\n7\n15 10\n15 10 25\n15 10\n");
        TokenContainer tc(iss);
        CloneDetector cd(tc, 2);
        cd.prune_non_clones();
        cd.create_line_region_clones();
        CPPUNIT_ASSERT_EQUAL(1, cd.get_number_of_clone_groups());
        CPPUNIT_ASSERT_EQUAL(2, cd.get_number_of_clones());
    }

    void test_create_line_extended_region_clones() {
        std::istringstream iss(
        //              0  1  2    3  4  5  6
                "Fname\n12 42 4\n\n7\n12 42 4");
        TokenContainer tc(iss);
        CloneDetector cd(tc, 2);
        cd.prune_non_clones();
        cd.create_line_region_clones();
        CPPUNIT_ASSERT_EQUAL(1, cd.get_number_of_clone_groups());
        CPPUNIT_ASSERT_EQUAL(2, cd.get_number_of_clones());

        for (auto clone_group: cd.clone_view()) {
            CPPUNIT_ASSERT_EQUAL(size_t(2), clone_group.size());
            for (auto clone: clone_group) {
                CPPUNIT_ASSERT_EQUAL(size_t(3), clone.size());
            }
        }
    }

    void test_extend_clones_same() {
        //                             0  1  2  3  4  5  6  7  8 9
        std::istringstream iss("Fname\n12 42 3\n4\n7\n12 42 3\n4");
        TokenContainer tc(iss);
        CloneDetector cd(tc, 2);
        cd.prune_non_clones();
        cd.create_line_region_clones();
        CPPUNIT_ASSERT_EQUAL(1, cd.get_number_of_clone_groups());
        CPPUNIT_ASSERT_EQUAL(2, cd.get_number_of_clones());

        for (auto clone_group: cd.clone_view()) {
            CPPUNIT_ASSERT_EQUAL(size_t(2), clone_group.size());
            for (auto clone: clone_group)
                CPPUNIT_ASSERT_EQUAL(size_t(3), clone.size());
        }

        cd.extend_clones();
        for (auto clone_group: cd.clone_view()) {
            CPPUNIT_ASSERT_EQUAL(size_t(2), clone_group.size());
            for (auto clone: clone_group)
                CPPUNIT_ASSERT_EQUAL(size_t(4), clone.size());
        }
    }

    void test_extend_clones_different() {
        //                             0  1  2  3  4  5  6  7  8 9
        std::istringstream iss("Fname\n12 42 3\n4 5\n7\n12 42 3\n4 6");
        TokenContainer tc(iss);
        CloneDetector cd(tc, 2);
        cd.prune_non_clones();
        cd.create_line_region_clones();
        CPPUNIT_ASSERT_EQUAL(1, cd.get_number_of_clone_groups());
        CPPUNIT_ASSERT_EQUAL(2, cd.get_number_of_clones());

        for (auto clone_group: cd.clone_view()) {
            CPPUNIT_ASSERT_EQUAL(size_t(2), clone_group.size());
            for (auto clone: clone_group)
                CPPUNIT_ASSERT_EQUAL(size_t(3), clone.size());
        }

        cd.extend_clones();
        for (auto clone_group: cd.clone_view()) {
            CPPUNIT_ASSERT_EQUAL(size_t(2), clone_group.size());
            for (auto clone: clone_group)
                CPPUNIT_ASSERT_EQUAL(size_t(3), clone.size());
        }
    }

    void test_extend_clones_two_lines() {
        //                             0  1  2  3  4  5  6  7  8 9
        std::istringstream iss("Fname\n12 42 3\n4 5\n6 7\n\n7 8\n12 42 3\n4 5\n6 7\n7 9");
        TokenContainer tc(iss);
        CloneDetector cd(tc, 3);
        cd.prune_non_clones();
        cd.create_line_region_clones();

        CPPUNIT_ASSERT_EQUAL(2, cd.get_number_of_clone_groups());
        CPPUNIT_ASSERT_EQUAL(4, cd.get_number_of_clones());

        for (auto clone_group: cd.clone_view()) {
            CPPUNIT_ASSERT_EQUAL(size_t(2), clone_group.size());
            for (auto clone: clone_group)
                CPPUNIT_ASSERT(clone.size() == 3 || clone.size() == 4);
        }

        cd.extend_clones();
        for (auto clone_group: cd.clone_view()) {
            CPPUNIT_ASSERT_EQUAL(size_t(2), clone_group.size());
            for (auto clone: clone_group)
                CPPUNIT_ASSERT(clone.size() == 4 || clone.size() == 7);
        }
    }
};
