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

### Usage

Create a parser that can be reused without the need to manually manage or destroy memory after parsing is complete.

```cpp
#include "kkjson.h"

auto [status, js] = kkjson::parse("[ null , false , true , 123 , \"abc\", {\"123\": 111} ]");
if (status == parse_status::OK)
{
    js.get_type();   // return ValueType::Array

    js[0].get_type(); // return ValueType::Null
    
    js[1].get_type(); // return ValueType::False
    js[1].as_bool(); // return bool(false)

    js[3].get_type(); // return ValueType::Number
    js[3].as_number(); // return double(123)

    js[4].get_type(); // return ValueType::String
    js[4].as_string(); // return std::string("abc")

    js[5].get_type(); // return ValueType::Object
    js[5]["123"].get_type(); // return ValueType::Number
    js[5]["123"].as_number(); // return double(111)
}
```

### Updates

+ 2023-9-27
  + Makes the interface easier to use, but the json object is still read-only. simple editing function of the json object will be updated in a few days.
  + Remove the use of most pointers. While the previous version used raw pointers to avoid construct and destruct calls, now it turns to use move-semantics to avoid pointer involvement.

### Warnings

This project, which took me about 3 ~ 4 days to develop, served as a warm-up toy as I had just started programming with C++. If you're a beginner, note that many of the implementations in this project may not be the ones recommended in C++, such as a lot of direct pointer manipulation (maybe use smart pointers?). Please use it for reference only and do not use it as learning material.

Perhaps I will further improve it in the future.
