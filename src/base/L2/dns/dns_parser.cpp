/*

    Parses DNS response information

    The scanner sends a CHAOS TXT query for "version.bind". This module parses
    DNS in order to find the response string.

*/

#include <cstring>
#include <cstdlib>

#include <arpa/inet.h>
#include <arpa/nameser.h>

#include "dns/dns_parser.hpp"


#define VERIFY_REMAINING(n) if (offset+(n) > length) return;


/****************************************************************************
 * This skips over a name field while parsing the packet. If the name
 * is just a two-byte compression field like 0xc0 0x1a, then it'll skip
 * those two bytes. However, when it does the skip, it does validate
 * the name. Thus, if it's a compressed name, it'll follow the compression
 * links to validate things like long names and infinite recursion.
 ****************************************************************************/
static unsigned
dns_name_skip_validate(const unsigned char *px, unsigned offset, unsigned length, unsigned name_length)
{
    unsigned ERROR = length + 1;
    unsigned result = offset + 2;
    unsigned recursion = 0;

    /* 'for all labels' */
    for (;;) {
        unsigned len;

        /* validate: the eventual uncompressed name will be less than 255 */
        if (name_length >= 255)
            return ERROR;

        /* validate: haven't gone off end of packet */
        if (offset >= length)
            return ERROR;

        /* grab length of next label */
        len = px[offset];

        /* Do two types of processing, either a compression code or a
         * original label. Note that we can alternate back and forth
         * between these two states. */
        if (len & 0xC0) {
            /* validate: top 2 bits are 11*/
            if ((len & 0xC0) != 0xC0)
                return ERROR;

            /* validate: enough bytes left for 2 byte compression field */
            if (offset + 1 >= length)
                return ERROR;

            /* follow the compression pointer to the next location */
            offset = (px[offset]&0x3F)<<8 | px[offset+1];

            /* validate: follow a max of 4 links */
            if (++recursion > 4)
                return ERROR;
        } else {
            /* we have a normal label */
            recursion = 0;

            /* If the label-length is zero, then that means we've reached
             * the end of the name */
            if (len == 0) {
                return result; /* end of domain name */
            }

            /* There are more labels to come, therefore skip this and go
             * to the next one */
            name_length += len + 1;
            offset += len + 1;
        }
    }
}

/****************************************************************************
 * Just skip the name, without validating whether it's valid or not. This
 * is for re-parsing the packet usually, after we've validated that all
 * the names are OK.
 ****************************************************************************/
unsigned
dns_name_skip(const unsigned char px[], unsigned offset, unsigned max)
{
    unsigned name_length = 0;

    /* Loop through all labels
     * NOTE: the only way this loops around is in the case of a normal
     * label. All other conditions cause a 'return' from this function */
    for (;;) {
        if (name_length >= 255)
            return max + 1;

        if (offset >= max)
            return max + 1;

        switch (px[offset]>>6) {
        case 0:
            /* uncompressed label */
            if (px[offset] == 0) {
                return offset+1; /* end of domain name */
            } else {
                name_length += px[offset] + 1;
                offset += px[offset] + 1;
                continue;
            }
            break;
        case 3:
            /* 0xc0 = compressed name */
            return dns_name_skip_validate(px, offset, max, name_length);
        case 2:
            /* 0x40 - ENDS0 extended label type
             * rfc2671 section 3.1
             * I have no idea how to parse this */
            return max + 1; /* totally clueless how to parse it */
        case 1:
            return max + 1;
        }
    }
}

/****************************************************************************
 ****************************************************************************/
void
dns_extract_name(const unsigned char px[], unsigned offset, unsigned max,
                 struct DomainPointer *name)
{
    name->length = 0;

    for (;;) {
        unsigned len;

        if (offset >= max)
            return;

        len = px[offset];
        if (len & 0xC0) {
            if ((len & 0xC0) != 0xC0)
                return;
            else if (offset + 1 >= max)
                return;
            else {
                offset = (px[offset]&0x3F)<<8 | px[offset+1];
            }
        } else {
            if (len == 0) {
                return; /* end of domain name */
            } else {
                memcpy((unsigned char*)name->name+name->length, px+offset, len+1);
                name->length = (unsigned char)(name->length + len + 1);
                offset += len + 1;
            }
        }
    }
}

char* read_rr_name(const unsigned char* px,
    unsigned int* packet_p,
    unsigned int id_pos,
    unsigned int len)
{
    unsigned int i, next, pos = *packet_p;
    unsigned int end_pos = 0;
    unsigned int name_len = 0;
    unsigned int steps = 0;
    char* name;

    // Scan through the name, one character at a time. We need to look at 
    // each character to look for values we can't print in order to allocate
    // extra space for escaping them.  'next' is the next position to look
    // for a compression jump or name end.
    // It's possible that there are endless loops in the name. Our protection
    // against this is to make sure we don't read more bytes in this process
    // than twice the length of the data.  Names that take that many steps to 
    // read in should be impossible.
    next = pos;
    while (pos < len && !(next == pos && px[pos] == 0)
        && steps < len * 2) {
        unsigned char c = px[pos];
        steps++;
        if (next == pos) {
            // Handle message compression.  
            // If the length byte starts with the bits 11, then the rest of
            // this byte and the next form the offset from the dns proto start
            // to the start of the remainder of the name.
            if ((c & 0xc0) == 0xc0) {
                if (pos + 1 >= len) return nullptr;
                if (end_pos == 0) end_pos = pos + 1;
                pos = id_pos + ((c & 0x3f) << 8) + px[pos + 1];
                next = pos;
            }
            else {
                name_len++;
                pos++;
                next = next + c + 1;
            }
        }
        else {
            if (c >= '!' && c <= 'z' && c != '\\') name_len++;
            else name_len += 4;
            pos++;
        }
    }
    if (end_pos == 0) end_pos = pos;

    // Due to the nature of DNS name compression, it's possible to get a
    // name that is infinitely long. Return an error in that case.
    // We use the len of the packet as the limit, because it shouldn't 
    // be possible for the name to be that long.
    if (steps >= 2 * len || pos >= len) return nullptr;

    name_len++;

    name = (char*)malloc(sizeof(char) * name_len);
    pos = *packet_p;

    //Now actually assemble the name.
    //We've already made sure that we don't exceed the packet length, so
    // we don't need to make those checks anymore.
    // Non-printable and whitespace characters are replaced with a question
    // mark. They shouldn't be allowed under any circumstances anyway.
    // Other non-allowed characters are kept as is, as they appear sometimes
    // regardless.
    // This shouldn't interfere with IDNA (international
    // domain names), as those are ascii encoded.
    next = pos;
    i = 0;
    while (next != pos || px[pos] != 0) {
        if (pos == next) {
            if ((px[pos] & 0xc0) == 0xc0) {
                pos = id_pos + ((px[pos] & 0x3f) << 8) + px[pos + 1];
                next = pos;
            }
            else {
                // Add a period except for the first time.
                if (i != 0) name[i++] = '.';
                next = pos + px[pos] + 1;
                pos++;
            }
        }
        else {
            unsigned char c = px[pos];
            if (c >= '!' && c <= '~' && c != '\\') {
                name[i] = px[pos];
                i++; pos++;
            }
            else {
                name[i] = '\\';
                name[i + 1] = 'x';
                name[i + 2] = c / 16 + 0x30;
                name[i + 3] = c % 16 + 0x30;
                if (name[i + 2] > 0x39) name[i + 2] += 0x27;
                if (name[i + 3] > 0x39) name[i + 3] += 0x27;
                i += 4;
                pos++;
            }
        }
    }
    name[i] = 0;

    *packet_p = end_pos + 1;

    return name;
}


/****************************************************************************
 ****************************************************************************/
void
proto_dns_parse(struct DNS_Incoming *dns, const unsigned char px[], unsigned offset, unsigned max)
{
    static const unsigned MAX_RRs = sizeof(dns->rr_offset)/sizeof(dns->rr_offset[0]);
    unsigned i;

    dns->is_valid = 0; /* not valid yet until we've successfully parsed*/

    dns->req = px;
    dns->req_length = max-offset;
    dns->edns0.payload_size = 512; /* RFC 1035 4.2.1 */


    /*
                                    1  1  1  1  1  1
      0  1  2  3  4  5  6  7  8  9  0  1  2  3  4  5
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                      ID                       |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |QR|   Opcode  |AA|TC|RD|RA|   Z    |   RCODE   |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                    QDCOUNT                    |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                    ANCOUNT                    |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                    NSCOUNT                    |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                    ARCOUNT                    |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    */
    if (max - offset < 12)
        return;
    dns->id = px[offset+0]<<8 | px[offset+1];
    dns->qr = (px[offset+2]>>7)&1;
    dns->aa = (px[offset+2]>>2)&1;
    dns->tc = (px[offset+2]>>1)&1;
    dns->rd = (px[offset+2]>>0)&1;
    dns->ra = (px[offset+3]>>7)&1;
    dns->z = (px[offset+3]>>4)&7;
    dns->opcode = (px[offset+2]>>3)&0xf;
    dns->rcode = (px[offset+3]>>0)&0xf;
    dns->qdcount = px[offset+4]<<8 | px[offset+5];
    dns->ancount = px[offset+6]<<8 | px[offset+7];
    dns->nscount = px[offset+8]<<8 | px[offset+9];
    dns->arcount = px[offset+10]<<8 | px[offset+11];
    dns->rr_count = 0; /* so far */
    offset += 12;
    dns->is_valid = 1;
    dns->is_formerr = 1; /* is "formate-error" until we've finished parsing */

    /*
                                    1  1  1  1  1  1
      0  1  2  3  4  5  6  7  8  9  0  1  2  3  4  5
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                                               |
    /                     QNAME                     /
    /                                               /
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                     QTYPE                     |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                     QCLASS                    |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    */
    for (i=0; i<dns->qdcount; i++) {
        unsigned xclass;
        unsigned xtype;
        if (dns->rr_count >= MAX_RRs)
            return;
        dns->rr_offset[dns->rr_count++] = (unsigned short)offset;
        offset = dns_name_skip(px, offset, max);
        offset += 4; /* length of type and class */
        if (offset > max)
            return;
        xclass = px[offset-2]<<8 | px[offset-1];
        if (xclass != 1 && xclass != 255 && xclass != 3)
            return;
        xtype = px[offset-4]<<8 | px[offset-3];
        dns->query_type = xtype;
    }

    /*
                                    1  1  1  1  1  1
      0  1  2  3  4  5  6  7  8  9  0  1  2  3  4  5
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                                               |
    /                                               /
    /                      NAME                     /
    |                                               |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                      TYPE                     |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                     CLASS                     |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                      TTL                      |
    |                                               |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                   RDLENGTH                    |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--|
    /                     RDATA                     /
    /                                               /
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    */
    for (i=0; i<dns->ancount + dns->nscount; i++) {
        unsigned rdlength;
        if (dns->rr_count >= sizeof(dns->rr_offset)/sizeof(dns->rr_offset[0]))
            return;
        dns->rr_offset[dns->rr_count++] = (unsigned short)offset;
        offset = dns_name_skip(px, offset, max);
        offset += 10;
        if (offset > max)
            return;
        rdlength = px[offset-2]<<8 | px[offset-1];
        offset += rdlength;
        if (offset > max)
            return;
    }
    for (i=0; i<dns->arcount; i++) {
        unsigned rdlength;
        if (dns->rr_count >= sizeof(dns->rr_offset)/sizeof(dns->rr_offset[0]))
            return;
        dns->rr_offset[dns->rr_count++] = (unsigned short)offset;

        /* ENDS0 OPT parsing */
        if (offset + 11 <= max && px[offset] == 0 && px[offset+1] == 0 && px[offset+2] == 0x29) {
            dns->edns0.payload_size = px[offset+3]<<8 | px[offset+4];
            if (dns->edns0.payload_size < 512)
                return;
            dns->rcode |= px[offset+5]<<4;
            dns->edns0.version = px[offset+6];
            dns->is_edns0 = 1;
        }

        offset = dns_name_skip(px, offset, max);
        offset += 10;
        if (offset > max)
            return;
        rdlength = px[offset-2]<<8 | px[offset-1];
        offset += rdlength;
        if (offset > max)
            return;
    }

    dns->query_name.name = dns->query_name_buffer;
    dns_extract_name(px, dns->rr_offset[0], max, &dns->query_name);

    dns->is_formerr = 0;
    return;
}


bool parse_dns_protocol(const unsigned char px[], unsigned Length, PDnsParseResult ParseResult)
{
    bool rResult = false;
    do {
        unsigned int pxLength = Length;
        unsigned int pxOffset = 0;

        DNS_Incoming di = { };
        if (!px || !Length || Length >= 0x1000) {
            break;
        }
        
        proto_dns_parse(&di, px, 0, pxLength);
        
        if (!di.is_valid || di.is_formerr) {
            break;
        }
        
        if (di.qdcount > 1) {
            break;
        }
        
        pxOffset = di.rr_offset[0];
        char* szQueryName = read_rr_name(px, &pxOffset, 0, pxLength);
        if (!szQueryName) {
            break;
        }
        
        ParseResult->QueryName = szQueryName;
        free(szQueryName);
        
        ParseResult->QueryOptions = 0;
        ParseResult->QueryType = di.query_type;
        if (di.rcode) {
            ParseResult->QueryResult = di.rcode + 9000;
        }
        else {
            ParseResult->QueryResult = 0;
        }

        /* 仍然需要上报，具体原因还需要确认  */
//        if (di.rcode != 0) {
//            break;
//        }

        std::string answerName;
        // type: 5 www.a.shifen.com;type: 28 ::ffff:110.242.68.3;type: 28 ::ffff:110.242.68.4;
        for (unsigned int i = 0; i < di.ancount; i++) {
            unsigned answerOffset = 0;
            unsigned type = 0;
            //unsigned xclass = 0;
            //unsigned ttl = 0;
            //unsigned rrlen = 0;
            answerOffset = di.rr_offset[i + 1];
            pxOffset = dns_name_skip(px, answerOffset, pxLength);
            
            if (pxOffset >= pxLength) {
                break;
            }
            
            type = px[pxOffset + 0] << 8 | px[pxOffset + 1];
            //xclass = px[pxOffset + 2] << 8 | px[pxOffset + 3];
            //ttl = px[pxOffset + 4] << 24 | px[pxOffset + 5] << 16 | px[pxOffset + 6] << 8 | px[pxOffset + 7];
            //rrlen = px[pxOffset + 8] << 8 | px[pxOffset + 9];
            
            pxOffset += 10;
            
            switch (type) {
                case ns_t_a: {
                    unsigned int ipv4_address;
                    ipv4_address = *((uint32_t *)(px + pxOffset));
                    ipv4_address = ntohl(ipv4_address);

                    char szIpAddress[16] = { 0 };
                    
                    snprintf(szIpAddress, sizeof(szIpAddress), "%u.%u.%u.%u",
                             (ipv4_address >> 24) & 0xFF,
                             (ipv4_address >> 16) & 0xFF,
                             (ipv4_address >> 8) & 0xFF,
                             ipv4_address & 0xFF);
                    answerName += "type: 1 ";
                    answerName += szIpAddress;
                    answerName += ";";
                }
                break;
                    
                case ns_t_aaaa: {
                    char ipv6Address[16] = {0};
                    char szIpAddress[INET6_ADDRSTRLEN] = {0};
                    memcpy(&ipv6Address, px + pxOffset, 16);
                    inet_ntop(AF_INET6, ipv6Address, szIpAddress, INET6_ADDRSTRLEN);
                    
                    answerName += "type: 28 ";
                    answerName += szIpAddress;
                    answerName += ";";
                }
                break;
                    
                case ns_t_cname: {
                    char* cname = read_rr_name(px, &pxOffset, 0, pxLength);
                    if (cname) {
                        answerName += "type: 5 ";
                        answerName += cname;
                        answerName += ";";
                        free(cname);
                    }
                }
                break;
                    
                default:
                    break;
            }
        }
        rResult = true;
        ParseResult->AnswerName = std::move(answerName);
    } while (false);

    return rResult;
}
