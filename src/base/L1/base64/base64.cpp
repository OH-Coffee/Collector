
#include <string>
#include "base64.hpp"

std::string base64_encode(const std::string& in_data)
{
    if (in_data.empty()) { return ""; }

    std::string out_data;

    int out_len = base64_enc_len(static_cast<int>(in_data.size()));
    out_data.resize(static_cast<std::string::size_type>(out_len), '\0');
    base64_encode(const_cast<char*>(out_data.data()),
                  const_cast<char*>(in_data.data()),
                  static_cast<int>(in_data.size()));

    return out_data;
}

std::string base64_decode(const std::string& in_data)
{
    if (in_data.empty()) {return ""; }

    std::string out_data;
    int out_len = base64_dec_len(const_cast<char*>(in_data.c_str()),
                                 static_cast<int>(in_data.size()));

    out_data.resize(static_cast<std::string::size_type>(out_len), '\0');

    base64_decode(const_cast<char*>(out_data.data()),
                  const_cast<char*>(in_data.data()),
                  static_cast<int>(in_data.size()));

    return out_data;
}


const char b64_alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                            "abcdefghijklmnopqrstuvwxyz"
                            "0123456789+/";

inline void a3_to_a4(unsigned char * a4, const unsigned char * a3);
inline void a4_to_a3(unsigned char * a3, const unsigned char * a4);
inline unsigned char b64_lookup(char c);

int base64_encode(char *output, const char *input, int inputLen)
{
    int i = 0, j = 0;
    int encLen = 0;
    unsigned char a3[3];
    unsigned char a4[4];

    while (inputLen--) {
        a3[i++] = static_cast<unsigned char>(*(input++));
        if (i == 3) {
            a3_to_a4(a4, a3);

            for (i = 0; i < 4; i++) {
                output[encLen++] = b64_alphabet[a4[i]];
            }

            i = 0;
        }
    }

    if (i) {
        for (j = i; j < 3; j++) { a3[j] = '\0'; }

        a3_to_a4(a4, a3);

        for (j = 0; j < i + 1; j++) { output[encLen++] = b64_alphabet[a4[j]]; }

        while ((i++ < 3)) { output[encLen++] = '='; }
    }

    return encLen;
}

int base64_decode(char *output, const char * input, int inputLen)
{
    int i = 0, j = 0;
    int decLen = 0;
    unsigned char a3[3] = {0, 0, 0};
    unsigned char a4[4] = {0, 0, 0};

    while (inputLen--) {
        if (*input == '=') { break; }

        a4[i++] = static_cast<unsigned char>(*(input++));
        if (i == 4) {
            for (i = 0; i < 4; i++) {
                a4[i] = b64_lookup(static_cast<char>(a4[i]));
            }

            a4_to_a3(a3, a4);

            for (i = 0; i < 3; i++) {
                output[decLen++] = static_cast<char>(a3[i]);
            }
            i = 0;
        }
    }

    if (i) {
        for (j = i; j < 4; j++) { a4[j] = '\0'; }

        for (j = 0; j < 4; j++) {
            a4[j] = b64_lookup(static_cast<char>(a4[j]));
        }

        a4_to_a3(a3, a4);

        for (j = 0; j < i - 1; j++) {
            output[decLen++] = static_cast<char>(a3[j]);
        }
    }

    return decLen;
}

int base64_enc_len(int plainLen)
{
    return (plainLen + 2 - ((plainLen + 2) % 3)) / 3 * 4;
}

int base64_dec_len(const char *input, int inputLen)
{
    int i     = 0;
    int numEq = 0;

    for (i = inputLen - 1; input[i] == '='; i--) { numEq++; }

    return ((6 * inputLen) / 8) - numEq;
}

inline void a3_to_a4(unsigned char * a4, const unsigned char * a3)
{
    a4[0] = static_cast<unsigned char>((a3[0] & 0xfc) >> 2);
    a4[1] = static_cast<unsigned char>(((a3[0] & 0x03) << 4) + ((a3[1] & 0xf0) >> 4));
    a4[2] = static_cast<unsigned char>(((a3[1] & 0x0f) << 2) + ((a3[2] & 0xc0) >> 6));
    a4[3] = static_cast<unsigned char>((a3[2] & 0x3f));
}

inline void a4_to_a3(unsigned char * a3, const unsigned char * a4)
{
    a3[0] = static_cast<unsigned char>((a4[0] << 2) + ((a4[1] & 0x30) >> 4));
    a3[1] = static_cast<unsigned char>(((a4[1] & 0xf) << 4) + ((a4[2] & 0x3c) >> 2));
    a3[2] = static_cast<unsigned char>(((a4[2] & 0x3) << 6) + a4[3]);
}

inline unsigned char b64_lookup(char c)
{
    if (c >= 'A' && c <= 'Z') { return static_cast<unsigned char>(c - 'A'); }
    if (c >= 'a' && c <= 'z') { return static_cast<unsigned char>(c - 71); }
    if (c >= '0' && c <= '9') { return static_cast<unsigned char>(c + 4); }

    if (c == '+') { return 62; }
    if (c == '/') { return 63; }

    return static_cast<unsigned char>(-1);
}
