#ifndef CAENSETTINGS_HH
#define CAENSETTINGS_HH
#include <iostream>
#include <vector>
#include <string>

#include "midas.h"

#include "CAENDigitizer.h"

#include "TEnv.h"

#include "CaenOdb.h"

enum class EBoardType : char { kDesktop, kNIM, kVME };

class ChannelSettings {
public:
	ChannelSettings() {}
	ChannelSettings(const V1730_TEMPLATE& templateSettings);
	ChannelSettings(const int& boardNumber, const int& channelNumber, TEnv*& settings);
	~ChannelSettings();

	void ReadCustomSettings(const HNDLE& db, const HNDLE& key);
	void Print();

	//setters
	void RecordLength(const uint32_t& val) { fRecordLength = val; }
	void DCOffset(const uint32_t& val) { fDCOffset = val; }
	void PreTrigger(const uint32_t& val) { fPreTrigger = val; }
	void PulsePolarity(const CAEN_DGTZ_PulsePolarity_t& val) { fPulsePolarity = val; }
	void EnableCfd(const bool& val) { fEnableCfd = val; }
	void CfdParameters(const uint32_t& val) { fCfdParameters = val; }
	void EnableCoinc(const bool& val) { fEnableCoinc = val; }
	void EnableCoincTrig(const bool& val) { fEnableCoincTrig = val; }
	void EnableBaseline(const bool& val) { fEnableBaseline = val; }
	void CoincWindow(const uint32_t& val) { fCoincWindow = val; }
	void CoincLatency(const uint32_t& val) { fCoincLatency = val; }

	//getters
	uint32_t RecordLength() const { return fRecordLength; }
	uint32_t DCOffset() const { return fDCOffset; }
	uint32_t PreTrigger() const { return fPreTrigger; }
	CAEN_DGTZ_PulsePolarity_t PulsePolarity() const { return fPulsePolarity; }
	bool EnableCfd() const { return fEnableCfd; }
	uint32_t CfdParameters() const { return fCfdParameters; }
	bool EnableCoinc() const { return fEnableCoinc; }
	bool EnableCoincTrig() const { return fEnableCoincTrig; }
	bool EnableBaseline() const { return fEnableBaseline; }
	uint32_t CoincWindow() const { return fCoincWindow; }
	uint32_t CoincLatency() const { return fCoincLatency; }

private:
	uint32_t fRecordLength;
	uint32_t fDCOffset;
	uint32_t fPreTrigger;
	CAEN_DGTZ_PulsePolarity_t fPulsePolarity; //enum
	bool fEnableCfd;
	uint16_t fCfdParameters;
  	bool fEnableCoinc;
  	bool fEnableCoincTrig;
  	bool fEnableBaseline;
  	uint32_t fCoincWindow;
  	uint32_t fCoincLatency;
};

class BoardSettings {
public:
	BoardSettings() { }
	BoardSettings(const int& nofChannels, const V1730_TEMPLATE& templateSettings);
	BoardSettings(const int& boardNumber, const int& nofChannels, TEnv*& settings);
	~BoardSettings();

	void NumberOfChannels(const int& nofChannels) { fChannelSettings.resize(nofChannels); }
	void ReadCustomSettings(const HNDLE& db, const HNDLE& key);
	void ReadCustomChannelSettings(const int& channel, const HNDLE& db, const HNDLE& key);

	void Print();

	//setters
	void LinkType(const CAEN_DGTZ_ConnectionType& val) { fLinkType = val; }
	void BoardType(const EBoardType& val) { fBoardType = val; }
	void VmeBaseAddress(const uint32_t& val) { fVmeBaseAddress = val; }
	void AcquisitionMode(const CAEN_DGTZ_DPP_AcqMode_t& val) { fAcquisitionMode = val; }
	void IOLevel(const CAEN_DGTZ_IOLevel_t& val) { fIOLevel = val; }
	void ChannelMask(const uint32_t& val) { fChannelMask = val; }
	void RunSync(const CAEN_DGTZ_RunSyncMode_t& val) { fRunSync = val; }
	void EventAggregation(const int& val) { fEventAggregation = val; }
	void TriggerMode(const CAEN_DGTZ_TriggerMode_t& val) { fTriggerMode = val; }
	void ChannelParameter(const CAEN_DGTZ_DPP_PSD_Params_t& val) { fChannelParameter = val; }
	void PortNumber(const int& val) { fPortNumber = val; }
	void DeviceNumber(const int& val) { fDeviceNumber = val; }

	//getters
	CAEN_DGTZ_ConnectionType LinkType() const { std::cout<<"fLinkType "<<fLinkType<<std::endl; return fLinkType; }
	EBoardType BoardType() const { return fBoardType; }
	uint32_t VmeBaseAddress() const { return fVmeBaseAddress; }
	CAEN_DGTZ_DPP_AcqMode_t AcquisitionMode() const { return fAcquisitionMode; }
	CAEN_DGTZ_IOLevel_t IOLevel() const { return fIOLevel; }
	uint32_t ChannelMask() const { return fChannelMask; }
	CAEN_DGTZ_RunSyncMode_t RunSync() const { return fRunSync; }
	int EventAggregation() const { return fEventAggregation; }
	CAEN_DGTZ_TriggerMode_t TriggerMode() const { return fTriggerMode; }
	const CAEN_DGTZ_DPP_PSD_Params_t* ChannelParameter() const { return &fChannelParameter; }
	int PortNumber() const { return fPortNumber; }
	int DeviceNumber() const { return fDeviceNumber; }

	//channel setters
	void RecordLength(const int& i, const uint32_t& val) { fChannelSettings.at(i).RecordLength(val); }
	void DCOffset(const int& i, const uint32_t& val) { fChannelSettings.at(i).DCOffset(val); }
	void PreTrigger(const int& i, const uint32_t& val) { fChannelSettings.at(i).PreTrigger(val); }
	void PulsePolarity(const int& i, const CAEN_DGTZ_PulsePolarity_t& val) { fChannelSettings.at(i).PulsePolarity(val); }
	void EnableCfd(const int& i, const bool& val) { fChannelSettings.at(i).EnableCfd(val); }
	void CfdParameters(const int& i, const uint32_t& val) { fChannelSettings.at(i).CfdParameters(val); }
	void EnableCoinc(const int& i, const bool& val) { fChannelSettings.at(i).EnableCoinc(val); }
	void EnableCoincTrig(const int& i, const bool& val) { fChannelSettings.at(i).EnableCoincTrig(val); }
	void EnableBaseline(const int& i, const bool& val) { fChannelSettings.at(i).EnableBaseline(val); }
	void CoincWindow(const int& i, const uint32_t& val) { fChannelSettings.at(i).CoincWindow(val); }
	void CoincLatency(const int& i, const uint32_t& val) { fChannelSettings.at(i).CoincLatency(val); }
	
	//channel getters
	uint32_t RecordLength(const int& i) const { return fChannelSettings.at(i).RecordLength(); }
	uint32_t DCOffset(const int& i) const { return fChannelSettings.at(i).DCOffset(); }
	uint32_t PreTrigger(const int& i) const { return fChannelSettings.at(i).PreTrigger(); }
	CAEN_DGTZ_PulsePolarity_t PulsePolarity(const int& i) const { return fChannelSettings.at(i).PulsePolarity(); }
	bool EnableCfd(const int& i) const { return fChannelSettings.at(i).EnableCfd(); }
	uint32_t CfdParameters(const int& i) const { return fChannelSettings.at(i).CfdParameters(); }
	bool EnableCoinc(const int& i) const { return fChannelSettings.at(i).EnableCoinc(); }
	bool EnableCoincTrig(const int& i) const { return fChannelSettings.at(i).EnableCoincTrig(); }
	bool EnableBaseline(const int& i) const { return fChannelSettings.at(i).EnableBaseline(); }
	uint32_t CoincWindow(const int& i) const { return fChannelSettings.at(i).CoincWindow(); }
	uint32_t CoincLatency(const int& i) const { return fChannelSettings.at(i).CoincLatency(); }
	
private:
	CAEN_DGTZ_ConnectionType fLinkType; //enum
	EBoardType fBoardType; // enum
	uint32_t fVmeBaseAddress;
	int fPortNumber;
	int fDeviceNumber;
	CAEN_DGTZ_DPP_AcqMode_t fAcquisitionMode; //enum
	CAEN_DGTZ_IOLevel_t fIOLevel; //enum
	uint32_t fChannelMask;
	CAEN_DGTZ_RunSyncMode_t fRunSync; //enum
	int fEventAggregation;
	CAEN_DGTZ_TriggerMode_t fTriggerMode; //enum
	CAEN_DGTZ_DPP_PSD_Params_t fChannelParameter;
	std::vector<ChannelSettings> fChannelSettings;
};

class CaenSettings {
public:
	CaenSettings(bool debug = false);
	~CaenSettings();

	void ReadOdb(HNDLE hDB);
	bool ReadSettingsFile(const std::string&);
	bool WriteOdb();
	void Print();

	int NumberOfBoards() const { return fNumberOfBoards; }
	int NumberOfChannels() const { return fNumberOfChannels; }
	bool UseExternalClock() const { return fUseExternalClock; }

	size_t NumberOfBoardSettings() const { return fBoardSettings.size(); }

	//board parameters
	CAEN_DGTZ_ConnectionType LinkType(int i) const { return fBoardSettings.at(i).LinkType(); }
	uint32_t VmeBaseAddress(int i) const { return fBoardSettings.at(i).VmeBaseAddress(); }
	int PortNumber(int i) const { return fBoardSettings.at(i).PortNumber(); }
	int DeviceNumber(int i) const { return fBoardSettings.at(i).DeviceNumber(); }
	CAEN_DGTZ_DPP_AcqMode_t AcquisitionMode(int i) const { return fBoardSettings.at(i).AcquisitionMode(); }
	CAEN_DGTZ_IOLevel_t IOLevel(int i) const { return fBoardSettings.at(i).IOLevel(); }
	uint32_t ChannelMask(int i) const { return fBoardSettings.at(i).ChannelMask(); }
	CAEN_DGTZ_RunSyncMode_t RunSync(int i) const { return fBoardSettings.at(i).RunSync(); }
	int EventAggregation(int i) const { return fBoardSettings.at(i).EventAggregation(); }
	CAEN_DGTZ_TriggerMode_t TriggerMode(int i) const { return fBoardSettings.at(i).TriggerMode(); }
	const CAEN_DGTZ_DPP_PSD_Params_t* ChannelParameter(int i) const { return fBoardSettings.at(i).ChannelParameter(); }
	EBoardType BoardType(int i) const { return fBoardSettings.at(i).BoardType(); }

	//channel parameters
	uint32_t RecordLength(int i, int j) const { return fBoardSettings.at(i).RecordLength(j); }
	uint32_t DCOffset(int i, int j) const { return fBoardSettings.at(i).DCOffset(j); }
	uint32_t PreTrigger(int i, int j) const { return fBoardSettings.at(i).PreTrigger(j); }
	CAEN_DGTZ_PulsePolarity_t PulsePolarity(int i, int j) const { return fBoardSettings.at(i).PulsePolarity(j); }
	bool EnableCfd(int i, int j) const { return fBoardSettings.at(i).EnableCfd(j); }
	uint16_t CfdParameters(int i, int j) const { return fBoardSettings.at(i).CfdParameters(j); }
	bool EnableCoinc(int i, int j) const { return fBoardSettings.at(i).EnableCoinc(j); }
	bool EnableCoincTrig(int i, int j) const { return fBoardSettings.at(i).EnableCoincTrig(j); }
	bool EnableBaseline(int i, int j) const { return fBoardSettings.at(i).EnableBaseline(j); }
	uint32_t CoincWindow(int i, int j) const { return fBoardSettings.at(i).CoincWindow(j); }
	uint32_t CoincLatency(int i, int j) const { return fBoardSettings.at(i).CoincLatency(j); }

	size_t BufferSize() const { return fBufferSize; }

	bool RawOutput() const { return fRawOutput; }

private:
	int fNumberOfBoards;
	int fNumberOfChannels;
	bool fUseExternalClock;

	std::vector<BoardSettings> fBoardSettings;

	size_t fBufferSize;

	bool fRawOutput;

	bool fDebug;
};
#endif
