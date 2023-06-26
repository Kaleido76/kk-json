# kkjson

This is a simple **read-only** json lib based on C++, inspired by leptjson.

> [leptjson](https://github.com/miloyip/json-tutorial)

### Descriptions

+ This library only includes **parsing** functionality for JSON-formatted strings, and the resulting object is read-only.
+ It comes with a total of 251 test samples.
+ Number values are limited to the double range, and parsing is based on the `strtod` function.
+ Unicode string parsing, such as `"\u00FD\uAA80abcd"`, is also supported.
+ It provides relatively complete parsing error return values.
+ It performs reasonably well, thanks to its use of a large number of direct pointer operations.
+ Despite being rudimentary, the parsing capabilities of this library comply with the JSON standard.

### Warnings

This project, which took me about 3 ~ 4 days to develop, served as a warm-up toy as I had just started programming with C++. If you're a beginner, note that many of the implementations in this project may not be the ones recommended in C++, such as a lot of direct pointer manipulation (maybe use smart pointers?). Please use it for reference only and do not use it as learning material.

Perhaps I will further improve it in the future.

### Usage

Create a parser that can be reused without the need to manually manage or destroy memory after parsing is complete.

```cpp
#include "kkjson.h"

...
kkjson::parser p;
kkjson::parse_status ps = p.exec_parse("[ null , false , true , 123 , \"abc\", {\"123\": 111} ]");
if (ps == parse_status::OK)
{
    p.result.get_type();   // return value_type::VT_ARRAY
    p.result[0].get_type(); // return value_type::VT_NULL
    p.result[1].get_type(); // return value_type::VT_FALSE
    p.result[2].get_type(); // return value_type::VT_TRUE
    p.result[3].get_type(); // return value_type::VT_NUMBER
    p.result[3].get_number(); // return 123
    p.result[4].get_type(); // return value_type::VT_STRING
    p.result[4].get_number(); // return std::string("abc")
    p.result[5].get_type(); // return value_type::VT_OBJECT
    p.result[5].get_value("123").get_type(); // return value_type::VT_NUMBER
    p.result[5].get_value("123").get_number(); // return 111
}
...

```
