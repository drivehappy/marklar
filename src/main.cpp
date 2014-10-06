#include <fstream>
#include <sstream>
#include <string>

#include <boost/program_options/cmdline.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>

#include "driver.h"

using namespace std;
namespace po = boost::program_options;

using namespace marklar::driver;


int main(int argc, char** argv) {
	// Build the supported options
	po::options_description desc("Allowed options");
	desc.add_options()
		("help", "produce help message")
		("input-file,i", po::value<string>(), "input file")
		("output-file,o", po::value<string>(), "output file")
	;

	po::variables_map vm;
	po::store(po::parse_command_line(argc, argv, desc), vm);
	po::notify(vm); 

	if (vm.count("help") > 0) {
		cout << desc << endl;
		return 1;
	}

	if (vm.count("input-file") > 0) {
		const string inputFilename = vm["input-file"].as<string>();
		const string tmpBitCodeFile = "output.bc";
		string outputFilename = "a.out";

		if (vm.count("output-file") > 0) {
			outputFilename = vm["output-file"].as<string>();
		}

		// Pull in the source file and generate the code
		ifstream in(inputFilename.c_str());
		const string fileContents(static_cast<stringstream const&>(stringstream() << in.rdbuf()).str());

		if (!generateOutput(fileContents, tmpBitCodeFile)) {
			return 2;
		}

		if (!optimizeAndLink(tmpBitCodeFile, outputFilename)) {
			return 3;
		}
	}

	cout << "Executable complete!" << endl;

	return 0;
}

