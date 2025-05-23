// ArduinoJson - https://arduinojson.org
// Copyright © 2014-2025, Benoit BLANCHON
// MIT License

#include <ArduinoJson.h>
#include <catch.hpp>

#include "Allocators.hpp"
#include "Literals.hpp"

using ArduinoJson::detail::sizeofObject;

TEST_CASE("deserialize JSON object") {
  SpyingAllocator spy;
  JsonDocument doc(&spy);

  SECTION("An empty object") {
    DeserializationError err = deserializeJson(doc, "{}");
    JsonObject obj = doc.as<JsonObject>();

    REQUIRE(err == DeserializationError::Ok);
    REQUIRE(doc.is<JsonObject>());
    REQUIRE(obj.size() == 0);
  }

  SECTION("Quotes") {
    SECTION("Double quotes") {
      DeserializationError err = deserializeJson(doc, "{\"key\":\"value\"}");
      JsonObject obj = doc.as<JsonObject>();

      REQUIRE(err == DeserializationError::Ok);
      REQUIRE(doc.is<JsonObject>());
      REQUIRE(obj.size() == 1);
      REQUIRE(obj["key"] == "value");
    }

    SECTION("Single quotes") {
      DeserializationError err = deserializeJson(doc, "{'key':'value'}");
      JsonObject obj = doc.as<JsonObject>();

      REQUIRE(err == DeserializationError::Ok);
      REQUIRE(doc.is<JsonObject>());
      REQUIRE(obj.size() == 1);
      REQUIRE(obj["key"] == "value");
    }

    SECTION("No quotes") {
      DeserializationError err = deserializeJson(doc, "{key:'value'}");
      JsonObject obj = doc.as<JsonObject>();

      REQUIRE(err == DeserializationError::Ok);
      REQUIRE(doc.is<JsonObject>());
      REQUIRE(obj.size() == 1);
      REQUIRE(obj["key"] == "value");
    }

    SECTION("No quotes, allow underscore in key") {
      DeserializationError err = deserializeJson(doc, "{_k_e_y_:42}");
      JsonObject obj = doc.as<JsonObject>();

      REQUIRE(err == DeserializationError::Ok);
      REQUIRE(doc.is<JsonObject>());
      REQUIRE(obj.size() == 1);
      REQUIRE(obj["_k_e_y_"] == 42);
    }
  }

  SECTION("Spaces") {
    SECTION("Before the key") {
      DeserializationError err = deserializeJson(doc, "{ \"key\":\"value\"}");
      JsonObject obj = doc.as<JsonObject>();

      REQUIRE(err == DeserializationError::Ok);
      REQUIRE(doc.is<JsonObject>());
      REQUIRE(obj.size() == 1);
      REQUIRE(obj["key"] == "value");
    }

    SECTION("After the key") {
      DeserializationError err = deserializeJson(doc, "{\"key\" :\"value\"}");
      JsonObject obj = doc.as<JsonObject>();

      REQUIRE(err == DeserializationError::Ok);
      REQUIRE(doc.is<JsonObject>());
      REQUIRE(obj.size() == 1);
      REQUIRE(obj["key"] == "value");
    }

    SECTION("Before the value") {
      DeserializationError err = deserializeJson(doc, "{\"key\": \"value\"}");
      JsonObject obj = doc.as<JsonObject>();

      REQUIRE(err == DeserializationError::Ok);
      REQUIRE(doc.is<JsonObject>());
      REQUIRE(obj.size() == 1);
      REQUIRE(obj["key"] == "value");
    }

    SECTION("After the value") {
      DeserializationError err = deserializeJson(doc, "{\"key\":\"value\" }");
      JsonObject obj = doc.as<JsonObject>();

      REQUIRE(err == DeserializationError::Ok);
      REQUIRE(doc.is<JsonObject>());
      REQUIRE(obj.size() == 1);
      REQUIRE(obj["key"] == "value");
    }

    SECTION("Before the comma") {
      DeserializationError err =
          deserializeJson(doc, "{\"key1\":\"value1\" ,\"key2\":\"value2\"}");
      JsonObject obj = doc.as<JsonObject>();

      REQUIRE(err == DeserializationError::Ok);
      REQUIRE(doc.is<JsonObject>());
      REQUIRE(obj.size() == 2);
      REQUIRE(obj["key1"] == "value1");
      REQUIRE(obj["key2"] == "value2");
    }

    SECTION("After the comma") {
      DeserializationError err =
          deserializeJson(doc, "{\"key1\":\"value1\", \"key2\":\"value2\"}");
      JsonObject obj = doc.as<JsonObject>();

      REQUIRE(err == DeserializationError::Ok);
      REQUIRE(doc.is<JsonObject>());
      REQUIRE(obj.size() == 2);
      REQUIRE(obj["key1"] == "value1");
      REQUIRE(obj["key2"] == "value2");
    }
  }

  SECTION("Values types") {
    SECTION("String") {
      DeserializationError err =
          deserializeJson(doc, "{\"key1\":\"value1\",\"key2\":\"value2\"}");
      JsonObject obj = doc.as<JsonObject>();

      REQUIRE(err == DeserializationError::Ok);
      REQUIRE(doc.is<JsonObject>());
      REQUIRE(obj.size() == 2);
      REQUIRE(obj["key1"] == "value1");
      REQUIRE(obj["key2"] == "value2");
    }

    SECTION("Integer") {
      DeserializationError err =
          deserializeJson(doc, "{\"key1\":42,\"key2\":-42}");
      JsonObject obj = doc.as<JsonObject>();

      REQUIRE(err == DeserializationError::Ok);
      REQUIRE(doc.is<JsonObject>());
      REQUIRE(obj.size() == 2);
      REQUIRE(obj["key1"] == 42);
      REQUIRE(obj["key2"] == -42);
    }

    SECTION("Float") {
      DeserializationError err =
          deserializeJson(doc, "{\"key1\":12.345,\"key2\":-7E3}");
      JsonObject obj = doc.as<JsonObject>();

      REQUIRE(err == DeserializationError::Ok);
      REQUIRE(doc.is<JsonObject>());
      REQUIRE(obj.size() == 2);
      REQUIRE(obj["key1"].as<float>() == Approx(12.345f));
      REQUIRE(obj["key2"] == -7E3f);
    }

    SECTION("Double") {
      DeserializationError err =
          deserializeJson(doc, "{\"key1\":12.3456789,\"key2\":-7E89}");
      JsonObject obj = doc.as<JsonObject>();

      REQUIRE(err == DeserializationError::Ok);
      REQUIRE(doc.is<JsonObject>());
      REQUIRE(obj.size() == 2);
      REQUIRE(obj["key1"].as<double>() == Approx(12.3456789));
      REQUIRE(obj["key2"] == -7E89);
    }

    SECTION("Booleans") {
      DeserializationError err =
          deserializeJson(doc, "{\"key1\":true,\"key2\":false}");
      JsonObject obj = doc.as<JsonObject>();

      REQUIRE(err == DeserializationError::Ok);
      REQUIRE(doc.is<JsonObject>());
      REQUIRE(obj.size() == 2);
      REQUIRE(obj["key1"] == true);
      REQUIRE(obj["key2"] == false);
    }

    SECTION("Null") {
      DeserializationError err =
          deserializeJson(doc, "{\"key1\":null,\"key2\":null}");
      JsonObject obj = doc.as<JsonObject>();

      REQUIRE(err == DeserializationError::Ok);
      REQUIRE(doc.is<JsonObject>());
      REQUIRE(obj.size() == 2);
      REQUIRE(obj["key1"].as<const char*>() == 0);
      REQUIRE(obj["key2"].as<const char*>() == 0);
    }

    SECTION("Array") {
      char jsonString[] = " { \"ab\" : [ 1 , 2 ] , \"cd\" : [ 3 , 4 ] } ";

      DeserializationError err = deserializeJson(doc, jsonString);
      JsonObject obj = doc.as<JsonObject>();

      JsonArray array1 = obj["ab"];
      const JsonArray array2 = obj["cd"];
      JsonArray array3 = obj["ef"];

      REQUIRE(err == DeserializationError::Ok);

      REQUIRE(array1.isNull() == false);
      REQUIRE(array2.isNull() == false);
      REQUIRE(array3.isNull() == true);

      REQUIRE(2 == array1.size());
      REQUIRE(2 == array2.size());
      REQUIRE(0 == array3.size());

      REQUIRE(1 == array1[0].as<int>());
      REQUIRE(2 == array1[1].as<int>());

      REQUIRE(3 == array2[0].as<int>());
      REQUIRE(4 == array2[1].as<int>());

      REQUIRE(0 == array3[0].as<int>());
    }
  }

  SECTION("Premature null terminator") {
    SECTION("After opening brace") {
      DeserializationError err = deserializeJson(doc, "{");

      REQUIRE(err == DeserializationError::IncompleteInput);
    }

    SECTION("After key") {
      DeserializationError err = deserializeJson(doc, "{\"hello\"");

      REQUIRE(err == DeserializationError::IncompleteInput);
    }

    SECTION("After colon") {
      DeserializationError err = deserializeJson(doc, "{\"hello\":");

      REQUIRE(err == DeserializationError::IncompleteInput);
    }

    SECTION("After value") {
      DeserializationError err = deserializeJson(doc, "{\"hello\":\"world\"");

      REQUIRE(err == DeserializationError::IncompleteInput);
    }

    SECTION("After comma") {
      DeserializationError err = deserializeJson(doc, "{\"hello\":\"world\",");

      REQUIRE(err == DeserializationError::IncompleteInput);
    }
  }

  SECTION("Misc") {
    SECTION("A quoted key without value") {
      DeserializationError err = deserializeJson(doc, "{\"key\"}");

      REQUIRE(err == DeserializationError::InvalidInput);
    }

    SECTION("A non-quoted key without value") {
      DeserializationError err = deserializeJson(doc, "{key}");

      REQUIRE(err == DeserializationError::InvalidInput);
    }

    SECTION("A dangling comma") {
      DeserializationError err = deserializeJson(doc, "{\"key1\":\"value1\",}");

      REQUIRE(err == DeserializationError::InvalidInput);
    }

    SECTION("null as a key") {
      DeserializationError err = deserializeJson(doc, "{null:\"value\"}");

      REQUIRE(err == DeserializationError::Ok);
    }

    SECTION("Repeated key") {
      DeserializationError err =
          deserializeJson(doc, "{alfa:{bravo:{charlie:1}},alfa:2}");

      REQUIRE(err == DeserializationError::Ok);
      REQUIRE(doc.as<std::string>() == "{\"alfa\":2}");
      REQUIRE(spy.log() ==
              AllocatorLog{
                  Allocate(sizeofStringBuffer()),
                  Allocate(sizeofPool()),
                  Reallocate(sizeofStringBuffer(), sizeofString("alfa")),
                  Allocate(sizeofStringBuffer()),
                  Reallocate(sizeofStringBuffer(), sizeofString("bravo")),
                  Allocate(sizeofStringBuffer()),
                  Reallocate(sizeofStringBuffer(), sizeofString("charlie")),
                  Allocate(sizeofStringBuffer()),
                  Deallocate(sizeofString("bravo")),
                  Deallocate(sizeofString("charlie")),
                  Deallocate(sizeofStringBuffer()),
                  Reallocate(sizeofPool(), sizeofObject(2) + sizeofObject(1)),
              });
    }

    SECTION("Repeated key with zero copy mode") {  // issue #1697
      char input[] = "{a:{b:{c:1}},a:2}";
      DeserializationError err = deserializeJson(doc, input);

      REQUIRE(err == DeserializationError::Ok);
      REQUIRE(doc["a"] == 2);
    }

    SECTION("NUL in keys") {
      DeserializationError err =
          deserializeJson(doc, "{\"x\":0,\"x\\u0000a\":1,\"x\\u0000b\":2}");

      REQUIRE(err == DeserializationError::Ok);
      REQUIRE(doc.as<std::string>() ==
              "{\"x\":0,\"x\\u0000a\":1,\"x\\u0000b\":2}");
    }
  }

  SECTION("Should clear the JsonObject") {
    deserializeJson(doc, "{\"hello\":\"world\"}");
    spy.clearLog();

    deserializeJson(doc, "{}");

    REQUIRE(doc.is<JsonObject>());
    REQUIRE(doc.size() == 0);
    REQUIRE(spy.log() == AllocatorLog{
                             Deallocate(sizeofObject(1)),
                             Deallocate(sizeofString("hello")),
                             Deallocate(sizeofString("world")),
                         });
  }

  SECTION("Issue #1335") {
    std::string json("{\"a\":{},\"b\":{}}");
    deserializeJson(doc, json);
    CHECK(doc.as<std::string>() == json);
  }
}

TEST_CASE("deserialize JSON object under memory constraints") {
  TimebombAllocator timebomb(1024);
  JsonDocument doc(&timebomb);

  SECTION("empty object requires no allocation") {
    timebomb.setCountdown(0);
    char input[] = "{}";

    DeserializationError err = deserializeJson(doc, input);

    REQUIRE(err == DeserializationError::Ok);
    REQUIRE(doc.as<std::string>() == "{}");
  }

  SECTION("key allocation fails") {
    timebomb.setCountdown(0);
    char input[] = "{\"a\":1}";

    DeserializationError err = deserializeJson(doc, input);

    REQUIRE(err == DeserializationError::NoMemory);
    REQUIRE(doc.as<std::string>() == "{}");
  }

  SECTION("pool allocation fails") {
    timebomb.setCountdown(1);
    char input[] = "{\"a\":1}";

    DeserializationError err = deserializeJson(doc, input);

    REQUIRE(err == DeserializationError::NoMemory);
    REQUIRE(doc.as<std::string>() == "{}");
  }

  SECTION("string allocation fails") {
    timebomb.setCountdown(3);
    char input[] = "{\"alfa\":\"bravo\"}";

    DeserializationError err = deserializeJson(doc, input);

    REQUIRE(err == DeserializationError::NoMemory);
    REQUIRE(doc.as<std::string>() == "{\"alfa\":null}");
  }
}
