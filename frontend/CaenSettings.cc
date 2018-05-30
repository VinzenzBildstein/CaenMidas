#include "CaenSettings.hh"

#include <iostream>
#include <fstream>
#include <iomanip>
#include <curses.h>
#include <cstdlib>

#include "midas.h"

//#include "TEnv.h"

#include "CaenOdb.h"

CaenSettings::CaenSettings()
{
}

void CaenSettings::ReadOdb(HNDLE hDB)
{
	if(hDB == 0) {
		std::cerr<<"Can't read settings from ODB, handle provided is "<<hDB<<std::endl;
		return;
	}
	// read ODB settings
	//HNDLE hDB;
	HNDLE hSet;
	DT5730_SETTINGS settings;
	//std::cout<<"connecting to database ..."<<std::endl;
	//if(cm_get_experiment_database(&hDB, NULL) != CM_SUCCESS) {
	//	std::cout<<"failed to get database handle "<<hDB<<std::endl;
	//	throw;
	//}

	DT5730_SETTINGS_STR(dt5730_settings_str);
	if(db_create_record(hDB, 0, "/Equipment/DT5730/Settings", strcomb(dt5730_settings_str)) != DB_SUCCESS) {
		std::cout<<"failed to create record"<<std::endl;
		throw;
	}

	db_find_key(hDB, 0, "/Equipment/DT5730/Settings", &hSet);
	int size = sizeof(settings);
	if(db_get_record(hDB, hSet, &settings, &size, 0) != DB_SUCCESS) {
		std::cout<<"Error occured trying to read \"/Equipment/DT5730/Settings\""<<std::endl;
		throw;
	}

	fNumberOfBoards = settings.number_of_digitizers;
	if(fNumberOfBoards < 1) {
		std::cout<<fNumberOfBoards<<" boards is not possible!"<<std::endl;
		throw;
	}
	fNumberOfChannels = settings.channels_per_digitizer;
	if(fNumberOfChannels < 1) {
		std::cout<<fNumberOfChannels<<" maximum channels is not possible!"<<std::endl;
		throw;
	}
	fBufferSize = 100000;

	fLinkType.resize(fNumberOfBoards);
	fVmeBaseAddress.resize(fNumberOfBoards);
	fAcquisitionMode.resize(fNumberOfBoards);
	fIOLevel.resize(fNumberOfBoards);
	fChannelMask.resize(fNumberOfBoards);
	fRunSync.resize(fNumberOfBoards);
	fEventAggregation.resize(fNumberOfBoards);
	fTriggerMode.resize(fNumberOfBoards);
	fRecordLength.resize(fNumberOfBoards);
	fDCOffset.resize(fNumberOfBoards);
	fPreTrigger.resize(fNumberOfBoards);
	fPulsePolarity.resize(fNumberOfBoards);
	fEnableCfd.resize(fNumberOfBoards);
	fCfdParameters.resize(fNumberOfBoards);
	fChannelParameter.resize(fNumberOfBoards, new CAEN_DGTZ_DPP_PSD_Params_t);
	for(int i = 0; i < fNumberOfBoards; ++i) {
		fLinkType[i]         = static_cast<CAEN_DGTZ_ConnectionType>(settings.link_type[i]);
		fVmeBaseAddress[i]   = settings.vme_base_address[i];
		fAcquisitionMode[i]  = static_cast<CAEN_DGTZ_DPP_AcqMode_t>(settings.acquisition_mode[i]);
		fIOLevel[i]          = static_cast<CAEN_DGTZ_IOLevel_t>(settings.io_level[i]);
		fTriggerMode[i]      = static_cast<CAEN_DGTZ_TriggerMode_t>(settings.trigger_mode[i]);
		fChannelMask[i]      = settings.channel_mask[i];
		fRunSync[i]          = static_cast<CAEN_DGTZ_RunSyncMode_t>(settings.runsync_mode[i]);
		fEventAggregation[i] = settings.event_aggregation[i];

		fRecordLength[i].resize(fNumberOfChannels);
		fDCOffset[i].resize(fNumberOfChannels);
		fPreTrigger[i].resize(fNumberOfChannels);
		fPulsePolarity[i].resize(fNumberOfChannels);
		fEnableCfd[i].resize(fNumberOfChannels);
		fCfdParameters[i].resize(fNumberOfChannels);
		for(int ch = 0; ch < fNumberOfChannels; ++ch) {
			fRecordLength[i][ch]  = settings.record_length[i*fNumberOfChannels + ch];
			fDCOffset[i][ch]      = settings.dc_offset[i*fNumberOfChannels + ch];
			fPreTrigger[i][ch]    = settings.pre_trigger[i*fNumberOfChannels + ch];
			fPulsePolarity[i][ch] = static_cast<CAEN_DGTZ_PulsePolarity_t>(settings.pulse_polarity[i*fNumberOfChannels + ch]);
			fEnableCfd[i][ch]     = settings.enable_cfd[i*fNumberOfChannels + ch];
			fCfdParameters[i][ch] = (settings.cfd_delay[i*fNumberOfChannels + ch] & 0xff);
			fCfdParameters[i][ch] |= (settings.cfd_fraction[i*fNumberOfChannels + ch] & 0x3) << 8;
			fCfdParameters[i][ch] |= (settings.cfd_interpolation_points[i*fNumberOfChannels + ch] & 0x3) << 10;
		}

		fChannelParameter[i]->purh   = static_cast<CAEN_DGTZ_DPP_PUR_t>(settings.pile_up_rejection_mode[i]);
		fChannelParameter[i]->purgap = settings.pile_up_gap[i];
		fChannelParameter[i]->blthr  = settings.baseline_threshold[i];
		fChannelParameter[i]->bltmo  = settings.baseline_timeout[i];
		fChannelParameter[i]->trgho  = settings.trigger_holdoff[i];
		for(int ch = 0; ch < fNumberOfChannels; ++ch) {
			fChannelParameter[i]->thr[ch]   = settings.threshold[i*fNumberOfChannels + ch];
			fChannelParameter[i]->nsbl[ch]  = settings.baseline_samples[i*fNumberOfChannels + ch];
			fChannelParameter[i]->lgate[ch] = settings.long_gate[i*fNumberOfChannels + ch];
			fChannelParameter[i]->sgate[ch] = settings.short_gate[i*fNumberOfChannels + ch];
			fChannelParameter[i]->pgate[ch] = settings.pre_gate[i*fNumberOfChannels + ch];
			fChannelParameter[i]->selft[ch] = settings.self_trigger[i*fNumberOfChannels + ch];
			fChannelParameter[i]->trgc[ch]  = static_cast<CAEN_DGTZ_DPP_TriggerConfig_t>(settings.trigger_configuration[i*fNumberOfChannels + ch]);
			fChannelParameter[i]->tvaw[ch]  = settings.trigger_validation_window[i*fNumberOfChannels + ch];
			fChannelParameter[i]->csens[ch] = settings.charge_sensitivity[i*fNumberOfChannels + ch];
		}
	}
	fRawOutput = settings.raw_output;
	//Print();
}

//bool CaenSettings::ReadSettingsFile(const std::string& filename)
//{
//	auto settings = new TEnv(filename.c_str());
//	if(settings == NULL || settings->ReadFile(filename.c_str(), kEnvLocal) != 0) {
//		return false;
//	}
//
//	fNumberOfBoards = settings->GetValue("NumberOfBoards", 1);
//	if(fNumberOfBoards < 1) {
//		std::cout<<fNumberOfBoards<<" boards is not possible!"<<std::endl;
//		return false;
//	}
//	fNumberOfChannels = settings->GetValue("NumberOfChannels", 8);
//	if(fNumberOfChannels < 1) {
//		std::cout<<fNumberOfChannels<<" maximum channels is not possible!"<<std::endl;
//		return false;
//	}
//	fBufferSize = settings->GetValue("BufferSize", 100000);
//
//	fLinkType.resize(fNumberOfBoards);
//	fVmeBaseAddress.resize(fNumberOfBoards);
//	fAcquisitionMode.resize(fNumberOfBoards);
//	fIOLevel.resize(fNumberOfBoards);
//	fChannelMask.resize(fNumberOfBoards);
//	fRunSync.resize(fNumberOfBoards);
//	fEventAggregation.resize(fNumberOfBoards);
//	fTriggerMode.resize(fNumberOfBoards);
//	fRecordLength.resize(fNumberOfBoards);
//	fDCOffset.resize(fNumberOfBoards);
//	fPreTrigger.resize(fNumberOfBoards);
//	fPulsePolarity.resize(fNumberOfBoards);
//	fEnableCfd.resize(fNumberOfBoards);
//	fCfdParameters.resize(fNumberOfBoards);
//	fChannelParameter.resize(fNumberOfBoards, new CAEN_DGTZ_DPP_PSD_Params_t);
//	for(int i = 0; i < fNumberOfBoards; ++i) {
//		fLinkType[i]         = CAEN_DGTZ_USB;//0
//		fVmeBaseAddress[i]   = 0;
//		fAcquisitionMode[i]  = static_cast<CAEN_DGTZ_DPP_AcqMode_t>(settings->GetValue(Form("Board.%d.AcquisitionMode", i), CAEN_DGTZ_DPP_ACQ_MODE_Mixed));//2
//		fIOLevel[i]          = static_cast<CAEN_DGTZ_IOLevel_t>(settings->GetValue(Form("Board.%d.IOlevel", i), CAEN_DGTZ_IOLevel_NIM));//0
//		fChannelMask[i]      = settings->GetValue(Form("Board.%d.ChannelMask", i), 0xff);
//		fRunSync[i]          = static_cast<CAEN_DGTZ_RunSyncMode_t>(settings->GetValue(Form("Board.%d.RunSync", i), CAEN_DGTZ_RUN_SYNC_Disabled));//0
//		fEventAggregation[i] = settings->GetValue(Form("Board.%d.EventAggregate", i), 0);
//		fTriggerMode[i]      = static_cast<CAEN_DGTZ_TriggerMode_t>(settings->GetValue(Form("Board.%d.TriggerMode", i), CAEN_DGTZ_TRGMODE_ACQ_ONLY));//1
//
//		fRecordLength[i].resize(fNumberOfChannels);
//		fDCOffset[i].resize(fNumberOfChannels);
//		fPreTrigger[i].resize(fNumberOfChannels);
//		fPulsePolarity[i].resize(fNumberOfChannels);
//		fEnableCfd[i].resize(fNumberOfChannels);
//		fCfdParameters[i].resize(fNumberOfChannels);
//		for(int ch = 0; ch < fNumberOfChannels; ++ch) {
//			fRecordLength[i][ch]  = settings->GetValue(Form("Board.%d.Channel.%d.RecordLength", i, ch), 192);
//			fDCOffset[i][ch]      = settings->GetValue(Form("Board.%d.Channel.%d.RunSync", i, ch), 0x8000);
//			fPreTrigger[i][ch]    = settings->GetValue(Form("Board.%d.Channel.%d.RunSync", i, ch), 80);
//			fPulsePolarity[i][ch] = static_cast<CAEN_DGTZ_PulsePolarity_t>(settings->GetValue(Form("Board.%d.Channel.%d.PulsePolarity", i, ch), CAEN_DGTZ_PulsePolarityNegative));//1
//			fEnableCfd[i][ch]     = settings->GetValue(Form("Board.%d.Channel.%d.EnableCfd", i, ch), true);
//			fCfdParameters[i][ch] = (settings->GetValue(Form("Board.%d.Channel.%d.CfdDelay", i, ch), 5) & 0xff);
//			fCfdParameters[i][ch] |= (settings->GetValue(Form("Board.%d.Channel.%d.CfdFraction", i, ch), 0) & 0x3) << 8;
//			fCfdParameters[i][ch] |= (settings->GetValue(Form("Board.%d.Channel.%d.CfdInterpolationPoints", i, ch), 0) & 0x3) << 10;
//		}
//
//		fChannelParameter[i]->purh   = static_cast<CAEN_DGTZ_DPP_PUR_t>(settings->GetValue(Form("Board.%d.PileUpRejection", i), CAEN_DGTZ_DPP_PSD_PUR_DetectOnly));//0
//		fChannelParameter[i]->purgap = settings->GetValue(Form("Board.%d.PurityGap", i), 100);
//		fChannelParameter[i]->blthr  = settings->GetValue(Form("Board.%d.BaseLine.Threshold", i), 3);
//		fChannelParameter[i]->bltmo  = settings->GetValue(Form("Board.%d.BaseLine.Timeout", i), 100);
//		fChannelParameter[i]->trgho  = settings->GetValue(Form("Board.%d.TriggerHoldOff", i), 8);
//		for(int ch = 0; ch < fNumberOfChannels; ++ch) {
//			fChannelParameter[i]->thr[ch]   = settings->GetValue(Form("Board.%d.Channel.%d.Threshold", i, ch), 50);
//			fChannelParameter[i]->nsbl[ch]  = settings->GetValue(Form("Board.%d.Channel.%d.BaselineSamples", i, ch), 4);
//			fChannelParameter[i]->lgate[ch] = settings->GetValue(Form("Board.%d.Channel.%d.LongGate", i, ch), 32);
//			fChannelParameter[i]->sgate[ch] = settings->GetValue(Form("Board.%d.Channel.%d.ShortGate", i, ch), 24);
//			fChannelParameter[i]->pgate[ch] = settings->GetValue(Form("Board.%d.Channel.%d.PreGate", i, ch), 8);
//			fChannelParameter[i]->selft[ch] = settings->GetValue(Form("Board.%d.Channel.%d.SelfTrigger", i, ch), 1);
//			fChannelParameter[i]->trgc[ch]  = static_cast<CAEN_DGTZ_DPP_TriggerConfig_t>(settings->GetValue(Form("Board.%d.Channel.%d.TriggerConfiguration", i, ch), CAEN_DGTZ_DPP_TriggerConfig_Threshold));//1
//			fChannelParameter[i]->tvaw[ch]  = settings->GetValue(Form("Board.%d.Channel.%d.TriggerValidationAcquisitionWindow", i, ch), 50);
//			fChannelParameter[i]->csens[ch] = settings->GetValue(Form("Board.%d.Channel.%d.ChargeSensitivity", i, ch), 0);
//		}
//	}
//
//	return true;
//}

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

	script<<"odbedit -c \"set \\\"/Equipment/DT5730/Settings/Number of digitizers\\\" "<<fNumberOfBoards<<"\""<<std::endl;
	script<<"odbedit -c \"set \\\"/Equipment/DT5730/Settings/Channels per digitizer\\\" "<<fNumberOfChannels<<"\""<<std::endl;
	script<<"odbedit -c \"set \\\"/Equipment/DT5730/Settings/Raw output\\\" "<<fRawOutput<<"\""<<std::endl;
	for(int i = 0; i < fNumberOfBoards; ++i) {
		script<<"odbedit -c \"set \\\"/Equipment/DT5730/Settings/Link Type["<<i<<"]\\\" "<<fLinkType[i]<<"\""<<std::endl;
		script<<"odbedit -c \"set \\\"/Equipment/DT5730/Settings/VME base address["<<i<<"]\\\" "<<fVmeBaseAddress[i]<<"\""<<std::endl;
		script<<"odbedit -c \"set \\\"/Equipment/DT5730/Settings/Acquisition Mode["<<i<<"]\\\" "<<fAcquisitionMode[i]<<"\""<<std::endl;
		script<<"odbedit -c \"set \\\"/Equipment/DT5730/Settings/IO Level["<<i<<"]\\\" "<<fIOLevel[i]<<"\""<<std::endl;
		script<<"odbedit -c \"set \\\"/Equipment/DT5730/Settings/Trigger Mode["<<i<<"]\\\" "<<fTriggerMode[i]<<"\""<<std::endl;
		script<<"odbedit -c \"set \\\"/Equipment/DT5730/Settings/Channel Mask["<<i<<"]\\\" "<<fChannelMask[i]<<"\""<<std::endl;
		script<<"odbedit -c \"set \\\"/Equipment/DT5730/Settings/RunSync Mode["<<i<<"]\\\" "<<fRunSync[i]<<"\""<<std::endl;
		script<<"odbedit -c \"set \\\"/Equipment/DT5730/Settings/Event aggregation["<<i<<"]\\\" "<<fEventAggregation[i]<<"\""<<std::endl;

		for(int ch = 0; ch < fNumberOfChannels; ++ch) {
			script<<"odbedit -c \"set \\\"/Equipment/DT5730/Settings/Record length["<<i*fNumberOfChannels+ch<<"]\\\" "<<fRecordLength[i][ch]<<"\""<<std::endl;
			script<<"odbedit -c \"set \\\"/Equipment/DT5730/Settings/DC offset["<<i*fNumberOfChannels+ch<<"]\\\" "<<fDCOffset[i][ch]<<"\""<<std::endl;
			script<<"odbedit -c \"set \\\"/Equipment/DT5730/Settings/Pre trigger["<<i*fNumberOfChannels+ch<<"]\\\" "<<fPreTrigger[i][ch]<<"\""<<std::endl;
			script<<"odbedit -c \"set \\\"/Equipment/DT5730/Settings/Pulse polarity["<<i*fNumberOfChannels+ch<<"]\\\" "<<fPulsePolarity[i][ch]<<"\""<<std::endl;
			script<<"odbedit -c \"set \\\"/Equipment/DT5730/Settings/Enable CFD["<<i*fNumberOfChannels+ch<<"]\\\" "<<fEnableCfd[i][ch]<<"\""<<std::endl;
			script<<"odbedit -c \"set \\\"/Equipment/DT5730/Settings/CFD delay["<<i*fNumberOfChannels+ch<<"]\\\" "<<(fCfdParameters[i][ch] & 0xff)<<"\""<<std::endl;
			script<<"odbedit -c \"set \\\"/Equipment/DT5730/Settings/CFD fraction["<<i*fNumberOfChannels+ch<<"]\\\" "<<((fCfdParameters[i][ch]>>8) & 0x3)<<"\""<<std::endl;
			script<<"odbedit -c \"set \\\"/Equipment/DT5730/Settings/CFD interpolation points["<<i*fNumberOfChannels+ch<<"]\\\" "<<((fCfdParameters[i][ch]>>10) & 0x3)<<"\""<<std::endl;
		}

		script<<"odbedit -c \"set \\\"/Equipment/DT5730/Settings/Pile up rejection mode["<<i<<"]\\\" "<<fChannelParameter[i]->purh<<"\""<<std::endl;
		script<<"odbedit -c \"set \\\"/Equipment/DT5730/Settings/Pile up gap["<<i<<"]\\\" "<<fChannelParameter[i]->purgap<<"\""<<std::endl;
		script<<"odbedit -c \"set \\\"/Equipment/DT5730/Settings/Baseline threshold["<<i<<"]\\\" "<<fChannelParameter[i]->blthr<<"\""<<std::endl;
		script<<"odbedit -c \"set \\\"/Equipment/DT5730/Settings/Baseline timeout["<<i<<"]\\\" "<<fChannelParameter[i]->bltmo<<"\""<<std::endl;
		script<<"odbedit -c \"set \\\"/Equipment/DT5730/Settings/Trigger holdoff["<<i<<"]\\\" "<<fChannelParameter[i]->trgho<<"\""<<std::endl;
		for(int ch = 0; ch < fNumberOfChannels; ++ch) {
			script<<"odbedit -c \"set \\\"/Equipment/DT5730/Settings/Threshold["<<i*fNumberOfChannels+ch<<"]\\\" "<<fChannelParameter[i]->thr[ch]<<"\""<<std::endl;
			script<<"odbedit -c \"set \\\"/Equipment/DT5730/Settings/Baseline samples["<<i*fNumberOfChannels+ch<<"]\\\" "<<fChannelParameter[i]->nsbl[ch]<<"\""<<std::endl;
			script<<"odbedit -c \"set \\\"/Equipment/DT5730/Settings/Long gate["<<i*fNumberOfChannels+ch<<"]\\\" "<<fChannelParameter[i]->lgate[ch]<<"\""<<std::endl;
			script<<"odbedit -c \"set \\\"/Equipment/DT5730/Settings/Short gate["<<i*fNumberOfChannels+ch<<"]\\\" "<<fChannelParameter[i]->sgate[ch]<<"\""<<std::endl;
			script<<"odbedit -c \"set \\\"/Equipment/DT5730/Settings/Pre gate["<<i*fNumberOfChannels+ch<<"]\\\" "<<fChannelParameter[i]->pgate[ch]<<"\""<<std::endl;
			script<<"odbedit -c \"set \\\"/Equipment/DT5730/Settings/Self trigger["<<i*fNumberOfChannels+ch<<"]\\\" "<<fChannelParameter[i]->selft[ch]<<"\""<<std::endl;
			script<<"odbedit -c \"set \\\"/Equipment/DT5730/Settings/Trigger configuration["<<i*fNumberOfChannels+ch<<"]\\\" "<<fChannelParameter[i]->trgc[ch]<<"\""<<std::endl;
			script<<"odbedit -c \"set \\\"/Equipment/DT5730/Settings/Trigger validation window["<<i*fNumberOfChannels+ch<<"]\\\" "<<fChannelParameter[i]->tvaw[ch]<<"\""<<std::endl;
			script<<"odbedit -c \"set \\\"/Equipment/DT5730/Settings/Charge sensitivity["<<i*fNumberOfChannels+ch<<"]\\\" "<<fChannelParameter[i]->csens[ch]<<"\""<<std::endl;
		}
	}
	script<<std::endl;

	script.close();

	std::system("chmod +x writeSettings.sh; writeSettings.sh");

	return true;

	// write ODB settings
	HNDLE hDB;
	HNDLE hSet;
	DT5730_SETTINGS settings;

	std::cout<<"here ..."<<std::endl;
	// copy settings to DT5730_SETTINGS struct
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
	for(int i = 0; i < fNumberOfBoards; ++i) {
		settings.link_type[i] = fLinkType[i];
		settings.vme_base_address[i] = fVmeBaseAddress[i];
		settings.acquisition_mode[i] = fAcquisitionMode[i];
		settings.io_level[i] = fIOLevel[i];
		settings.trigger_mode[i] = fTriggerMode[i];
		settings.channel_mask[i] = fChannelMask[i];
		settings.runsync_mode[i] = fRunSync[i];
		settings.event_aggregation[i] = fEventAggregation[i];

		for(int ch = 0; ch < fNumberOfChannels; ++ch) {
			settings.record_length[i*fNumberOfChannels + ch] = fRecordLength[i][ch];
			settings.dc_offset[i*fNumberOfChannels + ch] = fDCOffset[i][ch];
			settings.pre_trigger[i*fNumberOfChannels + ch] = fPreTrigger[i][ch];
			settings.pulse_polarity[i*fNumberOfChannels + ch] = fPulsePolarity[i][ch];
			settings.enable_cfd[i*fNumberOfChannels + ch] = fEnableCfd[i][ch];
			settings.cfd_delay[i*fNumberOfChannels + ch] = fCfdParameters[i][ch] & 0xff;
			settings.cfd_fraction[i*fNumberOfChannels + ch] = (fCfdParameters[i][ch]>>8) & 0x3;
			settings.cfd_interpolation_points[i*fNumberOfChannels + ch] = (fCfdParameters[i][ch]>>10) & 0x3;
		}

		settings.pile_up_rejection_mode[i] = fChannelParameter[i]->purh;
		settings.pile_up_gap[i] = fChannelParameter[i]->purgap;
		settings.baseline_threshold[i] = fChannelParameter[i]->blthr;
		settings.baseline_timeout[i] = fChannelParameter[i]->bltmo;
		settings.trigger_holdoff[i] = fChannelParameter[i]->trgho;
		for(int ch = 0; ch < fNumberOfChannels; ++ch) {
			settings.threshold[i*fNumberOfChannels + ch] = fChannelParameter[i]->thr[ch];
			settings.baseline_samples[i*fNumberOfChannels + ch] = fChannelParameter[i]->nsbl[ch];
			settings.long_gate[i*fNumberOfChannels + ch] = fChannelParameter[i]->lgate[ch];
			settings.short_gate[i*fNumberOfChannels + ch] = fChannelParameter[i]->sgate[ch];
			settings.pre_gate[i*fNumberOfChannels + ch] = fChannelParameter[i]->pgate[ch];
			settings.self_trigger[i*fNumberOfChannels + ch] = fChannelParameter[i]->selft[ch];
			settings.trigger_configuration[i*fNumberOfChannels + ch] = fChannelParameter[i]->trgc[ch];
			settings.trigger_validation_window[i*fNumberOfChannels + ch] = fChannelParameter[i]->tvaw[ch];
			settings.charge_sensitivity[i*fNumberOfChannels + ch] = fChannelParameter[i]->csens[ch];
		}
	}
	settings.raw_output = fRawOutput;
	std::cout<<"connecting to database ..."<<std::endl;
	// connect to ODB
	if(cm_get_experiment_database(&hDB, NULL) != CM_SUCCESS) {
		std::cout<<"failed to get database handle "<<hDB<<std::endl;
		return false;
	}

	std::cout<<"creating record using handle "<<hDB<<" ..."<<std::endl;
	// create record (just in case it didn't exist already)
	DT5730_SETTINGS_STR(dt5730_settings_str);
	if(db_create_record(hDB, 0, "/Equipment/DT5730/Settings", strcomb(dt5730_settings_str)) != DB_SUCCESS) {
		std::cout<<"failed to create record"<<std::endl;
		return false;
	}

	std::cout<<"writing ..."<<std::endl;
	// write settings to ODB
	db_find_key(hDB, 0, "/Equipment/DT5730/Settings", &hSet);
	int size = sizeof(settings);
	if(db_set_record(hDB, hSet, &settings, size, 0) != DB_SUCCESS) {
		std::cout<<"Error occured trying to write \"/Equipment/DT5730/Settings\""<<std::endl;
		return false;
	}

	return true;
}

CaenSettings::~CaenSettings()
{
}

void CaenSettings::Print()
{
	std::cout<<fNumberOfBoards<<" boards with "<<fNumberOfChannels<<" channels:"<<std::endl;
	for(int i = 0; i < fNumberOfBoards; ++i) {
		std::cout<<"Board #"<<i<<":"<<std::endl;
		std::cout<<"  link type ";
		switch(fLinkType[i]) {
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
		std::cout<<"   VME base address 0x"<<std::hex<<fVmeBaseAddress[i]<<std::dec<<std::endl;
		std::cout<<"   acquisition mode ";
		switch(fAcquisitionMode[i]) {
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
		std::cout<<"   IO level ";
		switch(fIOLevel[i]) {
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
		std::cout<<"   channel mask 0x"<<std::hex<<fChannelMask[i]<<std::dec<<std::endl;
		std::cout<<"   run sync ";
		switch(fRunSync[i]) {
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
		std::cout<<"   event aggregation "<<fEventAggregation[i]<<std::endl;
		std::cout<<"   trigger mode "<<fTriggerMode[i]<<std::endl;
		for(int ch = 0; ch < fNumberOfChannels; ++ch) {
			std::cout<<"   Channel #"<<ch<<":"<<std::endl;
			std::cout<<"      record length "<<fRecordLength[i][ch]<<std::endl;
			std::cout<<"      DC offset 0x"<<std::hex<<fDCOffset[i][ch]<<std::dec<<std::endl;
			std::cout<<"      pre trigger "<<fPreTrigger[i][ch]<<std::endl;
			std::cout<<"      pulse polarity ";
			switch(fPulsePolarity[i][ch]) {
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
			if(fEnableCfd[i][ch]) {
				std::cout<<"      cfd enabled"<<std::endl;
				std::cout<<"      cfd parameters 0x"<<std::hex<<fCfdParameters[i][ch]<<std::dec<<std::endl;
			} else {
				std::cout<<"      cfd disabled"<<std::endl;
			}
		}
		std::cout<<"   pile-up rejection mode ";
		switch(fChannelParameter[i]->purh) {
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
		std::cout<<"   pile-up gap "<<fChannelParameter[i]->purgap<<std::endl;
		std::cout<<"   baseline threshold "<<fChannelParameter[i]->blthr<<std::endl;
		std::cout<<"   baseline timeout "<<fChannelParameter[i]->bltmo<<std::endl;
		std::cout<<"   trigger holdoff "<<fChannelParameter[i]->trgho<<std::endl;
		for(int ch = 0; ch < fNumberOfChannels; ++ch) {
			std::cout<<"   Channel #"<<ch<<":"<<std::endl;
			std::cout<<"      threshold "<<fChannelParameter[i]->thr[ch]<<std::endl;
			std::cout<<"      baseline samples "<<fChannelParameter[i]->nsbl[ch]<<std::endl;
			std::cout<<"      long gate "<<fChannelParameter[i]->lgate[ch]<<std::endl;
			std::cout<<"      short gate "<<fChannelParameter[i]->sgate[ch]<<std::endl;
			std::cout<<"      pre-gate "<<fChannelParameter[i]->pgate[ch]<<std::endl;
			std::cout<<"      self trigger "<<fChannelParameter[i]->selft[ch]<<std::endl;
			std::cout<<"      trigger conf. ";
			switch(fChannelParameter[i]->trgc[ch]) {
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
			std::cout<<"      trigger val. window "<<fChannelParameter[i]->tvaw[ch]<<std::endl;
			std::cout<<"      charge sensitivity "<<fChannelParameter[i]->csens[ch]<<std::endl;
		}
	}

	//std::vector<CAEN_DGTZ_DPP_PSD_Params_t*> fChannelParameter;
}

