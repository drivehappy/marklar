#include <fstream>
#include <regex>
#include <string>
#include <sstream>

#include <boost/program_options/cmdline.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/variant/get.hpp>

#include <llvm/Bitcode/ReaderWriter.h>
#include <llvm/IR/IRBuilder.h>
#include "llvm/IR/LLVMContext.h"
#include <llvm/IR/Module.h>
#include <llvm/Support/raw_ostream.h>

#include "parser.h"
#include "codegen.h"

using namespace marklar;
using namespace parser;

using namespace llvm;
using namespace std;
namespace po = boost::program_options;


bool generateOutput(const string& inputFilename, const string& outputBitCodeName) {
	ifstream in(inputFilename.c_str());

	const string fileContents(static_cast<stringstream const&>(stringstream() << in.rdbuf()).str());

	// Parse the source file
	cout << "Parsing..." << endl;
	base_expr_node rootAst;
	if (!parse(fileContents, rootAst)) {
		cerr << "Failed to parse source file!" << endl;
		return false;
	}

	// Generate the code
	cout << "Generating code..." << endl;
	LLVMContext &context = getGlobalContext();
	unique_ptr<Module> module(new Module("", context));
	IRBuilder<> builder(getGlobalContext());

	ast_codegen codeGenerator(module.get(), builder);

	// Generate code for each expression at the root level
	const base_expr* expr = boost::get<base_expr>(&rootAst);
	for (auto& itr : expr->children) {
		boost::apply_visitor(codeGenerator, itr);
	}

	// Perform an LLVM verify as a sanity check
	string errorInfo;
	raw_string_ostream errorOut(errorInfo);

	if (verifyModule(*module, &errorOut)) {
		cerr << "Failed to generate LLVM IR: " << errorInfo << endl;

		module->print(errorOut, nullptr);
		cerr << "Module:" << endl << errorInfo << endl;
		return false;
	}

	// Dump the LLVM IR to a file
	llvm::raw_fd_ostream outStream(outputBitCodeName.c_str(), errorInfo, llvm::sys::fs::F_Binary);
	llvm::WriteBitcodeToFile(module.get(), outStream);

	return true;
}

bool optimizeAndLink(const string& bitCodeFilename, const string& exeName = "") {
	// Optimize the generated bitcode with LLVM 'opt'
	{
		cout << "Optimizing..." << endl;
	}

	// Transform the bitcode into an object file with LLVM 'llc'
	{
		cout << "Linking..." << endl;

		const string tmpObjName = "output.o";
		const string llcCmd = "llc-3.5 -filetype=obj output.bc -o " + tmpObjName;

		const int retval = system(llcCmd.c_str());
		if (retval != 0) {
			cerr << "Error running 'llc'" << endl;
			return false;
		}
	}

	// Leverage gcc here to link the object file into the final executable
	// this is mainly to bypass the more complicated options that the system 'ld' needs
	{
		const string outputExeName = (exeName.empty() ? "a.out" : exeName);
		const string gccCmd = "gcc -o " + outputExeName + " output.o";
		
		const int retval = system(gccCmd.c_str());
		if (retval != 0) {
			cerr << "Error running 'gcc': \"" << gccCmd << "\"" << " -- returned: " << retval << endl;
			return false;
		}
	}

	return true;
}

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

		if (!generateOutput(inputFilename, tmpBitCodeFile)) {
			return 2;
		}

		if (!optimizeAndLink(tmpBitCodeFile, outputFilename)) {
			return 3;
		}
	}

	cout << "Executable complete!" << endl;

	return 0;
}

