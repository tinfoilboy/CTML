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

    REQUIRE(node.ToString() == "<p class=\"class test\" id=\"test\" disabled title=\"test title\">Hello world!</p>");
  }
}