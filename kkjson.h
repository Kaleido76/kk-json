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
    class Value;

    struct __char_stack;
    class __parser;
    class __array_iterator;  // random
    class __object_iterator; // bidirect

    using json = Value;

    std::pair<ParseStatus, json> parse(const char *str);

    enum class ValueType
    {
        None,
        Null,
        Bool,
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
        using bool_type = bool;
        using number_type = double;
        using string_type = std::string;
        using array_type = std::vector<Value>;
        using object_type = std::map<string_type, Value>;
        using pair_type = std::pair<string_type, Value>;
        using init_array_type = std::initializer_list<Value>;
        using init_obj_type = std::initializer_list<object_type::value_type>;

        ValueType type;
        union // anonymous
        {
            bool_type bool_val;
            number_type number_val;
            string_type *pstring;
            array_type *parray;
            object_type *pobject;
        };

    public:
        // iterator retated
        friend class __array_iterator;
        friend class __object_iterator;
        using array_iterator = __array_iterator;
        using object_iterator = __object_iterator;

    public:
        Value();
        Value(const Value &another);
        Value(Value &&another) noexcept;
        Value &operator=(const Value &another);
        Value &operator=(Value &&another) noexcept;
        ~Value();

        // read
        ValueType get_type() const;
        size_t get_size() const;
        // type judge
        bool is_none() const;
        bool is_null() const;
        bool is_bool() const;
        bool is_number() const;
        bool is_string() const;
        bool is_array() const;
        bool is_object() const;
        // as type
        bool_type &as_bool();
        number_type &as_number();
        string_type &as_string();
        // read & write
        Value &operator[](const string_type &k);
        Value &operator[](size_t idx);
        Value &operator=(bool_type v);
        Value &operator=(number_type n);
        Value &operator=(const string_type &s);
        Value &operator=(const init_array_type &l);
        Value &operator=(const init_obj_type &l);

        Value(bool_type v);
        Value(number_type n);
        Value(const string_type &s);
        Value(const init_array_type &l);
        Value(const init_obj_type &l);

        array_iterator array_begin();
        array_iterator array_end();
        object_iterator object_begin();
        object_iterator object_end();

    private:
        // modify value
        void set_literal(ValueType t);
        void set_bool(bool_type v);
        void set_number(number_type n);
        void set_string(const string_type &another);
        void set_string(const char *p, size_t n);
        void set_array(const init_array_type &l);
        void set_object(const init_obj_type &l);

        void init_array();
        void array_push_back(const Value &e);
        void array_push_back(Value &&e);

        void init_object();
        void object_insert(const string_type &k, const Value &v);
        void object_insert(const string_type &k, Value &&v);

        void clear();
    };

    class __array_iterator
    {
        friend class Value;
        using inner_iter_type = Value::array_type::iterator; // random iterator
        using self_type = __array_iterator;
        using difference_type = inner_iter_type::difference_type;
        using reference = inner_iter_type::reference;
        using pointer = inner_iter_type::pointer;

        inner_iter_type it;

    public:
        __array_iterator();
        __array_iterator(const self_type &another);
        __array_iterator(self_type &&another);
        __array_iterator(const inner_iter_type &it);
        __array_iterator(inner_iter_type &&it);
        ~__array_iterator();

        reference operator*() const;
        pointer operator->() const;

        difference_type operator-(const self_type &another) const;

        self_type &operator++();
        self_type operator++(int);
        self_type &operator+=(difference_type n);
        self_type operator+(difference_type n) const;
        self_type &operator--();
        self_type operator--(int);
        self_type &operator-=(difference_type n);
        self_type operator-(difference_type n) const;

        reference operator[](difference_type n) const;
        bool operator==(const self_type &another) const;
        bool operator!=(const self_type &another) const;
        bool operator<(const self_type &another) const;
        bool operator>(const self_type &another) const;
        bool operator<=(const self_type &another) const;
        bool operator>=(const self_type &another) const;
    };

    class __object_iterator
    {
        friend class Value;
        using inner_iter_type = Value::object_type::iterator; // bidirect iterator
        using self_type = __object_iterator;
        using reference = inner_iter_type::reference;
        using pointer = inner_iter_type::pointer;

        inner_iter_type it;

    public:
        __object_iterator();
        __object_iterator(const self_type &another);
        __object_iterator(self_type &&another);
        __object_iterator(const inner_iter_type &it);
        __object_iterator(inner_iter_type &&it);
        ~__object_iterator();

        reference operator*() const;
        pointer operator->() const;

        self_type &operator++();
        self_type operator++(int);

        self_type &operator--();
        self_type operator--(int);

        bool operator==(const self_type &another) const;
        bool operator!=(const self_type &another) const;
    };

    struct __char_stack
    {
        __char_stack();
        ~__char_stack();

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

        __char_stack cstack;
        const char *raw_iter;

        __parser(const char *cstr);
        __parser(const __parser &) = delete;
        ~__parser() = default;

        ParseStatus exec(Value &out);
        ParseStatus parse_whitespace();
        ParseStatus parse_value(Value &out);
        ParseStatus parse_literal(Value &out, const char *target, ValueType t);
        ParseStatus parse_bool(Value &out, const char *target, bool v);
        ParseStatus parse_string(Value &out);
        ParseStatus parse_string_raw(size_t &length_out);
        ParseStatus parse_array(Value &out);
        ParseStatus parse_object(Value &out);
        ParseStatus parse_number(Value &out);
    };
}

#endif /* _EZJSON_H__ */