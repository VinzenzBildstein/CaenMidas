#include <iostream>

#include "CAENDigitizer.h"

int main(int, char**)
{
	if(CAEN_DGTZ_CloseDigitizer(0) != 0) {
		std::cout<<"Failed to close digitizer 0"<<std::endl;
	}
	return 0;
}

