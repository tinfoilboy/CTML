# CTML

CTML is a C++ HTML document constructor, that was designed to be simple to use and implement. Has no dependencies on any other projects, only the C++ standard library.

# Including

To use CTML in your project, just point your compiler to the folder containing the `CTML.h`, `Document.h`, and `Node.h` files. Once you are done with that, just do `#include "CTML.h"` on the files where you use CTML, and voila, you're done.

# Usage

###### Including the Classes 

To get all of the classes provided in CTML just use, `#include "CTML.h"`.

###### Namespacing

Everything in CTML is namespaced with `CTML`. So for example, the `Node` class would be under `CTML::Node`

###### Documents

CTML provides a class for creating a simple HTML document, which is `CTML::Document`.

This creates a document with a head, and body tag. As well as a DOCTYPE tag.

The Document includes two methods for getting to a string. `CTML::Document::ToString(bool)` and `CTML::Document::ToTree()``.

There is only one argument in `CTML::Document::ToString(bool)`, which is `readable` which determines if the document should be returned as all one line. Or as an indented, and multiline string.

`CTML::Document::ToTree()` returns the document as a tree view that reflects the nesting of the actual document.

A simple example that returns the document as a string to the console is below.

```cpp
#include "CTML.h"
#include <iostream>

int main ()
{
    CTML::Document document = CTML::Document();
    std::cout << document.ToString(true);
}
```

The result of running this program in the console is below.

```
<!DOCTYPE html>
<html>
    <head>
    </head>
    <body>
    </body>
</html>
```

There is also the `CTML::Document::WriteToFile(std::string, bool)` method, which opens a stream to `filePath` and outputs the document string to the stream. `readable` determines whether or not the file should be written as inline or not.

Below is an example of `CTML::Document::WriteToFile(std::string, bool)`.

```cpp
#include "CTML.h"

int main()
{
    CTML::Document doc = CTML::Document();
    doc.AddNodeToBody(CTML::Node("a.link").SetContent("Anchor").SetAttribute("href", "http://www.example.com"));
    return doc.WriteToFile("index.html", true);
}
```

Which saves the document to the `index.html` file next to the executable, with the output of this.

```html
<!DOCTYPE html>
<html>
    <head>
    </head>
    <body>
        <a class="link" href="http://www.example.com">
            Anchor
        </a>
    </body>
</html>
```

###### Nodes

Along with the `CTML::Document` class, CTML provides a `CTML::Node` class. `CTML::Node` is the basis of all element representation in CTML.

`CTML::Node` contains eight methods for manipulation of a current node. Almost all of these methods are chainable.

There are two methods for getting a string from `CTML::Node`. The first of which is `CTML::Node::ToString(bool, int)`, which returns the current node and all of its children as a string representing each element. The parameter, `readable`, determines if the document should be returned as all one line. Or as an indented, and multiline string. The other parameter, `indentLevel` is an integer representing how many indents (four spaces each) should be used if `readable` is true. This is ignored if `readable` is false.

The other method is `CTML:::Node:GetTreeString(int)`, which returns the current node and it's children as a tree view. The `indentLevel` parameter is an integer representing how many indents (four spaces each) should be used in representing the nesting of the nodes.

**Note all of the methods below are chainable**

The `CTML::Node::SetName(std::string)` method sets the current name of the Node, such as div, span, e.t.c. You can also add classes to the name. For example, if you type `div.container.fluid` as the name, `CTML::Node::ToString(true, 0)` would return `<div class="container fluid"></div>`

The `CTML::Node::SetAttribute(std::string, std::string)` method sets an attribute determined by the `name` parameter to the `value` parameter. For example, `CTML::Node::SetAttribute("href", "#test")` would set the node's href attribute to `test`

The `CTML::Node::SetType(CTML::NodeType)` method sets the current type of the node, can be either `ELEMENT` or `DOCUMENT_TYPE`.

The `CTML::Node::SetContent(std::string)` method sets the current text content of the node, this is always outputted in the beginning of the node, before the children, unless the node type is `DOCUMENT_TYPE` in which then it is right after the `!DOCTYPE ` string.

The `CTML::Node::ToggleClass(std::string)` method either adds or removes a class from an element, depending on if the class is already on the element.

The `CTML::Node::AppendChild(CTML::Node)` method adds a node to this node as a child.

The `CTML::Node::SetUseBr(bool)` method forces use of the `<br>` tag instead of `\n` in this element's content.

Below is an example of a document with a div in the body, with an a tag as the child.

```cpp
#include "CTML.h"

int main()
{
    CTML::Document doc = CTML::Document();
    doc.AddNodeToBody(CTML::Node("a.link").SetContent("Anchor").SetAttribute("href", "http://www.example.com"));
    std::cout << doc.ToString(true);
    return 0;
}
```

Which returns the following in the console...

```
<!DOCTYPE html>
<html>
    <head>
    </head>
    <body>
        <a class="link" href="http://www.example.com">
            Anchor
        </a>
    </body>
</html>
```

# Credits

[Tinfoilboy - Project Creator](https://github.com/tinfoilboy)

# License

This project (CFML) is licensed under the MIT License, the terms can be seen [here](https://github.com/tinfoilboy/CFML).
