// ArduinoJson - https://arduinojson.org
// Copyright © 2014-2025, Benoit BLANCHON
// MIT License

#include <ArduinoJson.h>
#include <catch.hpp>

#include "Literals.hpp"

template <typename T>
static void checkVariant(T value, const char* expected_data,
                         size_t expected_len) {
  JsonDocument doc;
  JsonVariant variant = doc.to<JsonVariant>();
  variant.set(value);
  std::string expected(expected_data, expected_data + expected_len);
  std::string actual;
  size_t len = serializeMsgPack(variant, actual);
  CAPTURE(variant);
  REQUIRE(len == expected_len);
  REQUIRE(actual == expected);
}

template <typename T, size_t N>
static void checkVariant(T value, const char (&expected_data)[N]) {
  const size_t expected_len = N - 1;
  checkVariant(value, expected_data, expected_len);
}

template <typename T>
static void checkVariant(T value, const std::string& expected) {
  checkVariant(value, expected.data(), expected.length());
}

TEST_CASE("serialize MsgPack value") {
  SECTION("unbound") {
    checkVariant(JsonVariant(), "\xC0");  // we represent undefined as nil
  }

  SECTION("nil") {
    const char* nil = 0;  // ArduinoJson uses a string for null
    checkVariant(nil, "\xC0");
  }

  SECTION("bool") {
    checkVariant(false, "\xC2");
    checkVariant(true, "\xC3");
  }

  SECTION("positive fixint") {
    SECTION("signed") {
      checkVariant(0, "\x00");
      checkVariant(127, "\x7F");
    }
    SECTION("unsigned") {
      checkVariant(0U, "\x00");
      checkVariant(127U, "\x7F");
    }
  }

  SECTION("uint 8") {
    checkVariant(128, "\xCC\x80");
    checkVariant(255, "\xCC\xFF");
  }

  SECTION("uint 16") {
    checkVariant(256, "\xCD\x01\x00");
    checkVariant(0xFFFF, "\xCD\xFF\xFF");
  }

  SECTION("uint 32") {
    checkVariant(0x00010000U, "\xCE\x00\x01\x00\x00");
    checkVariant(0x12345678U, "\xCE\x12\x34\x56\x78");
    checkVariant(0xFFFFFFFFU, "\xCE\xFF\xFF\xFF\xFF");
  }

#if ARDUINOJSON_USE_LONG_LONG
  SECTION("uint 64") {
    checkVariant(0x0001000000000000U, "\xCF\x00\x01\x00\x00\x00\x00\x00\x00");
    checkVariant(0x123456789ABCDEF0U, "\xCF\x12\x34\x56\x78\x9A\xBC\xDE\xF0");
    checkVariant(0xFFFFFFFFFFFFFFFFU, "\xCF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF");
  }
#endif

  SECTION("negative fixint") {
    checkVariant(-1, "\xFF");
    checkVariant(-32, "\xE0");
  }

  SECTION("int 8") {
    checkVariant(-33, "\xD0\xDF");
    checkVariant(-128, "\xD0\x80");
  }

  SECTION("int 16") {
    checkVariant(-129, "\xD1\xFF\x7F");
    checkVariant(-32768, "\xD1\x80\x00");
  }

  SECTION("int 32") {
    checkVariant(-32769, "\xD2\xFF\xFF\x7F\xFF");
    checkVariant(-2147483647 - 1, "\xD2\x80\x00\x00\x00");
  }

#if ARDUINOJSON_USE_LONG_LONG
  SECTION("int 64") {
    checkVariant(int64_t(0xFEDCBA9876543210),
                 "\xD3\xFE\xDC\xBA\x98\x76\x54\x32\x10");
  }
#endif

  SECTION("float 32") {
    checkVariant(1.25, "\xCA\x3F\xA0\x00\x00");
    checkVariant(9.22337204e+18f, "\xca\x5f\x00\x00\x00");
  }

  SECTION("float 64") {
    checkVariant(3.1415, "\xCB\x40\x09\x21\xCA\xC0\x83\x12\x6F");
  }

  SECTION("fixstr") {
    checkVariant("", "\xA0");
    checkVariant("hello world hello world hello !",
                 "\xBFhello world hello world hello !");
  }

  SECTION("str 8") {
    checkVariant("hello world hello world hello !!",
                 "\xD9\x20hello world hello world hello !!");
  }

  SECTION("str 16") {
    std::string shortest(256, '?');
    checkVariant(shortest.c_str(), "\xDA\x01\x00"_s + shortest);

    std::string longest(65535, '?');
    checkVariant(longest.c_str(), "\xDA\xFF\xFF"_s + longest);
  }

  SECTION("str 32") {
    std::string shortest(65536, '?');
    checkVariant(JsonString(shortest.c_str(), true),  // force store by pointer
                 "\xDB\x00\x01\x00\x00"_s + shortest);
  }

  SECTION("serialized(const char*)") {
    checkVariant(serialized("\xDA\xFF\xFF"), "\xDA\xFF\xFF");
    checkVariant(serialized("\xDB\x00\x01\x00\x00", 5), "\xDB\x00\x01\x00\x00");
  }

  SECTION("bin 8") {
    checkVariant(MsgPackBinary("?", 1), "\xC4\x01?");
  }

  SECTION("bin 16") {
    auto str = std::string(256, '?');
    checkVariant(MsgPackBinary(str.data(), str.size()), "\xC5\x01\x00"_s + str);
  }

  // bin 32 is tested in string_length_size_4.cpp

  SECTION("fixext 1") {
    checkVariant(MsgPackExtension(1, "\x02", 1), "\xD4\x01\x02");
  }

  SECTION("fixext 2") {
    checkVariant(MsgPackExtension(1, "\x03\x04", 2), "\xD5\x01\x03\x04");
  }

  SECTION("fixext 4") {
    checkVariant(MsgPackExtension(1, "\x05\x06\x07\x08", 4),
                 "\xD6\x01\x05\x06\x07\x08");
  }

  SECTION("fixext 8") {
    checkVariant(MsgPackExtension(1, "????????", 8), "\xD7\x01????????");
  }

  SECTION("fixext 16") {
    checkVariant(MsgPackExtension(1, "????????????????", 16),
                 "\xD8\x01????????????????");
  }

  SECTION("ext 8") {
    checkVariant(MsgPackExtension(2, "???", 3), "\xC7\x03\x02???");
    checkVariant(MsgPackExtension(2, "?????", 5), "\xC7\x05\x02?????");
    checkVariant(MsgPackExtension(2, "???????", 7), "\xC7\x07\x02???????");
    checkVariant(MsgPackExtension(2, "?????????", 9), "\xC7\x09\x02?????????");
    checkVariant(MsgPackExtension(2, "???????????????", 15),
                 "\xC7\x0F\x02???????????????");
    checkVariant(MsgPackExtension(2, "?????????????????", 17),
                 "\xC7\x11\x02?????????????????");
  }

  SECTION("ext 16") {
    auto str = std::string(256, '?');
    checkVariant(MsgPackExtension(2, str.data(), str.size()),
                 "\xC8\x01\x00\x02"_s + str);
  }

  SECTION("serialize round double as integer") {  // Issue #1718
    checkVariant(-32768.0, "\xD1\x80\x00");
    checkVariant(-129.0, "\xD1\xFF\x7F");
    checkVariant(-128.0, "\xD0\x80");
    checkVariant(-33.0, "\xD0\xDF");
    checkVariant(-32.0, "\xE0");
    checkVariant(-1.0, "\xFF");
    checkVariant(0.0, "\x00");
    checkVariant(127.0, "\x7F");
    checkVariant(128.0, "\xCC\x80");
    checkVariant(255.0, "\xCC\xFF");
    checkVariant(256.0, "\xCD\x01\x00");
  }
}
