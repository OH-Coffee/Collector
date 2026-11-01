
#ifndef schema_helper_hpp
#define schema_helper_hpp

#include <cstdint>
#include <map>
#include <string>

#include "header/errcode.hpp"
#include "schema/schema.hpp"

#define SCHEMA_HEAD_SIZE sizeof(SCHEMA_HEAD)
#define SCHEMA_BODY_SIZE sizeof(SCHEMA_BODY)

const std::map<uint64_t, std::string>& schema_events();

ret_code    schema_init(SCHEMA *i_schema, uint64_t i_schema_size);
ret_code    schema_check(const SCHEMA *i_schema);

std::string schema2string(const SCHEMA &i_schema);
std::string schema2string(const SCHEMA *i_schema);
std::string schema2string(const SCHEMA_HEAD *i_head, const SCHEMA_BODY *i_body);


#endif /* schema_helper_hpp */
