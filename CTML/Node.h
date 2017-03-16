/*
	CTML - written by Tinfoilboy
	uses the MIT License (https://github.com/tinfoilboy/CFML/blob/master/LICENSE)
*/
#pragma once
#include <vector>
#include <unordered_map>
#include <string>
#include <sstream>
#include <algorithm>

namespace CTML {
	// the types of nodes used for the html
	// DOCUMENT_TYPE doesn't use the element name, but uses
	// the content to determine the document type to use
	// ELEMENT is just a normal element
	enum class NodeType { DOCUMENT_TYPE, ELEMENT };

	// a few enums for readability of the HTML
	// SINGLE_LINE returns the string as one line
	// MULTILINE returns the string as multiple lines, which is good for file outputs or readability.
	// MULTILINE_BR is essentially the same as MULTILINE, but the difference is that newlines in the content of the node are formatted to use <br> tags.
	enum class Readability { SINGLE_LINE, MULTILINE, MULTILINE_BR };

	// the state of the Node name parser
	// NONE means that nothing is being parsed
	// CLASS means that a class attribute is being parsed
	// ID means that an ID is being parsed for the node
	enum class NodeParser { NONE, CLASS, ID };

	class Node {
		// the type of node this
		NodeType m_type;
		// the name of this node, e.g. div, a, span, e.t.c.
		std::string m_name;
		// the classes for this node
		std::string m_classes;
		// the ids for this node
		std::string m_id;
		// the content of this node
		std::string m_content;
		// determines whether or not to add a closing tag (ie. </name>)
		// if this is false, it also doesn't add content to the tag
		// as there is nowhere to place content
		bool m_closeTag = true;
		// the child elements of this node
		std::vector<Node> m_children;
		// an unordered_map of attributes, name is the attribute name and the value is the attribute value
		std::unordered_map<std::string, std::string> m_attributes;
	public:
		// default constructor, does nothing
		Node() = default;

		// create a node with the name specified
		Node(const std::string& name) {
			this->m_type = NodeType::ELEMENT;
			this->SetName(name);
		}

		// create a node with the name specified, also containing the following content
		Node(const std::string& name, const std::string& content) {
			this->m_type = NodeType::ELEMENT;
			this->SetName(name);
			this->m_content = content;
		}

		// return this node as an html string
		std::string ToString(Readability readability, int indentLevel) const {
			// the element string that will be returned
			std::string elem = "";
			// the four space indent.
			std::string indent = "";
			std::string indentContent = "";
			// if the readabilty points is either multiline types, this would be true
			bool isMultiline = (readability == Readability::MULTILINE || readability == Readability::MULTILINE_BR);
			// increment the indent string by four spaces based on the indentLevel
			// but only if the readabilty is MULTILINE OR MULTILINE_BR
			if (isMultiline) {
				for (int i = 0; i < indentLevel; i++) {
					indent = std::string(INDENT_SPACES * indentLevel, ' ');
				}
				// set the m_content indent level to the indent level plus four more spaces
				indentContent = std::string(INDENT_SPACES  * (indentLevel + 1), ' ');
			}
			if (this->m_type == NodeType::ELEMENT) {
				// construct the first part of the element string, the tag beginning
				elem = ((isMultiline) ? indent : "") + "<" + m_name + "";
				// add the class list if it isn't empty
				if (!m_classes.empty()) {
					std::string classTag = "class=\"";
					elem += " " + classTag + m_classes + "\"";
				}
				// add the id list if it isn't empty
				if (!m_id.empty()) {
					std::string idTag = "id=\"";
					elem += " " + idTag + m_id + "\"";
				}
				// make an iterator for each attribute
				for (const auto& attr : m_attributes) {
					elem += " " + attr.first + "=\"" + attr.second + "\"";
				}
				// close the beginning tag
				elem += ">";
				// only add the content, as well as the closing tag if it is
				// specified to do so
				if (m_closeTag)
				{
					// if multiline is specified and the content/children aren't empty, add a newline
					if (isMultiline && (!m_content.empty() || !m_children.empty()))
						elem += "\n";
					// if we have m_content to append
					if (!m_content.empty()) {
						// format the elements content based on the readability, as well as the indent level for content
						elem += _GetFormattedContent(readability, indentContent);
					}
					// get every child node from the m_children list
					for (std::size_t i = 0; i < m_children.size(); ++i) {
						const auto& childNode = m_children[i];
						// append the child node to the elem string.
						// if this is not the last child node append a newline if multiline
						elem += childNode.ToString(readability, indentLevel + 1) + ((i != m_children.size() - 1 && isMultiline) ? "\n" : "");
					}
					// if multiline is specified and the content/children aren't empty, add a newline and indent
					elem += ((isMultiline && (!m_content.empty() || !m_children.empty())) ? "\n" + indent : "") + "</" + m_name + ">";
				}
			}
			else if (this->m_type == NodeType::DOCUMENT_TYPE) {
				// just construct the docm_type from the m_content given, if readability is wanted, add a newline
				elem += "<!DOCTYPE " + m_content + ">" + ((isMultiline) ? "\n" : "");
			}
			return elem;
		}

		std::string GetTreeString(int indentLevel) const {
			// the tree string
			std::string tree;
			// indent level
			std::string indent(INDENT_SPACES * indentLevel, ' ');
			// turn the class list into actual classes for the elements
			std::string classList = m_classes;
			std::replace(classList.begin(), classList.end(), ' ', '.');
			// if the class list isn't empty, prepend a period
			if (!classList.empty())
				classList = '.' + classList;
			// add the current element to the tree
			tree += indent + " |_ " + this->m_name + classList + '\n';
			// for each child
			for (const auto& child : m_children) {
				tree += child.GetTreeString(indentLevel + 1) + '\n';
			}
			return tree;
		}

		Node& SetName(const std::string& name) {
			// the index of a period
			const auto periodIndex = name.find('.');
			// the index of a pound sign
			const auto poundIndex = name.find('#');
			// if there are classes in the name
			if (periodIndex != std::string::npos || poundIndex != std::string::npos) {
				// if the pound index comes before the period index
				bool poundBefore = (poundIndex != std::string::npos && poundIndex < periodIndex);
				// get the first index for parsing
				// if pound comes first, or there are no periods, use the first pound index first
				// else use the first period index
				const auto ind = ((poundBefore || (periodIndex == std::string::npos && poundIndex != std::string::npos)) ? poundIndex : periodIndex);
				// get the element name
				std::string elemName = name.substr(0, ind);
				// parse the current ids and classes
				_ParseClassesAndIDS(name.substr(ind));
				// set the element name to the built element name
				this->m_name = elemName;
			}
			else {
				this->m_name = name;
			}
			return *this;
		}

		std::string GetAttribute(const std::string& name) const {
			// the class attribute is tracked with m_classes, so we return that instead of m_attributes[name]
			if (name != "class" && name != "id" && m_attributes.count(name) > 0)
				return m_attributes.at(name);
			else if (name == "class")
				return m_classes;
			else if (name == "id")
				return m_id;
			else
				return "";
		}

		std::string GetSelector() const {
			std::string classesPeriod = _ReplaceAllOccurrences(m_classes, " ", ".");
			return m_name + classesPeriod + "#" + m_id;
		}

		Node& SetAttribute(std::string name, std::string value) {
			// setting the "class" attribute would make there be two class attributes on the element
			// so therefore, if the name of this is class, we just override "m_classes"
			if (name != "class" && name != "id")
				m_attributes[name] = value;
			else if (name == "class")
				m_classes = value;
			else if (name == "id")
				m_id = value;
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
			size_t findIndex = m_classes.find(className);
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

		Node& UseClosingTag(bool close) {
			this->m_closeTag = close;
			return *this;
		}

	private:
		std::string _GetFormattedContent(Readability readability, const std::string& indent) const {
			std::string result;
			std::istringstream iss(m_content);
			// if we are using either variant of multiple lines, run this.
			if (readability == Readability::MULTILINE || readability == Readability::MULTILINE_BR) {
				// the newline string, differs between MULTILINE and MULTILINE_BR
				std::string newline = ((readability == Readability::MULTILINE_BR) ? "\n" + indent + "<br>\n" : "\n");
				// the current line iterated
				int curLine = 0;
				// iterate through each line in this node
				for (std::string line; std::getline(iss, line);)
				{
					result += ((curLine > 0) ? newline : "") + indent + line;
					curLine++;
				}
			}
			else {
				// iterate through each line in this node
				for (std::string line; std::getline(iss, line);)
				{
					result += line;
				}
			}
			// replaces all instances of "<" in the content with "&lt;", to escape rogue HTML
			result = _ReplaceAllOccurrences(result, "<", "&lt;");
			// replaces all instances of ">" in the content with "&gt;" to escape rogue HTML
			result = _ReplaceAllOccurrences(result, ">", "&gt;");
			// return the result of the content
			return result;
		}
		std::string _ReplaceAllOccurrences(std::string replacer, const std::string& replacable, const std::string& replace) const {
			// the start of the current replacable string
			size_t start = 0;
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
		int _CountOccurrences(std::string finder, const std::string& findable) const {
			// the occurrences of the string
			int occurrences = 0;
			// the start of the current replacable string
			size_t start = 0;
			// try and find each occurrence of replaceable until it can't be found
			while ((start = finder.find(findable, start)) != std::string::npos) {
				// replace the actual string
				occurrences++;
				// add to the start so that find can be run again
				start += findable.length();
			}
			// return the replaced string
			return occurrences;
		}
		void _ParseClassesAndIDS(std::string classesAndIDs) {
			// what is currently being parsed
			// zero for nothing
			// one for class
			// two for id
			NodeParser currentlyParsing = NodeParser::NONE;
			// the string for the class or ID
			std::string attrString;
			// iterate through each character in the string
			for (unsigned int i = 0; i < classesAndIDs.size(); i++) {
				// the current character being iterated
				char curChar = classesAndIDs[i];
				if (currentlyParsing == NodeParser::NONE) {
					// if the current character is a period, set the current parsing to class
					// else if the current character is a pound sign, set the current parsing to id
					if (curChar == '.') {
						currentlyParsing = NodeParser::CLASS;
					}
					else if (curChar == '#') {
						currentlyParsing = NodeParser::ID;
					}
				}
				else {
					// if the current character is a period, set the current parsing to class
					// else if the current character is a pound sign, set the current parsing to id
					if (curChar == '.' || curChar == '#') {
						if (currentlyParsing == NodeParser::CLASS)
							m_classes += attrString + " ";
						else
							// if we hit an id, we just reset the id
							// this is because HTML only allows for a single id on each element
							m_id = attrString;
						attrString.clear();
						currentlyParsing = ((curChar == '.') ? NodeParser::CLASS : NodeParser::ID);
					}
					else {
						// add the current character to the class or id string
						attrString += curChar;
					}
				}
				// if we are at the last character, and are still parsing something, add it to the respective attr
				if (currentlyParsing != NodeParser::NONE && i == classesAndIDs.size() - 1) {
					if (currentlyParsing == NodeParser::CLASS)
						m_classes += attrString;
					else
						// if we hit an id, we just reset the id
						// this is because HTML only allows for a single id on each element
						m_id = attrString;
					attrString.clear();
				}
			}
			// if there is an extra space at the end of m_classes, remove it
			if (!m_classes.empty()) {
				if (isspace(m_classes.at(m_classes.size() - 1)))
					m_classes = m_classes.substr(0, m_classes.size() - 1);
			}
		}
	};
}