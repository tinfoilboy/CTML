/*
	CTML - written by Tinfoilboy
	uses the MIT License (https://github.com/tinfoilboy/CFML/blob/master/LICENSE)
*/
#pragma once
#include <vector>
#include <map>
#include <string>
#include <sstream>
#include <iostream>
#include <algorithm>

namespace CTML {
	enum NodeType { DOCUMENT_TYPE, ELEMENT };

	// a few enums for readability of the HTML
	// SINGLE_LINE returns the string as one line, useful for 
	enum Readability { SINGLE_LINE, MULTILINE, MULTILINE_BR };

	class Node {
		// the type of node this
		NodeType m_type;
		// the name of this node, e.g. div, a, span, e.t.c.
		std::string m_name;
		// the classes for this node
		std::string m_classes;
		// the content of this node
		std::string m_content;
		// the child elements of this node
		std::vector<Node> m_children;
		// a map of attributes, name is the attribute name and the value is the attribute value
		std::map<std::string, std::string> m_attributes;
	public:
		Node() {}
		
		// create a node with the name specified
		Node(const std::string& name) {
			this->m_type = ELEMENT;
			this->SetName(name);
		}

		Node(const std::string& name, const std::string& m_content) {
			this->m_type = ELEMENT;
			this->SetName(name);
			this->m_content = m_content;
		}

		std::string ToString(Readability readability, int indentLevel) {
			std::string elem = "";
			// the four space indent.
			std::string indent = "";
			std::string indentContent = "";
			// if the readabilty points is either multiline types, this would be true
			bool isMultiline = (readability == MULTILINE || readability == MULTILINE_BR);
			// increment the indent string by four spaces based on the indentLevel
			// but only if the readabilty is MULTILINE OR MULTILINE_BR
			if (isMultiline) {
				for (int i = 0; i < indentLevel; i++) {
					indent += "    ";
				}
				// set the m_content indent level to the indent level plus four more spaces
				indentContent = indent + "    ";
			}
			if (this->m_type == ELEMENT) {
				// construct the first part of the element string, the tag beginning
				elem = ((isMultiline) ? indent : "") + "<" + m_name + "";
				// add the class list if it isn't empty
				if (!m_classes.empty()) {
					std::string classTag = "class=\"";
					elem += " " + classTag + m_classes + "\"";
				}
				// make an iterator for each attribute
				for (auto attr : m_attributes) {
					elem += " " + attr.first + "=\"" + attr.second + "\"";
				}
				// close the beginning tag
				elem += ">";
				// if multiline is specified, add a newline
				if (isMultiline)
					elem += "\n";
				// if we have m_content to append
				if (!m_content.empty()) {
					// format the elements content based on the readability, as well as the indent level for content
					elem += this->_GetFormattedContent(readability, indentContent);
				}
				// get every child node from the m_children list
				for (unsigned int i = 0; i < m_children.size(); i++) {
					Node childNode = m_children[i];
					// append the child node to the elem string.
					elem += childNode.ToString(readability, indentLevel + 1);
				}
				elem += ((isMultiline) ? indent : "") + "</" + m_name + ">" + ((isMultiline) ? "\n" : "");
			}
			else if (this->m_type == DOCUMENT_TYPE) {
				// just construct the docm_type from the m_content given, if readability is wanted, add a newline
				elem += "<!DOCTYPE " + m_content + ">" + ((isMultiline) ? "\n" : "");
			}
			return elem;
		}

		std::string GetTreeString(int indentLevel) {
			// the tree string
			std::string tree = "";
			// indent level
			std::string indent = "";
			// get the current indent string
			for (int i = 0; i < indentLevel; i++) {
				indent += "   ";
			}
			// turn the class list into actual classes for the elements
			std::string classList = m_classes;
			std::replace(classList.begin(), classList.end(), ' ', '.');
			// if the class list isn't empty, prepend a period
			if (!classList.empty())
				classList = '.' + classList;
			// add the current element to the tree
			tree += indent + " |_ " + this->m_name + classList + '\n';
			// for each child
			for (unsigned int i = 0; i < m_children.size(); i++) {
				tree += m_children[i].GetTreeString(indentLevel + 1) + '\n';
			}
			return tree;
		}

		Node& SetName(const std::string& name) {
			// the index of a period
			int periodIndex = name.find('.');
			// if there are classes in the name
			if (periodIndex != std::string::npos) {
				// get the element name
				std::string elemName = name.substr(0, periodIndex);
				// get the classes for this element
				std::string classes = name.substr(periodIndex + 1);
				// replace each class period with a space
				std::replace(classes.begin(), classes.end(), '.', ' ');
				this->m_name = elemName;
				this->m_classes = classes;
			}
			else {
				this->m_name = name;
			}
			return *this;
		}
		Node& SetAttribute(std::string name, std::string value) {
			// setting the "class" attribute would make there be two class attributes on the element
			// so therefore, if the name of this is class, we just override "m_classes"
			if (name != "class")
				m_attributes[name] = value;
			else
				m_classes = value;
			return *this;
		}

		Node& SetType(NodeType type) {
			this->m_type = type;
			return *this;
		}

		Node& SetContent(const std::string& text) {
			this->m_content = text;
			return *this;
		}

		Node& ToggleClass(const std::string& className) {
			int findIndex = m_classes.find(className);
			if (findIndex == std::string::npos) {
				// append the class
				m_classes += ((!m_classes.empty()) ? " " : "") + className;
			}
			else {
				// remove the class
				m_classes.erase(findIndex, className.size());
			}
			return *this;
		}
		
		Node& AppendChild(Node child) {
			m_children.push_back(child);
			return *this;
		}

		~Node() {
			m_attributes.clear();
			m_children.clear();
		}
	private:
		std::string _GetFormattedContent(Readability readability, const std::string& indent) {
			std::string result;
			std::istringstream iss(m_content);
			// if we are using either varient of multiple lines, run this.
			if (readability == MULTILINE || readability == MULTILINE_BR) {
				std::string newline = ((readability == MULTILINE_BR) ? "\n" + indent + "<br>\n" : "\n");
				// iterate through each line in this node
				for (std::string line; std::getline(iss, line);)
				{
					result += indent + line + newline;
				}
				return result;
			}
			else {
				// iterate through each line in this node
				for (std::string line; std::getline(iss, line);)
				{
					result = line;
				}
			}
			// replaces all instances of "<" in the content with "&lt;", to escape rogue HTML
			result = ReplaceAllOccurrences(result, "<", "&lt;");
			// replaces all instances of ">" in the content with "&gt;" to escape rogue HTML
			result = ReplaceAllOccurrences(result, ">", "&gt;");
			// return the result of the content
			return result;
		}
		std::string ReplaceAllOccurrences(std::string replacer, const std::string& replacable, const std::string& replace) {
			// the start of the current replacable string
			int start = 0;
			// try and find each occurrence of replaceable until it can't be found
			while ((start = replacer.find(replacable, start)) != std::string::npos) {
				// replace the actual string
				replacer.replace(start, replacable.length(), replace);
				// add to the start so that find can be run again
				start += replace.length();
			}
			// return the replaced string
			return replacer;
		}
	};
};