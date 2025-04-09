#include <iostream>
#include <string>
#include <filesystem>
#include <FileSeeker.hxx>
#include <TaskPool.hxx>

namespace fs = std::filesystem;

void printUsage(const std::string& programName) {
    std::cerr << "Usage: " << programName << " <directory> <action-argument>\n"
              << "Recursively processes files in <directory> with specified action.\n"
              << "Example:\n"
              << "  " << programName << " /path/to/dir print\n";
}

int main( int argc, char** argv )
{
	if (argc < 2)
	{
		printUsage( argv[0] );
		return 1;
	}

	const std::string directory = argv[1];

	FileSeeker fs;
	fs.recursively_directory_action(directory, [](const fs::path& p) { std::cout << p << '\n'; } );
	return 0;
}
