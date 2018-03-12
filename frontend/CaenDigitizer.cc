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

CaenDigitizer::CaenDigitizer(bool debug)
	: fSettings(new CaenSettings), fDebug(debug)
{
	// fSettings(new CaenSettings) automatically loads settings from the odb
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
			fBuffer.resize(fSettings->NumberOfBoards(), nullptr);
			fBufferSize.resize(fSettings->NumberOfBoards(), 0);
			//fEvents.resize(fSettings->NumberOfBoards(), nullptr);
			//fNofEvents.resize(fSettings->NumberOfBoards(), std::vector<uint32_t>(fSettings->NumberOfChannels(), 0));
			fWaveforms.resize(fSettings->NumberOfBoards(), nullptr);
		} catch(std::exception e) {
			std::cerr<<"Failed to resize vectors for "<<fSettings->NumberOfBoards()<<" boards, and "<<fSettings->NumberOfChannels()<<" channels: "<<e.what()<<std::endl;
			throw e;
		}
		for(int b = 0; b < fSettings->NumberOfBoards(); ++b) {
			if(fHandle[b] == -1) {
				if(fDebug) std::cout<<"setting up board "<<b<<std::endl;
				// open digitizers
				errorCode = CAEN_DGTZ_OpenDigitizer(fSettings->LinkType(b), b, 0, fSettings->VmeBaseAddress(b), &fHandle[b]);
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

			// we always re-program the digitizer in case settings have been changed
			ProgramDigitizer(b);
		
			// we don't really need to know how many bytes have been allocated, so we use fBufferSize here
			free(fBuffer[b]);
			if(fDebug) std::cout<<fHandle[b]<<"/"<<fBuffer.size()<<": trying to allocate memory for readout buffer "<<static_cast<void*>(fBuffer[b])<<std::endl;
			errorCode = CAEN_DGTZ_MallocReadoutBuffer(fHandle[b], &fBuffer[b], &fBufferSize[b]);
			if(errorCode != 0) {
				CAEN_DGTZ_CloseDigitizer(fHandle[b]);
				throw std::runtime_error(format("Error %d when allocating readout buffer", errorCode));
			}
			if(fDebug) std::cout<<"allocated "<<fBufferSize[0]<<" bytes of buffer for board "<<b<<std::endl;
			// again, we don't care how many bytes have been allocated, so we use fNofEvents here
			//fEvents[b] = new CAEN_DGTZ_DPP_PSD_Event_t*[fSettings->NumberOfChannels()];
			//for(int i = 0; i < fSettings->NumberOfChannels(); ++i) {
			//	fEvents[b][i] == nullptr;
			//}
			//try{
			// free(fEvents[b]);
			//	errorCode = CAEN_DGTZ_MallocDPPEvents(fHandle[b], reinterpret_cast<void**>(fEvents[b]), fNofEvents[b].data());
			//	if(errorCode != 0) {
			//		throw std::runtime_error(format("Error %d when allocating DPP events", errorCode));
			//	}
			//} catch(std::exception& e) {
			//	CAEN_DGTZ_CloseDigitizer(fHandle[b]);
			//	std::cout<<"failed to allocate dpp events: "<<e.what()<<std::endl;
			//	throw e;
			//}
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
	} // if(fHandle.size() < fSettings->NumberOfBoards)
}

CaenDigitizer::~CaenDigitizer()
{
	for(int b = 0; b < fSettings->NumberOfBoards(); ++b) {
		CAEN_DGTZ_FreeReadoutBuffer(&fBuffer[b]);
		//CAEN_DGTZ_FreeDPPEvents(fHandle[b], reinterpret_cast<void**>(fEvents[b]));
#ifdef USE_WAVEFORMS
		CAEN_DGTZ_FreeDPPWaveforms(fHandle[b], reinterpret_cast<void*>(fWaveforms[b]));
#endif
		CAEN_DGTZ_CloseDigitizer(fHandle[b]);
	}
}

void CaenDigitizer::StartAcquisition()
{
	// re-load settings from ODB and set digitzer up (again)
	fSettings->ReadOdb();
	Setup();
	if(fSettings->RawOutput()) {
		// open raw output file
		fRawOutput.open("raw.dat");
	}
	// start acquisition
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

INT CaenDigitizer::DataReady()
{
	int errorCode = 0;

	// read data
	if(fDebug) {
		std::cout<<"--------------------------------------------------------------------------------"<<std::endl;
	}
	bool gotData = false;
	for(int b = 0; b < fSettings->NumberOfBoards(); ++b) {
		// reset fNofEvents
		for(int ch = 0; ch < fSettings->NumberOfChannels(); ++ch) {
			//fNofEvents[b][ch] = 0;
		}
		errorCode = CAEN_DGTZ_ReadData(fHandle[b], CAEN_DGTZ_SLAVE_TERMINATED_READOUT_MBLT, fBuffer[b], &fBufferSize[b]);
		if(errorCode != 0) {
			std::cerr<<"Error "<<errorCode<<" when reading data"<<std::endl;
			return -1.;
		}
		if(fDebug) {
			std::cout<<"Read "<<fBufferSize[b]<<" bytes"<<std::endl;
		}
		if(fBufferSize[b] > 0) {
			gotData = true;
		}
	}
	if(fDebug) {
		std::cout<<"----------------------------------------"<<std::endl;
	}

	if(gotData) return TRUE;
	return FALSE;
}

void CaenDigitizer::ReadData(char* event, char* bankName)
{
	// creates bank at <event> and copies all data from fBuffer to it
	// no checks for valid events done, nor any identification of board/channel???
	DWORD* data;
	//create bank - returns pointer to data area of bank
	bk_create(event, bankName, TID_DWORD, reinterpret_cast<void**>(&data));

	//copy all events from fEvents to data
	for(int b = 0; b < fSettings->NumberOfBoards(); ++b) {
		if(fBufferSize[b] == 0) continue;
		//copy buffer of this board
		std::memcpy(data, fBuffer[b], fBufferSize[b]);
		data += fBufferSize[b]/sizeof(DWORD);
		if(fRawOutput.is_open()) {
			fRawOutput.write(fBuffer[b], fBufferSize[b]);
		}
	}
	
	//close bank
	bk_close(event, data);
}

void CaenDigitizer::ProgramDigitizer(int b)
{
	if(fDebug) std::cout<<"programming digitizer "<<b<<std::endl;
	CAEN_DGTZ_ErrorCode errorCode;

	errorCode = CAEN_DGTZ_Reset(fHandle[b]);

	if(errorCode != 0) {
		throw std::runtime_error(format("Error %d when resetting digitizer", errorCode));
	}

	errorCode = CAEN_DGTZ_SetDPPAcquisitionMode(fHandle[b], fSettings->AcquisitionMode(b), CAEN_DGTZ_DPP_SAVE_PARAM_EnergyAndTime);

	if(errorCode != 0) {
		throw std::runtime_error(format("Error %d when setting DPP acquisition mode", errorCode));
	}

	errorCode = CAEN_DGTZ_SetAcquisitionMode(fHandle[b], CAEN_DGTZ_SW_CONTROLLED);

	if(errorCode != 0) {
		throw std::runtime_error(format("Error %d when setting acquisition mode", errorCode));
	}

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

	errorCode = CAEN_DGTZ_SetRunSynchronizationMode(fHandle[b], CAEN_DGTZ_RUN_SYNC_Disabled); // change to settings

	if(errorCode != 0) {
		throw std::runtime_error(format("Error %d when setting run sychronization", errorCode));
	}

	errorCode = CAEN_DGTZ_SetDPPParameters(fHandle[b], fSettings->ChannelMask(b), static_cast<void*>(fSettings->ChannelParameter(b)));

	if(errorCode != 0) {
		throw std::runtime_error(format("Error %d when setting dpp parameters", errorCode));
	}

	// write some special registers directly
	uint32_t address;
	uint32_t data;
	// enable EXTRA word
	address = 0x8000;
	CAEN_DGTZ_ReadRegister(fHandle[b], address, &data);
	data |= 0x10000; // no mask necessary, we just set one bit
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
			data = (data & ~0x700) | 0x300;
			CAEN_DGTZ_WriteRegister(fHandle[b], address, data);
		}
	}

	errorCode = CAEN_DGTZ_SetDPPEventAggregation(fHandle[b], fSettings->EventAggregation(b), 0);

	errorCode = CAEN_DGTZ_SetDPP_VirtualProbe(fHandle[b], ANALOG_TRACE_2,  CAEN_DGTZ_DPP_VIRTUALPROBE_CFD);

	errorCode = CAEN_DGTZ_SetDPP_VirtualProbe(fHandle[b], DIGITAL_TRACE_1, CAEN_DGTZ_DPP_DIGITALPROBE_Gate);

	errorCode = CAEN_DGTZ_SetDPP_VirtualProbe(fHandle[b], DIGITAL_TRACE_2, CAEN_DGTZ_DPP_DIGITALPROBE_GateShort);
	if(fDebug) std::cout<<"done with digitizer "<<b<<std::endl;
}

bool CaenDigitizer::CheckEvent(const CAEN_DGTZ_DPP_PSD_Event_t& event)
{
	if(event.TimeTag == 0 && (event.Extras>>16) == 0 && (event.Extras & 0x3ff) == 0) {
		if(fDebug) {
			std::cout<<"empty time"<<std::endl;
		}
		return false;
	}
	if(fDebug) { 
		std::cout<<"times: "<<(event.Extras>>16)<<", "<<event.TimeTag<<", "<<(event.Extras & 0x3ff)<<std::endl;
	}
	return true;
}

