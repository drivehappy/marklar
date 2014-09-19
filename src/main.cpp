#include <regex>
#include <string>

#include <boost/program_options/cmdline.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>

using namespace std;
namespace po = boost::program_options;

/*
def main() {
	int i = 0;
	int j = 1;

	int r = i + j;

	return r;
}
*/



int main(int argc, char** argv) {
	// Declare the supported options.
	po::options_description desc("Allowed options");
	desc.add_options()
		("help", "produce help message")
		("input-file,I", po::value< vector<string> >(), "input file")
	;

	po::variables_map vm;
	po::store(po::parse_command_line(argc, argv, desc), vm);
	po::notify(vm); 

	if (vm.count("help")) {
		cout << desc << "\n";
		return 1;
	}

	return 0;
}
