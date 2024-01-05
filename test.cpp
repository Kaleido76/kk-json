#define SHOW_PASSED false
#define SHOW_STR_CONTENT false
#define COLUMN_1_WIDTH 35
#define COLUMN_2_WIDTH 35

#include <iostream>
#include <iomanip>
#include "kkjson.h"
using kkjson::parse, kkjson::ParseStatus,
    kkjson::ValueType, kkjson::json;

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

std::ostream &operator<<(std::ostream &o, ParseStatus ps)
{
#define ENUM_OUTPUT_CASE_STATUS(code) \
    case ParseStatus::code:           \
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

std::ostream &operator<<(std::ostream &o, ValueType vt)
{
#define ENUM_OUTPUT_CASE_VTYPE(s) \
    case ValueType::s:            \
        o << "VTYPE(" #s ")";     \
        break

    switch (vt)
    {
        ENUM_OUTPUT_CASE_VTYPE(None);
        ENUM_OUTPUT_CASE_VTYPE(Null);
        ENUM_OUTPUT_CASE_VTYPE(Bool);
        ENUM_OUTPUT_CASE_VTYPE(Number);
        ENUM_OUTPUT_CASE_VTYPE(String);
        ENUM_OUTPUT_CASE_VTYPE(Array);
        ENUM_OUTPUT_CASE_VTYPE(Object);
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
#define EXPECT_BOOL(expect, actual) EXPECT_BASE((expect) == (actual), (expect), (actual))
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

#define TEST_LITERAL(_vtype, _str)         \
    do                                     \
    {                                      \
        auto [_ps, _j] = parse(_str);      \
        EXPECT_INT(ParseStatus::OK, _ps);  \
        EXPECT_INT(_vtype, _j.get_type()); \
    } while (0)

#define TEST_BOOL(_value, _str)                     \
    do                                              \
    {                                               \
        auto [_ps, _j] = parse(_str);               \
        EXPECT_INT(ParseStatus::OK, _ps);           \
        EXPECT_INT(ValueType::Bool, _j.get_type()); \
        EXPECT_BOOL(_value, _j.as_bool());          \
    } while (0)

#define TEST_NUMBER(_value, _str)                     \
    do                                                \
    {                                                 \
        auto [_ps, _j] = parse(_str);                 \
        EXPECT_INT(ParseStatus::OK, _ps);             \
        EXPECT_INT(ValueType::Number, _j.get_type()); \
        EXPECT_DOUBLE(_value, _j.as_number());        \
    } while (0)

#define TEST_STRING(_value, _str)                     \
    do                                                \
    {                                                 \
        auto [_ps, _j] = parse(_str);                 \
        EXPECT_INT(ParseStatus::OK, _ps);             \
        EXPECT_INT(ValueType::String, _j.get_type()); \
        EXPECT_STRING(_value, _j.as_string());        \
    } while (0)

#define TEST_ARRAY_STAT(_value_count, _str)          \
    do                                               \
    {                                                \
        auto [_ps, _j] = parse(_str);                \
        EXPECT_INT(ParseStatus::OK, _ps);            \
        EXPECT_INT(ValueType::Array, _j.get_type()); \
        EXPECT_SIZE_T(_value_count, _j.get_size());  \
        tmp = std::move(_j);                         \
    } while (0)

#define TEST_OBJECT_STAT(_str)                        \
    do                                                \
    {                                                 \
        auto [_ps, _j] = parse(_str);                 \
        EXPECT_INT(ParseStatus::OK, _ps);             \
        EXPECT_INT(ValueType::Object, _j.get_type()); \
        tmp = std::move(_j);                          \
    } while (0)

#define TEST_ERROR(_status, _str)     \
    do                                \
    {                                 \
        auto [_ps, _j] = parse(_str); \
        EXPECT_INT(_status, _ps);     \
    } while (0)

namespace
{
    json tmp;

    void test_parse_literal()
    {
        TEST_LITERAL(ValueType::Null, "null");
        TEST_BOOL(true, "true");
        TEST_BOOL(false, "false");
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
        EXPECT_INT(ValueType::Null, tmp[0].get_type());
        EXPECT_INT(ValueType::Bool, tmp[1].get_type());
        EXPECT_INT(ValueType::Bool, tmp[2].get_type());
        EXPECT_INT(ValueType::Number, tmp[3].get_type());
        EXPECT_DOUBLE(123, tmp[3].as_number());
        EXPECT_INT(ValueType::String, tmp[4].get_type());
        EXPECT_DOUBLE("abc", tmp[4].as_string());

        TEST_ARRAY_STAT(2, "[ [    [ null, 123.1234,  \"213xx\\n\"  ], []   ], [ ] ]");
        EXPECT_INT(ValueType::Array, tmp[0].get_type());
        EXPECT_INT(2, tmp[0].get_size());
        EXPECT_INT(ValueType::Array, tmp[0][0].get_type());
        EXPECT_INT(3, tmp[0][0].get_size());
        EXPECT_INT(ValueType::Null, tmp[0][0][0].get_type());
        EXPECT_INT(ValueType::Number, tmp[0][0][1].get_type());
        EXPECT_DOUBLE(123.1234, tmp[0][0][1].as_number());
        EXPECT_INT(ValueType::String, tmp[0][0][2].get_type());
        EXPECT_STRING("213xx\n", tmp[0][0][2].as_string());
        EXPECT_INT(ValueType::Array, tmp[0][1].get_type());
        EXPECT_INT(0, tmp[0][1].get_size());
        EXPECT_INT(ValueType::Array, tmp[1].get_type());
        EXPECT_INT(0, tmp[1].get_size());
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
        EXPECT_INT(ValueType::Null, tmp["n"].get_type());
        EXPECT_INT(ValueType::Bool, tmp["f"].get_type());
        EXPECT_INT(ValueType::Bool, tmp["t"].get_type());
        EXPECT_INT(ValueType::Number, tmp["i"].get_type());
        EXPECT_DOUBLE(123, tmp["i"].as_number());
        EXPECT_INT(ValueType::String, tmp["s"].get_type());
        EXPECT_STRING("abc", tmp["s"].as_string());
        EXPECT_INT(ValueType::Array, tmp["a"].get_type());
        EXPECT_DOUBLE(1, tmp["a"][0].as_number());
        EXPECT_INT(ValueType::Number, tmp["a"][0].get_type());
        EXPECT_DOUBLE(2, tmp["a"][1].as_number());
        EXPECT_INT(ValueType::Number, tmp["a"][1].get_type());
        EXPECT_DOUBLE(3, tmp["a"][2].as_number());
        EXPECT_INT(ValueType::Number, tmp["a"][2].get_type());
        EXPECT_INT(ValueType::Object, tmp["o"].get_type());
        EXPECT_INT(ValueType::Number, tmp["o"]["1"].get_type());
        EXPECT_DOUBLE(1, tmp["o"]["1"].as_number());
        EXPECT_INT(ValueType::Number, tmp["o"]["2"].get_type());
        EXPECT_DOUBLE(2, tmp["o"]["2"].as_number());
        EXPECT_INT(ValueType::Number, tmp["o"]["323"].get_type());
        EXPECT_DOUBLE(123.31, tmp["o"]["323"].as_number());
    }

    void test_error_unexpected_symbol()
    {
        TEST_ERROR(ParseStatus::UNEXPECTED_SYMBOL, "");
        TEST_ERROR(ParseStatus::UNEXPECTED_SYMBOL, " ");
    }

    void test_error_invalid_value()
    {
        TEST_ERROR(ParseStatus::INVALID_VALUE, "x");
        TEST_ERROR(ParseStatus::INVALID_VALUE, "nuls");
        TEST_ERROR(ParseStatus::INVALID_VALUE, "truA");
        TEST_ERROR(ParseStatus::INVALID_VALUE, "falSe");

        TEST_ERROR(ParseStatus::INVALID_VALUE, "+0");
        TEST_ERROR(ParseStatus::INVALID_VALUE, "+1");
        TEST_ERROR(ParseStatus::INVALID_VALUE, ".123");
        TEST_ERROR(ParseStatus::INVALID_VALUE, "1.");
        TEST_ERROR(ParseStatus::INVALID_VALUE, "INF");
        TEST_ERROR(ParseStatus::INVALID_VALUE, "inf");
        TEST_ERROR(ParseStatus::INVALID_VALUE, "NAN");
        TEST_ERROR(ParseStatus::INVALID_VALUE, "nan");
        TEST_ERROR(ParseStatus::INVALID_VALUE, "0x0");
        TEST_ERROR(ParseStatus::INVALID_VALUE, "0x123");
        TEST_ERROR(ParseStatus::INVALID_VALUE, "0123");
    }

    void test_error_root_not_singular()
    {
        TEST_ERROR(ParseStatus::ROOT_NOT_SINGULAR, "null xx");
        TEST_ERROR(ParseStatus::ROOT_NOT_SINGULAR, "true xx");
        TEST_ERROR(ParseStatus::ROOT_NOT_SINGULAR, "false abc");
        TEST_ERROR(ParseStatus::ROOT_NOT_SINGULAR, "falsef");

        TEST_ERROR(ParseStatus::ROOT_NOT_SINGULAR, "1.324 abc");
        TEST_ERROR(ParseStatus::ROOT_NOT_SINGULAR, "-2.000 abc");

        TEST_ERROR(ParseStatus::ROOT_NOT_SINGULAR, "\"sa\" abc");
        TEST_ERROR(ParseStatus::ROOT_NOT_SINGULAR, "\"sa\"xx");
    }

    void test_error_number_too_large()
    {
        TEST_ERROR(ParseStatus::NUMBER_TOO_LARGE, "1e309");
        TEST_ERROR(ParseStatus::NUMBER_TOO_LARGE, "1e999");
        TEST_ERROR(ParseStatus::NUMBER_TOO_LARGE, "-1e309");
        TEST_ERROR(ParseStatus::NUMBER_TOO_LARGE, "-1e9999");
    }

    void test_error_miss_quotation_mark()
    {
        TEST_ERROR(ParseStatus::MISS_QUOTATION_MARK, "\"");
        TEST_ERROR(ParseStatus::MISS_QUOTATION_MARK, "\"dwq");
    }

    void test_error_invalid_string_escape()
    {
        TEST_ERROR(ParseStatus::INVALID_STRING_ESCAPE, "\"\\v\"");
        TEST_ERROR(ParseStatus::INVALID_STRING_ESCAPE, "\"\\'\"");
        TEST_ERROR(ParseStatus::INVALID_STRING_ESCAPE, "\"\\0\"");
        TEST_ERROR(ParseStatus::INVALID_STRING_ESCAPE, "\"\\x12\"");
    }

    void test_error_invalid_string_char()
    {
        TEST_ERROR(ParseStatus::INVALID_STRING_CHAR, "\"\x01\"");
        TEST_ERROR(ParseStatus::INVALID_STRING_CHAR, "\"\x1F\"");
    }

    void test_error_invalid_unicode_hex()
    {
        TEST_ERROR(ParseStatus::INVALID_UNICODE_HEX, "\"\\u\"");
        TEST_ERROR(ParseStatus::INVALID_UNICODE_HEX, "\"\\u0\"");
        TEST_ERROR(ParseStatus::INVALID_UNICODE_HEX, "\"\\u01\"");
        TEST_ERROR(ParseStatus::INVALID_UNICODE_HEX, "\"\\u012\"");
        TEST_ERROR(ParseStatus::INVALID_UNICODE_HEX, "\"\\u/000\"");
        TEST_ERROR(ParseStatus::INVALID_UNICODE_HEX, "\"\\uG000\"");
        TEST_ERROR(ParseStatus::INVALID_UNICODE_HEX, "\"\\u0/00\"");
        TEST_ERROR(ParseStatus::INVALID_UNICODE_HEX, "\"\\u0G00\"");
        TEST_ERROR(ParseStatus::INVALID_UNICODE_HEX, "\"\\u0/00\"");
        TEST_ERROR(ParseStatus::INVALID_UNICODE_HEX, "\"\\u00G0\"");
        TEST_ERROR(ParseStatus::INVALID_UNICODE_HEX, "\"\\u000/\"");
        TEST_ERROR(ParseStatus::INVALID_UNICODE_HEX, "\"\\u000G\"");
        TEST_ERROR(ParseStatus::INVALID_UNICODE_HEX, "\"\\u 123\"");
    }

    void test_error_invalid_unicode_surrogate()
    {
        TEST_ERROR(ParseStatus::INVALID_UNICODE_SURROGATE, "\"\\uD800\"");
        TEST_ERROR(ParseStatus::INVALID_UNICODE_SURROGATE, "\"\\uDBFF\"");
        TEST_ERROR(ParseStatus::INVALID_UNICODE_SURROGATE, "\"\\uD800\\\\\"");
        TEST_ERROR(ParseStatus::INVALID_UNICODE_SURROGATE, "\"\\uD800\\uDBFF\"");
        TEST_ERROR(ParseStatus::INVALID_UNICODE_SURROGATE, "\"\\uD800\\uE000\"");
    }

    void test_error_miss_array_symbol()
    {
        TEST_ERROR(ParseStatus::MISS_ARRAY_SYMBOL, "[1");
        TEST_ERROR(ParseStatus::MISS_ARRAY_SYMBOL, "[1}");
        TEST_ERROR(ParseStatus::MISS_ARRAY_SYMBOL, "[1, 2");
        TEST_ERROR(ParseStatus::MISS_ARRAY_SYMBOL, "[[]");
        TEST_ERROR(ParseStatus::MISS_ARRAY_SYMBOL, "[1 3");
    }

    void test_error_miss_object_key()
    {
        TEST_ERROR(ParseStatus::MISS_OBJECT_KEY, "{:1,");
        TEST_ERROR(ParseStatus::MISS_OBJECT_KEY, "{1:1,");
        TEST_ERROR(ParseStatus::MISS_OBJECT_KEY, "{true:1,");
        TEST_ERROR(ParseStatus::MISS_OBJECT_KEY, "{false:1,");
        TEST_ERROR(ParseStatus::MISS_OBJECT_KEY, "{null:1,");
        TEST_ERROR(ParseStatus::MISS_OBJECT_KEY, "{[]:1,");
        TEST_ERROR(ParseStatus::MISS_OBJECT_KEY, "{{}:1,");
        TEST_ERROR(ParseStatus::MISS_OBJECT_KEY, "{\"a\":1,");
    }

    void test_error_miss_object_symbol()
    {
        TEST_ERROR(ParseStatus::MISS_OBJECT_SYMBOL, "{\"a\"}");
        TEST_ERROR(ParseStatus::MISS_OBJECT_SYMBOL, "{\"a\",\"b\"}");
        TEST_ERROR(ParseStatus::MISS_OBJECT_SYMBOL, "{\"a\":1");
        TEST_ERROR(ParseStatus::MISS_OBJECT_SYMBOL, "{\"a\":1]");
        TEST_ERROR(ParseStatus::MISS_OBJECT_SYMBOL, "{\"a\":1 \"b\"");
        TEST_ERROR(ParseStatus::MISS_OBJECT_SYMBOL, "{\"a\":{}");
    }

    void test_array_iterator()
    {
        auto [st, js] = parse("[1, 2, 3, 4, 5]");
        json::array_iterator ait = js.array_begin();
        EXPECT_INT(ValueType::Number, (*ait).get_type());
        EXPECT_INT(ValueType::Number, ait->get_type());
        EXPECT_DOUBLE(1, ait->as_number());
        EXPECT_DOUBLE(1, (ait++)->as_number());
        EXPECT_DOUBLE(2, ait->as_number());
        EXPECT_DOUBLE(3, (++ait)->as_number());
        EXPECT_DOUBLE(2, (--ait)->as_number());
        EXPECT_DOUBLE(2, (ait--)->as_number());
        ait += 1;
        EXPECT_DOUBLE(2, ait->as_number());
        ait -= 1;
        EXPECT_DOUBLE(1, ait->as_number());
        EXPECT_INT(5, (js.array_end()) - ait);
        EXPECT_INT(3, (js.array_end() - 1) - (ait + 1));
        EXPECT_DOUBLE(2, ait[1].as_number());
        EXPECT_BOOL(true, js.array_begin() == ait);
        EXPECT_BOOL(false, js.array_end() == ait);
        EXPECT_BOOL(true, js.array_begin() < js.array_end());
        EXPECT_BOOL(false, js.array_begin() > js.array_end());
        EXPECT_BOOL(false, js.array_begin() >= js.array_end());
        EXPECT_BOOL(true, js.array_begin() <= js.array_begin());
    }

    void test_object_iterator()
    {
        auto [st, js] = parse("{\"a\":1, \"b\": 2 }");
        json::object_iterator oit = js.object_begin();
        EXPECT_INT(ValueType::Number, (*oit).second.get_type());
        EXPECT_INT(ValueType::Number, oit->second.get_type());
        EXPECT_DOUBLE(1, oit->second.as_number());
        EXPECT_STRING("a", oit->first);
        EXPECT_DOUBLE(1, (oit++)->second.as_number());
        EXPECT_STRING("b", oit->first);
        EXPECT_DOUBLE(2, oit->second.as_number());
        EXPECT_DOUBLE(2, (oit--)->second.as_number());
        EXPECT_DOUBLE(2, (++oit)->second.as_number());
        EXPECT_DOUBLE(1, (--oit)->second.as_number());
    }
}

int main()
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

    // iterator
    test_array_iterator();
    test_object_iterator();
    output_statistics_data();
    return exist_err ? 1 : 0;
}