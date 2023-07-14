#pragma once

#include <cppunit/extensions/HelperMacros.h>
#include <sstream>

#include "TokenContainer.h"

class TokenContainerTest : public CppUnit::TestFixture  {
    CPPUNIT_TEST_SUITE(TokenContainerTest);
    CPPUNIT_TEST(test_construct);
    CPPUNIT_TEST(test_line_number_empty_last_full);
    CPPUNIT_TEST(test_line_number_empty_last_empty);
    CPPUNIT_TEST(test_line_view);
    CPPUNIT_TEST(test_remaining_tokens);
    CPPUNIT_TEST(test_line_begin);
    CPPUNIT_TEST(test_line_offset);
    CPPUNIT_TEST(test_get_file_name);
    CPPUNIT_TEST(test_get_token_line_number);
    CPPUNIT_TEST(test_get_offset_begin);
    CPPUNIT_TEST(test_line_end);
    CPPUNIT_TEST(test_get_token);
    CPPUNIT_TEST(test_get_preceding_eol_offset);
    CPPUNIT_TEST_SUITE_END();
public:
    void test_construct() {
        std::istringstream iss("Fname\n12 42\n\n7\n");
        TokenContainer tc(iss);

        for (const auto& file : tc.file_view()) {
            CPPUNIT_ASSERT_EQUAL(std::string("name"), file.get_name());
        }
    }

    void test_line_number_empty_last_full() {
        std::istringstream iss("Fname\n12 42\n\n7\n");
        TokenContainer tc(iss);

        for (const auto& file : tc.file_view()) {
            CPPUNIT_ASSERT(!file.line_is_empty(0));
            CPPUNIT_ASSERT(file.line_is_empty(1));
            CPPUNIT_ASSERT(!file.line_is_empty(2));
        }
    }

    void test_line_number_empty_last_empty() {
        std::istringstream iss("Fname\n12 42\n\n7\n\n");
        TokenContainer tc(iss);

        for (const auto& file : tc.file_view()) {
            CPPUNIT_ASSERT(!file.line_is_empty(0));
            CPPUNIT_ASSERT(file.line_is_empty(1));
            CPPUNIT_ASSERT(!file.line_is_empty(2));
            CPPUNIT_ASSERT(file.line_is_empty(3));
        }
    }

    void test_line_view() {
        std::istringstream iss("Fname\n12 42\n\n7\n");
        TokenContainer tc(iss);

        for (const auto& file : tc.file_view()) {
            FileData::line_number_type n = 0;
            for (const auto& line : file.line_view()) {
                CPPUNIT_ASSERT_EQUAL(n, line);
                ++n;
            }
        }
    }

    void test_remaining_tokens() {
        std::istringstream iss("Fname\n12 42\n\n7\n\n");
        TokenContainer tc(iss);

        for (const auto& file : tc.file_view()) {
            CPPUNIT_ASSERT_EQUAL((FileData::line_number_type)3, file.remaining_tokens(0));
            CPPUNIT_ASSERT_EQUAL((FileData::line_number_type)1, file.remaining_tokens(1));
            CPPUNIT_ASSERT_EQUAL((FileData::line_number_type)1, file.remaining_tokens(2));
            CPPUNIT_ASSERT_EQUAL((FileData::line_number_type)0, file.remaining_tokens(3));
        }
    }

    void test_line_begin() {
        std::istringstream iss("Fname\n12 42\n\n7\n\n");
        TokenContainer tc(iss);

        for (const auto& file : tc.file_view()) {
            CPPUNIT_ASSERT_EQUAL(FileData::token_type(12), *file.line_begin(0));
            CPPUNIT_ASSERT_EQUAL(FileData::token_type(7), *file.line_begin(1));
            CPPUNIT_ASSERT_EQUAL(FileData::token_type(7), *file.line_begin(2));
        }
    }

    void test_line_offset() {
        std::istringstream iss("Fname\n12 42\n\n7\n\n");
        TokenContainer tc(iss);

        for (const auto& file : tc.file_view()) {
            CPPUNIT_ASSERT_EQUAL(FileData::token_offset_type(0), file.line_offset(0));
            CPPUNIT_ASSERT_EQUAL(FileData::token_offset_type(2), file.line_offset(1));
            CPPUNIT_ASSERT_EQUAL(FileData::token_offset_type(2), file.line_offset(2));
            CPPUNIT_ASSERT_EQUAL(FileData::token_offset_type(3), file.line_offset(3));
        }
    }

    void test_get_file_name() {
        std::istringstream iss("Fname\n12 42\n\n7\n");
        TokenContainer tc(iss);

        CPPUNIT_ASSERT_EQUAL(std::string("name"), tc.get_file_name(0));
    }

    void test_get_token_line_number() {
        //                    Lines:   0      1 2  3
        //                    Tokens:   0  1    2
        std::istringstream iss("Fname\n12 42\n\n7\n\n");
        TokenContainer tc(iss);

        for (const auto& file : tc.file_view()) {
            CPPUNIT_ASSERT_EQUAL(FileData::line_number_type(0), file.get_token_line_number(0));
            CPPUNIT_ASSERT_EQUAL(FileData::line_number_type(0), file.get_token_line_number(1));
            CPPUNIT_ASSERT_EQUAL(FileData::line_number_type(2), file.get_token_line_number(2));
        }
        CPPUNIT_ASSERT_EQUAL(FileData::line_number_type(2), tc.get_token_line_number(0, 2));
        CPPUNIT_ASSERT_EQUAL(FileData::line_number_type(3), tc.get_token_line_number(0, 3));

        std::istringstream iss2("Fname\n12 42\n2");
        TokenContainer tc2(iss2);
        CPPUNIT_ASSERT_EQUAL(FileData::line_number_type(1), tc2.get_token_line_number(0, 3));
    }

    void test_get_offset_begin() {
        std::istringstream iss("Fname\n12 42\n\n7\n\n");
        TokenContainer tc(iss);

        for (const auto& file : tc.file_view()) {
            CPPUNIT_ASSERT_EQUAL((FileData::token_type)42, *file.offset_begin(1));
        }
        CPPUNIT_ASSERT_EQUAL((FileData::token_type)7, *tc.offset_begin(0, 2));
    }

    void test_line_end() {
        //                             0 1 2  3    4    5
        std::istringstream iss("Fname\n1 2 3\n4\n\n7\n\n");
        TokenContainer tc(iss);

        // Note that newlines are not stored or counted
        CPPUNIT_ASSERT_EQUAL(long(3), tc.line_from_offset_end(0, 0) - tc.offset_begin(0, 0));
        CPPUNIT_ASSERT_EQUAL(long(3), tc.line_from_offset_end(0, 1) - tc.offset_begin(0, 0));
        CPPUNIT_ASSERT_EQUAL(long(3), tc.line_from_offset_end(0, 2) - tc.offset_begin(0, 0));
        CPPUNIT_ASSERT_EQUAL(long(4), tc.line_from_offset_end(0, 3) - tc.offset_begin(0, 0));
        CPPUNIT_ASSERT_EQUAL(long(5), tc.line_from_offset_end(0, 4) - tc.offset_begin(0, 0));

        std::istringstream iss2("Fname\n1 2 3");
        TokenContainer tc2(iss2);
        CPPUNIT_ASSERT_EQUAL(long(3), tc2.line_from_offset_end(0, 0) - tc2.offset_begin(0, 0));
        CPPUNIT_ASSERT_EQUAL(long(3), tc2.line_from_offset_end(0, 2) - tc2.offset_begin(0, 0));
    }

    void test_get_token() {
        std::istringstream iss("Fname\n12 42\n\n7\n\n");
        TokenContainer tc(iss);

        for (const auto& file : tc.file_view()) {
            CPPUNIT_ASSERT_EQUAL((FileData::token_type)42, file.get_token(1));
        }
        CPPUNIT_ASSERT_EQUAL((FileData::token_type)7, tc.get_token(0, 2));
    }

    void test_get_preceding_eol_offset() {
	//                             0  1   2  3 4     5  6   7
        std::istringstream iss("Fname\n12 42\n9\n7 44\n\n33 55\n");
        TokenContainer tc(iss);

        for (const auto& file : tc.file_view()) {
            CPPUNIT_ASSERT_EQUAL(FileData::token_offset_type(3), file.get_preceding_eol_offset(4));
        }
        CPPUNIT_ASSERT_EQUAL(FileData::token_offset_type(0), tc.get_preceding_eol_offset(0, 0));
        CPPUNIT_ASSERT_EQUAL(FileData::token_offset_type(0), tc.get_preceding_eol_offset(0, 1));
        CPPUNIT_ASSERT_EQUAL(FileData::token_offset_type(2), tc.get_preceding_eol_offset(0, 2));
        CPPUNIT_ASSERT_EQUAL(FileData::token_offset_type(3), tc.get_preceding_eol_offset(0, 3));
        CPPUNIT_ASSERT_EQUAL(FileData::token_offset_type(3), tc.get_preceding_eol_offset(0, 4));
        CPPUNIT_ASSERT_EQUAL(FileData::token_offset_type(7), tc.get_preceding_eol_offset(0, 7));

        //                              0  1  2  3  4  5  6  7  8 9
        std::istringstream iss2("Fname\n12 42 3\n4\n7\n12 42 3\n4");
        TokenContainer tc2(iss2);
        CPPUNIT_ASSERT_EQUAL(FileData::token_offset_type(8), tc2.get_preceding_eol_offset(0, 8));
        CPPUNIT_ASSERT_EQUAL(FileData::token_offset_type(9), tc2.get_preceding_eol_offset(0, 9));
    }


};
