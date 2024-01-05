#include <cstdlib>
#include <cerrno>
#include <cmath>
#include "kkjson.h"

#pragma region tools

// char stack
#define CHAR_STACK_INIT_CAP 256
#define EXTEND_SIZE(x) (x += x >> 1)
#define PUSH_CHAR(stk, c) (*(char *)stk.push(1) = c)

// char handle
#define IS_WHITESPACE(x) ((x) == ' ' || (x) == '\t' || (x) == '\n' || (x) == '\r')
#define IS_ZEROEND(x) ((x) == '\0')
#define IS_DIGIT09(x) ((x) >= '0' && (x) <= '9')
#define IS_DIGIT19(x) ((x) >= '1' && (x) <= '9')
#define IS_SURROGATE_H(x) ((x) >= 0xD800 && (x) <= 0xDBFF)
#define IS_SURROGATE_L(x) ((x) < 0xDC00 || (x) > 0xDFFF)
#define CALC_CODEPOINT(uh, ul) ((((uh - 0xD800) << 10) | (ul - 0xDC00)) + 0x10000)

static bool hex4_to_ui(const char *iter, unsigned &ux)
{
    ux = 0;
    char c;
    for (int i = 0; i < 4; i++)
    {
        c = *iter++;
        ux <<= 4;
        if (c >= '0' && c <= '9')
            ux |= c - '0';
        else if (c >= 'A' && c <= 'F')
            ux |= c - ('A' - 10);
        else if (c >= 'a' && c <= 'f')
            ux |= c - ('a' - 10);
        else
            return false;
    }
    return true;
}

#pragma endregion

namespace kkjson
{
    using std::move, std::forward;

#pragma region value related

    Value::Value() : type(ValueType::None) {}

    Value::Value(const Value &another) : type(ValueType::None) { operator=(another); }

    Value::Value(Value &&another) noexcept : type(ValueType::None) { operator=(forward<Value>(another)); }

    Value::~Value() { clear(); }

    Value &Value::operator=(const Value &another)
    {
        clear();
        switch (another.type)
        {
        case ValueType::Object:
            pobject = new object_type(*(another.pobject));
            break;
        case ValueType::String:
            pstring = new string_type(*(another.pstring));
            break;
        case ValueType::Array:
            parray = new array_type(*(another.parray));
            break;
        case ValueType::Number:
            number_val = another.number_val;
            break;
        case ValueType::Bool:
            bool_val = another.bool_val;
        case ValueType::None:
        case ValueType::Null:
        default:
            break;
        }
        type = another.type;
        return *this;
    }

    Value &Value::operator=(Value &&another) noexcept
    {
        clear();
        switch (another.type)
        {
        case ValueType::Object:
            pobject = another.pobject;
            another.pobject = nullptr;
            break;
        case ValueType::Array:
            parray = another.parray;
            another.parray = nullptr;
            break;
        case ValueType::String:
            pstring = another.pstring;
            another.pstring = nullptr;
            break;
        case ValueType::Number:
            number_val = another.number_val;
            break;
        case ValueType::Bool:
            bool_val = another.bool_val;
        case ValueType::None:
        case ValueType::Null:
        default:
            break;
        }
        type = another.type;
        return *this;
    }

    ValueType Value::get_type() const { return type; }

    size_t Value::get_size() const
    {
        switch (type)
        {
        case ValueType::Object:
            return pobject->size();
        case ValueType::String:
            return pstring->size();
        case ValueType::Array:
            return parray->size();
        case ValueType::Number:
        case ValueType::Bool:
        case ValueType::None:
        case ValueType::Null:
        default:
            return size_t(0);
        }
    }

    bool Value::is_none() const { return (type == ValueType::Object); }

    bool Value::is_null() const { return (type == ValueType::Null); }

    bool Value::is_bool() const { return (type == ValueType::Bool); }

    bool Value::is_number() const { return (type == ValueType::Number); }

    bool Value::is_string() const { return (type == ValueType::String); }

    bool Value::is_array() const { return (type == ValueType::Array); }

    bool Value::is_object() const { return (type == ValueType::Object); }

    Value::bool_type &Value::as_bool() { return bool_val; }

    Value::number_type &Value::as_number() { return number_val; }

    Value::string_type &Value::as_string() { return *pstring; }

    Value &Value::operator[](const string_type &k)
    {
        auto iter = pobject->find(k);
        if (iter == pobject->end())
        {
            iter = pobject->insert(pair_type(k, Value())).first;
        }
        return iter->second;
    }

    Value &Value::operator[](size_t idx)
    {
        return parray->operator[](idx);
    }

    Value &Value::operator=(bool_type v)
    {
        set_bool(v);
        return *this;
    }

    Value &Value::operator=(number_type n)
    {
        set_number(n);
        return *this;
    }

    Value &Value::operator=(const string_type &s)
    {
        set_string(s);
        return *this;
    }

    Value &Value::operator=(const init_array_type &l)
    {
        set_array(l);
        return *this;
    }

    Value &Value::operator=(const init_obj_type &l)
    {
        set_object(l);
        return *this;
    }

    Value::Value(bool_type v) { set_bool(v); }

    Value::Value(number_type n) { set_number(n); }

    Value::Value(const string_type &s) { set_string(s); }

    Value::Value(const init_array_type &l) { set_array(l); }

    Value::Value(const init_obj_type &l) { set_object(l); }

    Value::array_iterator Value::array_begin() { return array_iterator(parray->begin()); }
    Value::array_iterator Value::array_end() { return array_iterator(parray->end()); }
    Value::object_iterator Value::object_begin() { return object_iterator(pobject->begin()); }
    Value::object_iterator Value::object_end() { return object_iterator(pobject->end()); }

    void Value::set_literal(ValueType t)
    {
        clear();
        type = t;
    }

    void Value::set_bool(bool_type v)
    {
        clear();
        type = ValueType::Bool;
        bool_val = v;
    }

    void Value::set_number(number_type n)
    {
        clear();
        type = ValueType::Number;
        number_val = n;
    }

    void Value::set_string(const string_type &another)
    {
        clear();
        type = ValueType::String;
        pstring = new string_type(another);
    }

    void Value::set_string(const char *p, size_t n)
    {
        clear();
        type = ValueType::String;
        pstring = new string_type(p, n);
    }

    void Value::set_array(const init_array_type &list)
    {
        clear();
        type = ValueType::Array;
        parray = new array_type(list);
    }

    void Value::set_object(const init_obj_type &list)
    {
        clear();
        type = ValueType::Object;
        pobject = new object_type(list);
    }

    void Value::init_array()
    {
        clear();
        type = ValueType::Array;
        parray = new array_type;
    }

    void Value::array_push_back(const Value &e)
    {
        if (type == ValueType::Array && parray != nullptr)
        {
            parray->push_back(e);
        }
    }

    void Value::array_push_back(Value &&e)
    {
        if (type == ValueType::Array && parray != nullptr)
        {
            parray->push_back(forward<Value>(e));
        }
    }

    void Value::init_object()
    {
        clear();
        type = ValueType::Object;
        pobject = new object_type;
    }

    void Value::object_insert(const string_type &k, const Value &v)
    {
        if (type == ValueType::Object && pobject != nullptr)
        {
            pobject->insert(pair_type(k, v));
        }
    }

    void Value::object_insert(const string_type &k, Value &&v)
    {
        if (type == ValueType::Object && pobject != nullptr)
        {
            pobject->insert(pair_type(k, forward<Value>(v)));
        }
    }

    void Value::clear()
    {
        switch (type)
        {
        case ValueType::Object:
            if (pobject != nullptr)
            {
                delete pobject;
                pobject = nullptr;
            }
            break;
        case ValueType::Array:
            if (parray != nullptr)
            {
                delete parray;
                pobject = nullptr;
            }
            break;
        case ValueType::String:
            if (pstring != nullptr)
            {
                delete pstring;
                pstring = nullptr;
            }
            break;
        case ValueType::Number:
            number_val = 0;
        case ValueType::Bool:
        case ValueType::Null:
        case ValueType::None:
        default:
            break;
        }
        type = ValueType::None;
    }

#pragma endregion

#pragma region iterator related

    __array_iterator::__array_iterator() = default;
    __array_iterator::__array_iterator(const self_type &another) : it(another.it) {}
    __array_iterator::__array_iterator(self_type &&another) : it(move(another.it)) {}
    __array_iterator::__array_iterator(const inner_iter_type &a_it) : it(a_it) {}
    __array_iterator::__array_iterator(inner_iter_type &&a_it) : it(forward<inner_iter_type>(a_it)) {}
    __array_iterator::~__array_iterator() = default;

    __array_iterator::reference __array_iterator::operator*() const { return *it; }
    __array_iterator::pointer __array_iterator::operator->() const { return it.operator->(); }
    __array_iterator::difference_type __array_iterator::operator-(const self_type &another) const { return it - another.it; }

    __array_iterator::self_type &__array_iterator::operator++()
    {
        ++it;
        return *this;
    }

    __array_iterator::self_type __array_iterator::operator++(int)
    {
        self_type tmp(*this);
        ++it;
        return tmp;
    }

    __array_iterator::self_type &__array_iterator::operator+=(difference_type n)
    {
        it += n;
        return *this;
    }

    __array_iterator::self_type __array_iterator::operator+(difference_type n) const
    {
        self_type tmp(*this);
        tmp += n;
        return tmp;
    }

    __array_iterator::self_type &__array_iterator::operator--()
    {
        --it;
        return *this;
    }
    __array_iterator::self_type __array_iterator::operator--(int)
    {
        self_type tmp(*this);
        --it;
        return tmp;
    }

    __array_iterator::self_type &__array_iterator::operator-=(difference_type n)
    {
        it -= n;
        return *this;
    }

    __array_iterator::self_type __array_iterator::operator-(difference_type n) const
    {
        self_type tmp(*this);
        tmp -= n;
        return tmp;
    }

    __array_iterator::reference __array_iterator::operator[](difference_type n) const { return it[n]; }
    bool __array_iterator::operator==(const self_type &another) const { return it == another.it; }
    bool __array_iterator::operator!=(const self_type &another) const { return it != another.it; }
    bool __array_iterator::operator<(const self_type &another) const { return it < another.it; }
    bool __array_iterator::operator>(const self_type &another) const { return it > another.it; }
    bool __array_iterator::operator<=(const self_type &another) const { return it <= another.it; }
    bool __array_iterator::operator>=(const self_type &another) const { return it >= another.it; }


    __object_iterator::__object_iterator() = default;
    __object_iterator::__object_iterator(const self_type &another) : it(another.it) {}
    __object_iterator::__object_iterator(self_type &&another) : it(move(another.it)) {}
    __object_iterator::__object_iterator(const inner_iter_type &a_it) : it(a_it) {}
    __object_iterator::__object_iterator(inner_iter_type &&a_it) : it(forward<inner_iter_type>(a_it)) {}
    __object_iterator::~__object_iterator() = default;

    __object_iterator::reference __object_iterator::operator*() const { return *it; }
    __object_iterator::pointer __object_iterator::operator->() const { return it.operator->(); }

    __object_iterator::self_type &__object_iterator::operator++()
    {
        ++it;
        return *this;
    }

    __object_iterator::self_type __object_iterator::operator++(int)
    {
        self_type tmp(*this);
        ++it;
        return tmp;
    }

    __object_iterator::self_type &__object_iterator::operator--()
    {
        --it;
        return *this;
    }

    __object_iterator::self_type __object_iterator::operator--(int)
    {
        self_type tmp(*this);
        --it;
        return tmp;
    }

    bool __object_iterator::operator==(const self_type &another) const { return it == another.it; }
    bool __object_iterator::operator!=(const self_type &another) const { return it != another.it; }

#pragma endregion

#pragma region char_stack

    __char_stack::__char_stack()
    {
        top = 0;
        capability = CHAR_STACK_INIT_CAP;
        ptr = (char *)std::malloc(CHAR_STACK_INIT_CAP);
    }

    __char_stack::~__char_stack()
    {
        std::free(ptr);
        ptr = nullptr;
        capability = 0;
        top = 0;
    }

    void *__char_stack::push(size_t size)
    {
        void *ret;
        if (top + size >= capability)
        {
            do
                EXTEND_SIZE(capability);
            while (top + size >= capability);
            ptr = (char *)std::realloc(ptr, capability);
        }
        ret = ptr + top;
        top += size;
        return ret;
    }

    void *__char_stack::pop(size_t size)
    {
        top -= size;
        return ptr + top;
    }

    size_t __char_stack::get_top()
    {
        return top;
    }

    void __char_stack::set_top(size_t n)
    {
        top = n;
    }

#pragma endregion

#pragma region __parser

    __parser::__parser(const char *cstr) : raw_iter(cstr) {}

    ParseStatus __parser::exec(Value &out)
    {
        ParseStatus ret;
        parse_whitespace();
        if ((ret = parse_value(out)) == ParseStatus::OK)
        {
            parse_whitespace();
            if (!IS_ZEROEND(*raw_iter))
            {
                ret = ParseStatus::ROOT_NOT_SINGULAR;
            }
        }
        return ret;
    }

    ParseStatus __parser::parse_whitespace()
    {
        while (IS_WHITESPACE(*raw_iter))
            raw_iter++;
        return ParseStatus::OK;
    }

    ParseStatus __parser::parse_value(Value &out)
    {
        ParseStatus status;
        switch (*raw_iter)
        {
        case 't':
            status = parse_bool(out, "true", true);
            break;
        case 'f':
            status = parse_bool(out, "false", false);
            break;
        case 'n':
            status = parse_literal(out, "null", ValueType::Null);
            break;
        case '"':
            status = parse_string(out);
            break;
        case '[':
            status = parse_array(out);
            break;
        case '{':
            status = parse_object(out);
            break;
        case '\0':
            status = ParseStatus::UNEXPECTED_SYMBOL;
            break;
        default:
            status = parse_number(out);
            break;
        }
        return status;
    }

    ParseStatus __parser::parse_literal(Value &out, const char *target, ValueType t)
    {
        size_t idx;
        for (idx = 0; target[idx]; idx++)
        {
            if (raw_iter[idx] != target[idx])
                return ParseStatus::INVALID_VALUE;
        }
        raw_iter += idx;
        out.set_literal(t);
        return ParseStatus::OK;
    }

    ParseStatus __parser::parse_bool(Value &out, const char *target, bool v)
    {
        size_t idx;
        for (idx = 0; target[idx]; idx++)
        {
            if (raw_iter[idx] != target[idx])
                return ParseStatus::INVALID_VALUE;
        }
        raw_iter += idx;
        out.set_bool(v);
        return ParseStatus::OK;
    }

    ParseStatus __parser::parse_string(Value &out)
    {
        size_t length;
        ParseStatus ret;
        if ((ret = parse_string_raw(length)) == ParseStatus::OK)
            out.set_string((char *)cstack.pop(length), length);
        return ret;
    }

    ParseStatus __parser::parse_string_raw(size_t &length_out)
    {
        size_t top_bak = cstack.get_top();
        raw_iter++;
        const char *iter = raw_iter;
        char cur;
        unsigned uh, ul;
        while (true)
        {
            cur = *iter++;
            switch (cur)
            {
            case '"':
                length_out = cstack.get_top() - top_bak;
                raw_iter = iter;
                return ParseStatus::OK;
            case '\\':
                switch (*iter++)
                {
                case '"':
                    PUSH_CHAR(cstack, '"');
                    break;
                case '\\':
                    PUSH_CHAR(cstack, '\\');
                    break;
                case '/':
                    PUSH_CHAR(cstack, '/');
                    break;
                case 'b':
                    PUSH_CHAR(cstack, '\b');
                    break;
                case 'f':
                    PUSH_CHAR(cstack, '\f');
                    break;
                case 'n':
                    PUSH_CHAR(cstack, '\n');
                    break;
                case 'r':
                    PUSH_CHAR(cstack, '\r');
                    break;
                case 't':
                    PUSH_CHAR(cstack, '\t');
                    break;
                case 'u':
                    // unicode
                    if (!hex4_to_ui(iter, uh))
                    {
                        cstack.set_top(top_bak);
                        return ParseStatus::INVALID_UNICODE_HEX;
                    }
                    iter += 4;
                    if (IS_SURROGATE_H(uh))
                    {
                        if (*iter++ != '\\')
                        {
                            cstack.set_top(top_bak);
                            return ParseStatus::INVALID_UNICODE_SURROGATE;
                        }
                        if (*iter++ != 'u')
                        {
                            cstack.set_top(top_bak);
                            return ParseStatus::INVALID_UNICODE_SURROGATE;
                        }
                        if (!(hex4_to_ui(iter, ul)))
                        {
                            cstack.set_top(top_bak);
                            return ParseStatus::INVALID_UNICODE_HEX;
                        }
                        iter += 4;
                        if (IS_SURROGATE_L(ul))
                        {
                            cstack.set_top(top_bak);
                            return ParseStatus::INVALID_UNICODE_SURROGATE;
                        }
                        uh = CALC_CODEPOINT(uh, ul);
                    }

                    if (uh <= 0x7F)
                        PUSH_CHAR(cstack, uh & 0xFF);
                    else if (uh <= 0x7FF)
                    {
                        PUSH_CHAR(cstack, 0xC0 | ((uh >> 6) & 0xFF));
                        PUSH_CHAR(cstack, 0x80 | (uh & 0x3F));
                    }
                    else if (uh <= 0xFFFF)
                    {
                        PUSH_CHAR(cstack, 0xE0 | ((uh >> 12) & 0xFF));
                        PUSH_CHAR(cstack, 0x80 | ((uh >> 6) & 0x3F));
                        PUSH_CHAR(cstack, 0x80 | (uh & 0x3F));
                    }
                    else
                    {
                        PUSH_CHAR(cstack, 0xF0 | ((uh >> 18) & 0xFF));
                        PUSH_CHAR(cstack, 0x80 | ((uh >> 12) & 0x3F));
                        PUSH_CHAR(cstack, 0x80 | ((uh >> 6) & 0x3F));
                        PUSH_CHAR(cstack, 0x80 | (uh & 0x3F));
                    }
                    break;
                default:
                    cstack.set_top(top_bak);
                    return ParseStatus::INVALID_STRING_ESCAPE;
                }
                break;
            case '\0':
                cstack.set_top(top_bak);
                return ParseStatus::MISS_QUOTATION_MARK;
            default:
                if ((unsigned char)cur < 0x20)
                {
                    cstack.set_top(top_bak);
                    return ParseStatus::INVALID_STRING_CHAR;
                }
                PUSH_CHAR(cstack, cur);
            }
        }
    }

    ParseStatus __parser::parse_array(Value &out)
    {
        raw_iter++;
        out.init_array();
        ParseStatus ret;
        parse_whitespace();
        if (*raw_iter == ']')
        {
            raw_iter++;
            return ParseStatus::OK;
        }
        while (true)
        {
            Value tmp;
            if ((ret = parse_value(tmp)) != ParseStatus::OK)
            {
                break;
            }
            out.array_push_back(move(tmp));
            parse_whitespace();
            if (*raw_iter == ',')
            {
                raw_iter++;
                parse_whitespace();
            }
            else if (*raw_iter == ']')
            {
                raw_iter++;
                return ParseStatus::OK;
            }
            else
            {
                ret = ParseStatus::MISS_ARRAY_SYMBOL;
                break;
            }
        }
        out.set_literal(ValueType::None);
        return ret;
    }

    ParseStatus __parser::parse_object(Value &out)
    {
        raw_iter++;
        out.init_object();
        ParseStatus ret;
        parse_whitespace();
        if (*raw_iter == '}')
        {
            raw_iter++;
            return ParseStatus::OK;
        }
        while (true)
        {
            if (*raw_iter != '"')
            {
                ret = ParseStatus::MISS_OBJECT_KEY;
                break;
            }
            size_t str_len;
            if ((ret = parse_string_raw(str_len)) != ParseStatus::OK)
                break;
            parse_whitespace();
            if (*raw_iter != ':')
            {
                ret = ParseStatus::MISS_OBJECT_SYMBOL;
                break;
            }
            raw_iter++;
            parse_whitespace();
            Value tmp;
            if ((ret = parse_value(tmp)) != ParseStatus::OK)
            {
                break;
            }
            out.object_insert(std::string((char *)cstack.pop(str_len), 0, str_len), move(tmp));
            parse_whitespace();
            if (*raw_iter == ',')
            {
                raw_iter++;
                parse_whitespace();
            }
            else if (*raw_iter == '}')
            {
                raw_iter++;
                break;
            }
            else
            {
                ret = ParseStatus::MISS_OBJECT_SYMBOL;
                break;
            }
        }
        return ret;
    }

    ParseStatus __parser::parse_number(Value &out)
    {
        const char *iter = raw_iter;
        char *endp = nullptr;
        if (*iter == '-')
            iter++;

        if (*iter == '0')
            iter++;
        else
        {
            if (!IS_DIGIT19(*iter))
                return ParseStatus::INVALID_VALUE;
            do
                iter++;
            while (IS_DIGIT09(*iter));
        }

        if (*iter == '.')
        {
            iter++;
            if (!IS_DIGIT09(*iter))
                return ParseStatus::INVALID_VALUE;
            do
                iter++;
            while (IS_DIGIT09(*iter));
        }

        if (*iter == 'e' || *iter == 'E')
        {
            iter++;
            if (*iter == '+' || *iter == '-')
                iter++;
            if (!IS_DIGIT09(*iter))
                return ParseStatus::INVALID_VALUE;
            do
                iter++;
            while (IS_DIGIT09(*iter));
        }
        errno = 0;
        out.set_number(std::strtod(raw_iter, &endp));
        if (endp != iter)
            return ParseStatus::INVALID_VALUE;
        if (errno == ERANGE && (out.as_number() == HUGE_VAL || out.as_number() == -HUGE_VAL))
            return ParseStatus::NUMBER_TOO_LARGE;
        raw_iter = iter;
        return ParseStatus::OK;
    }

    std::pair<ParseStatus, json> parse(const char *str)
    {
        Value result;
        __parser ps(str);
        auto status = ps.exec(result);
        return {status, move(result)};
    }

#pragma endregion

}