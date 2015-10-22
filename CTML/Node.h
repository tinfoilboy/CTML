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
		NodeType m_type;
		std::string m_name;
		std::string m_classes;
		std::string m_content;
		std::vector<Node> m_children;
		// a map of attributes, name is the attribute name and the value is the attribute value
		std::map<std::string, std::string> m_attributes;
		// should new lines in m_content be represented with a br tag
		bool useBrForNewLine = false;
	public:
		Node() {}
		// create a node with the name specified
		Node(std::string name);
		Node(std::string name, std::string m_content);
		std::string ToString(bool readable, int indentLevel);
		std::string GetTreeString(int indentLevel);
		Node& SetName(std::string name);
		Node& SetAttribute(std::string name, std::string value);
		Node& SetType(NodeType type);
		Node& SetContent(std::string text);
		Node& ToggleClass(std::string className);
		Node& AppendChild(Node child);
		Node& SetUseBr(bool useBr);
		~Node();
	private:
		std::string GetFormattedContent(std::string indent);
	};
};