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
		// should new lines in m_content be represented with a br tag
		bool m_newLineBR = false;
	public:
		Node() {}
		
		// create a node with the name specified
		Node(std::string name) {
			this->m_type = ELEMENT;
			this->SetName(name);
		}

		Node(std::string name, std::string m_content) {
			this->m_type = ELEMENT;
			this->SetName(name);
			this->m_content = m_content;
		}

		std::string ToString(bool readable, int indentLevel) {
			std::string elem = "";
			// the four space indent.
			std::string indent = "";
			std::string indentContent = "";
			// increment the indent string by four spaces based on the indentLevel
			for (int i = 0; i < indentLevel; i++) {
				indent += "    ";
			}
			// set the m_content indent level to the indent level plus four more spaces
			indentContent = indent + "    ";
			if (this->m_type == ELEMENT) {
				// construct the first part of the element string, the tag beginning
				elem = ((readable) ? indent : "") + "<" + m_name + "";
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
				// if readablity is wanted, add a newline
				if (readable)
					elem += "\n";
				// if we have m_content to append
				if (!m_content.empty()) {
					// append the m_content to the node, if readability is wanted, add four spaces to the m_content and add a new line
					if (!readable)
						elem += m_content;
					else
						elem += this->_GetFormattedContent(indentContent);
				}
				// get every child node from the m_children list
				for (unsigned int i = 0; i < m_children.size(); i++) {
					Node childNode = m_children[i];
					// append the child node to the elem string.
					elem += childNode.ToString(readable, indentLevel + 1);
				}
				elem += ((readable) ? indent : "") + "</" + m_name + ">" + ((readable) ? "\n" : "");
			}
			else if (this->m_type == DOCUMENT_TYPE) {
				// just construct the docm_type from the m_content given, if readability is wanted, add a newline
				elem += "<!DOCTYPE " + m_content + ">" + ((readable) ? "\n" : "");
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

		Node& SetName(std::string name) {
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
			m_attributes[name] = value;
			return *this;
		}

		Node& SetType(NodeType type) {
			this->m_type = type;
			return *this;
		}

		Node& SetContent(std::string text) {
			this->m_content = text;
			return *this;
		}

		Node& ToggleClass(std::string className) {
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

		Node& SetUseBr(bool useBr) {
			this->m_newLineBR = useBr;
			return *this;
		}

		~Node() {
			m_attributes.clear();
			m_children.clear();
		}
	private:
		std::string _GetFormattedContent(std::string indent) {
			std::string newline = ((this->m_newLineBR) ? "\n" + indent + "<br>\n" : "\n");
			std::string result;
			std::istringstream iss(m_content);
			// iterate through each line in this node
			for (std::string line; std::getline(iss, line);)
			{
				result += indent + line + newline;
			}
			return result;
		}
	};
};