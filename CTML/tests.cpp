#include "CTML.h"

// tests if two strings are equivelent to each other
bool assert_strings_equal(const std::string& left, const std::string& right) {
	if (left == right)
		return true;
	return false;
}

// this test ensures that HTML inside of the content is escaped correctly.
void run_escape_test() {
	// what the node's to string should be equal to
	std::string htmlString = "<a class=\"button\">&lt;script&gt;alert(\"ha ha hacked!\")&lt;/script&gt;</a>";
	CTML::Node node = CTML::Node("a.button", "<script>alert(\"ha ha hacked!\")</script>");
	// the node's string output
	std::string nodeString = node.ToString(CTML::SINGLE_LINE, 0);
	bool test = assert_strings_equal(htmlString, nodeString);
	std::cout << "Escape Test " << ((test) ? "passed!" : "failed!") << std::endl <<
		"HTML Output: " << nodeString << std::endl <<
		// use a double end line at the end for spacing between tests
		"Expected Output: " << htmlString << std::endl << std::endl;
}

// this test ensures that the HTML document created is equal to a correct HTML5 document
void run_document_test() {
	// what the document's to string should be equal to
	std::string htmlString = "<!DOCTYPE html><html><head></head><body></body></html>";
	CTML::Document doc = CTML::Document();
	// the string output of the document
	std::string docString = doc.ToString(CTML::SINGLE_LINE);
	bool test = assert_strings_equal(htmlString, docString);
	std::cout << "Document Test " << ((test) ? "passed!" : "failed!") << std::endl <<
		"HTML Output: " << docString << std::endl <<
		// use a double end line at the end for spacing between tests
		"Expected Output: " << htmlString << std::endl << std::endl;
}

int main() {
	run_escape_test();
	run_document_test();
	return 0;
}