#pragma once

#include <string>


namespace marklar {

	namespace driver {

		
		bool generateOutput(const std::string& inputFilename, const std::string& outputBitCodeName);

		bool optimizeAndLink(const std::string& bitCodeFilename, const std::string& exeName = "");

	}

}

