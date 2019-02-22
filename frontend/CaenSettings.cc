#include "CaenSettings.hh"

#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <curses.h>
#include <cstdlib>

#include "midas.h"

#include "TEnv.h"

#include "CaenOdb.h"

ChannelSettings::ChannelSettings(const V1730_TEMPLATE& templateSettings)
{
	fRecordLength   = templateSettings.record_length;
	fDCOffset       = templateSettings.dc_offset;
	fPreTrigger     = templateSettings.pre_trigger;
	fPulsePolarity  = static_cast<CAEN_DGTZ_PulsePolarity_t>(templateSettings.pulse_polarity);
	fEnableCfd      = templateSettings.enable_cfd;
	fCfdParameters  = templateSettings.cfd_delay & 0xff;
	fCfdParameters |= (templateSettings.cfd_fraction & 0x3) << 8;
	fCfdParameters |= (templateSettings.cfd_interpolation_points & 0x3) << 10;
	fEnableCoinc    = templateSettings.enable_coinc;
	fEnableBaseline = templateSettings.enable_baseline;
	fCoincWindow    = templateSettings.coinc_window;
	fCoincLatency   = templateSettings.coinc_latency;
}

ChannelSettings::ChannelSettings(const int& boardNumber, const int& channelNumber, TEnv*& settings)
{
	fRecordLength   = settings->GetValue(Form("Board.%d.Channel.%d.RecordLength", boardNumber, channelNumber), 192);
	fDCOffset       = settings->GetValue(Form("Board.%d.Channel.%d.DCOffset", boardNumber, channelNumber), 0x8000);
	fPreTrigger     = settings->GetValue(Form("Board.%d.Channel.%d.RunSync", boardNumber, channelNumber), 80);
	fPulsePolarity  = static_cast<CAEN_DGTZ_PulsePolarity_t>(settings->GetValue(Form("Board.%d.Channel.%d.PulsePolarity", boardNumber, channelNumber), CAEN_DGTZ_PulsePolarityNegative));//1
	fEnableCfd      = settings->GetValue(Form("Board.%d.Channel.%d.EnableCfd", boardNumber, channelNumber), true);
	fCfdParameters  = (settings->GetValue(Form("Board.%d.Channel.%d.CfdDelay", boardNumber, channelNumber), 5) & 0xff);
	fCfdParameters |= (settings->GetValue(Form("Board.%d.Channel.%d.CfdFraction", boardNumber, channelNumber), 0) & 0x3) << 8;
	fCfdParameters |= (settings->GetValue(Form("Board.%d.Channel.%d.CfdInterpolationPoints", boardNumber, channelNumber), 0) & 0x3) << 10;
	fEnableCoinc    = settings->GetValue(Form("Board.%d.Channel.%d.EnableCoinc", boardNumber, channelNumber), false);
	fEnableBaseline = settings->GetValue(Form("Board.%d.Channel.%d.EnableBaseline", boardNumber, channelNumber), false);
	fCoincWindow    = settings->GetValue(Form("Board.%d.Channel.%d.CoincWindow", boardNumber, channelNumber), 5);
	fCoincLatency   = settings->GetValue(Form("Board.%d.Channel.%d.CoincLatency", boardNumber, channelNumber), 2);
}

void ChannelSettings::ReadCustomSettings(const HNDLE& hDb, const HNDLE& hKey)
{
	// loop over sub-keys
	HNDLE hSubKey;
	KEY key;
	int size;
	for(int i = 0; ; ++i) {
		db_enum_key(hDb, hKey, i, &hSubKey);
		if(!hSubKey) break; // end of list reached
		db_get_key(hDb, hSubKey, &key);
		if(strcmp(key.name, "Record length") == 0 && key.num_values == 1) {
			size = sizeof(fRecordLength);
			db_get_data(hDb, hSubKey, &fRecordLength, &size, TID_WORD);
		} else if(strcmp(key.name, "DC offset") == 0 && key.num_values == 1) {
			size = sizeof(fDCOffset);
			db_get_data(hDb, hSubKey, &fDCOffset, &size, TID_WORD);
		} else if(strcmp(key.name, "Pre trigger") == 0 && key.num_values == 1) {
			size = sizeof(fPreTrigger);
			db_get_data(hDb, hSubKey, &fPreTrigger, &size, TID_WORD);
		} else if(strcmp(key.name, "Pulse polarity") == 0 && key.num_values == 1) {
			size = sizeof(fPulsePolarity);
			db_get_data(hDb, hSubKey, &fPulsePolarity, &size, TID_WORD);
			//fPulsePolarity = static_cast<CAEN_DGTZ_PulsePolarity_t>(*reinterpret_cast<WORD*>(key.data));
		} else if(strcmp(key.name, "Enable CFD") == 0 && key.num_values == 1) {
			size = sizeof(fEnableCfd);
			db_get_data(hDb, hSubKey, &fEnableCfd, &size, TID_BOOL);
		} else if(strcmp(key.name, "CFD delay") == 0 && key.num_values == 1) {
			int tmp;
			size = sizeof(tmp);
			db_get_data(hDb, hSubKey, &tmp, &size, TID_WORD);
			fCfdParameters &= ~0xff;
			fCfdParameters |= (tmp & 0xff);
		} else if(strcmp(key.name, "CFD fraction") == 0 && key.num_values == 1) {
			int tmp;
			size = sizeof(tmp);
			db_get_data(hDb, hSubKey, &tmp, &size, TID_WORD);
			fCfdParameters &= ~0x300;
			fCfdParameters |= (tmp & 0x3) << 8;
		} else if(strcmp(key.name, "CFD interpolation points") == 0 && key.num_values == 1) {
			int tmp;
			size = sizeof(tmp);
			db_get_data(hDb, hSubKey, &tmp, &size, TID_WORD);
			fCfdParameters &= ~0xc00;
			fCfdParameters |= (tmp & 0x3) << 10;
		} else if(strcmp(key.name, "Enable coincidence") == 0 && key.num_values == 1) {
			size = sizeof(fEnableCoinc);
			db_get_data(hDb, hSubKey, &fEnableCoinc, &size, TID_BOOL);
		} else if(strcmp(key.name, "Enable baseline") == 0 && key.num_values == 1) {
			size = sizeof(fEnableBaseline);
			db_get_data(hDb, hSubKey, &fEnableBaseline, &size, TID_BOOL);
		} else if(strcmp(key.name, "Coincidence window") == 0 && key.num_values == 1) {
			size = sizeof(fCoincWindow);
			db_get_data(hDb, hSubKey, &fCoincWindow, &size, TID_WORD);
		} else if(strcmp(key.name, "Coincidence latency") == 0 && key.num_values == 1) {
			size = sizeof(fCoincLatency);
			db_get_data(hDb, hSubKey, &fCoincLatency, &size, TID_WORD);
		} else {
			// we keep both channel and channelparameter (which are "per board") settings
			// in the "Channel x" directory, so there might be unrecognized entries
		}
	}
}

ChannelSettings::~ChannelSettings()
{
}

void ChannelSettings::Print()
{
	std::cout<<"      record length "<<fRecordLength<<std::endl;
	std::cout<<"      DC offset 0x"<<std::hex<<fDCOffset<<std::dec<<std::endl;
	std::cout<<"      pre trigger "<<fPreTrigger<<std::endl;
	std::cout<<"      pulse polarity ";
	switch(fPulsePolarity) {
		case CAEN_DGTZ_PulsePolarityPositive:
			std::cout<<"positive"<<std::endl;
			break;
		case CAEN_DGTZ_PulsePolarityNegative:
			std::cout<<"negative"<<std::endl;
			break;
		default:
			std::cout<<"unknown"<<std::endl;
			break;
	}
	if(fEnableCfd) {
		std::cout<<"      cfd enabled"<<std::endl;
		std::cout<<"      cfd parameters 0x"<<std::hex<<fCfdParameters<<std::dec<<std::endl;
	} else {
		std::cout<<"      cfd disabled"<<std::endl;
	}
	std::cout<<"      coincidence "<<(fEnableCoinc?"enabled":"disabled")<<std::endl
		<<"      baseline "<<(fEnableBaseline?"enabled":"disabled")<<std::endl
		<<"      coincidence window "<<fCoincWindow<<std::endl
		<<"      coincidence latency "<<fCoincLatency<<std::endl;
}

BoardSettings::BoardSettings(const int& nofChannels, const V1730_TEMPLATE& templateSettings)
{
	fLinkType         = static_cast<CAEN_DGTZ_ConnectionType>(templateSettings.link_type);
	fBoardType        = static_cast<EBoardType>(templateSettings.board_type);
	fVmeBaseAddress   = templateSettings.vme_base_address;
	fPortNumber       = 0;
	fDeviceNumber     = 0;
	fAcquisitionMode  = static_cast<CAEN_DGTZ_DPP_AcqMode_t>(templateSettings.acquisition_mode);
	fIOLevel          = static_cast<CAEN_DGTZ_IOLevel_t>(templateSettings.io_level);
	fChannelMask      = templateSettings.channel_mask;
	fRunSync          = static_cast<CAEN_DGTZ_RunSyncMode_t>(templateSettings.runsync_mode);
	fEventAggregation = templateSettings.event_aggregation;
	fTriggerMode      = static_cast<CAEN_DGTZ_TriggerMode_t>(templateSettings.trigger_mode);
	fChannelSettings.resize(nofChannels, ChannelSettings(templateSettings));
	fChannelParameter.purh   = static_cast<CAEN_DGTZ_DPP_PUR_t>(templateSettings.pile_up_rejection_mode);
	fChannelParameter.purgap = templateSettings.pile_up_gap;
	fChannelParameter.blthr  = templateSettings.baseline_threshold;
	fChannelParameter.bltmo  = templateSettings.baseline_timeout;
	fChannelParameter.trgho  = templateSettings.trigger_holdoff;
	for(int ch = 0; ch < nofChannels; ++ch) {
		fChannelParameter.thr[ch]   = templateSettings.threshold;
		fChannelParameter.nsbl[ch]  = templateSettings.baseline_samples;
		fChannelParameter.lgate[ch] = templateSettings.long_gate;
		fChannelParameter.sgate[ch] = templateSettings.short_gate;
		fChannelParameter.pgate[ch] = templateSettings.pre_gate;
		fChannelParameter.selft[ch] = templateSettings.self_trigger;
		fChannelParameter.trgc[ch]  = static_cast<CAEN_DGTZ_DPP_TriggerConfig_t>(templateSettings.trigger_configuration);
		fChannelParameter.tvaw[ch]  = templateSettings.trigger_validation_window;
		fChannelParameter.csens[ch] = templateSettings.charge_sensitivity;
	}
}

BoardSettings::BoardSettings(const int& boardNumber, const int& nofChannels, TEnv*& settings)
{
	fLinkType         = static_cast<CAEN_DGTZ_ConnectionType>(settings->GetValue(Form("Board.%d.LinkType", boardNumber), CAEN_DGTZ_USB));//0
	fBoardType        = static_cast<EBoardType>(settings->GetValue(Form("Board.%d.BoardType", boardNumber), 2));//EBoardType::kVME
	fVmeBaseAddress   = 0;
	fPortNumber       = boardNumber;
	fDeviceNumber     = 0;
	fAcquisitionMode  = static_cast<CAEN_DGTZ_DPP_AcqMode_t>(settings->GetValue(Form("Board.%d.AcquisitionMode", boardNumber), CAEN_DGTZ_DPP_ACQ_MODE_Mixed));//2
	fIOLevel          = static_cast<CAEN_DGTZ_IOLevel_t>(settings->GetValue(Form("Board.%d.IOlevel", boardNumber), CAEN_DGTZ_IOLevel_NIM));//0
	fTriggerMode      = static_cast<CAEN_DGTZ_TriggerMode_t>(settings->GetValue(Form("Board.%d.TriggerMode", boardNumber), CAEN_DGTZ_TRGMODE_ACQ_ONLY));//1
	fChannelMask      = settings->GetValue(Form("Board.%d.ChannelMask", boardNumber), 0xff);
	fRunSync          = static_cast<CAEN_DGTZ_RunSyncMode_t>(settings->GetValue(Form("Board.%d.RunSync", boardNumber), CAEN_DGTZ_RUN_SYNC_Disabled));//0
	fEventAggregation = settings->GetValue(Form("Board.%d.EventAggregate", boardNumber), 0);

	fChannelSettings.resize(nofChannels);
	fChannelParameter.purh   = static_cast<CAEN_DGTZ_DPP_PUR_t>(settings->GetValue(Form("Board.%d.PileUpRejection", boardNumber), CAEN_DGTZ_DPP_PSD_PUR_DetectOnly));//0
	fChannelParameter.purgap = settings->GetValue(Form("Board.%d.PurityGap", boardNumber), 100);
	fChannelParameter.blthr  = settings->GetValue(Form("Board.%d.BaseLine.Threshold", boardNumber), 3);
	fChannelParameter.bltmo  = settings->GetValue(Form("Board.%d.BaseLine.Timeout", boardNumber), 100);
	fChannelParameter.trgho  = settings->GetValue(Form("Board.%d.TriggerHoldOff", boardNumber), 8);
	for(int ch = 0; ch < nofChannels; ++ch) {
		fChannelSettings[ch] = ChannelSettings(boardNumber, ch, settings);
		fChannelParameter.thr[ch]   = settings->GetValue(Form("Board.%d.Channel.%d.Threshold", boardNumber, ch), 50);
		fChannelParameter.nsbl[ch]  = settings->GetValue(Form("Board.%d.Channel.%d.BaselineSamples", boardNumber, ch), 4);
		fChannelParameter.lgate[ch] = settings->GetValue(Form("Board.%d.Channel.%d.LongGate", boardNumber, ch), 32);
		fChannelParameter.sgate[ch] = settings->GetValue(Form("Board.%d.Channel.%d.ShortGate", boardNumber, ch), 24);
		fChannelParameter.pgate[ch] = settings->GetValue(Form("Board.%d.Channel.%d.PreGate", boardNumber, ch), 8);
		fChannelParameter.selft[ch] = settings->GetValue(Form("Board.%d.Channel.%d.SelfTrigger", boardNumber, ch), 1);
		fChannelParameter.trgc[ch]  = static_cast<CAEN_DGTZ_DPP_TriggerConfig_t>(settings->GetValue(Form("Board.%d.Channel.%d.TriggerConfiguration", boardNumber, ch), CAEN_DGTZ_DPP_TriggerConfig_Threshold));//1
		fChannelParameter.tvaw[ch]  = settings->GetValue(Form("Board.%d.Channel.%d.TriggerValidationAcquisitionWindow", boardNumber, ch), 50);
		fChannelParameter.csens[ch] = settings->GetValue(Form("Board.%d.Channel.%d.ChargeSensitivity", boardNumber, ch), 0);
	}
}

void BoardSettings::ReadCustomSettings(const HNDLE& hDb, const HNDLE& hKey)
{
	// loop over sub-keys
	HNDLE hSubKey;
	KEY key;
	int size;
	for(int i = 0; ; ++i) {
		db_enum_key(hDb, hKey, i, &hSubKey);
		if(!hSubKey) break; // end of list reached
		db_get_key(hDb, hSubKey, &key);
		if(strcmp(key.name, "Link Type") == 0 && key.num_values == 1) {
			size = sizeof(fLinkType);
			db_get_data(hDb, hSubKey, &fLinkType, &size, TID_WORD);
			//fLinkType = static_cast<CAEN_DGTZ_ConnectionType>(*reinterpret_cast<WORD*>(key.data));
		} else if(strcmp(key.name, "Board Type") == 0 && key.num_values == 1) {
			size = sizeof(fBoardType);
			db_get_data(hDb, hSubKey, &fBoardType, &size, TID_WORD);
		} else if(strcmp(key.name, "VME base address") == 0 && key.num_values == 1) {
			size = sizeof(fVmeBaseAddress);
			db_get_data(hDb, hSubKey, &fVmeBaseAddress, &size, TID_WORD);
		} else if(strcmp(key.name, "Port number") == 0 && key.num_values == 1) {
			size = sizeof(fPortNumber);
			db_get_data(hDb, hSubKey, &fPortNumber, &size, TID_WORD);
		} else if(strcmp(key.name, "Device number") == 0 && key.num_values == 1) {
			size = sizeof(fDeviceNumber);
			db_get_data(hDb, hSubKey, &fDeviceNumber, &size, TID_WORD);
		} else if(strcmp(key.name, "Acquisition Mode") == 0 && key.num_values == 1) {
			size = sizeof(fAcquisitionMode);
			db_get_data(hDb, hSubKey, &fAcquisitionMode, &size, TID_WORD);
			//fAcquisitionMode = static_cast<CAEN_DGTZ_DPP_AcqMode_t>(*reinterpret_cast<WORD*>(key.data));
		} else if(strcmp(key.name, "IO Level") == 0 && key.num_values == 1) {
			size = sizeof(fIOLevel);
			db_get_data(hDb, hSubKey, &fIOLevel, &size, TID_WORD);
			//fIOLevel = static_cast<CAEN_DGTZ_IOLevel_t>(*reinterpret_cast<WORD*>(key.data));
		} else if(strcmp(key.name, "Trigger Mode") == 0 && key.num_values == 1) {
			size = sizeof(fTriggerMode);
			db_get_data(hDb, hSubKey, &fTriggerMode, &size, TID_WORD);
			//fTriggerMode = static_cast<CAEN_DGTZ_TriggerMode_t>(*reinterpret_cast<BOOL*>(key.data));
		} else if(strcmp(key.name, "Channel Mask") == 0 && key.num_values == 1) {
			size = sizeof(fChannelMask);
			db_get_data(hDb, hSubKey, &fChannelMask, &size, TID_WORD);
		} else if(strcmp(key.name, "RunSync mode") == 0 && key.num_values == 1) {
			size = sizeof(fRunSync);
			db_get_data(hDb, hSubKey, &fRunSync, &size, TID_WORD);
			//fRunSync = static_cast<CAEN_DGTZ_RunSyncMode_t>(*reinterpret_cast<WORD*>(key.data));
		} else if(strcmp(key.name, "Event aggregation") == 0 && key.num_values == 1) {
			size = sizeof(fEventAggregation);
			db_get_data(hDb, hSubKey, &fEventAggregation, &size, TID_WORD);
		} else if(strcmp(key.name, "Pile up rejection mode") == 0 && key.num_values == 1) {
			size = sizeof(fChannelParameter.purh);
			db_get_data(hDb, hSubKey, &fChannelParameter.purh, &size, TID_WORD);
			//fChannelParameter.purh = static_cast<CAEN_DGTZ_DPP_PUR_t>(*reinterpret_cast<WORD*>(key.data));
		} else if(strcmp(key.name, "Pile up gap") == 0 && key.num_values == 1) {
			size = sizeof(fChannelParameter.purgap);
			db_get_data(hDb, hSubKey, &fChannelParameter.purgap, &size, TID_WORD);
		} else if(strcmp(key.name, "Baseline threshold") == 0 && key.num_values == 1) {
			size = sizeof(fChannelParameter.blthr);
			db_get_data(hDb, hSubKey, &fChannelParameter.blthr, &size, TID_WORD);
		} else if(strcmp(key.name, "Baseline timeout") == 0 && key.num_values == 1) {
			size = sizeof(fChannelParameter.bltmo);
			db_get_data(hDb, hSubKey, &fChannelParameter.bltmo, &size, TID_WORD);
		} else if(strcmp(key.name, "Trigger holdoff") == 0 && key.num_values == 1) {
			size = sizeof(fChannelParameter.trgho);
			db_get_data(hDb, hSubKey, &fChannelParameter.trgho, &size, TID_WORD);
		} else if(strncmp(key.name, "Channel ", 8) != 0) {
			std::cout<<"unrecognized custom entry "<<key.name<<std::endl;
		}
	}
}

void BoardSettings::ReadCustomChannelSettings(const int& channel, const HNDLE& hDb, const HNDLE& hKey)
{
	fChannelSettings.at(channel).ReadCustomSettings(hDb, hKey);
	// loop over sub-keys
	HNDLE hSubKey;
	KEY key;
	int size;
	for(int i = 0; ; ++i) {
		db_enum_key(hDb, hKey, i, &hSubKey);
		if(!hSubKey) break; // end of list reached
		db_get_key(hDb, hSubKey, &key);
		if(strcmp(key.name, "Threshold") == 0 && key.num_values == 1) {
			size = sizeof(fChannelParameter.thr[channel]);
			db_get_data(hDb, hSubKey, &fChannelParameter.thr[channel], &size, TID_WORD);
		} else if(strcmp(key.name, "Baseline samples") == 0 && key.num_values == 1) {
			size = sizeof(fChannelParameter.nsbl[channel]);
			db_get_data(hDb, hSubKey, &fChannelParameter.nsbl[channel], &size, TID_WORD);
		} else if(strcmp(key.name, "Long gate") == 0 && key.num_values == 1) {
			size = sizeof(fChannelParameter.lgate[channel]);
			db_get_data(hDb, hSubKey, &fChannelParameter.lgate[channel], &size, TID_WORD);
		} else if(strcmp(key.name, "Short gate") == 0 && key.num_values == 1) {
			size = sizeof(fChannelParameter.sgate[channel]);
			db_get_data(hDb, hSubKey, &fChannelParameter.sgate[channel], &size, TID_WORD);
		} else if(strcmp(key.name, "Pre gate") == 0 && key.num_values == 1) {
			size = sizeof(fChannelParameter.pgate[channel]);
			db_get_data(hDb, hSubKey, &fChannelParameter.pgate[channel], &size, TID_WORD);
		} else if(strcmp(key.name, "Self trigger") == 0 && key.num_values == 1) {
			size = sizeof(fChannelParameter.selft[channel]);
			db_get_data(hDb, hSubKey, &fChannelParameter.selft[channel], &size, TID_WORD);
		} else if(strcmp(key.name, "Trigger configuration") == 0 && key.num_values == 1) {
			size = sizeof(fChannelParameter.trgc[channel]);
			db_get_data(hDb, hSubKey, &fChannelParameter.trgc[channel], &size, TID_WORD);
			//fChannelParameter.trgc[channel] = static_cast<CAEN_DGTZ_DPP_TriggerConfig_t>(*reinterpret_cast<WORD*>(key.data));
		} else if(strcmp(key.name, "Trigger validation window") == 0 && key.num_values == 1) {
			size = sizeof(fChannelParameter.tvaw[channel]);
			db_get_data(hDb, hSubKey, &fChannelParameter.tvaw[channel], &size, TID_WORD);
		} else if(strcmp(key.name, "Charge sensitivity") == 0 && key.num_values == 1) {
			size = sizeof(fChannelParameter.csens[channel]);
			db_get_data(hDb, hSubKey, &fChannelParameter.csens[channel], &size, TID_WORD);
		} else {
			// we keep both channel and channelparameter (which are "per board") settings
			// in the "Channel x" directory, so there might be unrecognized entries
		}
	}
}

BoardSettings::~BoardSettings()
{
}

void BoardSettings::Print()
{
	std::cout<<"  link type "<<fLinkType<<" = ";
	switch(fLinkType) {
		case CAEN_DGTZ_USB:
			std::cout<<"USB"<<std::endl;
			break;
		case CAEN_DGTZ_OpticalLink:
			std::cout<<"Optical Link"<<std::endl;
			break;
		default:
			std::cout<<"unknown"<<std::endl;
			break;
	}
	std::cout<<"  board type "<<static_cast<int>(fBoardType)<<" = ";
	switch(fBoardType) {
		case EBoardType::kDesktop:
			std::cout<<"Desktop"<<std::endl;
			break;
		case EBoardType::kNIM:
			std::cout<<"NIM"<<std::endl;
			break;
		case EBoardType::kVME:
			std::cout<<"VME"<<std::endl;
			break;
		default:
			std::cout<<"unknown"<<std::endl;
			break;
	}
	std::cout<<"   VME base address 0x"<<std::hex<<fVmeBaseAddress<<std::dec<<std::endl;
	std::cout<<"   Port number "<<fPortNumber<<std::endl;
	std::cout<<"   Device number "<<fDeviceNumber<<std::endl;
	std::cout<<"   acquisition mode "<<fAcquisitionMode<<" = ";
	switch(fAcquisitionMode) {
		case CAEN_DGTZ_DPP_ACQ_MODE_Oscilloscope:
			std::cout<<"oscilloscope"<<std::endl;
			break;
		case CAEN_DGTZ_DPP_ACQ_MODE_List:
			std::cout<<"list mode"<<std::endl;
			break;
		case CAEN_DGTZ_DPP_ACQ_MODE_Mixed:
			std::cout<<"mixed"<<std::endl;
			break;
			//case CAEN_DGTZ_SW_CONTROLLED:
			//	std::cout<<"software controlled"<<std::endl;
			//	break;
			//case CAEN_DGTZ_S_IN_CONTROLLED:
			//	std::cout<<"external signal controlled"<<std::endl;
			//	break;
			//case CAEN_DGTZ_FIRST_TRG_CONTROLLED:
			//	std::cout<<"first trigger controlled"<<std::endl;
			//	break;
		default:
			std::cout<<"unknown"<<std::endl;
			break;
	}
	std::cout<<"   IO level "<<fIOLevel<<" = ";
	switch(fIOLevel) {
		case CAEN_DGTZ_IOLevel_NIM:
			std::cout<<"NIM"<<std::endl;
			break;
		case CAEN_DGTZ_IOLevel_TTL:
			std::cout<<"TTL"<<std::endl;
			break;
		default:
			std::cout<<"unknown"<<std::endl;
			break;
	}
	std::cout<<"   channel mask 0x"<<std::hex<<fChannelMask<<std::dec<<std::endl;
	std::cout<<"   run sync "<<fRunSync<<" = ";
	switch(fRunSync) {
		case CAEN_DGTZ_RUN_SYNC_Disabled:
			std::cout<<"disabled"<<std::endl;
			break;
		case CAEN_DGTZ_RUN_SYNC_TrgOutTrgInDaisyChain:
			std::cout<<"trigger out/trigger in chain"<<std::endl;
			break;
		case CAEN_DGTZ_RUN_SYNC_TrgOutSinDaisyChain:
			std::cout<<"trigger out/s in chain"<<std::endl;
			break;
		case CAEN_DGTZ_RUN_SYNC_SinFanout:
			std::cout<<"s in fanout"<<std::endl;
			break;
		case CAEN_DGTZ_RUN_SYNC_GpioGpioDaisyChain:
			std::cout<<"gpio chain"<<std::endl;
			break;
		default:
			std::cout<<"unknown"<<std::endl;
			break;
	}
	std::cout<<"   event aggregation "<<fEventAggregation<<std::endl;
	std::cout<<"   trigger mode "<<fTriggerMode<<std::endl;

	std::cout<<"   pile-up rejection mode "<<fChannelParameter.purh<<" = ";
	switch(fChannelParameter.purh) {
		case CAEN_DGTZ_DPP_PSD_PUR_DetectOnly:
			std::cout<<"detection only"<<std::endl;
			break;
		case CAEN_DGTZ_DPP_PSD_PUR_Enabled:
			std::cout<<"enabled"<<std::endl;
			break;
		default:
			std::cout<<"unknown"<<std::endl;
			break;
	}
	std::cout<<"   pile-up gap "<<fChannelParameter.purgap<<std::endl;
	std::cout<<"   baseline threshold "<<fChannelParameter.blthr<<std::endl;
	std::cout<<"   baseline timeout "<<fChannelParameter.bltmo<<std::endl;
	std::cout<<"   trigger holdoff "<<fChannelParameter.trgho<<std::endl;
	for(int ch = 0; ch < static_cast<int>(fChannelSettings.size()); ++ch) {
		std::cout<<"   Channel #"<<ch<<":"<<std::endl;
		fChannelSettings[ch].Print();
		std::cout<<"      threshold "<<fChannelParameter.thr[ch]<<std::endl;
		std::cout<<"      baseline samples "<<fChannelParameter.nsbl[ch]<<std::endl;
		std::cout<<"      long gate "<<fChannelParameter.lgate[ch]<<std::endl;
		std::cout<<"      short gate "<<fChannelParameter.sgate[ch]<<std::endl;
		std::cout<<"      pre-gate "<<fChannelParameter.pgate[ch]<<std::endl;
		std::cout<<"      self trigger "<<fChannelParameter.selft[ch]<<std::endl;
		std::cout<<"      trigger conf. "<<fChannelParameter.trgc[ch]<<" = ";
		switch(fChannelParameter.trgc[ch]) {
			case CAEN_DGTZ_DPP_TriggerConfig_Peak:
				std::cout<<" peak"<<std::endl;
				break;
			case CAEN_DGTZ_DPP_TriggerConfig_Threshold:
				std::cout<<" threshold"<<std::endl;
				break;
			default:
				std::cout<<"unknown"<<std::endl;
				break;
		}
		std::cout<<"      trigger val. window "<<fChannelParameter.tvaw[ch]<<std::endl;
		std::cout<<"      charge sensitivity "<<fChannelParameter.csens[ch]<<std::endl;
	}
}

CaenSettings::CaenSettings(bool debug)
{
	fDebug = debug;
}

CaenSettings::~CaenSettings()
{
}

void CaenSettings::ReadOdb(HNDLE hDB)
{
	if(hDB == 0) {
		std::cerr<<"Can't read settings from ODB, handle provided is "<<hDB<<std::endl;
		return;
	}
	// read template from ODB
	HNDLE hSet;
	V1730_TEMPLATE templateSettings;
	V1730_TEMPLATE_STR(v1730TemplateStr);
	if(db_create_record(hDB, 0, "/DAQ/params/VX1730/template", strcomb(v1730TemplateStr)) != DB_SUCCESS) {
		std::cout<<"failed to create record"<<std::endl;
		throw;
	}

	db_find_key(hDB, 0, "/DAQ/params/VX1730/template", &hSet);
	int size = sizeof(templateSettings);
	if(db_get_record(hDB, hSet, &templateSettings, &size, 0) != DB_SUCCESS) {
		std::cout<<"Error occured trying to read \"/DAQ/params/VX1730/template\""<<std::endl;
		throw;
	}

	fNumberOfBoards = templateSettings.number_of_digitizers;
	if(fNumberOfBoards < 1) {
		std::cout<<fNumberOfBoards<<" boards is not possible!"<<std::endl;
		throw;
	}
	fNumberOfChannels = templateSettings.channels_per_digitizer;
	if(fNumberOfChannels < 1) {
		std::cout<<fNumberOfChannels<<" maximum channels is not possible!"<<std::endl;
		throw;
	}
	fUseExternalClock = templateSettings.use_external_clock;
	fRawOutput = templateSettings.raw_output;
	fBufferSize = 100000;

	if(fDebug) {
		// print template settings
		std::cout<<"template settings:"<<std::endl
			<<"number_of_digitizer "<<templateSettings.number_of_digitizers<<std::endl
			<<"channels_per_digitizer "<<templateSettings.channels_per_digitizer<<std::endl
			<<"raw_output "<<templateSettings.raw_output<<std::endl
			<<"link_type "<<templateSettings.link_type<<std::endl
			<<"vme_base_address "<<templateSettings.vme_base_address<<std::endl
			<<"acquisition_mode "<<templateSettings.acquisition_mode<<std::endl
			<<"io_level "<<templateSettings.io_level<<std::endl
			<<"trigger_mode "<<templateSettings.trigger_mode<<std::endl
			<<"channel_mask "<<templateSettings.channel_mask<<std::endl
			<<"runsync_mode "<<templateSettings.runsync_mode<<std::endl
			<<"event_aggregation "<<templateSettings.event_aggregation<<std::endl
			<<"record_length "<<templateSettings.record_length<<std::endl
			<<"dc_offset "<<templateSettings.dc_offset<<std::endl
			<<"pre_trigger "<<templateSettings.pre_trigger<<std::endl
			<<"pulse_polarity "<<templateSettings.pulse_polarity<<std::endl
			<<"enable_cfd "<<templateSettings.enable_cfd<<std::endl
			<<"cfd_delay "<<templateSettings.cfd_delay<<std::endl
			<<"cfd_fraction "<<templateSettings.cfd_fraction<<std::endl
			<<"cfd_interpolation_points "<<templateSettings.cfd_interpolation_points<<std::endl
			<<"enable_coinc "<<templateSettings.enable_coinc<<std::endl
			<<"enable_baseline "<<templateSettings.enable_baseline<<std::endl
			<<"coinc_window "<<templateSettings.coinc_window<<std::endl
			<<"coinc_latency "<<templateSettings.coinc_latency<<std::endl
			<<"pile_up_rejection_mode "<<templateSettings.pile_up_rejection_mode<<std::endl
			<<"pile_up_gap "<<templateSettings.pile_up_gap<<std::endl
			<<"baseline_threshold "<<templateSettings.baseline_threshold<<std::endl
			<<"baseline_timeout "<<templateSettings.baseline_timeout<<std::endl
			<<"trigger_holdoff "<<templateSettings.trigger_holdoff<<std::endl
			<<"threshold "<<templateSettings.threshold<<std::endl
			<<"baseline_samples "<<templateSettings.baseline_samples<<std::endl
			<<std::endl;
	}
	fBoardSettings.resize(fNumberOfBoards);
	for(int i = 0; i < fNumberOfBoards; ++i) {
		fBoardSettings[i] = BoardSettings(fNumberOfChannels, templateSettings);
		fBoardSettings[i].PortNumber(i); //this might get overwritten by custom settings
	}

	// search for custom information
	HNDLE hBoard;
	HNDLE hChannel;
	std::ostringstream oss;
	if(db_find_key(hDB, 0, "/DAQ/params/VX1730/custom", &hSet) == DB_SUCCESS) {
		for(int board = 0; board < fNumberOfBoards; ++board) {
			oss.str("");
			oss.clear();
			oss<<"Board "<<board;
			if(db_find_key(hDB, hSet, oss.str().c_str(), &hBoard) == DB_SUCCESS) {
				// found a set of custom settings for this board so we pass them along
				fBoardSettings[board].ReadCustomSettings(hDB, hBoard);
				for(int channel = 0; channel < fNumberOfChannels; ++channel) {
					oss.str("");
					oss.clear();
					oss<<"Channel "<<channel;
					//std::cout<<oss.str().c_str()<<std::endl;
					if(db_find_key(hDB, hBoard, oss.str().c_str(), &hChannel) == DB_SUCCESS) {
						//std::cout<<"found"<<std::endl;
						// found a set of custom settings for this channel so we pass them along
						fBoardSettings[board].ReadCustomChannelSettings(channel, hDB, hChannel);
					}
				}
			}
		}
	} else {
		db_create_key(hDB, 0, "/DAQ/params/VX1730/custom", TID_KEY);
	}

	Print();
}

bool CaenSettings::ReadSettingsFile(const std::string& filename)
{
	auto settings = new TEnv(filename.c_str());
	if(settings == NULL || settings->ReadFile(filename.c_str(), kEnvLocal) != 0) {
		return false;
	}

	fNumberOfBoards = settings->GetValue("NumberOfBoards", 1);
	if(fNumberOfBoards < 1) {
		std::cout<<fNumberOfBoards<<" boards is not possible!"<<std::endl;
		return false;
	}
	fNumberOfChannels = settings->GetValue("NumberOfChannels", 8);
	if(fNumberOfChannels < 1) {
		std::cout<<fNumberOfChannels<<" maximum channels is not possible!"<<std::endl;
		return false;
	}
	fUseExternalClock = settings->GetValue("UseExternalClocl", false);
	fBufferSize = settings->GetValue("BufferSize", 100000);

	fBoardSettings.resize(fNumberOfBoards);
	for(int i = 0; i < fNumberOfBoards; ++i) {
		fBoardSettings[i] = BoardSettings(i, fNumberOfChannels, settings);
	}

	return true;
}

bool CaenSettings::WriteOdb()
{
	// for some reason using the handle doesn't work, cm_get_experiment_database(&hDB, NULL) gives hDB = 0
	// which then fails when used
	// so instead we create a script which uses odbedit and run it

	// for some reason WriteOdb does not work, so we create a script 'writeSettings.sh' and run it
	std::ofstream script("writeSettings.sh", std::ios::out);

	script<<"#!/bin/bash"<<std::endl
		<<"#"<<std::endl
		<<"# script to update ODB settings, automatically created by WriteOdb"<<std::endl
		<<std::endl;

	// template
	script<<"odbedit -c \"set \\\"/DAQ/params/VX1730/custom/Number of digitizers\\\" "<<fNumberOfBoards<<"\""<<std::endl;
	script<<"odbedit -c \"set \\\"/DAQ/params/VX1730/custom/Channels per digitizer\\\" "<<fNumberOfChannels<<"\""<<std::endl;
	script<<"odbedit -c \"set \\\"/DAQ/params/VX1730/custom/Use external clock\\\" "<<fUseExternalClock<<"\""<<std::endl;
	script<<"odbedit -c \"set \\\"/DAQ/params/VX1730/custom/Raw output\\\" "<<fRawOutput<<"\""<<std::endl;
	//script<<"odbedit -c \"set \\\"/DAQ/params/VX1730/custom/Link Type\\\" "<<fLinkType<<"\""<<std::endl;
	//script<<"odbedit -c \"set \\\"/DAQ/params/VX1730/custom/VME base address\\\" "<<fVmeBaseAddress<<"\""<<std::endl;
	//script<<"odbedit -c \"set \\\"/DAQ/params/VX1730/custom/Acquisition Mode\\\" "<<fAcquisitionMode<<"\""<<std::endl;
	//script<<"odbedit -c \"set \\\"/DAQ/params/VX1730/custom/IO Level\\\" "<<fIOLevel<<"\""<<std::endl;
	//script<<"odbedit -c \"set \\\"/DAQ/params/VX1730/custom/Trigger Mode\\\" "<<fTriggerMode<<"\""<<std::endl;
	//script<<"odbedit -c \"set \\\"/DAQ/params/VX1730/custom/Channel Mask\\\" "<<fChannelMask<<"\""<<std::endl;
	//script<<"odbedit -c \"set \\\"/DAQ/params/VX1730/custom/RunSync Mode\\\" "<<fRunSync<<"\""<<std::endl;
	//script<<"odbedit -c \"set \\\"/DAQ/params/VX1730/custom/Event aggregation\\\" "<<fEventAggregation<<"\""<<std::endl;

	//script<<"odbedit -c \"set \\\"/DAQ/params/VX1730/custom/Record length\\\" "<<fRecordLength<<"\""<<std::endl;
	//script<<"odbedit -c \"set \\\"/DAQ/params/VX1730/custom/DC offset\\\" "<<fDCOffset<<"\""<<std::endl;
	//script<<"odbedit -c \"set \\\"/DAQ/params/VX1730/custom/Pre trigger\\\" "<<fPreTrigger<<"\""<<std::endl;
	//script<<"odbedit -c \"set \\\"/DAQ/params/VX1730/custom/Pulse polarity\\\" "<<fPulsePolarity<<"\""<<std::endl;
	//script<<"odbedit -c \"set \\\"/DAQ/params/VX1730/custom/Enable CFD\\\" "<<fEnableCfd<<"\""<<std::endl;
	//script<<"odbedit -c \"set \\\"/DAQ/params/VX1730/custom/CFD delay\\\" "<<(fCfdParameters & 0xff)<<"\""<<std::endl;
	//script<<"odbedit -c \"set \\\"/DAQ/params/VX1730/custom/CFD fraction\\\" "<<((fCfdParameters>>8) & 0x3)<<"\""<<std::endl;
	//script<<"odbedit -c \"set \\\"/DAQ/params/VX1730/custom/CFD interpolation points\\\" "<<((fCfdParameters>>10) & 0x3)<<"\""<<std::endl;
	//script<<"odbedit -c \"set \\\"/DAQ/params/VX1730/custom/Enable Coincidence\\\" "<<fEnableCoinc<<"\""<<std::endl;
	//script<<"odbedit -c \"set \\\"/DAQ/params/VX1730/custom/Enable Baseline\\\" "<<fEnableBaseline<<"\""<<std::endl;
	//script<<"odbedit -c \"set \\\"/DAQ/params/VX1730/custom/Coincidence Window\\\" "<<fCoincWindow <<"\""<<std::endl;
	//script<<"odbedit -c \"set \\\"/DAQ/params/VX1730/custom/Coincidence Latency\\\" "<<fCoincLatency <<"\""<<std::endl;

	//script<<"odbedit -c \"set \\\"/DAQ/params/VX1730/custom/Pile up rejection mode\\\" "<<fChannelParameter.purh<<"\""<<std::endl;
	//script<<"odbedit -c \"set \\\"/DAQ/params/VX1730/custom/Pile up gap\\\" "<<fChannelParameter.purgap<<"\""<<std::endl;
	//script<<"odbedit -c \"set \\\"/DAQ/params/VX1730/custom/Baseline threshold\\\" "<<fChannelParameter.blthr<<"\""<<std::endl;
	//script<<"odbedit -c \"set \\\"/DAQ/params/VX1730/custom/Baseline timeout\\\" "<<fChannelParameter.bltmo<<"\""<<std::endl;
	//script<<"odbedit -c \"set \\\"/DAQ/params/VX1730/custom/Trigger holdoff\\\" "<<fChannelParameter.trgho<<"\""<<std::endl;
	//for(int ch = 0; ch < fNumberOfChannels; ++ch) {
	//	script<<"odbedit -c \"set \\\"/DAQ/params/VX1730/custom/Threshold["<<ch<<"]\\\" "<<fChannelParameter.thr[ch]<<"\""<<std::endl;
	//	script<<"odbedit -c \"set \\\"/DAQ/params/VX1730/custom/Baseline samples["<<ch<<"]\\\" "<<fChannelParameter.nsbl[ch]<<"\""<<std::endl;
	//	script<<"odbedit -c \"set \\\"/DAQ/params/VX1730/custom/Long gate["<<ch<<"]\\\" "<<fChannelParameter.lgate[ch]<<"\""<<std::endl;
	//	script<<"odbedit -c \"set \\\"/DAQ/params/VX1730/custom/Short gate["<<ch<<"]\\\" "<<fChannelParameter.sgate[ch]<<"\""<<std::endl;
	//	script<<"odbedit -c \"set \\\"/DAQ/params/VX1730/custom/Pre gate["<<ch<<"]\\\" "<<fChannelParameter.pgate[ch]<<"\""<<std::endl;
	//	script<<"odbedit -c \"set \\\"/DAQ/params/VX1730/custom/Self trigger["<<ch<<"]\\\" "<<fChannelParameter.selft[ch]<<"\""<<std::endl;
	//	script<<"odbedit -c \"set \\\"/DAQ/params/VX1730/custom/Trigger configuration["<<ch<<"]\\\" "<<fChannelParameter.trgc[ch]<<"\""<<std::endl;
	//	script<<"odbedit -c \"set \\\"/DAQ/params/VX1730/custom/Trigger validation window["<<ch<<"]\\\" "<<fChannelParameter.tvaw[ch]<<"\""<<std::endl;
	//	script<<"odbedit -c \"set \\\"/DAQ/params/VX1730/custom/Charge sensitivity["<<ch<<"]\\\" "<<fChannelParameter.csens[ch]<<"\""<<std::endl;
	//}
	script<<std::endl;

	script.close();

	std::system("chmod +x writeSettings.sh; ./writeSettings.sh; rm writeSettings.sh");

	return true;

	// write ODB settings
	HNDLE hDB;
	HNDLE hSet;
	V1730_TEMPLATE settings;

	std::cout<<"here ..."<<std::endl;
	// copy settings to V1730_TEMPLATE struct
	if(fNumberOfBoards > MAX_BOARDS) {
		std::cerr<<"Too many boards specified in settings: "<<fNumberOfBoards<<" > "<<MAX_BOARDS<<std::endl;
		return false;
	}
	if(fNumberOfChannels*fNumberOfBoards > MAX_CHANNELS) {
		std::cerr<<"Too many channels specified in settings: "<<fNumberOfChannels*fNumberOfBoards<<" > "<<MAX_CHANNELS<<std::endl;
		return false;
	}
	settings.number_of_digitizers = fNumberOfBoards;
	settings.channels_per_digitizer = fNumberOfChannels;
	settings.use_external_clock = fUseExternalClock;
	//settings.link_type = fLinkType;
	//settings.vme_base_address = fVmeBaseAddress;
	//settings.acquisition_mode = fAcquisitionMode;
	//settings.io_level = fIOLevel;
	//settings.trigger_mode = fTriggerMode;
	//settings.channel_mask = fChannelMask;
	//settings.runsync_mode = fRunSync;
	//settings.event_aggregation = fEventAggregation;

	//settings.record_length = fRecordLength;
	//settings.dc_offset = fDCOffset;
	//settings.pre_trigger = fPreTrigger;
	//settings.pulse_polarity = fPulsePolarity;
	//settings.enable_cfd = fEnableCfd;
	//settings.cfd_delay = fCfdParameters & 0xff;
	//settings.cfd_fraction = (fCfdParameters>>8) & 0x3;
	//settings.cfd_interpolation_points = (fCfdParameters>>10) & 0x3;

	//settings.enable_coinc = fEnableCoinc;
	//settings.enable_baseline = fEnableBaseline;
	//settings.coinc_window = fCoincWindow;
	//settings.coinc_latency = fCoincLatency;

	//settings.pile_up_rejection_mode = fChannelParameter.purh;
	//settings.pile_up_gap = fChannelParameter.purgap;
	//settings.baseline_threshold = fChannelParameter.blthr;
	//settings.baseline_timeout = fChannelParameter.bltmo;
	//settings.trigger_holdoff = fChannelParameter.trgho;
	//for(int ch = 0; ch < fNumberOfChannels; ++ch) {
	//	settings.threshold[ch] = fChannelParameter.thr[ch];
	//	settings.baseline_samples[ch] = fChannelParameter.nsbl[ch];
	//	settings.long_gate[ch] = fChannelParameter.lgate[ch];
	//	settings.short_gate[ch] = fChannelParameter.sgate[ch];
	//	settings.pre_gate[ch] = fChannelParameter.pgate[ch];
	//	settings.self_trigger[ch] = fChannelParameter.selft[ch];
	//	settings.trigger_configuration[ch] = fChannelParameter.trgc[ch];
	//	settings.trigger_validation_window[ch] = fChannelParameter.tvaw[ch];
	//	settings.charge_sensitivity[ch] = fChannelParameter.csens[ch];
	//}
	settings.raw_output = fRawOutput;
	std::cout<<"connecting to database ..."<<std::endl;
	// connect to ODB
	if(cm_get_experiment_database(&hDB, NULL) != CM_SUCCESS) {
		std::cout<<"failed to get database handle "<<hDB<<std::endl;
		return false;
	}

	std::cout<<"creating record using handle "<<hDB<<" ..."<<std::endl;
	// create record (just in case it didn't exist already)
	V1730_TEMPLATE_STR(v1730TemplateStr);
	if(db_create_record(hDB, 0, "/DAQ/params/VX1730/template", strcomb(v1730TemplateStr)) != DB_SUCCESS) {
		std::cout<<"failed to create record"<<std::endl;
		return false;
	}

	std::cout<<"writing ..."<<std::endl;
	// write settings to ODB
	db_find_key(hDB, 0, "/DAQ/params/VX1730/template", &hSet);
	int size = sizeof(settings);
	if(db_set_record(hDB, hSet, &settings, size, 0) != DB_SUCCESS) {
		std::cout<<"Error occured trying to write \"/DAQ/params/VX1730/template\""<<std::endl;
		return false;
	}

	return true;
}

void CaenSettings::Print()
{
	std::cout<<(fUseExternalClock?"Using ":"Not using ")<<" external clock for "<<fNumberOfBoards<<" board(s) with "<<fNumberOfChannels<<" channels each:"<<std::endl;
	for(size_t i = 0; i < fBoardSettings.size(); ++i) {
		std::cout<<"Board #"<<i<<std::endl;
		fBoardSettings[i].Print();
	}
}

