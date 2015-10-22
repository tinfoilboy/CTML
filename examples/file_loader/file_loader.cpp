#include "CTML.h"
#include "dirent.h"

bool add_files_to_doc(std::string folderPath, CTML::Node* containerNode, bool readable) {
	DIR *directory = NULL;
	// open the folder path
	directory = opendir(folderPath.c_str());
	// return false if it doesn't exist
	if (directory == NULL) {
		std::cout << "Directory not found!\n";
		return false;
	}
	// the directory entry struct
	struct dirent *ent;
	while ((ent = readdir(directory)) != NULL)
	{
		// if the current entry is a file
		if (ent->d_type == DT_REG) {
			// the file name as a string
			std::string fileName = std::string(ent->d_name);
			// the content of the document
			std::string docContent;
			std::ifstream textFile(folderPath + fileName);
			std::string line;
			// loop through the lines in the file
			while (std::getline(textFile, line))
			{
				// append the line to the doc content
				if (readable) docContent += line + '\n';
				else docContent += line;
			}
			// hacky way of removing the last new line in the doc content
			int lastNewLine = docContent.find_last_of('\n');
			if (lastNewLine != std::string::npos)
				docContent.erase(docContent.find_last_of('\n'));
			// remove the extention from the file name
			fileName = fileName.substr(0, fileName.find('.'));
			// the container for this file
			CTML::Node fileContainer = CTML::Node("div.post");
			CTML::Node fileTitle = CTML::Node("h1", fileName);
			CTML::Node fileContent = CTML::Node("p", docContent);
			// since we are using a paragraph element, we need to use br tags for newlines, force this onn the element
			fileContent.SetUseBr(true);
			// append the title and content to the file container
			fileContainer.AppendChild(fileTitle).AppendChild(fileContent);
			// append the file container to the root container
			containerNode->AppendChild(fileContainer);
		}
	}
	// if we can't close the directory for some reason
	if (closedir(directory) < 0) {
		std::cout << "Could not close the stream to the directory!\n";
		return false;
	}
	// finished
	return true;
}

int main()
{
	CTML::Document doc = CTML::Document();
	doc.AddNodeToHead(CTML::Node("title", "CTML Test"));
	// force utf-8 encoding
	doc.AddNodeToHead(CTML::Node("meta").SetAttribute("charset", "UTF-8"));
	CTML::Node containerNode = CTML::Node("div.container");
	std::string dir;
	// input the directory to look for files
	std::cout << "Input the directory name with the files: ";
	std::cin >> dir;
	bool result = add_files_to_doc(dir, &containerNode, true);
	// add the files container to the body
	doc.AddNodeToBody(containerNode);
	bool fileWriteResult = false;
	// if finding files was successful, try to write to a file
	if (result)
		fileWriteResult = doc.WriteToFile("index.html", true);
	// if we wrote to the file successfully
	if (fileWriteResult)
		std::cout << "All files in \"" + dir + "\" written to index.html successfully!\n";
	else
		std::cout << "Writing to index.html was unsuccessful!\n";
	return fileWriteResult;
}