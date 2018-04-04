#include <iostream>

#include "CaenSettings.hh"

int main(int argc, char** argv)
{
	if(argc != 2) {
		std::cerr<<"Usage: "<<argv[0]<<" <settings file>"<<std::endl;
		return 1;
	}

	CaenSettings* settings = new CaenSettings;

	if(!settings->ReadSettingsFile(argv[1])) {
		std::cerr<<"Failed to read settings file "<<argv[1]<<std::endl;
		return 1;
	}

	settings->WriteOdb();

	return 0;
}
