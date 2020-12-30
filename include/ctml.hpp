/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2020 Maxwell Flynn
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
    inline std::string replace_all(
        std::string& original,
        const std::string& target,
        const std::string& replacement)
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
    inline std::string html_escape(std::string value, bool escape_quotes=true)
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

    inline bool string_starts_with(const std::string& src, const std::string& comp)
    {
        if (src.size() < comp.size())
        {
            return false;
        }

        for (size_t index = 0; index < comp.size(); index++)
        {
            // if one character in sequence is not equal, then return false
            if (comp[index] != src[index])
            {
                return false;
            }
        }

        return true;
    }

    inline bool string_ends_with(const std::string& src, const std::string& comp)
    {
        if (src.size() < comp.size())
        {
            return false;
        }

        size_t compIndex = comp.size() - 1;

        for (size_t index = src.size() - 1; index > 0; index--)
        {
            // if one character in sequence is not equal, then return false
            if (comp[compIndex] != src[index])
            {
                return false;
            }

            if (compIndex > 0)
            {
                compIndex--;
            }
            else
            {
                break;
            }
        }

        return true;
    }

    /**
     * Internal structure for anything that compares strings by word boundaries.
     * 
     * Instead of copying strings for substr, this just stores the begin and end iterators that represent the word in
     * the string. Which then is iterated over for comparison to the search word.
     */
    struct WordBounds
    {
        std::string::const_iterator begin;
        std::string::const_iterator end;

        WordBounds() = default;

        WordBounds(std::string::const_iterator begin, std::string::const_iterator end)
            : begin(begin)
            , end(end) {}

        bool equals(const std::string& string)
        {
            // add one to distance to account for full size
            size_t boundLength = std::distance(begin, end);

            // word cannot equal string if they are not the same size
            if (boundLength != string.size())
            {
                return false;
            }

            size_t index = 0;

            for (auto& itr = begin; itr != end; ++itr)
            {
                if ((*itr) != string[index])
                {
                    return false;
                }

                index++;
            }

            return true;
        }
    };

    inline bool string_contains_word(const std::string& src, const std::string& word)
    {
        if (src.size() < word.size())
        {
            return false;
        }

        // split string into words by space
        std::vector<WordBounds> words;
        
        size_t beginIndex = 0;

        for (size_t index = 0; index < src.size(); index++)
        {
            char current = src.at(index);

            if (current == ' ')
            {
                // start the word at the begin index, and end it at the index minus one (last char before space)
                words.push_back({ src.begin() + beginIndex, src.begin() + index });

                // start the next word boundary at the subsequent character following this space
                beginIndex = (index + 1);

                continue;
            }
        }

        // check if we have anything left over for this string for words, and if so, push the final word back
        if (beginIndex < word.size())
        {
            words.push_back({ src.begin() + beginIndex, src.end() });
        }

        for (auto& bound : words)
        {
            // if equals between a bound and word equals true, then we can return that we found a word
            if (bound.equals(word))
            {
                return true;
            }
        }

        return false;
    }

    /**
     * This is probably an *awful* function name but it's to support the |= op for selector attributes
     * 
     * The operator basically checks if a string is the whole word *or* if the string starts with the word followed by
     * a hyphen.
     */
    inline bool string_is_or_begin_hyphen(const std::string& src, const std::string& word)
    {
        if (src == word)
        {
            return true;
        }

        if (!string_starts_with(src, word))
        {
            return false;
        }

        // if the string isn't big enough to contain the word plus a hyphen, ignore
        if (src.size() < word.size() + 1)
        {
            return false;
        }

        // since sizes aren't zero based, this should be where the hyphen is
        if (src.at(word.size()) != '-')
        {
            return false;
        }

        return true;
    }

    /**
     * An enum representing the types of HTML nodes that can be constructed.
     */
    enum class NodeType : uint8_t
    {
        COMMENT,
        DOCUMENT_TYPE,
        ELEMENT,
        TEXT,
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
        ATTRIBUTE_VALUE,
    };

    /**
     * An enum representing the formatting of the ToString for a Node.
     */
    enum class StringFormatting : uint8_t
    {
        SINGLE_LINE,
        MULTIPLE_LINES,
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
        ATTRIBUTE_VALUE,
        ATTRIBUTE_COMPARE,
        SELECTOR_SEPARATOR, // To support searching by selector as well as advanced construction
    };

    /**
     * An enum for the different types of comparison methods that an attribute can have.
     */
    enum class AttributeComparisonType : uint8_t
    {
        NONE,
        ATTRIBUTE_EQUAL,
        ATTRIBUTE_CONTAINS,
        ATTRIBUTE_CONTAINS_WORD,
        ATTRIBUTE_STARTS_WITH,
        ATTRIBUTE_IS_OR_BEGIN_HYPHEN,
        ATTRIBUTE_ENDS_WITH,
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
            bool escapeContent=true)
            : formatting(formatting)
            , trailingNewline(trailingNewLine)
            , indentLevel(indentLevel)
            , escapeContent(escapeContent) {}
    };

    /**
     * A struct for selector tokens from parsing a selector.
     */
    struct SelectorToken
    {
        SelectorTokenType       type       = SelectorTokenType::ELEMENT;
        std::string             value      = "";
        AttributeComparisonType comparison = AttributeComparisonType::NONE;

        SelectorToken(
            SelectorTokenType type=SelectorTokenType::ELEMENT,
            std::string value="",
            AttributeComparisonType comparison=AttributeComparisonType::NONE)
            : type(type)
            , value(value)
            , comparison(comparison) {}
    };

    /**
     * Adds a single selector token to the token vector passed in based on the
     * state of the parser that was passed.
     */
    inline void add_selector_token(
        std::vector<SelectorToken>& tokens,
        SelectorParserState state,
        const std::string& value)
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
     * Grabs the character that is one ahead of the index passed in.
     * 
     * Used as a convenience method for selector parsing to get a lookahead while also doing bounds checking.
     * 
     * Returns either the character that is at the position one ahead of the index, or a null char.
     */
    inline const char get_lookahead_char(const std::string& selector, size_t index)
    {
        if (selector.size() <= index + 1)
        {
            return '\0';
        }

        return selector.at(index + 1);
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
    inline std::vector<SelectorToken> parse_selector(const std::string& selector)
    {
        std::vector<SelectorToken> tokens;

        // start the parser to be parsing an element by default
        SelectorParserState state = SelectorParserState::ELEMENT;

        std::string temp = "";

        // whether to skip the following character in the selector, used for attribute matching parsing when we perform
        // a lookahead for the equal sign on a character, if the following character is an equal sign, then we skip it
        bool skipNext = false;

        for (size_t index = 0; index < selector.size(); index++)
        {
            const char& current = selector.at(index);

            if (skipNext)
            {
                skipNext = false;

                continue;
            }

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

                // add an extra token to the token vector that signifies a split
                tokens.push_back({ SelectorTokenType::SELECTOR_SEPARATOR, "" });

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
            else if (current == '~' && state == SelectorParserState::ATTRIBUTE_NAME && get_lookahead_char(selector, index) == '=')
            {
                if (!temp.empty())
                    add_selector_token(tokens, state, temp);

                // add an extra token for selector match equal
                tokens.push_back({ SelectorTokenType::ATTRIBUTE_COMPARE, "~=", AttributeComparisonType::ATTRIBUTE_CONTAINS_WORD });

                temp = "";

                state = SelectorParserState::ATTRIBUTE_VALUE;

                skipNext = true;

                continue;
            }
            else if (current == '|' && state == SelectorParserState::ATTRIBUTE_NAME && get_lookahead_char(selector, index) == '=')
            {
                if (!temp.empty())
                    add_selector_token(tokens, state, temp);

                // add an extra token for selector match equal
                tokens.push_back({ SelectorTokenType::ATTRIBUTE_COMPARE, "|=", AttributeComparisonType::ATTRIBUTE_IS_OR_BEGIN_HYPHEN });

                temp = "";

                state = SelectorParserState::ATTRIBUTE_VALUE;

                skipNext = true;

                continue;
            }
            else if (current == '^' && state == SelectorParserState::ATTRIBUTE_NAME && get_lookahead_char(selector, index) == '=')
            {
                if (!temp.empty())
                    add_selector_token(tokens, state, temp);

                // add an extra token for selector match equal
                tokens.push_back({ SelectorTokenType::ATTRIBUTE_COMPARE, "^=", AttributeComparisonType::ATTRIBUTE_STARTS_WITH });

                temp = "";

                state = SelectorParserState::ATTRIBUTE_VALUE;

                skipNext = true;

                continue;
            }
            else if (current == '$' && state == SelectorParserState::ATTRIBUTE_NAME && get_lookahead_char(selector, index) == '=')
            {
                if (!temp.empty())
                    add_selector_token(tokens, state, temp);

                // add an extra token for selector match equal
                tokens.push_back({ SelectorTokenType::ATTRIBUTE_COMPARE, "$=", AttributeComparisonType::ATTRIBUTE_ENDS_WITH });

                temp = "";

                state = SelectorParserState::ATTRIBUTE_VALUE;

                skipNext = true;

                continue;
            }
            else if (current == '*' && state == SelectorParserState::ATTRIBUTE_NAME && get_lookahead_char(selector, index) == '=')
            {
                if (!temp.empty())
                    add_selector_token(tokens, state, temp);

                // add an extra token for selector match equal
                tokens.push_back({ SelectorTokenType::ATTRIBUTE_COMPARE, "*=", AttributeComparisonType::ATTRIBUTE_CONTAINS });

                temp = "";

                state = SelectorParserState::ATTRIBUTE_VALUE;

                skipNext = true;

                continue;
            }
            else if (current == '=' && state == SelectorParserState::ATTRIBUTE_NAME)
            {
                if (!temp.empty())
                    add_selector_token(tokens, state, temp);

                // add an extra token for selector match equal
                tokens.push_back({ SelectorTokenType::ATTRIBUTE_COMPARE, "=", AttributeComparisonType::ATTRIBUTE_EQUAL });

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

    using SelectorTokenItr = std::vector<SelectorToken>::iterator;

    /**
     * Structure used in selector-based search to reference groups of selectors for querying individual node matches.
     * 
     * Only stores begin and end iterators to cut down on copying/memory usage.
     */
    struct SelectorGroup
    {
        SelectorTokenItr begin;
        SelectorTokenItr end;

        SelectorGroup() = default;

        SelectorGroup(SelectorTokenItr begin, SelectorTokenItr end)
            : begin(begin)
            , end(end) {}
    };

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
        Node(const NodeType& type,
            std::string name="",
            std::string content="")
            : m_type(type)
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
        }

        /**
         * Create an element node with the name specified.
         */
        Node(std::string name)
            : m_type(NodeType::ELEMENT)
        {
            this->SetName(name);
        }

        /**
         * Create an element node with the specified name and containing a text node
         * with the content specified.
         */
        Node(std::string name, std::string content)
            : m_type(NodeType::ELEMENT)
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

            return output.str();
        }

        /**
         * Set the name of this element.
         * 
         * You may also enter a CSS selector-like string for the name to fill
         * in pieces of this element, such as classes, id, and attributes.
         * 
         * Also for this function, the element name should be the first piece
         * of the selector, and if it is not, the name will not be set correctly.
         */
        Node& SetName(const std::string& name)
        {
            std::vector<SelectorToken> tokens = parse_selector(name);

            return SetName(std::move(tokens));
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
         * 
         * Note: This method only will return one child that is referenced by its name. If you need multiple matches
         * or need more fine-grained search with classes, IDs, or attributes, use the QuerySelector method.
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
         * Searches recursively through the child nodes to find matches to the provided string selector.
         * 
         * Keep in mind that this will only search *this* node's *children*, it will not return the current node.
         */
        std::vector<Node*> QuerySelector(const std::string& selector)
        {
            std::vector<SelectorToken> tokens = parse_selector(selector);
            std::vector<SelectorGroup> groups;
            size_t currentGroup = 0;

            // start by pushing an initial group that spans the whole selector token vector, this will be updated if
            // there are other groups in this vector 
            groups.push_back(SelectorGroup(tokens.begin(), tokens.end()));

            for (size_t index = 0; index < tokens.size(); index++)
            {
                auto& token = tokens.at(index);

                if (token.type == SelectorTokenType::SELECTOR_SEPARATOR)
                {
                    // end this group at the previous token
                    groups[currentGroup].end = tokens.begin() + (index - 1);

                    // create a new group at the token after this one (if the vector has more)
                    if (tokens.size() <= index + 1)
                    {
                        break;
                    }

                    groups.push_back(SelectorGroup(tokens.begin() + (index + 1), tokens.end()));

                    currentGroup++;
                }
            }

            std::vector<Node*> matches;

            // now that we have selector groups, we can start our search for matches
            for (auto& child : m_children)
            {
                std::vector<Node*> submatches = child.GetSelectorMatches(groups, 0);

                matches.insert(matches.end(), submatches.begin(), submatches.end());
            }

            return matches;
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

    protected:
        /**
         * Check if any child nodes within this Node match the selector group passed in.
         * 
         * Any matches will be in the returned vector. 
         */
        std::vector<Node*> GetSelectorMatches(std::vector<SelectorGroup>& groups, size_t currentIndex)
        {
            std::vector<Node*> matches;

            auto& group = groups.at(currentIndex);
            
            for (auto& node : m_children)
            {
                if (node.SelectorMatch(group.begin, group.end))
                {
                    // check if this is the last group, if so, add this node to the vector
                    if (currentIndex + 1 == groups.size())
                    {
                        matches.push_back(&node);
                    }
                    else
                    {
                        std::vector<Node*> submatches = node.GetSelectorMatches(groups, currentIndex + 1);

                        matches.insert(matches.end(), submatches.begin(), submatches.end());
                    }
                }

                // we still want to search from the beginning of the selector on this node to recurse, so run this
                // method again but instead with the currentIndex at 0 to restart the search at this node
                std::vector<Node*> recurse = node.GetSelectorMatches(groups, 0);
                
                matches.insert(matches.end(), recurse.begin(), recurse.end());
            }

            return matches;
        }

        /**
         * Compares the current node to a list of tokens represented by a begin and end iterator.
         */
        bool SelectorMatch(SelectorTokenItr begin, SelectorTokenItr end)
        {
            // variable used for storing name of an attribute for checking against
            std::string attribName = "";

            // comparison method used for this particular attribute, set to NONE when not an attrib
            AttributeComparisonType attribComp = AttributeComparisonType::NONE;

            for (auto& itr = begin; itr != end; ++itr)
            {
                if (itr->type == SelectorTokenType::ELEMENT && itr->value != m_name)
                {
                    return false;
                }

                if (itr->type == SelectorTokenType::ID && itr->value != m_id)
                {
                    return false;
                }

                if (itr->type == SelectorTokenType::ATTRIBUTE_NAME)
                {
                    attribName = itr->value;

                    continue;
                }

                if (itr->type == SelectorTokenType::ATTRIBUTE_COMPARE)
                {
                    attribComp = itr->comparison;
                    
                    continue;
                }

                if (itr->type == SelectorTokenType::CLASS)
                {
                    auto find = std::find_if(m_classes.begin(), m_classes.end(), [&](const std::string& cls) {
                        return cls == itr->value;
                    });

                    if (find == m_classes.end())
                    {
                        return false;
                    }
                }

                if (itr->type == SelectorTokenType::ATTRIBUTE_VALUE)
                {
                    auto find = std::find_if(m_attributes.begin(), m_attributes.end(), [&](std::pair<std::string, std::string> val) {
                        if (val.first != attribName)
                        {
                            return false;
                        }

                        if (attribComp == AttributeComparisonType::ATTRIBUTE_EQUAL && val.second != itr->value)
                        {
                            return false;
                        }

                        if (attribComp == AttributeComparisonType::ATTRIBUTE_CONTAINS)
                        {
                            auto contains = val.second.find(itr->value);

                            if (contains == std::string::npos)
                            {
                                return false;
                            }
                        }

                        if (attribComp == AttributeComparisonType::ATTRIBUTE_STARTS_WITH && !string_starts_with(val.second, itr->value))
                        {
                            return false;
                        }

                        if (attribComp == AttributeComparisonType::ATTRIBUTE_ENDS_WITH && !string_ends_with(val.second, itr->value))
                        {
                            return false;   
                        }

                        if (attribComp == AttributeComparisonType::ATTRIBUTE_CONTAINS_WORD && !string_contains_word(val.second, itr->value))
                        {
                            return false;   
                        }

                        if (attribComp == AttributeComparisonType::ATTRIBUTE_IS_OR_BEGIN_HYPHEN && !string_is_or_begin_hyphen(val.second, itr->value))
                        {
                            return false;   
                        }

                        return true;
                    });

                    if (find == m_attributes.end())
                    {
                        return false;
                    }

                    attribName.clear();
                    attribComp = AttributeComparisonType::NONE;
                }
            }

            return true;
        }

        /**
         * Set the name of this element from a vector of selector tokens.
         * 
         * Used internally by SetName(const std::string&) for nested node creation with selectors.
         */
        Node& SetName(std::vector<SelectorToken>&& tokens)
        {
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

                // this could be an odd way to handle this, but for supporting creating multiple nested elements
                // from a name selector, just check if we are at one such separator, then create a new node from
                // the remaining subset of tokens that were parsed.
                if (token.type == SelectorTokenType::SELECTOR_SEPARATOR && tokens.size() > index + 1)
                {
                    Node childNode;
                    childNode.SetName(std::vector<SelectorToken>(tokens.begin() + (index + 1), tokens.end()));

                    this->AppendChild(childNode);

                    break;
                }

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

                    // try and get the token after the ATTRIBUTE_COMPARE token as a lookahead
                    // for construction, the compare token is ignored to just set the token value
                    if (tokens.size() > index + 2)
                    {
                        SelectorToken& next = tokens.at(index + 2);
                    
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
            : m_doctype(NodeType::DOCUMENT_TYPE, "html")
            , m_html("html")
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
         * Searches a selector from the root of the document.
         * 
         * Essentially shorthand for document.html().QuerySelector(const std::string&).
         */
        std::vector<Node*> QuerySelector(const std::string& selector)
        {
            return m_html.QuerySelector(selector);
        }

        /**
         * Return the root HTML document node.
         */
        Node& html()
        {
            return m_html;
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
