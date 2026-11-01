#ifndef schema__schema_head_hpp
#define schema__schema_head_hpp

#include <cstdint>

#include "event/event_type.h"

#define SCHEMA_HEAD_MAGIC "Arch-V"
#define SCHEMA_HEAD_VERSION 1
typedef enum
{
    SCHEMA_UNKNOWN,
    SCHEMA_CMD,
    SCHEMA_RULE,
    SCHEMA_EVENT,
} SchemaType;

typedef enum
{
    EVENT_CLASS_UNKNOWN,
    EVENT_CLASS_AUDIT,
    EVENT_CLASS_NOTIFY,
} EventClass;

typedef struct
{
    char            magic[12];
    uint64_t        version;
    uint64_t        sequence;

    SchemaType      type;
    uint64_t        rule_id;

    uint64_t        pid;
    uint64_t        ppid;
    uint64_t        time;

    EventType       event_type;
    EventClass      event_class;

    uint64_t        data_len;
    
    uint32_t        reverse32;
    uint64_t        reverse64;
} SCHEMA_HEAD;

#endif // schema__schema_head_hpp