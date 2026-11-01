#ifndef DNS_PARSER_H
#define DNS_PARSER_H

#include <string>

#ifdef __cplusplus
extern "C" {
#endif

    struct DomainPointer
    {
        const unsigned char* name;
        unsigned length;
    };

    struct DNS_Incoming
    {
        unsigned id;                    /* transaction id */
        unsigned is_valid   : 1;
        unsigned is_formerr : 1;
        unsigned is_edns0   : 1;        /* edns0 features found */
        unsigned qr         : 1;        /* 'query' or 'response' */
        unsigned aa         : 1;        /* 'authoritative answer' */
        unsigned tc         : 1;        /* 'truncation' */
        unsigned rd         : 1;        /* 'recursion desired' */
        unsigned ra         : 1;        /* 'recursion available' */
        unsigned z          : 3;        /* reserved */
        unsigned opcode;
        unsigned rcode;                 /* response error code */
        unsigned qdcount;               /* query count */
        unsigned ancount;               /* answer count */
        unsigned nscount;               /* name-server/authority count */
        unsigned arcount;               /* additional record count */
        struct {
            unsigned payload_size;
            unsigned version;
            unsigned z;
        } edns0;
        const unsigned char* req;
        unsigned req_length;

        /* the query name */
        struct DomainPointer query_name;
        unsigned query_type;
        unsigned char query_name_buffer[256];

        unsigned rr_count;
        unsigned short rr_offset[1024];
        unsigned edns0_offset;
    };

    typedef struct _DNS_PARSE_RESULT
    {
        long QueryType;
        long QueryResult;
        long QueryOptions;
        std::string QueryName;
        std::string AnswerName;
    } DnsParseResult, *PDnsParseResult;

    void proto_dns_parse(struct DNS_Incoming* dns, const unsigned char px[], unsigned offset, unsigned max);

    char* read_rr_name(const unsigned char* px, unsigned int* packet_p, unsigned int id_pos, unsigned int len);

    void dns_extract_name(const unsigned char px[], unsigned offset, unsigned max, struct DomainPointer* name);

    unsigned dns_name_skip(const unsigned char px[], unsigned offset, unsigned max);

    bool parse_dns_protocol(const unsigned char px[], unsigned Length, PDnsParseResult ParseResult);

#ifdef __cplusplus
}
#endif

#endif
