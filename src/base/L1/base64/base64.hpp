
#ifndef base64_base64_hpp
#define base64_base64_hpp

#include <cstdio>
#include <string>

std::string base64_encode(const std::string& in_data);
std::string base64_decode(const std::string& in_data);

int base64_encode(char *output, const char *input, int inputLen);
int base64_decode(char *output, const char *input, int inputLen);

int base64_enc_len(int inputLen);
int base64_dec_len(const char *input, int inputLen);

#endif // base64_base64_hpp

