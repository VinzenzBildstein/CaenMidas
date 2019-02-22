#ifndef CAENDIGITIZER_HH
#define CAENDIGITIZER_HH
#include <vector>
#include <string>
#include <functional>
#include <fstream>

#include "midas.h"

#include "CaenSettings.hh"

class CaenDigitizer {
public:
	CaenDigitizer(HNDLE hDB, bool debug = false);
	~CaenDigitizer();

	void StartAcquisition(HNDLE hDB);
	void StopAcquisition();
	INT  DataReady();
	uint32_t ReadData(char* event, const char* bankName);
	void Calibrate();

private:
	void Setup();
	void ProgramDigitizer(int board);

	CaenSettings* fSettings;

	std::vector<int> fHandle;
	// raw readout data
	std::vector<char*>    fBuffer; 
	std::vector<uint32_t> fBufferSize;
	// DPP events
	//std::vector<CAEN_DGTZ_DPP_PSD_Event_t**> fEvents;
	//std::vector<std::vector<uint32_t> >      fNofEvents;
	// waveforms
	std::vector<CAEN_DGTZ_DPP_PSD_Waveforms_t*> fWaveforms;

	std::ofstream fRawOutput;

	bool fDebug;
};
#endif
