#ifndef _JSON_HPP_
#define _JSON_HPP_

#include <string>
#include "cJSON.hpp"

class JSON
{
public:
    cJSON * object;
    JSON();
    JSON(const std::string &);
    JSON(const char *);
    ~JSON();

    int get_number_from_key(const std::string);
    std::string get_string_from_key(const std::string);

    std::string to_string() const;
};

#endif
