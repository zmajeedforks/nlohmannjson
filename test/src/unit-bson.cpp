/*
    __ _____ _____ _____
 __|  |   __|     |   | |  JSON for Modern C++ (test suite)
|  |  |__   |  |  | | | |  version 3.2.0
|_____|_____|_____|_|___|  https://github.com/nlohmann/json

Licensed under the MIT License <http://opensource.org/licenses/MIT>.
SPDX-License-Identifier: MIT
Copyright (c) 2013-2018 Niels Lohmann <http://nlohmann.me>.

Permission is hereby  granted, free of charge, to any  person obtaining a copy
of this software and associated  documentation files (the "Software"), to deal
in the Software  without restriction, including without  limitation the rights
to  use, copy,  modify, merge,  publish, distribute,  sublicense, and/or  sell
copies  of  the Software,  and  to  permit persons  to  whom  the Software  is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE  IS PROVIDED "AS  IS", WITHOUT WARRANTY  OF ANY KIND,  EXPRESS OR
IMPLIED,  INCLUDING BUT  NOT  LIMITED TO  THE  WARRANTIES OF  MERCHANTABILITY,
FITNESS FOR  A PARTICULAR PURPOSE AND  NONINFRINGEMENT. IN NO EVENT  SHALL THE
AUTHORS  OR COPYRIGHT  HOLDERS  BE  LIABLE FOR  ANY  CLAIM,  DAMAGES OR  OTHER
LIABILITY, WHETHER IN AN ACTION OF  CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE  OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "catch.hpp"

#include <nlohmann/json.hpp>
using nlohmann::json;

#include <fstream>

TEST_CASE("BSON")
{
    SECTION("individual values not supported")
    {
        SECTION("discarded")
        {
            // discarded values are not serialized
            json j = json::value_t::discarded;
            const auto result = json::to_bson(j);
            CHECK(result.empty());
        }

        SECTION("null")
        {
            json j = nullptr;
            REQUIRE_THROWS_AS(json::to_bson(j), json::type_error);
        }

        SECTION("boolean")
        {
            SECTION("true")
            {
                json j = true;
                REQUIRE_THROWS_AS(json::to_bson(j), json::type_error);
            }

            SECTION("false")
            {
                json j = false;
                REQUIRE_THROWS_AS(json::to_bson(j), json::type_error);
            }
        }

        SECTION("number")
        {
            json j = 42;
            REQUIRE_THROWS_AS(json::to_bson(j), json::type_error);
        }

        SECTION("float")
        {
            json j = 4.2;
            REQUIRE_THROWS_AS(json::to_bson(j), json::type_error);
        }

        SECTION("string")
        {
            json j = "not supported";
            REQUIRE_THROWS_AS(json::to_bson(j), json::type_error);
        }

        SECTION("array")
        {
            json j = std::vector<int> {1, 2, 3, 4, 5, 6, 7};
            REQUIRE_THROWS_AS(json::to_bson(j), json::type_error);
        }
    }

    SECTION("objects")
    {
        SECTION("empty object")
        {
            json j = json::object();
            std::vector<uint8_t> expected =
            {
                0x05, 0x00, 0x00, 0x00, // size (little endian)
                // no entries
                0x00 // end marker
            };

            const auto result = json::to_bson(j);
            CHECK(result == expected);

            // roundtrip
            CHECK(json::from_bson(result) == j);
            CHECK(json::from_bson(result, true, false) == j);
        }

        SECTION("non-empty object with bool")
        {
            json j =
            {
                { "entry", true }
            };

            std::vector<uint8_t> expected =
            {
                0x0D, 0x00, 0x00, 0x00, // size (little endian)
                0x08,               // entry: boolean
                'e', 'n', 't', 'r', 'y', '\x00',
                0x01,           // value = true
                0x00                    // end marker
            };

            const auto result = json::to_bson(j);
            CHECK(result == expected);

            // roundtrip
            CHECK(json::from_bson(result) == j);
            CHECK(json::from_bson(result, true, false) == j);
        }

        SECTION("non-empty object with bool")
        {
            json j =
            {
                { "entry", false }
            };

            std::vector<uint8_t> expected =
            {
                0x0D, 0x00, 0x00, 0x00, // size (little endian)
                0x08,               // entry: boolean
                'e', 'n', 't', 'r', 'y', '\x00',
                0x00,           // value = false
                0x00                    // end marker
            };

            const auto result = json::to_bson(j);
            CHECK(result == expected);

            // roundtrip
            CHECK(json::from_bson(result) == j);
            CHECK(json::from_bson(result, true, false) == j);
        }

        SECTION("non-empty object with double")
        {
            json j =
            {
                { "entry", 4.2 }
            };

            std::vector<uint8_t> expected =
            {
                0x14, 0x00, 0x00, 0x00, // size (little endian)
                0x01, /// entry: double
                'e', 'n', 't', 'r', 'y', '\x00',
                0xcd, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0x10, 0x40,
                0x00 // end marker
            };

            const auto result = json::to_bson(j);
            CHECK(result == expected);

            // roundtrip
            CHECK(json::from_bson(result) == j);
            CHECK(json::from_bson(result, true, false) == j);
        }

        SECTION("non-empty object with string")
        {
            json j =
            {
                { "entry", "bsonstr" }
            };

            std::vector<uint8_t> expected =
            {
                0x18, 0x00, 0x00, 0x00, // size (little endian)
                0x02, /// entry: string (UTF-8)
                'e', 'n', 't', 'r', 'y', '\x00',
                0x08, 0x00, 0x00, 0x00, 'b', 's', 'o', 'n', 's', 't', 'r', '\x00',
                0x00 // end marker
            };

            const auto result = json::to_bson(j);
            CHECK(result == expected);

            // roundtrip
            CHECK(json::from_bson(result) == j);
            CHECK(json::from_bson(result, true, false) == j);
        }

        SECTION("non-empty object with null member")
        {
            json j =
            {
                { "entry", nullptr }
            };

            std::vector<uint8_t> expected =
            {
                0x0C, 0x00, 0x00, 0x00, // size (little endian)
                0x0A, /// entry: null
                'e', 'n', 't', 'r', 'y', '\x00',
                0x00 // end marker
            };

            const auto result = json::to_bson(j);
            CHECK(result == expected);

            // roundtrip
            CHECK(json::from_bson(result) == j);
            CHECK(json::from_bson(result, true, false) == j);
        }

        SECTION("non-empty object with integer (32-bit) member")
        {
            json j =
            {
                { "entry", std::int32_t{0x12345678} }
            };

            std::vector<uint8_t> expected =
            {
                0x10, 0x00, 0x00, 0x00, // size (little endian)
                0x10, /// entry: int32
                'e', 'n', 't', 'r', 'y', '\x00',
                0x78, 0x56, 0x34, 0x12,
                0x00 // end marker
            };

            const auto result = json::to_bson(j);
            CHECK(result == expected);

            // roundtrip
            CHECK(json::from_bson(result) == j);
            CHECK(json::from_bson(result, true, false) == j);
        }

        SECTION("non-empty object with integer (64-bit) member")
        {
            json j =
            {
                { "entry", std::int64_t{0x1234567804030201} }
            };

            std::vector<uint8_t> expected =
            {
                0x14, 0x00, 0x00, 0x00, // size (little endian)
                0x12, /// entry: int64
                'e', 'n', 't', 'r', 'y', '\x00',
                0x01, 0x02, 0x03, 0x04, 0x78, 0x56, 0x34, 0x12,
                0x00 // end marker
            };

            const auto result = json::to_bson(j);
            CHECK(result == expected);

            // roundtrip
            CHECK(json::from_bson(result) == j);
            CHECK(json::from_bson(result, true, false) == j);
        }

        SECTION("non-empty object with negative integer (32-bit) member")
        {
            json j =
            {
                { "entry", std::int32_t{-1} }
            };

            std::vector<uint8_t> expected =
            {
                0x10, 0x00, 0x00, 0x00, // size (little endian)
                0x10, /// entry: int32
                'e', 'n', 't', 'r', 'y', '\x00',
                0xFF, 0xFF, 0xFF, 0xFF,
                0x00 // end marker
            };

            const auto result = json::to_bson(j);
            CHECK(result == expected);

            // roundtrip
            CHECK(json::from_bson(result) == j);
            CHECK(json::from_bson(result, true, false) == j);
        }

        SECTION("non-empty object with negative integer (64-bit) member")
        {
            json j =
            {
                { "entry", std::int64_t{-1} }
            };

            std::vector<uint8_t> expected =
            {
                0x10, 0x00, 0x00, 0x00, // size (little endian)
                0x10, /// entry: int32
                'e', 'n', 't', 'r', 'y', '\x00',
                0xFF, 0xFF, 0xFF, 0xFF,
                0x00 // end marker
            };

            const auto result = json::to_bson(j);
            CHECK(result == expected);

            // roundtrip
            CHECK(json::from_bson(result) == j);
            CHECK(json::from_bson(result, true, false) == j);
        }

        SECTION("non-empty object with unsigned integer (64-bit) member")
        {
            // directly encoding uint64 is not supported in bson (only for timestamp values)
            json j =
            {
                { "entry", std::uint64_t{0x1234567804030201} }
            };

            std::vector<uint8_t> expected =
            {
                0x14, 0x00, 0x00, 0x00, // size (little endian)
                0x12, /// entry: int64
                'e', 'n', 't', 'r', 'y', '\x00',
                0x01, 0x02, 0x03, 0x04, 0x78, 0x56, 0x34, 0x12,
                0x00 // end marker
            };

            const auto result = json::to_bson(j);
            CHECK(result == expected);

            // roundtrip
            CHECK(json::from_bson(result) == j);
            CHECK(json::from_bson(result, true, false) == j);
        }

        SECTION("non-empty object with object member")
        {
            json j =
            {
                { "entry", json::object() }
            };

            std::vector<uint8_t> expected =
            {
                0x11, 0x00, 0x00, 0x00, // size (little endian)
                0x03, /// entry: embedded document
                'e', 'n', 't', 'r', 'y', '\x00',

                0x05, 0x00, 0x00, 0x00, // size (little endian)
                // no entries
                0x00, // end marker (embedded document)

                0x00 // end marker
            };

            const auto result = json::to_bson(j);
            CHECK(result == expected);

            // roundtrip
            CHECK(json::from_bson(result) == j);
            CHECK(json::from_bson(result, true, false) == j);
        }

        SECTION("non-empty object with array member")
        {
            json j =
            {
                { "entry", json::array() }
            };

            std::vector<uint8_t> expected =
            {
                0x11, 0x00, 0x00, 0x00, // size (little endian)
                0x04, /// entry: embedded document
                'e', 'n', 't', 'r', 'y', '\x00',

                0x05, 0x00, 0x00, 0x00, // size (little endian)
                // no entries
                0x00, // end marker (embedded document)

                0x00 // end marker
            };

            const auto result = json::to_bson(j);
            CHECK(result == expected);

            // roundtrip
            CHECK(json::from_bson(result) == j);
            CHECK(json::from_bson(result, true, false) == j);
        }

        SECTION("non-empty object with non-empty array member")
        {
            json j =
            {
                { "entry", json::array({1, 2, 3, 4, 5, 6, 7, 8}) }
            };

            std::vector<uint8_t> expected =
            {
                0x41, 0x00, 0x00, 0x00, // size (little endian)
                0x04, /// entry: embedded document
                'e', 'n', 't', 'r', 'y', '\x00',

                0x35, 0x00, 0x00, 0x00, // size (little endian)
                0x10, 0x00, 0x01, 0x00, 0x00, 0x00,
                0x10, 0x00, 0x02, 0x00, 0x00, 0x00,
                0x10, 0x00, 0x03, 0x00, 0x00, 0x00,
                0x10, 0x00, 0x04, 0x00, 0x00, 0x00,
                0x10, 0x00, 0x05, 0x00, 0x00, 0x00,
                0x10, 0x00, 0x06, 0x00, 0x00, 0x00,
                0x10, 0x00, 0x07, 0x00, 0x00, 0x00,
                0x10, 0x00, 0x08, 0x00, 0x00, 0x00,
                0x00, // end marker (embedded document)

                0x00 // end marker
            };

            const auto result = json::to_bson(j);
            CHECK(result == expected);

            // roundtrip
            CHECK(json::from_bson(result) == j);
            CHECK(json::from_bson(result, true, false) == j);
        }

        SECTION("Some more complex document")
        {
            // directly encoding uint64 is not supported in bson (only for timestamp values)
            json j =
            {
                {"double", 42.5},
                {"entry", 4.2},
                {"number", 12345},
                {"object", {{ "string", "value" }}}
            };

            std::vector<uint8_t> expected =
            {
                /*size */ 0x4f, 0x00, 0x00, 0x00,
                /*entry*/ 0x01, 'd',  'o',  'u',  'b',  'l',  'e',  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x45, 0x40,
                /*entry*/ 0x01, 'e',  'n',  't',  'r',  'y',  0x00, 0xcd, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0x10, 0x40,
                /*entry*/ 0x10, 'n',  'u',  'm',  'b',  'e',  'r',  0x00, 0x39, 0x30, 0x00, 0x00,
                /*entry*/ 0x03, 'o',  'b',  'j',  'e',  'c',  't',  0x00,
                /*entry: obj-size */ 0x17, 0x00, 0x00, 0x00,
                /*entry: obj-entry*/0x02, 's',  't',  'r',  'i',  'n',  'g', 0x00, 0x06, 0x00, 0x00, 0x00, 'v', 'a', 'l', 'u', 'e', 0,
                /*entry: obj-term.*/0x00,
                /*obj-term*/ 0x00
            };

            const auto result = json::to_bson(j);
            CHECK(result == expected);

            // roundtrip
            CHECK(json::from_bson(result) == j);
            CHECK(json::from_bson(result, true, false) == j);
        }
    }
}
