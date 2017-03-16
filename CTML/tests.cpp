#include <iostream>
#include "CTML.h"
#include <chrono>

// std chrono has the worst namespace abuse i've ever seen, so therefore, i'm typedeffing
typedef std::chrono::high_resolution_clock high_res_clock;
typedef std::chrono::milliseconds millis;

using namespace CTML;

// tests if two strings are equivelent to each other
bool assert_strings_equal(const std::string& left, const std::string& right) {
	return (left == right);
}

// this test ensures that HTML inside of the content is escaped correctly.
void run_escape_test() {
	// what the node's to string should be equal to
	const auto htmlString = "<a class=\"button\">&lt;script&gt;alert(\"ha ha hacked!\")&lt;/script&gt;</a>";
	Node node("a.button", "<script>alert(\"ha ha hacked!\")</script>");
	// the node's string output
	const auto nodeString = node.ToString(Readability::SINGLE_LINE, 0);
	auto test = assert_strings_equal(htmlString, nodeString);
	std::cout << "Escape Test " << ((test) ? "passed!" : "failed!") << std::endl <<
		"HTML Output: " << nodeString << std::endl <<
		// use a double end line at the end for spacing between tests
		"Expected Output: " << htmlString << std::endl << std::endl;
}

// this test ensures that the HTML document created is equal to a correct HTML5 document
void run_document_test() {
	// what the document's to string should be equal to
	const auto htmlString = "<!DOCTYPE html><html><head></head><body><h1>&lt;test!&gt;</h1></body></html>";
	Document doc;
	// the string output of the document
	doc.AddNodeToBody(Node("h1", "<test!>"));
	const auto docString = doc.ToString(Readability::SINGLE_LINE);
	auto test = assert_strings_equal(htmlString, docString);
	std::cout << "Document Test " << ((test) ? "passed!" : "failed!") << std::endl <<
		"HTML Output: " << docString << std::endl <<
		// use a double end line at the end for spacing between tests
		"Expected Output: " << htmlString << std::endl << std::endl;
}

// this test checks if the classes provided are correctly stored
void run_class_test() {
	const auto classString = "test classes are fun";
	Node testNode("a#test.test.classes.are.fun");
	// get the test node's classlist.
	const auto classList = testNode.GetAttribute("class");
	bool test = assert_strings_equal(classString, classList);
	std::cout << "Class Test " << ((test) ? "passed!" : "failed!") << std::endl <<
		"Class Output: " << classList << std::endl <<
		// use a double end line at the end for spacing between tests
		"Expected Output: " << classString << std::endl << std::endl;
}

// this test checks if the attributes provided are correctly stored and gotten
void run_attribute_test() {
	const auto attrOutput = "testAttr1";
	const auto attr2Output = "testAttr2";
	const auto attr3Output = "";
	Node testNode("a");
	// set two attributes on the node
	testNode.SetAttribute("attr1", "testAttr1").SetAttribute("attr2", "testAttr2");
	// get each attribute's output
	const auto attrOut = testNode.GetAttribute("attr1");
	const auto attr2Out = testNode.GetAttribute("attr2");
	const auto attr3Out = testNode.GetAttribute("attr3");
	// test each string
	auto test1 = assert_strings_equal(attrOutput, attrOut);
	auto test2 = assert_strings_equal(attr2Output, attr2Out);
	auto test3 = assert_strings_equal(attr3Output, attr3Out);
	std::cout << "Class Test " << ((test1 && test2 && test3) ? "passed!" : "failed!") << std::endl <<
		"Attr 1 Output: " << attrOut << std::endl <<
		"Expected Output: " << attrOutput << std::endl <<
		"Attr 2 Output: " << attr2Out << std::endl <<
		"Expected Output: " << attr2Output << std::endl <<
		"Attr 3 Output: " << attr3Out << std::endl <<
		// use a double end line at the end for spacing between tests
		"Expected Output: " << attr3Output << std::endl << std::endl;

}

// this test checks if the close tag method actually works
void run_no_close() {
	const auto nodeString = "<img src=\"funnypicture.png\" alt=\"Hilarious image\">";
	Node testNode = Node("img").SetAttribute("src", "funnypicture.png").SetAttribute("alt", "Hilarious image").UseClosingTag(false);
	// make sure the test node and the test string match
	std::string output = testNode.ToString(Readability::SINGLE_LINE, 1);
	bool test = assert_strings_equal(output, nodeString);
	std::cout << "No Close Test " << ((test) ? "passed!" : "failed!") << std::endl <<
		"Node Output: " << output << std::endl <<
		// use a double end line at the end for spacing between tests
		"Expected Output: " << nodeString << std::endl << std::endl;
}

int main() {
	high_res_clock::time_point begin = high_res_clock::now();
	run_escape_test();
	run_document_test();
	run_class_test();
	run_attribute_test();
	run_no_close();
	high_res_clock::time_point end = high_res_clock::now();
	millis ms = std::chrono::duration_cast<millis>(end - begin);
	std::cout << "Tests ran in " << ms.count() << "ms" << std::endl;
	return 0;
}