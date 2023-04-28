#ifndef _EZJSON_H__
#define _EZJSON_H__

#include <stack>
#include <vector>
#include <map>

namespace kkjson
{
    enum class value_type
    {
        VT_NONE,
        VT_NULL,
        VT_TRUE,
        VT_FALSE,
        VT_NUMBER,
        VT_STRING,
        VT_ARRAY,
        VT_OBJECT
    };

    enum class parse_status
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
    struct value_entry;
    struct char_stack;
    struct parser;

    struct value_entry
    {
        friend parser;
        value_entry();
        value_entry(const value_entry &ori_ve);
        value_entry &operator=(const value_entry &ori_ve);
        value_entry(value_entry &&ori_ve) noexcept;
        value_entry &operator=(value_entry &&ve) noexcept;
        ~value_entry();

        const value_type &get_type() const;
        const double &get_number() const;
        const std::string &get_string() const;
        size_t get_array_size() const;
        const value_entry &get_array_element(size_t idx) const;
        const value_entry &operator[](size_t idx) const;
        const value_entry &get_value(const std::string &key) const;

    private:
        void set_literal(value_type vt);
        void set_number(double value);
        void set_string(const char *ori, size_t len);
        void init_array();
        void add_array_element(value_entry *const &pve);
        void init_object();
        void add_key_value(const std::string &key, value_entry *const &value);

        value_type type;
        union
        {
            double number;
            std::string *pstr;
            std::vector<value_entry *> *array;
            std::map<std::string, value_entry *> *object;
        };
        void clean();
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

    struct parser
    {
        value_entry result;

        parser();
        ~parser() = default;
        parse_status exec_parse(const char *c_ptr);

    private:
        char_stack cstack;
        const char *raw_iter{nullptr};
        parse_status parse_whitespace();
        parse_status parse_literal(value_entry &ve, const char *target, value_type vt);
        parse_status parse_number(value_entry &ve);
        parse_status parse_value(value_entry &ve);
        parse_status parse_string(value_entry &ve);
        parse_status parse_string_raw(size_t &length);
        parse_status parse_array(value_entry &ve);
        parse_status parse_object(value_entry &ve);
    };
}

#endif /* _EZJSON_H__ */