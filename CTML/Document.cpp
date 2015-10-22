/*
	CTML - written by Tinfoilboy
	uses the MIT License (https://github.com/tinfoilboy/CFML/blob/master/LICENSE)
*/
#include "Document.h"

namespace CTML {
	Document::Document() {
		// create and set the doctype to html
		this->m_doctype = Node("", "html");
		this->m_doctype.SetType(DOCUMENT_TYPE);
		// create the head tag
		this->m_head = Node("head");
		// create the body tag
		this->m_body = Node("body");
	}

	void Document::AddNodeToHead(Node node) {
		this->m_head.AppendChild(node);
	}

	void Document::AddNodeToBody(Node node) {
		this->m_body.AppendChild(node);
	}

	std::string Document::ToString(bool readable) {
		std::string doc = "";
		// add the doctype to the string
		doc += m_doctype.ToString(readable, 0);
		// every document needs an html tag, add it
		doc += "<html>";
		// if we want readability, append a newline to the html beginning tag
		doc += ((readable) ? "\n" : "");
		// append the head tag and its children
		doc += m_head.ToString(readable, 1);
		// append the body tag and its children
		doc += m_body.ToString(readable, 1);
		// close the html tag
		doc += "</html>";
		return doc;
	}

	std::string Document::ToTree() {
		std::string treeStr = "";
		treeStr += "html\n";
		treeStr += m_head.GetTreeString(0);
		treeStr += m_body.GetTreeString(0);
		return treeStr;
	}

	bool Document::WriteToFile(std::string filePath, bool readable) {
		std::ofstream file = std::ofstream(filePath);
		if (file.is_open()) {
			file << this->ToString(readable);
			file.close();
			return true;
		}
		return false;
	}
}