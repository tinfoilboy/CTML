/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Maxwell Flynn
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#ifndef CTML_HPP_
#define CTML_HPP_

#include <cstdint>
#include <vector>
#include <string>
#include <unordered_map>
#include <sstream>
#include <algorithm>

namespace CTML
{
    /**
     * Searches the original string and replaces all occurances of the specified
     * string.
     */
    std::string replace_all(
        std::string& original,
        const std::string& target,
        const std::string& replacement
    )
    {
        size_t start = 0;

        // while we are not at the end of the string, find the replacable string
        while ((start = original.find(target, start)) != std::string::npos)
        {
            original.replace(start, target.length(), replacement);
            start += replacement.length();
        }
        
        return original;
    }

    /**
     * Convenience function to escape HTML characters from a value.
     * 
     * Optionally disable escaping double or single quote marks for text content
     * by setting the second value to false.
     */
    std::string html_escape(std::string value, bool escape_quotes=true)
    {
        std::string output = value;

        output = replace_all(output, "&", "&amp;");
        output = replace_all(output, "<", "&lt;");
        output = replace_all(output, ">", "&gt;");

        if (escape_quotes)
        {
            output = replace_all(output, "\"", "&quot;");
            output = replace_all(output, "'", "&apos;");
        }

        return output;
    }

    /**
     * An enum representing the types of HTML nodes that can be constructed.
     */
    enum class NodeType : uint8_t
    {
        COMMENT,
        DOCUMENT_TYPE,
        ELEMENT,

        /**
         *We need to add multiple elements.
         */
        MULTIPLE_ELEMENTS,
        
        TEXT
    };

    /**
     * An enum representing the state of the parser for a Node name, akin
     * to an Emmet abbreviation.
     */
    enum class SelectorParserState : uint8_t
    {
        NONE,
        ELEMENT,
        CLASS,
        ID,
        ATTRIBUTE_NAME,
        ATTRIBUTE_VALUE
    };

    /**
     * An enum representing the formatting of the ToString for a Node.
     */
    enum class StringFormatting : uint8_t
    {
        SINGLE_LINE,
        MULTIPLE_LINES
    };

    /**
     * An enum for the different types of selector tokens that can be parsed.
     */
    enum class SelectorTokenType : uint8_t
    {
        ELEMENT,
        CLASS,
        ID,
        ATTRIBUTE_NAME,
        ATTRIBUTE_VALUE
    };

    /**
     * A struct for options for a ToString call on a Node or Document.
     */
    struct ToStringOptions
    {
        StringFormatting formatting;
        bool trailingNewline;
        uint32_t indentLevel;
        bool escapeContent;

        ToStringOptions(
            StringFormatting formatting=StringFormatting::SINGLE_LINE,
            bool trailingNewLine=false,
            uint32_t indentLevel=0,
            bool escapeContent=true
            )
            :
            formatting(formatting),
            trailingNewline(trailingNewLine),
            indentLevel(indentLevel),
            escapeContent(escapeContent) {}
    };

    /**
     * A struct for selector tokens from parsing a selector.
     */
    struct SelectorToken
    {
        SelectorTokenType type;
        std::string       value;
    };

    /**
     * Adds a single selector token to the token vector passed in based on the
     * state of the parser that was passed.
     */
    void add_selector_token(
        std::vector<SelectorToken>& tokens,
        SelectorParserState state,
        const std::string& value
    )
    {
        switch (state)
        {
            case SelectorParserState::ELEMENT:
                tokens.push_back({ SelectorTokenType::ELEMENT, value });
                break;
            case SelectorParserState::CLASS:
                tokens.push_back({ SelectorTokenType::CLASS, value });
                break;
            case SelectorParserState::ID:
                tokens.push_back({ SelectorTokenType::ID, value });
                break;
            case SelectorParserState::ATTRIBUTE_NAME:
                tokens.push_back({ SelectorTokenType::ATTRIBUTE_NAME, value });
                break;
            case SelectorParserState::ATTRIBUTE_VALUE:
                tokens.push_back({ SelectorTokenType::ATTRIBUTE_VALUE, value });
                break;
            // do nothing for any other type as we can't determine a token
            case SelectorParserState::NONE:
            default:
                break;
        }
    }

    /**
     * Parses a string representation of a CSS selector into a vector of tokens.
     * 
     * This parser isn't meant to be comprehensive and robust, just merely to
     * allow searching for elements via common selectors and to parse Emmet-like
     * abbriviations for use in creating nodes.
     * 
     * This parser particularly just scans the string from start to finish
     * character by character, and thus might be slow on very large strings
     * or very complex selectors.
     */
    std::vector<SelectorToken> parse_selector(const std::string& selector)
    {
        std::vector<SelectorToken> tokens;

        // start the parser to be parsing an element by default
        SelectorParserState state = SelectorParserState::ELEMENT;

        std::string temp = "";

        for (char current : selector)
        {
            if (current == '.')
            {
                if (!temp.empty())
                    add_selector_token(tokens, state, temp);

                temp = "";

                state = SelectorParserState::CLASS;

                continue;
            }
            else if (current == '#')
            {
                if (!temp.empty())
                    add_selector_token(tokens, state, temp);

                temp = "";

                state = SelectorParserState::ID;

                continue;
            }
            // only treat spaces as a split character if we aren't parsing attributes
            // as these attribute strings are allowed to have spaces
            else if (current == ' ' && state != SelectorParserState::ATTRIBUTE_VALUE)
            {
                if (!temp.empty())
                    add_selector_token(tokens, state, temp);

                temp = "";

                state = SelectorParserState::ELEMENT;

                continue;
            }
            else if (current == '[')
            {
                if (!temp.empty())
                    add_selector_token(tokens, state, temp);

                temp = "";

                state = SelectorParserState::ATTRIBUTE_NAME;

                continue;
            }
            else if (current == '=' && state == SelectorParserState::ATTRIBUTE_NAME)
            {
                if (!temp.empty())
                    add_selector_token(tokens, state, temp);

                temp = "";

                state = SelectorParserState::ATTRIBUTE_VALUE;

                continue;
            }
            // ignore quotes for attribute values as they aren't necessary for
            // the value of the attribute
            else if (current == '"' && state == SelectorParserState::ATTRIBUTE_VALUE)
            {
                continue;
            }
            // ignore right square brackets if we are parsing an attribute name
            // or an attribute value
            else if (
                current == ']' &&
                (state == SelectorParserState::ATTRIBUTE_NAME ||
                 state == SelectorParserState::ATTRIBUTE_VALUE)
            )
            {
                if (!temp.empty())
                    add_selector_token(tokens, state, temp);

                temp = "";

                state = SelectorParserState::NONE;

                continue;
            }

            temp += current;
        }

        // if we still have a temporary value parsed, just add it to the tokens
        if (!temp.empty())
            add_selector_token(tokens, state, temp);

        return tokens;
    }

    /**
     * A class that represents any type of HTML node to construct in CTML.
     * 
     * For each node, there is at the minimum a type that represents the kind
     * of node that is being constructed, see the `NodeType` enum for the
     * different kinds of nodes that are able to be constructed at the moment.
     * 
     * Each node also has a vector of children that house other nodes.
     * 
     * Once you have constructed a node to your liking, you can get a string
     * representation of this node through the ToString method.
     */
    class Node
    {
    public:
        Node() = default;


        /**
         * Create an empty node of the type specified.
         * 
         * Optionally, you can specify a name as well as content
         * which will be added according to the type.
         */
        Node(
            const NodeType& type,
            std::string content="",
            std::string name=""
        )
            :
            m_type(type)
        {
            // Use the name of the Node for the content as content should be ignored
            if (type == NodeType::COMMENT)
                m_content = name;
            else if (type == NodeType::DOCUMENT_TYPE)
                m_content = name;
            else if (type == NodeType::TEXT)
                m_content = name;
            else if (type == NodeType::ELEMENT)
            {
                this->SetName(name);

                if (!content.empty())
                    this->AppendText(content);
            }
            else if(type == NodeType::MULTIPLE_ELEMENTS)
            {
                if(!name.empty())
                    this->SetName(name);
                m_content = content;
            }
        }

        /**
         * Create an element node with the name specified.
         */
        Node(std::string name)
            :
            m_type(NodeType::ELEMENT)
        {
            this->SetName(name);
        }

        /**
         * Create an element node with the specified name and containing a text node
         * with the content specified.
         */
        Node(std::string name, std::string content)
            :
            m_type(NodeType::ELEMENT)
        {
            this->SetName(name);
            this->AppendText(content);
        }

        /**
         * Generate a string for this Node instance.
         * 
         * This function is recursive, so ToString is called for every Node that is a
         * child of this node.
         *
         * You may optionally specify a StringFormatting enum for how to format the string
         * as well as an indent level to append a number of spaces before this string.
         */
        std::string ToString(ToStringOptions options={}) const
        {
            std::stringstream output;

            std::string indent = "";

            if (options.indentLevel > 0 && options.formatting != StringFormatting::SINGLE_LINE)
                indent = std::string(options.indentLevel * 4, ' ');

            // format a comment node with only the set content
            if (m_type == NodeType::COMMENT)
            {
                output << indent << "<!--" << m_content << "-->";

                if (options.formatting == StringFormatting::MULTIPLE_LINES)
                    output << "\n";
            }
            // format a special document type node with the content
            // as the specified type to use
            else if (m_type == NodeType::DOCUMENT_TYPE)
            {
                output << indent << "<!DOCTYPE " << m_content << ">";

                if (options.formatting == StringFormatting::MULTIPLE_LINES)
                    output << "\n";
            }
            // format a text node with just the content, this node doesn't
            // follow StringFormatting as it could potentially alter the
            // document output
            else if (m_type == NodeType::TEXT)
            {
                output << indent;

                if (options.escapeContent)
                    output << html_escape(m_content, false);
                else
                    output << m_content;
            }
            else if (m_type == NodeType::ELEMENT)
            {
                output << indent << "<" << m_name << "";

                // output classes if there are any to output
                if (!m_classes.empty())
                {
                    output << " class=\"";

                    for (size_t index = 0; index < m_classes.size(); index++)
                    {
                        output << m_classes.at(index);

                        if (index != m_classes.size() - 1)
                            output << " ";
                    }

                    output << "\"";
                }

                // output the ID of the class if one is specified
                if (!m_id.empty())
                    output << " id=\"" << m_id << "\"";

                for (const auto& attr : m_attributes)
                {
                    // escape the attribute value of invalid characters
                    std::string value = html_escape(attr.second);

                    // attributes with just the name are identical to blank valued attributes
                    // thus, output only the attribute name if a blank value is specified.
                    if (value.empty())
                        output << " " << attr.first;
                    else
                        output << " " << attr.first << "=\"" << value + "\"";
                }

                output << ">";

                if (options.formatting == StringFormatting::MULTIPLE_LINES)
                    output << "\n";

                // if we have a closing tag, then add children as well
                // as the closing tag to the output
                if (m_closeTag)
                {
                    for (const auto& child : m_children)
                        output << child.ToString(ToStringOptions(
                            options.formatting,
                            true,
                            options.indentLevel + 1,
                            true
                        ));

                    output << indent << "</" << m_name << ">";

                    if (options.formatting == StringFormatting::MULTIPLE_LINES && options.trailingNewline)
                        output << "\n";
                }
            }
            else if(m_type == NodeType::MULTIPLE_ELEMENTS)
            { 
                std::string to_output = "";
                
                for (const auto& child: m_children)
                { 
                    std::size_t pos = m_content.find("<"+child.Name()+">");   
                    pos += child.Name().length() + 2; // (tag) + (<) + (>) 
                    if(pos != std::string::npos)
                    {        
                        auto p0 = m_content.substr(0,pos);
                        auto p1 = m_content.substr(pos,m_content.length());     

                        p0 += child.Content();
                        to_output = p0 + p1; 
                    }
                }
                
                output << indent << to_output;
                if (options.formatting == StringFormatting::MULTIPLE_LINES && options.trailingNewline)
                    output << "\n";          
            }

            return output.str();
        }

        /**
         * Set the name of this element.
         * 
         * You may also enter a CSS selector-like string for the name to fill
         * in pieces of this element, such as classes, id, and attributes.
         * AppendChild
         * Also for this function, the element name should be the first piece
         * of the selector, and if it is not, the name will not be set correctly.
         */
        Node& SetName(const std::string& name)
        {
            std::vector<SelectorToken> tokens = parse_selector(name);

            bool firstToken = true;
            bool skipNext   = false;

            for (size_t index = 0; index < tokens.size(); index++)
            {
                if (skipNext)
                {
                    skipNext = false;

                    continue;
                }

                SelectorToken& token = tokens.at(index);

                // Cannot continue with selector if the first element is not
                // an actual element name token
                if (firstToken && token.type != SelectorTokenType::ELEMENT)
                    break;

                // For this method, only allow one name to be used at a time
                // thus any other name token will overwrite the name used.
                if (token.type == SelectorTokenType::ELEMENT)
                    this->m_name = token.value;

                // Add to the class list when a class token is hit
                if (token.type == SelectorTokenType::CLASS)
                    this->m_classes.push_back(token.value);

                // Overwrite the current ID if that token is hit
                if (token.type == SelectorTokenType::ID)
                    this->m_id = token.value;

                // Attributes are special in that the value can be ommitted for
                // a blank attribute, since that is still valid, a lookahead is
                // performed and if the value is found as the next token, it is
                // added to the attribute map and the next token is skipped.
                //
                // otherwise, the blank token is added
                if (token.type == SelectorTokenType::ATTRIBUTE_NAME)
                {
                    std::string attrValue = "";

                    // try and get the next token as a lookahead
                    if (tokens.size() > index + 1)
                    {
                        SelectorToken& next = tokens.at(index + 1);
                    
                        // found a value token, set the value string and skip
                        // this token after adding the attribute
                        if (next.type == SelectorTokenType::ATTRIBUTE_VALUE)
                        {
                            attrValue = next.value;
                        
                            skipNext = true;
                        }
                    }

                    m_attributes[token.value] = attrValue;
                }

                if (firstToken)
                    firstToken = false;
            }

            return *this;
        }

        /**
         * Return the element name for this Node.
         */
        std::string const& Name() const
        {
            return m_name;
        }

        /**
         * Get the value of an attribute associated with this element.
         * 
         * Returns blank if there is no attribute of that name.
         */
        std::string GetAttribute(const std::string& name) const
        {
            if (name == "class")
            {
                std::stringstream output;

                for (size_t index = 0; index < m_classes.size(); index++)
                {
                    output << m_classes.at(index);

                    if (index != m_classes.size() - 1)
                        output << " ";
                }

                return output.str();
            }

            if (name == "id")
                return m_id;

            auto find = m_attributes.find(name);

            if (find != m_attributes.end())
                return find->second;
            
            return "";
        }

        /**
         * Get a CSS selector like string for this element.
         */
        std::string GetSelector() const
        {
            std::stringstream output;

            output << m_name;

            // convert the class array to a string with dots
            for (auto& element : m_classes)
                output << "." << element;
            
            output << "#" << m_id;

            return output.str();
        }

        /**
         * Set a single attribute for a Node to a value.
         */
        Node& SetAttribute(std::string name, std::string value)
        {
            if (name == "id")
            {
                m_id = value;
                
                return *this;
            }

            // if trying to manually set a class attribute, split the value by
            // spaces and then add the classes to the class list
            if (name == "class")
            {
                m_classes.clear();

                // create a stringstream from the value for use in splitting
                std::istringstream stream(value);

                std::string temp = "";

                // getline also works with a specific delimeter, allowing us
                // to split by spaces easily
                while (std::getline(
                    stream,
                    temp,
                    ' '
                ))
                    m_classes.push_back(temp);

                return *this;
            }

            m_attributes[name] = value;
            
            return *this;
        }

        /**
         * Set the node type for this Node instance.
         */
        Node& SetType(NodeType type)
        {
            this->m_type = type;
        
            return *this;
        }

        /**
         * Sets the content of a non-element node.
         */
        Node& SetContent(const std::string& text)
        {
            this->m_content = text;
        
            return *this;
        }

        std::string const& Content() const
        {
            return this->m_content;
        }

        /**
         * Set the parent instance of this node.
         */
        Node& SetParent(Node* parent)
        {
            this->m_parent = parent;

            return *this;
        }

        /**
         * Toggle the state of a class based on its name.
         */
        Node& ToggleClass(std::string className)
        {
            std::vector<std::string>::iterator find = std::find(
                m_classes.begin(),
                m_classes.end(),
                className
            );
            
            // if the class exists, remove it, otherwise add it
            if (find != m_classes.end())
                m_classes.erase(find);
            else
                m_classes.push_back(className);

            return *this;
        }
        
        /**
         * Append a constant child node to this node.
         * 
         * This method is used mostly when constructing a node in an append call
         * after which then the parent node takes ownership of the node.
         */
        Node& AppendChild(const Node& child)
        {
            m_children.push_back(child);

            // once we push back, set the parent in vector so it is not lost
            m_children.back().SetParent(this);

            return *this;
        }

        /**
         * Append a child node to this node.
         * 
         * This method takes a reference to the node to be appended and changes
         * its parent to this node.
         * 
         * This is for when using the Remove() call on a node outside of the
         * children vector.
         */
        Node& AppendChild(Node& child)
        {
            m_children.push_back(child); 

            child.SetParent(this);

            return *this;
        }

        /**
         * Append a single text node to the element.
         * 
         * This is the recommended way to set content now
         * as opposed to using the SetContent method.
         */
        Node& AppendText(std::string text)
        {
            Node textNode;

            textNode.SetType(NodeType::TEXT)
                    .SetContent(text)
                    .SetParent(this);

            m_children.push_back(textNode);

            return *this;
        }

        /**
         * Removes this particular node from its parent.
         */
        void Remove()
        {
            std::vector<Node> parentChildren = m_parent->GetChildren();

            auto it = std::find_if(
                parentChildren.begin(),
                parentChildren.end(),
                [&](Node const& child) { return child.GetSelector() == this->GetSelector(); }
            );

            if (it != parentChildren.end())
                m_parent->RemoveChild(std::distance(parentChildren.begin(), it));
        }

        /**
         * Remove a Node from the children by index.
         * 
         * The index is zero-based for removal.
         */
        Node& RemoveChild(size_t index)
        {
            m_children.erase(m_children.begin() + index);

            return *this;
        }

        /**
         * Get a single child by its element name.
         */
        Node& GetChildByName(const std::string& name)
        {
            auto it = std::find_if(
                m_children.begin(),
                m_children.end(),
                [&name](Node const& child) { return child.Name() == name; }
            );

            return *it;
        }

        /**
         * Set whether or not this element should have a closing tag or not.
         */
        Node& UseClosingTag(bool close)
        {
            this->m_closeTag = close;

            return *this;
        }

        /**
         * Get the vector of child nodes from this node instance.
         */
        std::vector<Node> GetChildren()
        {
            return m_children;
        }

    private:
        /**
         * The parent node for this Node instance.
         * 
         * This is stored for purposes of removing the node and should be set
         * on an AppendChild call.
         */
        Node* m_parent = nullptr;

        /**
         * The type of Node for this instance.
         * 
         * Defaults to an ELEMENT.
         */
        NodeType m_type = NodeType::ELEMENT;

        /**
         * The name of the current node.
         * 
         * Only used in elements, and represents a tag name such as `div`.
         */
        std::string m_name = "";

        /**
         * A list of classes for the current Node.
         * 
         * Only used with an element type node.
         */
        std::vector<std::string> m_classes;

        /**
         * A singular ID for this element.
         */
        std::string m_id = "";

        /**
         * The content for this Node instance.
         * 
         * Only used for text and document type nodes. For text it is the actual
         * content that is sent when converting to a string. For a document type
         * it is the actual type to set, such as `html`.
         */
        std::string m_content = "";

        /**
         * Whether or not to close this current Node if it is an element.
         * 
         * If this is false, no children are to be printed after the node
         * as there would be nowhere to place them.
         * 
         * Defaults to true.
         */
        bool m_closeTag = true;

        /**
         * The child nodes of this Node instance.
         */
        std::vector<Node> m_children;

        /**
         * A map of attribute keys to values for a node.
         * 
         * This is only used for elements.
         */
        std::unordered_map<std::string, std::string> m_attributes;
    };

    /**
     * A simple class that represents a HTML5 document with an <html> tag
     * that houses <head> and <body> tags.
     */
    class Document
    {
    public:
        /**
         * Construct a simple HTML5 document with a head and body.
         */
        Document()
            :
            m_doctype(NodeType::DOCUMENT_TYPE, "html"),
            m_html("html")
        {
            // append a head and body tag to the html
            this->m_html.AppendChild(Node("head"));
            this->m_html.AppendChild(Node("body"));
        }

        /**
         * Append a single node element to the <head> tag.
         */
        void AppendNodeToHead(const Node& node)
        {
            this->head().AppendChild(node);
        }

        /**
         * Append a single node to the <body> tag.
         */
        void AppendNodeToBody(const Node& node)
        {
            this->body().AppendChild(node);   
        }

        /**
         * Grab the entire document as a string, with an optional
         * StringFormatting enum accepted to change between
         * outputting one line and multiple lines.
         */
        std::string ToString(ToStringOptions options={}) const
        {
            std::stringstream output;

            output << m_doctype.ToString(options);

            output << m_html.ToString(options);

            return output.str();
        }

        /**
         * Return the root HTML document node.
         */
        Node& html()
        {
            return this->m_html;
        }

        /**
         * Return the head element for the document.
         */
        Node& head()
        {
            return m_html.GetChildByName("head");
        }

        /**
         * Return the body element for the document.
         */
        Node& body()
        {
            return m_html.GetChildByName("body");
        }

    private:
        /**
         * The doctype node for this document.
         * 
         * Defaults to be a HTML5 doctype.
         */
        Node m_doctype;

        /**
         * The root HTML tag for this document.
         */
        Node m_html;

    };
}
#endif

