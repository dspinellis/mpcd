#pragma once

#include <cppunit/extensions/HelperMacros.h>
#include <sstream>

#include "TokenContainer.h"

class TokenContainerTest : public CppUnit::TestFixture  {
    CPPUNIT_TEST_SUITE(TokenContainerTest);
    CPPUNIT_TEST(testConstruct);
    CPPUNIT_TEST(testLineNumberEmptyLastFull);
    CPPUNIT_TEST(testLineNumberEmptyLastEmpty);
    CPPUNIT_TEST(testLineView);
    CPPUNIT_TEST(testReaminingTokens);
    CPPUNIT_TEST(testLineBegin);
    CPPUNIT_TEST(testLineOffset);
    CPPUNIT_TEST_SUITE_END();
public:
    void testConstruct() {
        std::istringstream iss("Fname\n12 42\n\n7\n");
        TokenContainer tc(iss);

        for (auto file : tc.file_view()) {
            CPPUNIT_ASSERT_EQUAL(std::string("name"), file.get_name());
        }
    }

    void testLineNumberEmptyLastFull() {
        std::istringstream iss("Fname\n12 42\n\n7\n");
        TokenContainer tc(iss);

        for (auto file : tc.file_view()) {
            CPPUNIT_ASSERT(!file.line_is_empty(0));
            CPPUNIT_ASSERT(file.line_is_empty(1));
            CPPUNIT_ASSERT(!file.line_is_empty(2));
        }
    }

    void testLineNumberEmptyLastEmpty() {
        std::istringstream iss("Fname\n12 42\n\n7\n\n");
        TokenContainer tc(iss);

        for (auto file : tc.file_view()) {
            CPPUNIT_ASSERT(!file.line_is_empty(0));
            CPPUNIT_ASSERT(file.line_is_empty(1));
            CPPUNIT_ASSERT(!file.line_is_empty(2));
            CPPUNIT_ASSERT(file.line_is_empty(3));
        }
    }

    void testLineView() {
        std::istringstream iss("Fname\n12 42\n\n7\n");
        TokenContainer tc(iss);

        for (auto file : tc.file_view()) {
            FileData::line_number_type n = 0;
            for (auto line : file.line_view()) {
                CPPUNIT_ASSERT_EQUAL(n, line);
                n++;
            }
        }
    }

    void testReaminingTokens() {
        std::istringstream iss("Fname\n12 42\n\n7\n\n");
        TokenContainer tc(iss);

        for (auto file : tc.file_view()) {
            CPPUNIT_ASSERT_EQUAL((FileData::line_number_type)3, file.remaining_tokens(0));
            CPPUNIT_ASSERT_EQUAL((FileData::line_number_type)1, file.remaining_tokens(1));
            CPPUNIT_ASSERT_EQUAL((FileData::line_number_type)1, file.remaining_tokens(2));
            CPPUNIT_ASSERT_EQUAL((FileData::line_number_type)0, file.remaining_tokens(3));
        }
    }

    void testLineBegin() {
        std::istringstream iss("Fname\n12 42\n\n7\n\n");
        TokenContainer tc(iss);

        for (auto file : tc.file_view()) {
            CPPUNIT_ASSERT_EQUAL(FileData::token_type(12), *file.line_begin(0));
            CPPUNIT_ASSERT_EQUAL(FileData::token_type(7), *file.line_begin(1));
            CPPUNIT_ASSERT_EQUAL(FileData::token_type(7), *file.line_begin(2));
        }
    }

    void testLineOffset() {
        std::istringstream iss("Fname\n12 42\n\n7\n\n");
        TokenContainer tc(iss);

        for (auto file : tc.file_view()) {
            CPPUNIT_ASSERT_EQUAL(FileData::token_offset_type(0), file.line_offset(0));
            CPPUNIT_ASSERT_EQUAL(FileData::token_offset_type(2), file.line_offset(1));
            CPPUNIT_ASSERT_EQUAL(FileData::token_offset_type(2), file.line_offset(2));
            CPPUNIT_ASSERT_EQUAL(FileData::token_offset_type(3), file.line_offset(3));
        }
    }
};
