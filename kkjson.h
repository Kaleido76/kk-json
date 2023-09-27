#ifndef _EZJSON_H__
#define _EZJSON_H__

#include <stack>
#include <string>
#include <vector>
#include <map>

namespace kkjson
{
    enum class ValueType;
    enum class ParseStatus;
    struct char_stack;
    // new
    class Value;
    class __parser;
    using json = Value;

    std::pair<ParseStatus, json> parse(const char *str);

    enum class ValueType
    {
        None,
        Null,
        True,
        False,
        Number,
        String,
        Array,
        Object
    };

    enum class ParseStatus
    {
        OK = 0,
        UNEXPECTED_SYMBOL,
        INVALID_VALUE,
        ROOT_NOT_SINGULAR,
        // number
        NUMBER_TOO_LARGE,
        // string
        INVALID_STRING_CHAR,
        INVALID_STRING_ESCAPE,
        MISS_QUOTATION_MARK,
        // unicode
        INVALID_UNICODE_HEX,
        INVALID_UNICODE_SURROGATE,
        // array
        MISS_ARRAY_SYMBOL,
        // object
        MISS_OBJECT_KEY,
        MISS_OBJECT_SYMBOL
    };

    class Value
    {
    private:
        friend class __parser;
        using number_ctn = double;
        using string_ctn = std::string;
        using array_ctn = std::vector<Value>;
        using object_ctn = std::map<string_ctn, Value>;
        using pair_ctn = std::pair<string_ctn, Value>;

        ValueType type;
        union // anonymous
        {
            number_ctn number;
            string_ctn *pstring;
            array_ctn *parray;
            object_ctn *pobject;
        };

    public:
        Value();
        Value(const Value &another);
        Value(Value &&another) noexcept;
        ~Value();
        Value &operator=(const Value &another);
        Value &operator=(Value &&another) noexcept;

        // read
        ValueType get_type() const;
        size_t get_size() const;
        bool is_none() const;
        bool is_null() const;
        bool is_bool() const;
        bool is_number() const;
        bool is_string() const;
        bool is_array() const;
        bool is_object() const;
        bool as_bool() const;
        const number_ctn &as_number() const;
        const string_ctn &as_string() const;
        // read & write
        Value &operator[](const string_ctn &k);
        Value &operator[](size_t idx);

    private:
        // modify value
        void set_literal(ValueType t);
        void set_number(number_ctn n);
        void set_string(const string_ctn &another);
        void set_string(const char *p, size_t n);
        void init_array();
        void array_push_back(const Value &e);
        void array_push_back(Value &&e);
        void init_object();
        void object_insert(const string_ctn &k, const Value &v);
        void object_insert(const string_ctn &k, Value &&v);
        void clear();
    };

    struct char_stack
    {
        char_stack();
        ~char_stack();

        void *push(size_t size);
        void *pop(size_t size);
        size_t get_top();
        void set_top(size_t n);

    private:
        size_t capability, top;
        char *ptr;
    };

    class __parser
    {
        friend std::pair<ParseStatus, json> parse(const char *str);

        char_stack cstack;
        const char *raw_iter;

        __parser(const char *cstr);
        __parser(const __parser&) = delete;
        ~__parser() = default;

        ParseStatus exec(Value &out);
        ParseStatus parse_whitespace();
        ParseStatus parse_value(Value &out);
        ParseStatus parse_literal(Value &out, const char *target, ValueType t);
        ParseStatus parse_string(Value &out);
        ParseStatus parse_string_raw(size_t &length_out);
        ParseStatus parse_array(Value &out);
        ParseStatus parse_object(Value &out);
        ParseStatus parse_number(Value &out);
    };
}

#endif /* _EZJSON_H__ */