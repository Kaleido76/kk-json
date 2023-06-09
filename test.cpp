#define SHOW_PASSED false
#define SHOW_STR_CONTENT false
#define COLUMN_1_WIDTH 35
#define COLUMN_2_WIDTH 35

#include <iostream>
#include <iomanip>
#include "kkjson.h"
using kkjson::parser, kkjson::parse_status,
    kkjson::value_type, kkjson::value_entry;

using std::cerr, std::cout, std::endl;
using std::ostream;

namespace
{
    unsigned int test_count = 0;
    unsigned int test_pass = 0;
    bool exist_err = false;

    void output_keys()
    {
        cerr << std::left;
        cout << std::left
             << "\033[34m"
             << std::setw(COLUMN_1_WIDTH) << "Expect"
             << std::setw(COLUMN_2_WIDTH) << "Actual"
             << "\033[39m" << endl;
    }

    void output_statistics_data()
    {
        double pass_rate;
        pass_rate = (test_pass == test_count) ? 100. : (test_pass * 100.0 / test_count);
        cout << "----------------" << endl
             << "Total:  " << test_pass << "/" << test_count << "  ("
             << std::fixed << std::setprecision(2) << pass_rate << "%)"
             << endl;
    }
}

std::ostream &operator<<(std::ostream &o, parse_status ps)
{
#define ENUM_OUTPUT_CASE_STATUS(code) \
    case parse_status::code:          \
        o << "STATUS(" #code ")";     \
        break

    switch (ps)
    {
        ENUM_OUTPUT_CASE_STATUS(OK);
        ENUM_OUTPUT_CASE_STATUS(INVALID_VALUE);
        ENUM_OUTPUT_CASE_STATUS(UNEXPECTED_SYMBOL);
        ENUM_OUTPUT_CASE_STATUS(ROOT_NOT_SINGULAR);
        // number
        ENUM_OUTPUT_CASE_STATUS(NUMBER_TOO_LARGE);
        // string
        ENUM_OUTPUT_CASE_STATUS(INVALID_STRING_CHAR);
        ENUM_OUTPUT_CASE_STATUS(INVALID_STRING_ESCAPE);
        ENUM_OUTPUT_CASE_STATUS(MISS_QUOTATION_MARK);
        // unicode
        ENUM_OUTPUT_CASE_STATUS(INVALID_UNICODE_HEX);
        ENUM_OUTPUT_CASE_STATUS(INVALID_UNICODE_SURROGATE);
        // array
        ENUM_OUTPUT_CASE_STATUS(MISS_ARRAY_SYMBOL);
        // object
        ENUM_OUTPUT_CASE_STATUS(MISS_OBJECT_KEY);
        ENUM_OUTPUT_CASE_STATUS(MISS_OBJECT_SYMBOL);
    default:
        o << "STATUS(UNKNOWN)";
        break;
    }
    return o;
}

std::ostream &operator<<(std::ostream &o, value_type vt)
{
#define ENUM_OUTPUT_CASE_VTYPE(s) \
    case value_type::s:           \
        o << "VTYPE(" #s ")";     \
        break

    switch (vt)
    {
        ENUM_OUTPUT_CASE_VTYPE(VT_NONE);
        ENUM_OUTPUT_CASE_VTYPE(VT_NULL);
        ENUM_OUTPUT_CASE_VTYPE(VT_TRUE);
        ENUM_OUTPUT_CASE_VTYPE(VT_FALSE);
        ENUM_OUTPUT_CASE_VTYPE(VT_NUMBER);
        ENUM_OUTPUT_CASE_VTYPE(VT_STRING);
        ENUM_OUTPUT_CASE_VTYPE(VT_ARRAY);
        ENUM_OUTPUT_CASE_VTYPE(VT_OBJECT);
    default:
        o << "VTYPE(UNKNOWN)";
        break;
    }
    return o;
}

#define EXPECT_BASE(equality, expect, actual)                                           \
    do                                                                                  \
    {                                                                                   \
        test_count++;                                                                   \
        if (equality)                                                                   \
        {                                                                               \
            test_pass++;                                                                \
            if (SHOW_PASSED)                                                            \
            {                                                                           \
                cout << std::setw(COLUMN_1_WIDTH) << expect                             \
                     << "\033[32m" << std::setw(COLUMN_2_WIDTH) << actual << "\033[39m" \
                     << " in   " << __FILE__ << ":" << __LINE__ << endl;                \
            }                                                                           \
        }                                                                               \
        else                                                                            \
        {                                                                               \
            cerr << std::setw(COLUMN_1_WIDTH) << expect                                 \
                 << "\033[31m" << std::setw(COLUMN_2_WIDTH) << actual << "\033[39m"     \
                 << " in   " << __FILE__ << ":" << __LINE__ << endl;                    \
            exist_err = true;                                                           \
        }                                                                               \
    } while (0)

#define EXPECT_INT(expect, actual) EXPECT_BASE((expect) == (actual), expect, actual)
#define EXPECT_SIZE_T(expect, actual) EXPECT_BASE((expect) == (actual), expect, actual)
#define EXPECT_DOUBLE(expect, actual) EXPECT_BASE((expect) == (actual), expect, actual)

#if SHOW_STR_CONTENT
#define EXPECT_STRING(expect, actual) EXPECT_BASE(                                           \
    (sizeof(expect) - 1) == actual.length() && actual.compare(0, actual.length(),            \
                                                              expect, actual.length()) == 0, \
    expect, actual)
#else
#define EXPECT_STRING(expect, actual) EXPECT_BASE(                                           \
    (sizeof(expect) - 1) == actual.length() && actual.compare(0, actual.length(),            \
                                                              expect, actual.length()) == 0, \
    "[...]", "[...]")
#endif

#define TEST_LITERAL(_vtype, _str)               \
    do                                           \
    {                                            \
        ps = p.exec_parse(_str);                 \
        EXPECT_INT(parse_status::OK, ps);        \
        EXPECT_INT(_vtype, p.result.get_type()); \
    } while (0)

#define TEST_NUMBER(_value, _str)                               \
    do                                                          \
    {                                                           \
        ps = p.exec_parse(_str);                                \
        EXPECT_INT(parse_status::OK, ps);                       \
        EXPECT_INT(value_type::VT_NUMBER, p.result.get_type()); \
        EXPECT_DOUBLE(_value, p.result.get_number());           \
    } while (0)

#define TEST_STRING(_value, _str)                               \
    do                                                          \
    {                                                           \
        ps = p.exec_parse(_str);                                \
        EXPECT_INT(parse_status::OK, ps);                       \
        EXPECT_INT(value_type::VT_STRING, p.result.get_type()); \
        EXPECT_STRING(_value, p.result.get_string());           \
    } while (0)

#define TEST_ARRAY_STAT(_value_count, _str)                     \
    do                                                          \
    {                                                           \
        ps = p.exec_parse(_str);                                \
        EXPECT_INT(parse_status::OK, ps);                       \
        EXPECT_INT(value_type::VT_ARRAY, p.result.get_type());  \
        EXPECT_SIZE_T(_value_count, p.result.get_array_size()); \
    } while (0)

#define TEST_OBJECT_STAT(_str)                                  \
    do                                                          \
    {                                                           \
        ps = p.exec_parse(_str);                                \
        EXPECT_INT(parse_status::OK, ps);                       \
        EXPECT_INT(value_type::VT_OBJECT, p.result.get_type()); \
    } while (0)

#define TEST_ERROR(_status, _str) \
    do                            \
    {                             \
        ps = p.exec_parse(_str);  \
        EXPECT_INT(_status, ps);  \
    } while (0)

namespace
{
    parser p;
    parse_status ps;

    void test_parse_literal()
    {
        TEST_LITERAL(value_type::VT_NULL, "null");
        TEST_LITERAL(value_type::VT_TRUE, "true");
        TEST_LITERAL(value_type::VT_FALSE, "false");
    }

    void test_parse_number()
    {
        TEST_NUMBER(0.0, "0");
        TEST_NUMBER(0.0, "-0");
        TEST_NUMBER(0.0, "-0.0");
        TEST_NUMBER(1.0, "1");
        TEST_NUMBER(-1.0, "-1");
        TEST_NUMBER(1.5, "1.5");
        TEST_NUMBER(-1.5, "-1.5");
        TEST_NUMBER(3.1416, "3.1416");
        TEST_NUMBER(1E10, "1E10");
        TEST_NUMBER(1e10, "1e10");
        TEST_NUMBER(1E+10, "1E+10");
        TEST_NUMBER(1E-10, "1E-10");
        TEST_NUMBER(-1E10, "-1E10");
        TEST_NUMBER(-1e10, "-1e10");
        TEST_NUMBER(-1E+10, "-1E+10");
        TEST_NUMBER(-1E-10, "-1E-10");
        TEST_NUMBER(1.234E+10, "1.234E+10");
        TEST_NUMBER(1.234E-10, "1.234E-10");
        TEST_NUMBER(0.0, "1e-10000");

        TEST_NUMBER(1.0000000000000002, "1.0000000000000002");
        TEST_NUMBER(4.9406564584124654e-324, "4.9406564584124654e-324");
        TEST_NUMBER(-4.9406564584124654e-324, "-4.9406564584124654e-324");
        TEST_NUMBER(2.2250738585072009e-308, "2.2250738585072009e-308");
        TEST_NUMBER(-2.2250738585072009e-308, "-2.2250738585072009e-308");
        TEST_NUMBER(2.2250738585072014e-308, "2.2250738585072014e-308");
        TEST_NUMBER(-2.2250738585072014e-308, "-2.2250738585072014e-308");
        TEST_NUMBER(1.7976931348623157e+308, "1.7976931348623157e+308");
        TEST_NUMBER(-1.7976931348623157e+308, "-1.7976931348623157e+308");
    }

    void test_parse_string()
    {
        TEST_STRING("", "\"\"");
        TEST_STRING("Hello", "\"Hello\"");
        TEST_STRING("Hello\nWorld", "\"Hello\\nWorld\"");
        TEST_STRING("\" \\ / \b \f \n \r \t", "\"\\\" \\\\ \\/ \\b \\f \\n \\r \\t\"");
        TEST_STRING("Hello\0World", "\"Hello\\u0000World\"");
        TEST_STRING("\xC2\xA2", "\"\\u00A2\"");
        TEST_STRING("\xE2\x82\xAC", "\"\u20AC\"");
        TEST_STRING("\xF0\x9D\x84\x9E", "\"\\uD834\\uDD1E\"");
        TEST_STRING("\xF0\x9D\x84\x9E", "\"\\ud834\\udd1e\"");
    }

    void test_parse_array()
    {
        TEST_ARRAY_STAT(0, "[]");
        TEST_ARRAY_STAT(0, "[\n]");
        TEST_ARRAY_STAT(0, "[   \n\t   ]");

        TEST_ARRAY_STAT(5, "[ null , false , true , 123 , \"abc\" ]");
        EXPECT_INT(value_type::VT_NULL, p.result[0].get_type());
        EXPECT_INT(value_type::VT_FALSE, p.result[1].get_type());
        EXPECT_INT(value_type::VT_TRUE, p.result[2].get_type());
        EXPECT_INT(value_type::VT_NUMBER, p.result[3].get_type());
        EXPECT_DOUBLE(123, p.result[3].get_number());
        EXPECT_INT(value_type::VT_STRING, p.result[4].get_type());
        EXPECT_DOUBLE("abc", p.result[4].get_string());

        TEST_ARRAY_STAT(2, "[ [    [ null, 123.1234,  \"213xx\\n\"  ], []   ], [ ] ]");
        EXPECT_INT(value_type::VT_ARRAY, p.result[0].get_type());
        EXPECT_INT(2, p.result[0].get_array_size());
        EXPECT_INT(value_type::VT_ARRAY, p.result[0][0].get_type());
        EXPECT_INT(3, p.result[0][0].get_array_size());
        EXPECT_INT(value_type::VT_NULL, p.result[0][0][0].get_type());
        EXPECT_INT(value_type::VT_NUMBER, p.result[0][0][1].get_type());
        EXPECT_DOUBLE(123.1234, p.result[0][0][1].get_number());
        EXPECT_INT(value_type::VT_STRING, p.result[0][0][2].get_type());
        EXPECT_STRING("213xx\n", p.result[0][0][2].get_string());
        EXPECT_INT(value_type::VT_ARRAY, p.result[0][1].get_type());
        EXPECT_INT(0, p.result[0][1].get_array_size());
        EXPECT_INT(value_type::VT_ARRAY, p.result[1].get_type());
        EXPECT_INT(0, p.result[1].get_array_size());
    }

    void test_parse_object()
    {
        TEST_OBJECT_STAT("{}");
        TEST_OBJECT_STAT(
            " { "
            "\"n\" : null , "
            "\"f\" : false , "
            "\"t\" : true , "
            "\"i\" : 123 , "
            "\"s\" : \"abc\", "
            "\"a\" : [ 1, 2, 3 ],"
            "\"o\" : { \"1\" : 1, \"2\" : 2, \"323\" : 123.31 }"
            " } ");
        EXPECT_INT(value_type::VT_NULL, p.result.get_value("n").get_type());
        EXPECT_INT(value_type::VT_FALSE, p.result.get_value("f").get_type());
        EXPECT_INT(value_type::VT_TRUE, p.result.get_value("t").get_type());
        EXPECT_INT(value_type::VT_NUMBER, p.result.get_value("i").get_type());
        EXPECT_DOUBLE(123, p.result.get_value("i").get_number());
        EXPECT_INT(value_type::VT_STRING, p.result.get_value("s").get_type());
        EXPECT_STRING("abc", p.result.get_value("s").get_string());
        EXPECT_INT(value_type::VT_ARRAY, p.result.get_value("a").get_type());
        EXPECT_DOUBLE(1, p.result.get_value("a")[0].get_number());
        EXPECT_INT(value_type::VT_NUMBER, p.result.get_value("a")[0].get_type());
        EXPECT_DOUBLE(2, p.result.get_value("a")[1].get_number());
        EXPECT_INT(value_type::VT_NUMBER, p.result.get_value("a")[1].get_type());
        EXPECT_DOUBLE(3, p.result.get_value("a")[2].get_number());
        EXPECT_INT(value_type::VT_NUMBER, p.result.get_value("a")[2].get_type());
        EXPECT_INT(value_type::VT_OBJECT, p.result.get_value("o").get_type());
        EXPECT_INT(value_type::VT_NUMBER, p.result.get_value("o").get_value("1").get_type());
        EXPECT_DOUBLE(1, p.result.get_value("o").get_value("1").get_number());
        EXPECT_INT(value_type::VT_NUMBER, p.result.get_value("o").get_value("2").get_type());
        EXPECT_DOUBLE(2, p.result.get_value("o").get_value("2").get_number());
        EXPECT_INT(value_type::VT_NUMBER, p.result.get_value("o").get_value("323").get_type());
        EXPECT_DOUBLE(123.31, p.result.get_value("o").get_value("323").get_number());
    }

    void test_error_unexpected_symbol()
    {
        TEST_ERROR(parse_status::UNEXPECTED_SYMBOL, "");
        TEST_ERROR(parse_status::UNEXPECTED_SYMBOL, " ");
    }

    void test_error_invalid_value()
    {
        TEST_ERROR(parse_status::INVALID_VALUE, "x");
        TEST_ERROR(parse_status::INVALID_VALUE, "nuls");
        TEST_ERROR(parse_status::INVALID_VALUE, "truA");
        TEST_ERROR(parse_status::INVALID_VALUE, "falSe");

        TEST_ERROR(parse_status::INVALID_VALUE, "+0");
        TEST_ERROR(parse_status::INVALID_VALUE, "+1");
        TEST_ERROR(parse_status::INVALID_VALUE, ".123");
        TEST_ERROR(parse_status::INVALID_VALUE, "1.");
        TEST_ERROR(parse_status::INVALID_VALUE, "INF");
        TEST_ERROR(parse_status::INVALID_VALUE, "inf");
        TEST_ERROR(parse_status::INVALID_VALUE, "NAN");
        TEST_ERROR(parse_status::INVALID_VALUE, "nan");
        TEST_ERROR(parse_status::INVALID_VALUE, "0x0");
        TEST_ERROR(parse_status::INVALID_VALUE, "0x123");
        TEST_ERROR(parse_status::INVALID_VALUE, "0123");
    }

    void test_error_root_not_singular()
    {
        TEST_ERROR(parse_status::ROOT_NOT_SINGULAR, "null xx");
        TEST_ERROR(parse_status::ROOT_NOT_SINGULAR, "true xx");
        TEST_ERROR(parse_status::ROOT_NOT_SINGULAR, "false abc");
        TEST_ERROR(parse_status::ROOT_NOT_SINGULAR, "falsef");

        TEST_ERROR(parse_status::ROOT_NOT_SINGULAR, "1.324 abc");
        TEST_ERROR(parse_status::ROOT_NOT_SINGULAR, "-2.000 abc");

        TEST_ERROR(parse_status::ROOT_NOT_SINGULAR, "\"sa\" abc");
        TEST_ERROR(parse_status::ROOT_NOT_SINGULAR, "\"sa\"xx");
    }

    void test_error_number_too_large()
    {
        TEST_ERROR(parse_status::NUMBER_TOO_LARGE, "1e309");
        TEST_ERROR(parse_status::NUMBER_TOO_LARGE, "1e999");
        TEST_ERROR(parse_status::NUMBER_TOO_LARGE, "-1e309");
        TEST_ERROR(parse_status::NUMBER_TOO_LARGE, "-1e9999");
    }

    void test_error_miss_quotation_mark()
    {
        TEST_ERROR(parse_status::MISS_QUOTATION_MARK, "\"");
        TEST_ERROR(parse_status::MISS_QUOTATION_MARK, "\"dwq");
    }

    void test_error_invalid_string_escape()
    {
        TEST_ERROR(parse_status::INVALID_STRING_ESCAPE, "\"\\v\"");
        TEST_ERROR(parse_status::INVALID_STRING_ESCAPE, "\"\\'\"");
        TEST_ERROR(parse_status::INVALID_STRING_ESCAPE, "\"\\0\"");
        TEST_ERROR(parse_status::INVALID_STRING_ESCAPE, "\"\\x12\"");
    }

    void test_error_invalid_string_char()
    {
        TEST_ERROR(parse_status::INVALID_STRING_CHAR, "\"\x01\"");
        TEST_ERROR(parse_status::INVALID_STRING_CHAR, "\"\x1F\"");
    }

    void test_error_invalid_unicode_hex()
    {
        TEST_ERROR(parse_status::INVALID_UNICODE_HEX, "\"\\u\"");
        TEST_ERROR(parse_status::INVALID_UNICODE_HEX, "\"\\u0\"");
        TEST_ERROR(parse_status::INVALID_UNICODE_HEX, "\"\\u01\"");
        TEST_ERROR(parse_status::INVALID_UNICODE_HEX, "\"\\u012\"");
        TEST_ERROR(parse_status::INVALID_UNICODE_HEX, "\"\\u/000\"");
        TEST_ERROR(parse_status::INVALID_UNICODE_HEX, "\"\\uG000\"");
        TEST_ERROR(parse_status::INVALID_UNICODE_HEX, "\"\\u0/00\"");
        TEST_ERROR(parse_status::INVALID_UNICODE_HEX, "\"\\u0G00\"");
        TEST_ERROR(parse_status::INVALID_UNICODE_HEX, "\"\\u0/00\"");
        TEST_ERROR(parse_status::INVALID_UNICODE_HEX, "\"\\u00G0\"");
        TEST_ERROR(parse_status::INVALID_UNICODE_HEX, "\"\\u000/\"");
        TEST_ERROR(parse_status::INVALID_UNICODE_HEX, "\"\\u000G\"");
        TEST_ERROR(parse_status::INVALID_UNICODE_HEX, "\"\\u 123\"");
    }

    void test_error_invalid_unicode_surrogate()
    {
        TEST_ERROR(parse_status::INVALID_UNICODE_SURROGATE, "\"\\uD800\"");
        TEST_ERROR(parse_status::INVALID_UNICODE_SURROGATE, "\"\\uDBFF\"");
        TEST_ERROR(parse_status::INVALID_UNICODE_SURROGATE, "\"\\uD800\\\\\"");
        TEST_ERROR(parse_status::INVALID_UNICODE_SURROGATE, "\"\\uD800\\uDBFF\"");
        TEST_ERROR(parse_status::INVALID_UNICODE_SURROGATE, "\"\\uD800\\uE000\"");
    }

    void test_error_miss_array_symbol()
    {
        TEST_ERROR(parse_status::MISS_ARRAY_SYMBOL, "[1");
        TEST_ERROR(parse_status::MISS_ARRAY_SYMBOL, "[1}");
        TEST_ERROR(parse_status::MISS_ARRAY_SYMBOL, "[1, 2");
        TEST_ERROR(parse_status::MISS_ARRAY_SYMBOL, "[[]");
        TEST_ERROR(parse_status::MISS_ARRAY_SYMBOL, "[1 3");
    }

    void test_error_miss_object_key()
    {
        TEST_ERROR(parse_status::MISS_OBJECT_KEY, "{:1,");
        TEST_ERROR(parse_status::MISS_OBJECT_KEY, "{1:1,");
        TEST_ERROR(parse_status::MISS_OBJECT_KEY, "{true:1,");
        TEST_ERROR(parse_status::MISS_OBJECT_KEY, "{false:1,");
        TEST_ERROR(parse_status::MISS_OBJECT_KEY, "{null:1,");
        TEST_ERROR(parse_status::MISS_OBJECT_KEY, "{[]:1,");
        TEST_ERROR(parse_status::MISS_OBJECT_KEY, "{{}:1,");
        TEST_ERROR(parse_status::MISS_OBJECT_KEY, "{\"a\":1,");
    }

    void test_error_miss_object_symbol()
    {
        TEST_ERROR(parse_status::MISS_OBJECT_SYMBOL, "{\"a\"}");
        TEST_ERROR(parse_status::MISS_OBJECT_SYMBOL, "{\"a\",\"b\"}");
        TEST_ERROR(parse_status::MISS_OBJECT_SYMBOL, "{\"a\":1");
        TEST_ERROR(parse_status::MISS_OBJECT_SYMBOL, "{\"a\":1]");
        TEST_ERROR(parse_status::MISS_OBJECT_SYMBOL, "{\"a\":1 \"b\"");
        TEST_ERROR(parse_status::MISS_OBJECT_SYMBOL, "{\"a\":{}");
    }
}

int main(int argc, char const *argv[])
{
    output_keys();
    test_parse_literal();
    test_parse_number();
    test_parse_string();
    test_parse_array();
    test_parse_object();

    test_error_unexpected_symbol();
    test_error_invalid_value();
    test_error_root_not_singular();
    // number
    test_error_number_too_large();
    // string
    test_error_miss_quotation_mark();
    test_error_invalid_string_escape();
    test_error_invalid_string_char();
    // unicode
    test_error_invalid_unicode_hex();
    test_error_invalid_unicode_surrogate();
    // array
    test_error_miss_array_symbol();
    // object
    test_error_miss_object_key();
    test_error_miss_object_symbol();
    output_statistics_data();
    return exist_err ? 1 : 0;
}