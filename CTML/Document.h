#pragma once
#include "Node.h"
#include <vector>
#include <map>
#include <string>
#include <iostream>
#include <algorithm>

namespace CTML {
	class Document {
		// the doctype of this document
		Node m_doctype;
		// the head tag of this document
		Node m_head;
		// the body tag of this document
		Node m_body;
	public:
		// the default constructor for a document
		Document();
		// add a node to the head element
		void AddNodeToHead(Node node);
		// add a node to the body element
		void AddNodeToBody(Node node);
		// gets the current document as a string
		std::string ToString(bool readable);
		// get the current document as a tree represented in a string
		std::string ToTree();
	};
}