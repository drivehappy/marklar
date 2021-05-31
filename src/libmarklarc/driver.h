#pragma once

#include <string>


namespace marklar {

	namespace driver {

		// Intermediate step that will accept Marklar source and output LLVM bitcode
		bool generateOutput(const std::string& input, const std::string& outputBitCodeName);

		// Find step that accepts the LLVM bitcode filename and produces an optimized executable
		bool optimizeAndLink(const std::string& bitCodeFilename, const std::string& exeName = "");

	}

}

