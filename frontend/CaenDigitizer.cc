#include "CaenDigitizer.hh"

#include <iostream>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <cstring>
#include <vector>
#include <string>
#include <cstdarg>
#include <cstring>
#include <cstdlib>

std::string format(const std::string& format, ...)
{
	va_list args;
	va_start (args, format);
	size_t len = std::vsnprintf(NULL, 0, format.c_str(), args);
	va_end (args);
	std::vector<char> vec(len + 1);
	va_start (args, format);
	std::vsnprintf(&vec[0], len + 1, format.c_str(), args);
	va_end (args);
	return &vec[0];
}

CaenDigitizer::CaenDigitizer(HNDLE hDB, bool debug)
	: fSettings(new CaenSettings(debug)), fDebug(debug)
{
	fSettings->ReadOdb(hDB);
	Setup();
}

void CaenDigitizer::Setup()
{
	if(fDebug) std::cout<<"setting up digitizer"<<std::endl;
	CAEN_DGTZ_ErrorCode errorCode;
	CAEN_DGTZ_BoardInfo_t boardInfo;
	int majorNumber;

	if(static_cast<int>(fHandle.size()) < fSettings->NumberOfBoards()) {
		// we have more boards now than before, so we need to initialize the additional boards
		try {
			fHandle.resize(fSettings->NumberOfBoards(), -1);
			fBuffer.resize(fSettings->NumberOfBoards(), NULL);
			fBufferSize.resize(fSettings->NumberOfBoards(), 0);
			fWaveforms.resize(fSettings->NumberOfBoards(), NULL);
		} catch(std::exception e) {
			std::cerr<<"Failed to resize vectors for "<<fSettings->NumberOfBoards()<<" boards, and "<<fSettings->NumberOfChannels()<<" channels: "<<e.what()<<std::endl;
			throw e;
		}
		for(int b = 0; b < fSettings->NumberOfBoards(); ++b) {
			if(fHandle[b] == -1) {
				if(fDebug) std::cout<<"setting up board "<<b<<std::endl;
				// open digitizers
				errorCode = CAEN_DGTZ_OpenDigitizer(fSettings->LinkType(b), fSettings->PortNumber(b), fSettings->DeviceNumber(b), fSettings->VmeBaseAddress(b), &fHandle[b]);
				if(errorCode != 0) {
					throw std::runtime_error(format("Error %d when opening digitizer", errorCode));
				}
				if(fDebug) std::cout<<"got handle "<<fHandle[b]<<" for board "<<b<<std::endl;
				// get digitizer info
				errorCode = CAEN_DGTZ_GetInfo(fHandle[b], &boardInfo);
				if(errorCode != 0) {
					CAEN_DGTZ_CloseDigitizer(fHandle[b]);
					throw std::runtime_error(format("Error %d when reading digitizer info", errorCode));
				}
#ifdef USE_CURSES
				printw("\nConnected to CAEN Digitizer Model %s as %d. board\n", boardInfo.ModelName, b);
				printw("\nFirmware is ROC %s, AMC %s\n", boardInfo.ROC_FirmwareRel, boardInfo.AMC_FirmwareRel);
#else
				std::cout<<std::endl<<"Connected to CAEN Digitizer Model "<<boardInfo.ModelName<<" as "<<b<<". board"<<std::endl;
				std::cout<<std::endl<<"Firmware is ROC "<<boardInfo.ROC_FirmwareRel<<", AMC "<<boardInfo.AMC_FirmwareRel<<std::endl;
#endif

				std::stringstream str(boardInfo.AMC_FirmwareRel);
				str>>majorNumber;
				if(majorNumber != 131 && majorNumber != 132 && majorNumber != 136) {
					CAEN_DGTZ_CloseDigitizer(fHandle[b]);
					throw std::runtime_error("This digitizer has no DPP-PSD firmware");
				}
			} // if(fHandle[b] == -1)
		}
	} // if(fHandle.size() < fSettings->NumberOfBoards)

	// we always re-program the digitizer in case settings have been changed
	for(int b = 0; b < fSettings->NumberOfBoards(); ++b) {
		ProgramDigitizer(b);

		// we don't really need to know how many bytes have been allocated, so we use fBufferSize here
		free(fBuffer[b]);
		//fBuffer[b] = static_cast<char*>(malloc(100*6504464));
		//1638416 bytes are allocated by CAEN_DGTZ_MallocReadoutBuffer (2 channels, 192 samples each)
		//changing this to 8 channels changed the number to 6504464
		if(fDebug) std::cout<<fHandle[b]<<"/"<<fBuffer.size()<<": trying to allocate memory for readout buffer "<<static_cast<void*>(fBuffer[b])<<std::endl;
		errorCode = CAEN_DGTZ_MallocReadoutBuffer(fHandle[b], &fBuffer[b], &fBufferSize[b]);
		if(errorCode != 0) {
			CAEN_DGTZ_CloseDigitizer(fHandle[b]);
			throw std::runtime_error(format("Error %d when allocating readout buffer", errorCode));
		}
		if(fDebug) std::cout<<"allocated "<<fBufferSize[0]<<" bytes of buffer for board "<<b<<std::endl;
#ifdef USE_WAVEFORMS
		// allocate waveforms, again not caring how many bytes have been allocated
		uint32_t size;
		free(fWaveforms[b]);
		errorCode = CAEN_DGTZ_MallocDPPWaveforms(fHandle[b], reinterpret_cast<void**>(&(fWaveforms[b])), &size);
		if(errorCode != 0) {
			CAEN_DGTZ_CloseDigitizer(fHandle[b]);
			throw std::runtime_error(format("Error %d when allocating DPP waveforms", errorCode));
		}
#endif
		if(fDebug) std::cout<<"done with board "<<b<<std::endl;
	} // for(int b = 0; b < fSettings->NumberOfBoards(); ++b)
}

CaenDigitizer::~CaenDigitizer()
{
	for(int b = 0; b < fSettings->NumberOfBoards(); ++b) {
		CAEN_DGTZ_FreeReadoutBuffer(&fBuffer[b]);
#ifdef USE_WAVEFORMS
		CAEN_DGTZ_FreeDPPWaveforms(fHandle[b], reinterpret_cast<void*>(fWaveforms[b]));
#endif
		CAEN_DGTZ_CloseDigitizer(fHandle[b]);
	}
}

void CaenDigitizer::StartAcquisition(HNDLE hDB)
{
	// re-load settings from ODB and set digitzer up (again)
	fSettings->ReadOdb(hDB);
	Setup();
	if(fSettings->RawOutput()) {
		// open raw output file
		fRawOutput.open("raw.dat");
	}
	// don't need to start acquisition, this is done by the s-in/gpi signal
	for(int b = 0; b < fSettings->NumberOfBoards(); ++b) {
		CAEN_DGTZ_SWStartAcquisition(fHandle[b]);
	}
}

void CaenDigitizer::StopAcquisition()
{
	// stop acquisition
	for(int b = 0; b < fSettings->NumberOfBoards(); ++b) {
		CAEN_DGTZ_SWStopAcquisition(fHandle[b]);
	}
	if(fRawOutput.is_open()) {
		fRawOutput.close();
	}
}

void CaenDigitizer::Calibrate()
{
	// calibrate all digitizers
	int errorCode = 0;
	for(int b = 0; b < fSettings->NumberOfBoards(); ++b) {
		errorCode = CAEN_DGTZ_Calibrate(fHandle[b]);
		if(errorCode != 0) {
			std::cerr<<"Error "<<errorCode<<" when trying to calibrate board "<<b<<", handle "<<fHandle[b]<<std::endl;
		}
	}
}

INT CaenDigitizer::DataReady()
{
	int errorCode = 0;

	// read data
	if(fDebug) {
		std::cout<<"--------------------------------------------------------------------------------"<<std::endl;
	}
	bool gotData = false;
	for(int b = 0; b < fSettings->NumberOfBoards(); ++b) {
		errorCode = CAEN_DGTZ_ReadData(fHandle[b], CAEN_DGTZ_SLAVE_TERMINATED_READOUT_MBLT, fBuffer[b], &fBufferSize[b]);
		if(errorCode != 0) {
			std::cerr<<"Error "<<errorCode<<" when reading data"<<std::endl;
			return -1.;
		}
		if(fDebug) {
			std::cout<<"Read "<<fBufferSize[b]<<" bytes"<<std::endl;
		}
		if(fBufferSize[b] > 0) {
			//std::cout<<std::endl<<__PRETTY_FUNCTION__<<": got "<<fBufferSize[b]<<" bytes from board "<<b<<std::endl;
			gotData = true;
		}
	}
	if(fDebug) {
		std::cout<<"----------------------------------------"<<std::endl;
	}

	// this is necessary because TRUE and FALSE are not booleans ...
	if(gotData) return TRUE;
	return FALSE;
}

uint32_t CaenDigitizer::ReadData(char* event, const char* bankName)
{
	// creates bank at <event> and copies all data from fBuffer to it
	// no checks for valid events done, nor any identification of board/channel???
	DWORD* data;
	//check if we have any data
	int sum = 0;
	for(int b = 0; b < fSettings->NumberOfBoards(); ++b) {
		if(fBufferSize[b] >= 0) sum += fBufferSize[b];
		else std::cerr<<"buffer size of board "<<b<<" is negative: "<<fBufferSize[b]<<std::endl;
	}
	if(sum == 0) {
		std::cout<<__PRETTY_FUNCTION__<<": no data"<<std::endl;
		return 0;
	}
	//create bank - returns pointer to data area of bank
	bk_create(event, bankName, TID_DWORD, reinterpret_cast<void**>(&data));
	//copy all events from fBuffer to data
	uint32_t sumEvents = 0;
	if(fDebug) std::cout<<"#events read: ";
	for(int b = 0; b < fSettings->NumberOfBoards(); ++b) {
		if(fBufferSize[b] == 0) continue;
		//copy buffer of this board
		std::memcpy(data, fBuffer[b], fBufferSize[b]);
		data += fBufferSize[b]/sizeof(DWORD);
		if(fRawOutput.is_open()) {
			fRawOutput.write(fBuffer[b], fBufferSize[b]);
		}

		uint32_t numEvents;
		CAEN_DGTZ_GetNumEvents(fHandle[b], fBuffer[b], fBufferSize[b], &numEvents);
		if(fDebug) std::cout<<"board "<<b<<": "<<std::setw(8)<<numEvents<<" ";
		sumEvents += numEvents;
	}
	if(fDebug) std::cout<<"total: "<<std::setw(8)<<sumEvents<<std::endl;
	
	//close bank
	bk_close(event, data);

	return sumEvents;
}

void CaenDigitizer::ProgramDigitizer(int b)
{
	uint32_t address;
	uint32_t data;

	if(fDebug) std::cout<<"programming digitizer "<<b<<std::endl;
	CAEN_DGTZ_ErrorCode errorCode;

	errorCode = CAEN_DGTZ_Reset(fHandle[b]);

	if(errorCode != 0) {
		throw std::runtime_error(format("Error %d when resetting digitizer", errorCode));
	}

	errorCode = CAEN_DGTZ_SetDPPAcquisitionMode(fHandle[b], fSettings->AcquisitionMode(b), CAEN_DGTZ_DPP_SAVE_PARAM_EnergyAndTime);
	//CAEN_DGTZ_DPP_AcqMode_t mode;
	//CAEN_DGTZ_DPP_SaveParam_t param;
	//std::cout<<"acquisition mode "<<CAEN_DGTZ_GetDPPAcquisitionMode(fHandle[b], &mode, &param)<<": mode "<<mode<<", param "<<param<<std::endl;

	if(errorCode != 0) {
		throw std::runtime_error(format("Error %d when setting DPP acquisition mode", errorCode));
	}

	// CAEN_DGTZ_SetAcquisitionMode gets overwritten later by CAEN_DGTZ_SetRunSynchronizationMode, so we don't bother with it

	errorCode = CAEN_DGTZ_SetIOLevel(fHandle[b], fSettings->IOLevel(b));

	if(errorCode != 0) {
		throw std::runtime_error(format("Error %d when setting IO level", errorCode));
	}

	errorCode = CAEN_DGTZ_SetExtTriggerInputMode(fHandle[b], fSettings->TriggerMode(b));

	if(errorCode != 0) {
		throw std::runtime_error(format("Error %d when setting external trigger DPP events", errorCode));
	}

	errorCode = CAEN_DGTZ_SetChannelEnableMask(fHandle[b], fSettings->ChannelMask(b));

	if(errorCode != 0) {
		throw std::runtime_error(format("Error %d when setting channel mask", errorCode));
	}

	// disabled turns acquisition mode back to SW controlled
	// both GpioGpioDaisyChain and SinFanout turn it to S_IN controlled
	// according to rev18 manual GpioGpioDaisyChain is not used!
	errorCode = CAEN_DGTZ_SetRunSynchronizationMode(fHandle[b], CAEN_DGTZ_RUN_SYNC_Disabled); // change to settings

	if(errorCode != 0) {
		throw std::runtime_error(format("Error %d when setting run sychronization", errorCode));
	}

	errorCode = CAEN_DGTZ_SetDPPParameters(fHandle[b], fSettings->ChannelMask(b), const_cast<void*>(static_cast<const void*>(fSettings->ChannelParameter(b))));

	if(errorCode != 0) {
		throw std::runtime_error(format("Error %d when setting dpp parameters", errorCode));
	}

	// write some special registers directly
	// enable EXTRA word
	address = 0x8000;
	CAEN_DGTZ_ReadRegister(fHandle[b], address, &data);
	data |= 0x20000; // no mask necessary, we just set one bit
	CAEN_DGTZ_WriteRegister(fHandle[b], address, data);

	for(int ch = 0; ch < fSettings->NumberOfChannels(); ++ch) {
		if((fSettings->ChannelMask(b) & (1<<ch)) != 0) {
			if(fDebug) std::cout<<"programming channel "<<ch<<std::endl;
			if(ch%2 == 0) {
				errorCode = CAEN_DGTZ_SetRecordLength(fHandle[b], fSettings->RecordLength(b, ch), ch);
			}

			errorCode = CAEN_DGTZ_SetChannelDCOffset(fHandle[b], ch, fSettings->DCOffset(b, ch));

			errorCode = CAEN_DGTZ_SetDPPPreTriggerSize(fHandle[b], ch, fSettings->PreTrigger(b, ch));

			errorCode = CAEN_DGTZ_SetChannelPulsePolarity(fHandle[b], ch, fSettings->PulsePolarity(b, ch));

			if(fSettings->EnableCfd(b, ch)) {
				if(fDebug) std::cout<<"enabling CFD on channel "<<ch<<std::endl;
				// enable CFD mode
				address = 0x1080 + ch*0x100;
				CAEN_DGTZ_ReadRegister(fHandle[b], address, &data);
				data |= 0x40; // no mask necessary, we just set one bit
				CAEN_DGTZ_WriteRegister(fHandle[b], address, data);

				// set CFD parameters
				address = 0x103c + ch*0x100;
				CAEN_DGTZ_ReadRegister(fHandle[b], address, &data);
				data = (data & ~0xfff) | fSettings->CfdParameters(b, ch);
				CAEN_DGTZ_WriteRegister(fHandle[b], address, data);
			}
			// write extended TS, flags, and fine TS (from CFD) to extra word
			address = 0x1084 + ch*0x100;
			CAEN_DGTZ_ReadRegister(fHandle[b], address, &data);
			data = (data & ~0x700) | 0x200;
			CAEN_DGTZ_WriteRegister(fHandle[b], address, data);
		}
	}

	errorCode = CAEN_DGTZ_SetDPPEventAggregation(fHandle[b], fSettings->EventAggregation(b), 0);

	// doesn't work??? we set it now by hand below
	errorCode = CAEN_DGTZ_SetDPP_VirtualProbe(fHandle[b], ANALOG_TRACE_2,  CAEN_DGTZ_DPP_VIRTUALPROBE_CFD);

	// this has been confirmed to work
	errorCode = CAEN_DGTZ_SetDPP_VirtualProbe(fHandle[b], DIGITAL_TRACE_1, CAEN_DGTZ_DPP_DIGITALPROBE_Gate);

	errorCode = CAEN_DGTZ_SetDPP_VirtualProbe(fHandle[b], DIGITAL_TRACE_2, CAEN_DGTZ_DPP_DIGITALPROBE_GateShort);

	// manually set analog traces to input and cfd
	address = 0x8000;
	CAEN_DGTZ_ReadRegister(fHandle[b], address, &data);
	data = (data & ~0x3000) | 0x2000;
	CAEN_DGTZ_WriteRegister(fHandle[b], address, data);

	// use external clock - this seems to be safer if done at the end of setting all parameters ???
	if(fSettings->UseExternalClock()) {
		if(fSettings->BoardType(b) == EBoardType::kVME) {
			// VME boards have an onboard switch to select the clock
			// so we check if it's enabled and give a warning otherwise
			address = 0x8104;
			CAEN_DGTZ_ReadRegister(fHandle[b], address, &data);
			if((data&0x20) != 0x20) {
				cm_msg(MERROR, "ProgramDigitizer", "Requested external clock for VME module, this has to be set by onboard switch S3 (0x%x)", data);
			}
		} else {
			address = 0x8100;
			CAEN_DGTZ_ReadRegister(fHandle[b], address, &data);
			data |= 0x40; // no mask necessary, we just set one bit
			CAEN_DGTZ_WriteRegister(fHandle[b], address, data);
		}
	}

	if(fDebug) std::cout<<"done with digitizer "<<b<<std::endl;
}

