#define CATCH_CONFIG_MAIN

#include <ctml.hpp>
#include "catch.hpp"

TEST_CASE("nodes behave correctly", "[node_behavior]")
{
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

        // attribute order does not matter, so just have both orders potentially for this case
        bool nodeStrEqual = (node.ToString() == "<img src=\"image.png\" alt=\"an image\">") ||
                            (node.ToString() == "<img alt=\"an image\" src=\"image.png\">");

        REQUIRE(nodeStrEqual == true);
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

    SECTION("remove node in place")
    {
        CTML::Node node("div");

        CTML::Node badNode("p.bad-one", "remove me!");

        node.AppendChild(CTML::Node("p.not-right#child", "hello world! don't remove me!"))
            .AppendChild(CTML::Node("p.nice-one", "don't remove me either!"))
            .AppendChild(badNode);

        badNode.Remove();

        REQUIRE(node.ToString() == "<div><p class=\"not-right\" id=\"child\">hello world! don't remove me!</p><p class=\"nice-one\">don't remove me either!</p></div>");
    }

    SECTION("attributes escaped")
    {
        CTML::Node node("p", "Hello world!");

        node.SetAttribute("title", "\"Hello world\"")
            .SetAttribute("onclick", "<onclick()>");

        // attribute order does not matter for this test case, so have both orders for this check
        bool nodeStrEquals = (node.ToString() == "<p onclick=\"&lt;onclick()&gt;\" title=\"&quot;Hello world&quot;\">Hello world!</p>") ||
                             (node.ToString() == "<p title=\"&quot;Hello world&quot;\" onclick=\"&lt;onclick()&gt;\">Hello world!</p>");

        REQUIRE(nodeStrEquals == true);
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
        document.AppendNodeToBody(CTML::Node("div a"));
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

    SECTION("search by selector recurses correctly")
    {
        CTML::Document document;

        document.AppendNodeToBody(CTML::Node("div.one div.two div.three"));
        document.AppendNodeToBody(CTML::Node("div.two div.one div.three"));
        document.AppendNodeToBody(CTML::Node("div.three div.two div.one"));
        document.AppendNodeToBody(CTML::Node("div.three div.one div.two"));
        document.AppendNodeToBody(CTML::Node("div.four div.five section.needle div.six"));
        document.AppendNodeToBody(CTML::Node("div.seven div.eight section.needle div.nine"));
        document.AppendNodeToBody(CTML::Node("section.needle section.needle div.ten div.eleven"));

        auto divMatches = document.QuerySelector("div.one");

        REQUIRE(divMatches.size() == 4);

        auto needleMatches = document.QuerySelector(".needle");

        REQUIRE(needleMatches.size() == 4);
    }

    SECTION("search by selector match by class and attribute")
    {
        CTML::Document document;

        document.AppendNodeToBody(CTML::Node("div.one div.two div.three div.five[data-test=\"do not find\"]"));
        document.AppendNodeToBody(CTML::Node("div.four div.five[data-test=\"find\"] div.six"));

        auto matches = document.QuerySelector(".five[data-test=\"find\"]");

        REQUIRE(matches.size() == 1);
    }
    
    SECTION("search by selector match by element name")
    {
        CTML::Document document;

        document.AppendNodeToBody(CTML::Node("div span section article header"));
        document.AppendNodeToBody(CTML::Node("video audio section ul table"));

        auto matches = document.QuerySelector("section");

        REQUIRE(matches.size() == 2);
    }

    SECTION("search by selector match by id")
    {
        CTML::Document document;

        document.AppendNodeToBody(CTML::Node("div#one div#two div#three"));
        document.AppendNodeToBody(CTML::Node("div#two div#four div#five"));

        auto matches = document.QuerySelector("#two");

        REQUIRE(matches.size() == 2);
    }

    SECTION("search by selector attribute match contains")
    {
        CTML::Document document;

        document.AppendNodeToBody(CTML::Node("div[data-test=\"testneedletest\"] div[data-test=\"testtesttest\"] div[data-test=\"testtesttest\"]"));
        document.AppendNodeToBody(CTML::Node("div[data-test=\"testtesttest\"] div[data-test=\"no\"] div[data-test=\"testneedletest\"]"));

        auto matches = document.QuerySelector("[data-test*=\"needle\"]");

        REQUIRE(matches.size() == 2);
    }

    SECTION("search by selector attribute match starts with")
    {
        CTML::Document document;

        document.AppendNodeToBody(CTML::Node("div[data-test=\"testcorrect\"] div[data-test=\"negativetest\"] div[data-test=\"negativetest\"]"));
        document.AppendNodeToBody(CTML::Node("div[data-test=\"negativetest\"] div[data-test=\"no\"] div[data-test=\"testcorrect\"]"));

        auto matches = document.QuerySelector("[data-test^=\"test\"]");

        REQUIRE(matches.size() == 2);
    }

    SECTION("search by selector attribute match ends with")
    {
        CTML::Document document;

        document.AppendNodeToBody(CTML::Node("div[data-test=\"testgood\"] div[data-test=\"testbad\"] div[data-test=\"testbad\"]"));
        document.AppendNodeToBody(CTML::Node("div[data-test=\"testbad\"] div[data-test=\"no\"] div[data-test=\"testgood\"]"));

        auto matches = document.QuerySelector("[data-test$=\"good\"]");

        REQUIRE(matches.size() == 2);
    }

    SECTION("search by selector attribute match containing word")
    {
        CTML::Document document;

        document.AppendNodeToBody(CTML::Node("div[data-test=\"test needle test\"] div[data-test=\"test test test\"] div[data-test=\"test test test\"]"));
        document.AppendNodeToBody(CTML::Node("div[data-test=\"test test test\"] div[data-test=\"negate\"] div[data-test=\"test needle test\"]"));

        auto matches = document.QuerySelector("[data-test~=\"needle\"]");

        REQUIRE(matches.size() == 2);
    }

    SECTION("search by selector attribute match is or begins hyphen word")
    {
        CTML::Document document;

        document.AppendNodeToBody(CTML::Node("div[data-test=\"test\"] div[data-test=\"rest-test\"] div[data-test=\"test-hyphen\"]"));
        document.AppendNodeToBody(CTML::Node("div[data-test=\"rest-test-\"] div[data-test=\"rest-test\"] div[data-test=\"rest-test\"]"));

        auto matches = document.QuerySelector("[data-test|=\"test\"]");

        REQUIRE(matches.size() == 2);
    }
}