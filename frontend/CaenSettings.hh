#ifndef CAENSETTINGS_HH
#define CAENSETTINGS_HH
#include <vector>
#include <string>

#include "midas.h"

#include "CAENDigitizer.h"

class CaenSettings {
public:
	CaenSettings();
	CaenSettings(const std::string& filename, bool debug);
	~CaenSettings();

	void ReadOdb(HNDLE hDB);
	bool ReadSettingsFile(const std::string&);
	bool WriteOdb();
	void Print();

	int NumberOfBoards() const { return fNumberOfBoards; }
	CAEN_DGTZ_ConnectionType LinkType(int i) const { return fLinkType[i]; }
	uint32_t VmeBaseAddress(int i) const { return fVmeBaseAddress[i]; }
	CAEN_DGTZ_DPP_AcqMode_t AcquisitionMode(int i) const { return fAcquisitionMode[i]; }
	CAEN_DGTZ_IOLevel_t IOLevel(int i) const { return fIOLevel[i]; }
	uint32_t ChannelMask(int i) const { return fChannelMask[i]; }
	uint32_t PrintBaseline(int i) const { return fPrintBaseline[i]; }
	CAEN_DGTZ_RunSyncMode_t RunSync(int i) const { return fRunSync[i]; }
	int EventAggregation(int i) const { return fEventAggregation[i]; }
	CAEN_DGTZ_TriggerMode_t TriggerMode(int i) const { return fTriggerMode[i]; }
	uint32_t RecordLength(int i, int j) const { return fRecordLength[i][j]; }
	uint32_t DCOffset(int i, int j) const { return fDCOffset[i][j]; }
	uint32_t PreTrigger(int i, int j) const { return fPreTrigger[i][j]; }
	CAEN_DGTZ_PulsePolarity_t PulsePolarity(int i, int j) const { return fPulsePolarity[i][j]; }
	bool EnableCfd(int i, int j) const { return fEnableCfd[i][j]; }
	uint16_t CfdParameters(int i, int j) const { return fCfdParameters[i][j]; }
        bool EnableCoinc(int i, int j) const { return fEnableCoinc[i][j]; }
        bool EnableCoincTrig(int i, int j) const { return fEnableCoincTrig[i][j]; }
        bool EnableBaseline(int i, int j) const { return fEnableBaseline[i][j]; }
        uint32_t CoincWindow(int i, int j) const { return fCoincWindow[i][j]; }
        uint32_t CoincLatency(int i, int j) const { return fCoincLatency[i][j]; }

	
	int NumberOfChannels() const { return fNumberOfChannels; }
	CAEN_DGTZ_DPP_PSD_Params_t* ChannelParameter(int i) const { return fChannelParameter[i]; }

	size_t BufferSize() const { return fBufferSize; }

	bool RawOutput() const { return fRawOutput; }

private:
	int fNumberOfBoards;
	std::vector<CAEN_DGTZ_ConnectionType> fLinkType; //enum
	std::vector<uint32_t> fVmeBaseAddress;
	std::vector<CAEN_DGTZ_DPP_AcqMode_t> fAcquisitionMode; //enum
	std::vector<CAEN_DGTZ_IOLevel_t> fIOLevel; //enum
	std::vector<uint32_t> fChannelMask;
	std::vector<uint32_t> fPrintBaseline;
	std::vector<CAEN_DGTZ_RunSyncMode_t> fRunSync; //enum
	std::vector<int> fEventAggregation;
	std::vector<CAEN_DGTZ_TriggerMode_t> fTriggerMode; //enum
	std::vector<std::vector<uint32_t> > fRecordLength;
	std::vector<std::vector<uint32_t> > fDCOffset;
	std::vector<std::vector<uint32_t> > fPreTrigger;
	std::vector<std::vector<CAEN_DGTZ_PulsePolarity_t> > fPulsePolarity; //enum
	std::vector<std::vector<bool> > fEnableCfd;
	std::vector<std::vector<uint16_t> > fCfdParameters;
  	std::vector<std::vector<bool> > fEnableCoinc;
  	std::vector<std::vector<bool> > fEnableCoincTrig;
  	std::vector<std::vector<bool> > fEnableBaseline;
  	std::vector<std::vector<uint32_t> > fCoincWindow;
  	std::vector<std::vector<uint32_t> > fCoincLatency;
	
	int fNumberOfChannels;
	std::vector<CAEN_DGTZ_DPP_PSD_Params_t*> fChannelParameter;

	size_t fBufferSize;

	bool fRawOutput;
};
#endif
