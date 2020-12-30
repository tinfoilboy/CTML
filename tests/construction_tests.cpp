#include <ctml.hpp>
#include "catch.hpp"

TEST_CASE("nodes are constructed correctly", "[node_construct]") {
    SECTION("basic blank element node constructed correctly") {
        CTML::Node node("p");

        REQUIRE(node.ToString() == "<p></p>");
    }

    SECTION("basic node with constructor content constructed correctly") {
        CTML::Node node("p", "Hello world!");

        REQUIRE(node.ToString() == "<p>Hello world!</p>");
    }

    SECTION("basic node with constructor content and classes and id constructed correctly") {
        CTML::Node node("p.class.test#test", "Hello world!");

        REQUIRE(node.ToString() == "<p class=\"class test\" id=\"test\">Hello world!</p>");
    }

    SECTION("basic node with constructor content, classes, id, and attributes constructed correctly") {
        CTML::Node node("p.class.test#test[title=\"test title\"][disabled]", "Hello world!");

        // attribute order does not matter for this test, so just have both orders
        bool nodeStrEqual = (node.ToString() == "<p class=\"class test\" id=\"test\" disabled title=\"test title\">Hello world!</p>") ||
                            (node.ToString() == "<p class=\"class test\" id=\"test\" title=\"test title\" disabled>Hello world!</p>");

        REQUIRE(nodeStrEqual == true);
    }

    SECTION("multiple node construction from selector-based name") {
        CTML::Node node("p.test div.nested section.selectors", "");

        REQUIRE(node.ToString() == "<p class=\"test\"><div class=\"nested\"><section class=\"selectors\"></section></div></p>");
    }
}