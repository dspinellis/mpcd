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
    CPPUNIT_TEST(test_create_block_region_clones_simple);
    CPPUNIT_TEST(test_create_block_region_clones_no_open);
    CPPUNIT_TEST(test_create_block_region_clones_nested_open);
    CPPUNIT_TEST(test_create_line_extended_region_clones);
    CPPUNIT_TEST(test_extend_clones_same);
    CPPUNIT_TEST(test_extend_clones_different);
    CPPUNIT_TEST(test_extend_clones_two_lines);
    CPPUNIT_TEST(test_remove_shadowed_groups);
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

    // Note '{' == 123 and '}' == 125
    void test_create_block_region_clones_simple() {
        std::istringstream iss(
// Line:        1              2  3
// Offset:      0  1   2  3    4  5  6   7  8
        "Fname\n12 123 42 125\n9\n12 123 42 125\n");
        TokenContainer tc(iss);
        CloneDetector cd(tc, 2);
        cd.prune_non_clones();
        cd.create_block_region_clones();
        CPPUNIT_ASSERT_EQUAL(1, cd.get_number_of_clone_groups());
        CPPUNIT_ASSERT_EQUAL(2, cd.get_number_of_clones());
    }

    void test_create_block_region_clones_no_open() {
        std::istringstream iss(
// Line:        1            2 3  4            5  6               7            8
// Offset:      0  1   2  3    4  5  6   7  8  9  10 11  12  13  14 15  16 17
        "Fname\n12 123 42 4\n\n7\n12 123 42 9\n7\n15 123 10 125\n15 123 10 25\n15 123 10 125\n");
        TokenContainer tc(iss);
        CloneDetector cd(tc, 2);
        cd.prune_non_clones();
        cd.create_block_region_clones();
        // cd.report_text();
        CPPUNIT_ASSERT_EQUAL(1, cd.get_number_of_clone_groups());
        CPPUNIT_ASSERT_EQUAL(2, cd.get_number_of_clones());
    }

    void test_create_block_region_clones_nested_open() {
        std::istringstream iss(
        "Fname\n12 123 42 4\n\n7\n12 123 42 9\n7\n15 123 10 123 12 125 33\n 125\n15 123 10 25\n15 123 10 123 12 125 33\n 125\n");
        TokenContainer tc(iss);
        CloneDetector cd(tc, 2);
        cd.prune_non_clones();
        cd.create_block_region_clones();
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

        for (const auto& clone_group: cd.clone_view()) {
            CPPUNIT_ASSERT_EQUAL(size_t(2), clone_group.size());
            for (const auto& clone: clone_group) {
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

        for (const auto& clone_group: cd.clone_view()) {
            CPPUNIT_ASSERT_EQUAL(size_t(2), clone_group.size());
            for (const auto& clone: clone_group)
                CPPUNIT_ASSERT_EQUAL(size_t(3), clone.size());
        }

        cd.extend_clones();
        for (const auto& clone_group: cd.clone_view()) {
            CPPUNIT_ASSERT_EQUAL(size_t(2), clone_group.size());
            for (const auto& clone: clone_group)
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

        for (const auto& clone_group: cd.clone_view()) {
            CPPUNIT_ASSERT_EQUAL(size_t(2), clone_group.size());
            for (const auto& clone: clone_group)
                CPPUNIT_ASSERT_EQUAL(size_t(3), clone.size());
        }

        cd.extend_clones();
        for (const auto& clone_group: cd.clone_view()) {
            CPPUNIT_ASSERT_EQUAL(size_t(2), clone_group.size());
            for (const auto& clone: clone_group)
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

        for (const auto& clone_group: cd.clone_view()) {
            CPPUNIT_ASSERT_EQUAL(size_t(2), clone_group.size());
            for (const auto& clone: clone_group)
                CPPUNIT_ASSERT(clone.size() == 3 || clone.size() == 4);
        }

        cd.extend_clones();
        for (const auto& clone_group: cd.clone_view()) {
            CPPUNIT_ASSERT_EQUAL(size_t(2), clone_group.size());
            for (const auto& clone: clone_group)
                CPPUNIT_ASSERT(clone.size() == 4 || clone.size() == 7);
        }
    }

    void test_remove_shadowed_groups() {
        std::istringstream iss("Fname\n12 42 3\n4 7\n12 42 3\n4 7");
        TokenContainer tc(iss);
        CloneDetector cd(tc, 2);
        cd.prune_non_clones();
        cd.create_line_region_clones();
        CPPUNIT_ASSERT_EQUAL(2, cd.get_number_of_clone_groups());
        CPPUNIT_ASSERT_EQUAL(4, cd.get_number_of_clones());
        CPPUNIT_ASSERT_EQUAL(std::size_t(5), cd.get_number_of_clone_tokens());

        cd.extend_clones();
        CPPUNIT_ASSERT_EQUAL(std::size_t(7), cd.get_number_of_clone_tokens());

        cd.remove_shadowed_groups();
        CPPUNIT_ASSERT_EQUAL(1, cd.get_number_of_clone_groups());
        CPPUNIT_ASSERT_EQUAL(2, cd.get_number_of_clones());
        CPPUNIT_ASSERT_EQUAL(std::size_t(5), cd.get_number_of_clone_tokens());
    }
};
