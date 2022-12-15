#include "JSON.hpp"
#include <iostream>

JSON::JSON()
{
    object = cJSON_CreateObject();
}

JSON::JSON(const std::string & string)
{
    object = cJSON_Parse(string.c_str());
}

JSON::JSON(const char * string)
{
    object = cJSON_Parse(string);
}

JSON::~JSON()
{
    cJSON_Delete(object);
}

int JSON::get_number_from_key(const std::string key)
{
    if (!cJSON_IsNumber(cJSON_GetObjectItem(object, key.c_str()))) {
        std::cerr << "Attempt to get a non number value as integer value (key: " << key << ")" << std::endl;
        exit(1);
    }
    return (int) cJSON_GetNumberValue(cJSON_GetObjectItem(object, key.c_str()));
}

std::string JSON::get_string_from_key(const std::string key)
{
    if (!cJSON_IsString(cJSON_GetObjectItem(object, key.c_str()))) {
        std::cerr << "Attempt to get a non string value as string value (key: " << key << ")" << std::endl;
        exit(1);
    }
    return std::string(cJSON_GetStringValue(cJSON_GetObjectItem(object, key.c_str())));
}

std::string JSON::to_string() const
{
    return std::string(cJSON_Print(object));
}
