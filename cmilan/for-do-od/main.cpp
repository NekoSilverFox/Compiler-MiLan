#include "parser.h"
#include <iostream>
#include <cstdlib>

using namespace std;

void printHelp()
{
	cout << "Usage: cmilan input_file" << endl;
}

int main(int argc, char** argv)
{
	if(argc < 2) {
		printHelp();
		return EXIT_FAILURE;
	}

	ifstream input;
        input.open(argv[1]);

	if(input) {
		Parser p(argv[1], input);
		p.parse();
		return EXIT_SUCCESS;
	}
	else {
		cerr << "File '" << argv[1] << "' not found" << endl;
		return EXIT_FAILURE;
	}
}

