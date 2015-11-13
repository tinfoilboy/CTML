#include "CTML.h"

// tests if two strings are equivelent to each other
const bool assert_strings_equal(const std::string& left, const std::string& right) {
	return (left == right);
}

// this test ensures that HTML inside of the content is escaped correctly.
void run_escape_test() {
	// what the node's to string should be equal to
	const std::string htmlString = "<a class=\"button\">&lt;script&gt;alert(\"ha ha hacked!\")&lt;/script&gt;</a>";
	CTML::Node node("a.button", "<script>alert(\"ha ha hacked!\")</script>");
	// the node's string output
	const std::string nodeString = node.ToString(CTML::SINGLE_LINE, 0);
	bool test = assert_strings_equal(htmlString, nodeString);
	std::cout << "Escape Test " << ((test) ? "passed!" : "failed!") << std::endl <<
		"HTML Output: " << nodeString << std::endl <<
		// use a double end line at the end for spacing between tests
		"Expected Output: " << htmlString << std::endl << std::endl;
}

// this test ensures that the HTML document created is equal to a correct HTML5 document
void run_document_test() {
	// what the document's to string should be equal to
	const std::string htmlString = "<!DOCTYPE html><html><head></head><body><h1>&lt;test!&gt;</h1></body></html>";
	CTML::Document doc;
	// the string output of the document
	doc.AddNodeToBody(CTML::Node("h1", "<test!>"));
	const std::string docString = doc.ToString(CTML::SINGLE_LINE);
	bool test = assert_strings_equal(htmlString, docString);
	std::cout << "Document Test " << ((test) ? "passed!" : "failed!") << std::endl <<
		"HTML Output: " << docString << std::endl <<
		// use a double end line at the end for spacing between tests
		"Expected Output: " << htmlString << std::endl << std::endl;
}

// this test checks if the classes provided are correctly stored
void run_class_test() {
	const std::string classString = "test classes are fun";
	CTML::Node testNode("a#test.test.classes.are.fun");
	std::cout << testNode.GetAttribute("hehhehtest") << std::endl;
	// get the test node's classlist.
	const std::string classList = testNode.GetAttribute("class");
	bool test = assert_strings_equal(classString, classList);
	std::cout << "Class Test " << ((test) ? "passed!" : "failed!") << std::endl <<
		"Class Output: " << classList << std::endl <<
		// use a double end line at the end for spacing between tests
		"Expected Output: " << classString << std::endl << std::endl;
}

int main() {
	run_escape_test();
	run_document_test();
	run_class_test();
	return 0;
}