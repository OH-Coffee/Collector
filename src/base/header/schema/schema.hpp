#ifndef schema__schema_hpp
#define schema__schema_hpp

#include "schema/schema_head.hpp"
#include "schema/schema_body.hpp"

typedef struct
{
    SCHEMA_HEAD   Head;
    SCHEMA_BODY   Body;
} SCHEMA;

#endif // schema__schema_hpp