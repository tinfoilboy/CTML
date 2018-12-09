/*
	CTML - written by Tinfoilboy
	uses the MIT License (https://github.com/tinfoilboy/CFML/blob/master/LICENSE)
*/
#pragma once
#include "Node.h"
#include <string>
#include <fstream>

namespace CTML {
	class Document {
	private:
		// the doctype of this document
		Node m_doctype;
		// the html of this document
		Node m_html;
	public:
		// the default constructor for a document
		Document() {
			// create and set the doctype to html
			this->m_doctype = Node("", "html");
			this->m_doctype.SetType(NodeType::DOCUMENT_TYPE);
			this->m_html = Node("html");
			// create the head tag
			this->m_html.AppendChild(Node("head"));
			// create the body tag
			this->m_html.AppendChild(Node("body"));
		}

		// add a node to the head element
		void AddNodeToHead(Node node) {
			this->head().AppendChild(node);
		}

		// add a node to the body element
		void AddNodeToBody(Node node) {
			this->body().AppendChild(node);
		}

		// gets the current document as a string
		std::string ToString(const Readability& readability) const {
			bool isMultiline = (readability == Readability::MULTILINE || readability == Readability::MULTILINE_BR);
			std::string doc = "";
			// add the doctype to the string
			doc += m_doctype.ToString(readability, 0);
			// every document needs an html tag, add it (and it's including head and body tags)
			doc += m_html.ToString(readability, 0) + ((isMultiline) ? "\n" : "");
			return doc;
		}

		// get the current document as a tree represented in a string
		std::string ToTree() const {
			std::string treeStr = "";
			treeStr += m_html.GetTreeString(0);
			return treeStr;
		}

		// write the current document to a file
		bool WriteToFile(const std::string& filePath, const Readability& readability) const {
			std::ofstream file(filePath.c_str());
			if (file.is_open()) {
				file << this->ToString(readability);
				file.close();
				return true;
			}
			return false;
		}

		// Return document root node (i.e. <html>)
		Node & html()
		{
			return this->m_html;
		}

		// Return head node 
		Node & head()
		{
			return m_html.GetChildByName("head");
		}

		// Return body node 
		Node & body()
		{
			return m_html.GetChildByName("body");
		}
	};
}