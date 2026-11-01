#include <array>
#include <cstddef>
#include <string>
#include <utility>

#include "gtest/gtest.h"
#include "base64/base64.hpp"

namespace {

constexpr std::array<std::pair<const char*, const char*>, 7> g_base64_vectors{{
    {"", ""},
    {"f", "Zg=="},
    {"fo", "Zm8="},
    {"foo", "Zm9v"},
    {"foob", "Zm9vYg=="},
    {"fooba", "Zm9vYmE="},
    {"foobar", "Zm9vYmFy"},
}};

TEST(Base64Test, EncodesStandardVectors)
{
    for (const auto& [plain, encoded] : g_base64_vectors) {
        EXPECT_EQ(base64_encode(plain), encoded) << "input: " << plain;
    }
}

TEST(Base64Test, DecodesStandardVectors)
{
    for (const auto& [plain, encoded] : g_base64_vectors) {
        EXPECT_EQ(base64_decode(encoded), plain) << "input: " << encoded;
    }
}

TEST(Base64Test, RoundTripsBinaryData)
{
    const std::string input{"\0\x01\x7f\x80\xff", 5};

    EXPECT_EQ(base64_decode(base64_encode(input)), input);
}

TEST(Base64Test, RawBufferInterfacesReportWrittenLengths)
{
    const std::string input{"\0\x01\x7f\x80\xff", 5};
    std::string encoded(static_cast<std::size_t>(base64_enc_len(static_cast<int>(input.size()))), '\0');

    const int encoded_length = base64_encode(encoded.data(), input.data(), static_cast<int>(input.size()));
    ASSERT_EQ(encoded_length, static_cast<int>(encoded.size()));
    EXPECT_EQ(encoded, "AAF/gP8=");
    EXPECT_EQ(base64_dec_len(encoded.data(), encoded_length), static_cast<int>(input.size()));

    std::string decoded(static_cast<std::size_t>(base64_dec_len(encoded.data(), encoded_length)), '\0');
    const int decoded_length = base64_decode(decoded.data(), encoded.data(), encoded_length);

    ASSERT_EQ(decoded_length, static_cast<int>(input.size()));
    EXPECT_EQ(decoded, input);
}

} // namespace
