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
    CPPUNIT_TEST(test_create_block_region_clones_bce);
    CPPUNIT_TEST(test_create_block_region_clones_same_prefix);
    CPPUNIT_TEST(test_create_block_region_clones_simple);
    CPPUNIT_TEST(test_create_block_region_clones_offset_success);
    CPPUNIT_TEST(test_create_block_region_clones_offset_internal_block_end_success);
    CPPUNIT_TEST(test_create_block_region_clones_offset_failure);
    CPPUNIT_TEST(test_create_block_region_clones_offset_internal_success);
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

    void test_create_block_region_clones_same_prefix() {
        std::istringstream iss(
// Line:        1    2   3              4  5    6   7
        "Fname\n125\n90\n12 123 42 125\n9\n125\n90\n12 123 42 125\n");
        TokenContainer tc(iss);
        CloneDetector cd(tc, 3);
        cd.prune_non_clones();
        cd.create_block_region_clones();
        CPPUNIT_ASSERT_EQUAL(1, cd.get_number_of_clone_groups());
        CPPUNIT_ASSERT_EQUAL(2, cd.get_number_of_clones());
        // cd.report_text();
        for (const auto& clone_group: cd.clone_view()) {
            CPPUNIT_ASSERT_EQUAL(size_t(2), clone_group.size());
            for (const auto& member: clone_group) {
                auto member_file_id = member.get_file_id();
                int line = tc.get_token_line_number(member_file_id, member.get_begin_token_offset()) + 1;
                CPPUNIT_ASSERT(line == 3 || line == 7);
            }
        }
    }

    void test_create_block_region_clones_bce() {
        std::istringstream iss(
        // From BigCloneEvaluate: very tight (start of file to EOF)
        // with enclosing shadowed element
        // selected,356996.java,50,90,selected,994186.java,47,87
        // { echo 'F994186.java\n'; tokenizer -l Java -o line -c "./ijadataset/bcb_reduced/2/selected/994186.java" | sed -n 47,87p ; echo 'F356996.java\n'; tokenizer -l Java -o line -c "./ijadataset/bcb_reduced/2/selected/356996.java" | sed -n 50,90p ; } | sed 's/\t/ /g;s/ $/\\n/' | tr -d \\n
        "F994186.java\n996 999 40 41 123\n999 999 61 362 59\n999 999 61 362 59\n999 999 61 362 59\n999 999 61 362 59\n830 123\n999 999 61 744 999 40 999 41 59\n999 999 61 40 999 41 999 46 999 40 41 59\n999 46 999 40 999 41 59\n999 46 999 40 999 41 59\n999 46 999 40 999 41 59\n999 46 999 40 362 41 59\n999 46 999 40 362 44 362 41 59\n999 46 999 40 362 44 362 41 59\n999 46 999 40 362 44 362 43 999 41 59\n999 999 61 744 999 40 999 46 999 40 41 41 59\n999 46 999 40 999 43 999 43 999 41 59\n999 46 999 40 362 43 999 46 999 40 999 46 999 40 362 41 43 998 41 43 362 43 999 41 59\n999 46 999 40 999 41 59\n999 999 61 744 999 40 999 41 59\n996 91 93 999 61 744 996 91 998 93 59\n996 999 61 998 59\n870 40 40 999 61 999 46 999 40 999 41 41 340 45 998 41 123\n999 46 999 40 999 44 998 44 999 41 59\n125\n999 46 999 40 41 59\n999 46 999 40 999 41 59\n999 46 999 40 999 43 999 43 999 43 999 41 59\n999 46 999 40 41 59\n999 999 61 999 46 999 40 41 59\n999 999 61 744 999 40 999 44 362 41 59\n999 999 61 744 999 40 999 41 59\n999 999 61 999 46 999 40 41 59\n999 46 999 40 823 44 999 44 999 46 999 41 46 999 40 41 59\n999 46 999 40 41 59\n999 46 999 40 41 59\n125 580 40 999 999 41 123\n999 46 999 40 41 59\n999 40 999 46 999 40 41 41 59\n125\n125\n"
        "F356996.java\n996 999 40 41 123\n999 999 61 362 59\n999 999 61 362 59\n999 999 61 362 59\n999 999 61 362 59\n830 123\n999 999 61 744 999 40 999 41 59\n999 999 61 40 999 41 999 46 999 40 41 59\n999 46 999 40 999 41 59\n999 46 999 40 999 41 59\n999 46 999 40 999 41 59\n999 46 999 40 362 41 59\n999 46 999 40 362 44 362 41 59\n999 46 999 40 362 44 362 41 59\n999 46 999 40 362 44 362 43 999 41 59\n999 999 61 744 999 40 999 46 999 40 41 41 59\n999 46 999 40 999 43 999 43 999 41 59\n999 46 999 40 362 43 999 46 999 40 999 46 999 40 362 41 43 998 41 43 362 43 999 41 59\n999 46 999 40 999 41 59\n999 999 61 744 999 40 999 41 59\n996 91 93 999 61 744 996 91 998 93 59\n996 999 61 998 59\n870 40 40 999 61 999 46 999 40 999 41 41 340 45 998 41 123\n999 46 999 40 999 44 998 44 999 41 59\n125\n999 46 999 40 41 59\n999 46 999 40 999 41 59\n999 46 999 40 999 43 999 43 999 43 999 41 59\n999 46 999 40 41 59\n999 999 61 999 46 999 40 41 59\n999 999 61 744 999 40 999 44 362 41 59\n999 999 61 744 999 40 999 41 59\n999 999 61 999 46 999 40 41 59\n999 46 999 40 823 44 999 44 999 46 999 41 46 999 40 41 59\n999 46 999 40 41 59\n999 46 999 40 41 59\n125 580 40 999 999 41 123\n999 46 999 40 41 59\n999 40 999 46 999 40 41 41 59\n125\n125\n"
        );
        TokenContainer tc(iss);
        CloneDetector cd(tc, 20);
        cd.prune_non_clones();
        cd.create_block_region_clones();
        cd.remove_shadowed_groups();
        // TODO: ensure all shadowed groups are removed
        // cd.report_text();
        // CPPUNIT_ASSERT_EQUAL(1, cd.get_number_of_clone_groups());
        // CPPUNIT_ASSERT_EQUAL(2, cd.get_number_of_clones());
        bool found = false;
        for (const auto& clone_group: cd.clone_view()) {
            CPPUNIT_ASSERT_EQUAL(size_t(2), clone_group.size());
            for (const auto& member: clone_group) {
                auto member_file_id = member.get_file_id();
                int begin_line = tc.get_token_line_number(member_file_id, member.get_begin_token_offset()) + 1;
                int end_line = tc.get_token_line_number(member_file_id, member.get_end_token_offset()) + 1;
                // std::cout << begin_line << ' '  << end_line << "\n";
                if (end_line - begin_line == 40)
                    found = true;
            }
        }
        CPPUNIT_ASSERT(found);
    }

    void test_create_block_region_clones_offset_success() {
        std::istringstream iss(
        //      No match Clone         No match Clone
        "Fname\n22 123 \n42 7\n125\n9\n12 123 \n42 7\n125\n");
        TokenContainer tc(iss);
        CloneDetector cd(tc, 2);
        cd.prune_non_clones();
        cd.create_block_region_clones();
        CPPUNIT_ASSERT_EQUAL(1, cd.get_number_of_clone_groups());
        CPPUNIT_ASSERT_EQUAL(2, cd.get_number_of_clones());
    }

    void test_create_block_region_clones_offset_internal_block_end_success() {
        std::istringstream iss(
        //      No match Clone           No match Clone
        "Fname\n22 123 \n42 125 11 7\n9\n12 123 \n42 125 11 7\n");
        TokenContainer tc(iss);
        CloneDetector cd(tc, 3);
        cd.prune_non_clones();
        cd.create_block_region_clones();
        CPPUNIT_ASSERT_EQUAL(1, cd.get_number_of_clone_groups());
        CPPUNIT_ASSERT_EQUAL(2, cd.get_number_of_clones());
    }

    void test_create_block_region_clones_offset_failure() {
        std::istringstream iss(
        //      No match Clone         No match Clone
        "Fname\n22 123 \n42 7\n125\n9\n12 123 \n42 7\n");
        TokenContainer tc(iss);
        CloneDetector cd(tc, 2);
        cd.prune_non_clones();
        cd.create_block_region_clones();
        CPPUNIT_ASSERT_EQUAL(0, cd.get_number_of_clone_groups());
    }

    void test_create_block_region_clones_offset_internal_success() {
        std::istringstream iss(
        //      No match Clone                No match Clone
        "Fname\n22 123 \n123 42 7 125\n125\n9\n12 123 \n123 42 7 125\n");
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
