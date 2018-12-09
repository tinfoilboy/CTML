#define CATCH_CONFIG_MAIN

#include <ctml.hpp>
#include "catch.hpp"

TEST_CASE("nodes are constructed correctly", "[node_construct]")
{
    SECTION("basic blank element node constructed correctly")
    {
        CTML::Node node("p");

        REQUIRE(node.ToString() == "<p></p>");
    }

    SECTION("basic node with constructor content constructed correctly")
    {
        CTML::Node node("p", "Hello world!");

        REQUIRE(node.ToString() == "<p>Hello world!</p>");
    }

    SECTION("basic node with constructor content and classes/id constructed correctly")
    {
        CTML::Node node("p.class.test#test", "Hello world!");

        REQUIRE(node.ToString() == "<p class=\"class test\" id=\"test\">Hello world!</p>");
    }
    
    SECTION("basic node with constructor content, classes/id, and attributes constructed correctly")
    {
        CTML::Node node("p.class.test#test", "Hello world!");

        node.SetAttribute("title", "test title")
            .SetAttribute("onclick", "onclick()");

        REQUIRE(node.ToString() == "<p class=\"class test\" id=\"test\" onclick=\"onclick()\" title=\"test title\">Hello world!</p>");
    }

    SECTION("toggle class removes and adds correctly")
    {
        CTML::Node node("p.class");

        node.ToggleClass("paragraph");

        REQUIRE(node.ToString() == "<p class=\"class paragraph\"></p>");

        node.ToggleClass("class");

        REQUIRE(node.ToString() == "<p class=\"paragraph\"></p>");
    }

    SECTION("append nodes and text in place correctly")
    {
        CTML::Node node("div");

        node.AppendText("Hello ")
            .AppendChild(CTML::Node("span", "Maxwell"))
            .AppendText(", welcome back!");

        REQUIRE(node.ToString() == "<div>Hello <span>Maxwell</span>, welcome back!</div>");
    }

    SECTION("selector generation is correct")
    {
        CTML::Node node("p.class.names#identify");
    
        REQUIRE(node.GetSelector() == "p.class.names#identify");
    }

    SECTION("grab a child by name")
    {
        CTML::Node node("div");

        node.AppendChild(CTML::Node("section"))
            .AppendChild(CTML::Node("article"))
            .AppendChild(CTML::Node("nav"));

        REQUIRE(node.GetChildByName("nav").Name() == "nav");
    }

    SECTION("no closing tag supported")
    {
        CTML::Node node("img");
        
        node.SetAttribute("alt", "an image")
            .SetAttribute("src", "image.png")
            .UseClosingTag(false);

        REQUIRE(node.ToString() == "<img src=\"image.png\" alt=\"an image\">");
    }

    SECTION("multiple line to string")
    {
        CTML::Document document;

        document.AppendNodeToBody(CTML::Node("a"));
        document.AppendNodeToBody(CTML::Node("div").AppendChild(CTML::Node("a")));
        document.AppendNodeToBody(CTML::Node("nav"));

        REQUIRE(document.ToString(CTML::StringFormatting::MULTIPLE_LINES) == R"(<!DOCTYPE html>
<html>
    <head>
    </head>
    <body>
        <a>
        </a>
        <div>
            <a>
            </a>
        </div>
        <nav>
        </nav>
    </body>
</html>)");
    }
}