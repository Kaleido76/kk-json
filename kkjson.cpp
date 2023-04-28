#include <iostream>
#include <string>
#include <cstdlib>
#include <cerrno>
#include <cmath>
#include "kkjson.h"

#define PROBE(x) std::cout << (x) << std::endl

// mix stack
#define MIX_STACK_INIT_CAP 256
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

namespace kkjson
{
    // char_stack
    char_stack::char_stack()
    {
        top = 0;
        capability = MIX_STACK_INIT_CAP;
        ptr = (char *)std::malloc(MIX_STACK_INIT_CAP);
    }

    char_stack::~char_stack()
    {
        std::free(ptr);
        ptr = nullptr;
        capability = 0;
        top = 0;
    }

    void *char_stack::push(size_t size)
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

    void *char_stack::pop(size_t size)
    {
        top -= size;
        return ptr + top;
    }

    size_t char_stack::get_top()
    {
        return top;
    }

    void char_stack::set_top(size_t n)
    {
        top = n;
    }

    // value_entry
    value_entry::value_entry() : type(value_type::VT_NONE) {}

    value_entry::value_entry(const value_entry &another)
    {
        operator=(another);
    }

    value_entry &value_entry::operator=(const value_entry &another)
    {
        clean();
        switch (another.type)
        {
        case value_type::VT_OBJECT:
            object = new std::map<std::string, value_entry *>();
            for (auto it = another.object->begin(); it != another.object->end(); it++)
            {
                value_entry *pve = new value_entry(*(it->second));
                object->insert(std::pair<std::string, value_entry *>(it->first, pve));
            }
            break;
        case value_type::VT_STRING:
            pstr = new std::string(*(another.pstr));
            break;
        case value_type::VT_ARRAY:
            array = new std::vector<value_entry *>();
            array->reserve(another.array->size());
            for (auto it = another.array->begin(); it != another.array->end(); it++)
                array->push_back(new value_entry(**it));
            break;
        case value_type::VT_NUMBER:
            number = another.number;
        case value_type::VT_NONE:
        case value_type::VT_NULL:
        case value_type::VT_TRUE:
        case value_type::VT_FALSE:
        default:
            break;
        }
        type = another.type;
        return *this;
    }

    value_entry::value_entry(value_entry &&another) noexcept
    {
        operator=(another);
    }

    value_entry &value_entry::operator=(value_entry &&another) noexcept
    {
        clean();
        switch (another.type)
        {
        case value_type::VT_OBJECT:
            object = another.object;
            object = nullptr;
            break;
        case value_type::VT_ARRAY:
            array = another.array;
            another.array = nullptr;
            break;
        case value_type::VT_STRING:
            pstr = another.pstr;
            another.pstr = nullptr;
            break;
        case value_type::VT_NUMBER:
            number = another.number;
        case value_type::VT_NONE:
        case value_type::VT_NULL:
        case value_type::VT_TRUE:
        case value_type::VT_FALSE:
            break;
        default:
            break;
        }
        type = another.type;
        return *this;
    }

    value_entry::~value_entry()
    {
        clean();
    }

    void value_entry::set_literal(value_type vt)
    {
        clean();
        type = vt;
    }

    const value_type &value_entry::get_type() const
    {
        return type;
    }

    void value_entry::set_number(double value)
    {
        clean();
        type = value_type::VT_NUMBER;
        number = value;
    }

    const double &value_entry::get_number() const
    {
        return number;
    }

    void value_entry::set_string(const char *ori, size_t len)
    {
        clean();
        type = value_type::VT_STRING;
        pstr = new std::string(ori, len);
    }

    const std::string &value_entry::get_string() const
    {
        return *pstr;
    }

    // array
    void value_entry::init_array()
    {
        clean();
        type = value_type::VT_ARRAY;
        array = new std::vector<value_entry *>();
    }

    void value_entry::add_array_element(value_entry *const &pve)
    {
        if (type != value_type::VT_ARRAY)
            return;
        array->push_back(pve);
    }

    size_t value_entry::get_array_size() const
    {
        return array != nullptr ? array->size() : 0;
    }

    const value_entry &value_entry::get_array_element(size_t idx) const
    {
        return *(array->operator[](idx));
    }

    const value_entry &value_entry::operator[](size_t idx) const
    {
        return *(array->operator[](idx));
    }

    // object
    void value_entry::init_object()
    {
        clean();
        type = value_type::VT_OBJECT;
        object = new std::map<std::string, value_entry *>();
    }

    void value_entry::add_key_value(const std::string &key, value_entry *const &pve)
    {
        object->insert(std::pair<std::string, value_entry *>(std::string(key), pve));
    }

    const value_entry &value_entry::get_value(const std::string &key) const
    {
        using kv_iter_t = std::map<std::string, value_entry *>::iterator;
        kv_iter_t iter = object->find(key);
        return *(iter->second);
    }

    void value_entry::clean()
    {
        switch (type)
        {
        case value_type::VT_OBJECT:
            if (object == nullptr)
                break;
            for (auto it = object->begin(); it != object->end(); it++)
                delete it->second;
            delete object;
            break;
        case value_type::VT_ARRAY:
            if (array == nullptr)
                break;
            for (auto it = array->begin(); it != array->end(); it++)
                delete *it;
            delete array;
            break;
        case value_type::VT_STRING:
            if (pstr != nullptr)
            {
                delete pstr;
                pstr = nullptr;
            }
            break;
        case value_type::VT_NUMBER:
            number = 0;
        case value_type::VT_FALSE:
        case value_type::VT_TRUE:
        case value_type::VT_NULL:
        case value_type::VT_NONE:
        default:
            break;
        }
        type = value_type::VT_NONE;
    }

    // parser
    parser::parser()
    {
        result = value_entry();
    }

    parse_status parser::exec_parse(const char *c_ptr)
    {
        raw_iter = c_ptr;
        parse_status ret;
        parse_whitespace();
        if ((ret = parse_value(result)) == parse_status::OK)
        {
            parse_whitespace();
            if (!IS_ZEROEND(*raw_iter))
            {
                ret = parse_status::ROOT_NOT_SINGULAR;
            }
        }
        return ret;
    }

    parse_status parser::parse_whitespace()
    {
        while (IS_WHITESPACE(*raw_iter))
            raw_iter++;
        return parse_status::OK;
    }

    parse_status parser::parse_literal(value_entry &ve, const char *target, value_type vt)
    {
        size_t idx;
        for (idx = 0; target[idx]; idx++)
        {
            if (raw_iter[idx] != target[idx])
                return parse_status::INVALID_VALUE;
        }
        raw_iter += idx;
        ve.set_literal(vt);
        return parse_status::OK;
    }

    parse_status parser::parse_number(value_entry &ve)
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
                return parse_status::INVALID_VALUE;
            do
                iter++;
            while (IS_DIGIT09(*iter));
        }

        if (*iter == '.')
        {
            iter++;
            if (!IS_DIGIT09(*iter))
                return parse_status::INVALID_VALUE;
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
                return parse_status::INVALID_VALUE;
            do
                iter++;
            while (IS_DIGIT09(*iter));
        }
        errno = 0;
        ve.set_number(std::strtod(raw_iter, &endp));
        if (endp != iter)
            return parse_status::INVALID_VALUE;
        if (errno == ERANGE && (ve.get_number() == HUGE_VAL || ve.get_number() == -HUGE_VAL))
            return parse_status::NUMBER_TOO_LARGE;
        raw_iter = iter;
        return parse_status::OK;
    }

    parse_status parser::parse_string_raw(size_t &length)
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
                length = cstack.get_top() - top_bak;
                raw_iter = iter;
                return parse_status::OK;
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
                        return parse_status::INVALID_UNICODE_HEX;
                    }
                    iter += 4;
                    if (IS_SURROGATE_H(uh))
                    {
                        if (*iter++ != '\\')
                        {
                            cstack.set_top(top_bak);
                            return parse_status::INVALID_UNICODE_SURROGATE;
                        }
                        if (*iter++ != 'u')
                        {
                            cstack.set_top(top_bak);
                            return parse_status::INVALID_UNICODE_SURROGATE;
                        }
                        if (!(hex4_to_ui(iter, ul)))
                        {
                            cstack.set_top(top_bak);
                            return parse_status::INVALID_UNICODE_HEX;
                        }
                        iter += 4;
                        if (IS_SURROGATE_L(ul))
                        {
                            cstack.set_top(top_bak);
                            return parse_status::INVALID_UNICODE_SURROGATE;
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
                    return parse_status::INVALID_STRING_ESCAPE;
                }
                break;
            case '\0':
                cstack.set_top(top_bak);
                return parse_status::MISS_QUOTATION_MARK;
            default:
                if ((unsigned char)cur < 0x20)
                {
                    cstack.set_top(top_bak);
                    return parse_status::INVALID_STRING_CHAR;
                }
                PUSH_CHAR(cstack, cur);
            }
        }
    }

    parse_status parser::parse_string(value_entry &ve)
    {
        size_t length;
        parse_status ret;
        if ((ret = parse_string_raw(length)) == parse_status::OK)
            ve.set_string((char *)cstack.pop(length), length);
        return ret;
    }

    parse_status parser::parse_array(value_entry &ve)
    {
        raw_iter++;
        ve.init_array();
        parse_status ret;
        parse_whitespace();
        if (*raw_iter == ']')
        {
            raw_iter++;
            return parse_status::OK;
        }
        while (true)
        {
            value_entry *pve = new value_entry();
            if ((ret = parse_value(*pve)) != parse_status::OK)
            {
                delete pve;
                break;
            }
            ve.add_array_element(pve);
            pve = nullptr;
            parse_whitespace();
            if (*raw_iter == ',')
            {
                raw_iter++;
                parse_whitespace();
            }
            else if (*raw_iter == ']')
            {
                raw_iter++;
                return parse_status::OK;
            }
            else
            {
                ret = parse_status::MISS_ARRAY_SYMBOL;
                break;
            }
        }
        ve.set_literal(value_type::VT_NONE);
        return ret;
    }

    parse_status parser::parse_object(value_entry &ve)
    {
        raw_iter++;
        ve.init_object();
        parse_status ret;
        parse_whitespace();
        if (*raw_iter == '}')
        {
            raw_iter++;
            return parse_status::OK;
        }
        while (true)
        {
            if (*raw_iter != '"')
            {
                ret = parse_status::MISS_OBJECT_KEY;
                break;
            }
            size_t str_len;
            if ((ret = parse_string_raw(str_len)) != parse_status::OK)
                break;
            parse_whitespace();
            if (*raw_iter != ':')
            {
                ret = parse_status::MISS_OBJECT_SYMBOL;
                break;
            }
            raw_iter++;
            parse_whitespace();
            value_entry *pve = new value_entry();
            if ((ret = parse_value(*pve)) != parse_status::OK)
            {
                delete pve;
                break;
            }
            ve.add_key_value(std::string((char *)cstack.pop(str_len), 0, str_len), pve);
            pve = nullptr;
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
                ret = parse_status::MISS_OBJECT_SYMBOL;
                break;
            }
        }
        return ret;
    }

    parse_status parser::parse_value(value_entry &ve)
    {
        parse_status status;
        switch (*raw_iter)
        {
        case 't':
            status = parse_literal(ve, "true", value_type::VT_TRUE);
            break;
        case 'f':
            status = parse_literal(ve, "false", value_type::VT_FALSE);
            break;
        case 'n':
            status = parse_literal(ve, "null", value_type::VT_NULL);
            break;
        case '"':
            status = parse_string(ve);
            break;
        case '[':
            status = parse_array(ve);
            break;
        case '{':
            status = parse_object(ve);
            break;
        case '\0':
            status = parse_status::UNEXPECTED_SYMBOL;
            break;
        default:
            status = parse_number(ve);
            break;
        }
        return status;
    }
}