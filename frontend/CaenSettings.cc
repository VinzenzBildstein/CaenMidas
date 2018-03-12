#include "CaenSettings.hh"

#include <iostream>
#include <iomanip>
#include <curses.h>

#include "midas.h"

#include "CaenOdb.h"

CaenSettings::CaenSettings()
{
	ReadOdb();
}

void CaenSettings::ReadOdb()
{
	// read ODB settings
	HNDLE hDB;
	HNDLE hSet;
	DT5730_COMMON settings;
	cm_get_experiment_database(&hDB, nullptr);
	DT5730_COMMON_STR(dt5730_common_str);

	db_create_record(hDB, 0, "/Equipment/DT5730/Common", strcomb(dt5730_common_str));
	db_find_key(hDB, 0, "/Equipment/DT5730/Common", &hSet);
	int size = sizeof(settings);
	if(db_get_record(hDB, hSet, &settings, &size, 0) != DB_SUCCESS) {
		std::cout<<"Error occured trying to read \"/Equipment/DT5730/Common\""<<std::endl;
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
	Print();
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

