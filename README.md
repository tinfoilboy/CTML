# CTML

[![Build Status](https://travis-ci.com/tinfoilboy/CTML.svg?branch=master)](https://travis-ci.com/tinfoilboy/CTML)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Github Releases](https://img.shields.io/github/release/tinfoilboy/CTML.svg)](https://github.com/tinfoilboy/CTML/releases)

CTML is a C++ HTML document constructor, that was designed to be simple to use and implement.
Has no dependencies on any other projects, only the C++ standard library.

## Building

For use in a project, you may copy the `ctml.hpp` file into your project and include that way.
Alternatively, if you use CMake, you could add CTML as a dependency to your project.

## Tests

Tests are included with the library and are written using the [Catch2](https://github.com/catchorg/Catch2) header-only test library.
These tests are located in the `tests/tests.cpp` file.

## Usage

### Namespacing

Every class and enum in CTML is enclosed in the `CTML` namespace.
For instance, the `Node` class would be under `CTML::Node`.

### Chaining

Most methods for operating on `CTML::Node` instances are chainable, meaning that you may run these operations multiple times on the same expression.

### Nodes

The basis of CTML is the `CTML::Node` class, which allows you to create
simple HTML nodes and convert them to a `std::string` value.

The most simple valid HTML node that can be represented is, for instance, an empty paragraph tag, which can be created with the following code.

```cpp
CTML::Node node("p");
```

Which would output in string form as:

```html
<p></p>
```

To get this string output, you would use the `CTML::Node::ToString(CTML::ToStringOptions)` method. This method outputs a string representation of the Node and its children using the options supplied.

This `ToStringOptions` structure allows the user to change whether the string outputs elements using multiple lines or one line, if nodes should have a trailing new line at the end, the indentation level of the node if outputting to multiple lines, and whether text content of an element should be escaped.

You can add simple text content to this Node by changing that line to the following:

```cpp
CTML::Node node("p", "Hello world!");
```

Which would output as the following:

```html
<p>Hello world!</p>
```

You can quickly add classes and IDs to a Node (possibly attributes in the future) with a syntax in the name field that mimics Emmet Abbriviations.
This is shown in the following definition:

```cpp
CTML::Node node("p.text#para", "Hello world!");
```

Which would output the following HTML:

```html
<p class="text" id="para">Hello world!</p>
```

You can then append children to these Node instances by using the `CTML::Node::AppendChild(CTML::Node)` method, like below:

```cpp
CTML::Node node("div");

node.AppendChild(CTML::Node("p", "Hello world!"));
```

Which would give this output:

```html
<div><p>Hello world!</p></div>
```

You can also append more text to the parent node with the `CTML::Node::AppendText(std::string)` method, which simply
adds a Node with the type of `TEXT` to the children.
This is shown below:

```cpp
CTML::Node node("div");

node.AppendChild(CTML::Node("p", "Hello world!"))
    .AppendText("Hello again!");
```

Which would output as:

```html
<div><p>Hello world!</p> Hello again!</div>
```

You can also set attributes on a Node, modifying the below example to do so looks like:

```cpp
CTML::Node node("div");

node.SetAttribute("title", "Hello title!")
    .AppendChild(CTML::Node("p", "Hello world!"))
    .AppendText("Hello again!");
```

Which would output as:

```html
<div title="Hello title!"><p>Hello world!</p> Hello again!</div>
```

If you wish to make a self-closing element, such as an `<img>` element, you will want to use the `CTML::Node::UseClosingTag(bool)` method. This method allows you to toggle the use of the closing tag. Keep in mind that toggling this also doesn't append any of the child nodes to the output. An example of the method is below:

```cpp
CTML::Node image("img");

image.SetAttribute("src", "/animage.png")
     .UseClosingTag(false);
```

This has the following output:

```html
<img src="/animage.png">
```

### Documents

To create an HTML document that contains these nodes, you can use the `CTML::Document` class. This class includes doctype, head, and body nodes for adding nodes to.

A simple HTML document would be created with:

```cpp
CTML::Document document;
```

You can then output this as a string with the `CTML::Document::ToString(CTML::ToStringOptions)` method. This method uses the same structure that the node class uses.

By using the default empty `Document::ToString` method you would get an output of:

```html
<!DOCTYPE html><html><head></head><body></body></html>
```

You can then append nodes to it using the `CTML::Document::AppendNodeToHead(CTML::Node)` or `CTML::Document::AppendNodeToBody(CTML::Node)` methods.

### Searching Nodes

There are two ways to search through the document tree for nodes. The first of these ways is to use the `CTML::Node::GetChildByName(const std::string&)` method.
This will only search the node's immediate children without recursion, and only operates based on the name of the node, such as `div` or `section`. In addition for legacy reasons, it will only return a single node, even if there are multiple matches.

If you need more fine-grained searching for nodes, you can use the `CTML::Node::QuerySelector(const std::string&)` method, which also has an alias of `CTML::Document::QuerySelector(const std::string&)` for searching the entire document for matches. This method operates similarly to `querySelectorAll` in JavaScript, in that it will recursively search the a node's children for matches to the selector, and return only that node match.

For example, if you have the following document:

```html
<div class="one"><div class="two"><div class="three"></div></div><div>
```

And you use the following code:

```cpp
std::vector<Node*> matches = CTML::Document::QuerySelector(".one .two .three")
```

The `matches` vector will only have one child, a pointer to the node for `div.three`. This method supports searching by any combination of element name, attribute name and value, class, and ID.

**Note:** This method currently only supports searching attributes as by checking equality. Any other CSS selector equivalency tests are not supported currently.

## License

CTML is licensed under the MIT License, the terms of which can be seen [here](https://github.com/tinfoilboy/CTML/blob/master/LICENSE).
