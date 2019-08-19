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

        std::string welcome = ", welcome back!";

        node.AppendText("Hello ")
            .AppendChild(CTML::Node("span", "Maxwell"))
            .AppendText(welcome);

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

    SECTION("remove child by index")
    {
        CTML::Node node("div");

        node.AppendChild(CTML::Node("p", "hello world!"))
            .AppendChild(CTML::Node("span", "this is a removal test"))
            .AppendChild(CTML::Node("a", "i should be removed"));

        node.RemoveChild(2);

        REQUIRE(node.ToString() == "<div><p>hello world!</p><span>this is a removal test</span></div>");
    }

    SECTION("remove child by basic selector")
    {
        CTML::Node node("div");

        node.AppendChild(CTML::Node("p.not-right#child", "hello world! don't remove me!"))
            .AppendChild(CTML::Node("p.nice-one", "don't remove me either!"))
            .AppendChild(CTML::Node("p.bad-one", "remove me!"));

        node.RemoveChild("p.bad-one");

        REQUIRE(node.ToString() == "<div><p class=\"not-right\" id=\"child\">hello world! don't remove me!</p><p class=\"nice-one\">don't remove me either!</p></div>");
    }

    SECTION("attributes escaped")
    {
        CTML::Node node("p", "Hello world!");

        node.SetAttribute("title", "\"Hello world\"")
            .SetAttribute("onclick", "<onclick()>");

        REQUIRE(node.ToString() == "<p onclick=\"&lt;onclick()&gt;\" title=\"&quot;Hello world&quot;\">Hello world!</p>");
    }

    SECTION("blank attribute output as name only")
    {
        CTML::Node node("button", "Log in");

        node.SetAttribute("disabled", "");

        REQUIRE(node.ToString() == "<button disabled>Log in</button>");
    }

    SECTION("multiple line to string")
    {
        CTML::Document document;

        document.AppendNodeToBody(CTML::Node("a"));
        document.AppendNodeToBody(CTML::Node("div").AppendChild(CTML::Node("a")));
        document.AppendNodeToBody(CTML::Node("nav"));

        REQUIRE(document.ToString(CTML::ToStringOptions(CTML::StringFormatting::MULTIPLE_LINES)) == R"(<!DOCTYPE html>
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