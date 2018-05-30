
/********************************************************************\

  Name:         fecaen.cxx
  Created by:   V. Bildstein
  Based on:     fevme.cxx by K.Olchanski

  Contents:     Frontend for the CAEN digitizers
$Id$
\********************************************************************/

#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/time.h>
#include <assert.h>
#include "midas.h"

#include "CaenDigitizer.hh"

/* make frontend functions callable from the C framework */
#ifdef __cplusplus
extern "C" {
#endif

/*-- Globals -------------------------------------------------------*/

/* The frontend name (client name) as seen by other MIDAS clients   */
   const char* frontend_name = "fecaen";
/* The frontend file name, don't change it */
   const char* frontend_file_name = __FILE__;

/* frontend_loop is called periodically if this variable is TRUE    */
   BOOL frontend_call_loop = FALSE;

/* a frontend status page is displayed with this frequency in ms */
   INT display_period = 000;

/* maximum event size produced by this frontend */
   INT max_event_size = 1000*1024;

/* maximum event size for fragmented events (EQ_FRAGMENTED) */
   INT max_event_size_frag = 1024*1024;

/* buffer size to hold events */
   INT event_buffer_size = 2000*1024;

  extern INT run_state;
  extern HNDLE hDB;

/*-- Function declarations -----------------------------------------*/
  INT frontend_init();
  INT frontend_exit();
  INT begin_of_run(INT run_number, char *error);
  INT end_of_run(INT run_number, char *error);
  INT pause_run(INT run_number, char *error);
  INT resume_run(INT run_number, char *error);
  INT frontend_loop();

  INT read_event(char *pevent, INT off);
/*-- Bank definitions ----------------------------------------------*/

/*-- Equipment list ------------------------------------------------*/

  EQUIPMENT equipment[] = {

    {"Trigger",               /* equipment name */
     {1, TRIGGER_ALL,         /* event ID, trigger mask */
      "SYSTEM",               /* event buffer */
      EQ_POLLED,              /* equipment type */
      LAM_SOURCE(0, 0xFFFFFF),                      /* event source */
      "MIDAS",                /* format */
      TRUE,                   /* enabled */
      RO_RUNNING,             /* read only when running */

      500,                    /* poll for 500ms */
      0,                      /* stop run after this event limit */
      0,                      /* number of sub events */
      0,                      /* don't log history */
      "", "", "",}
     ,
     read_event,      /* readout routine */
     NULL, NULL,
     NULL,       /* bank list */
    }
    ,

    {""}
  };

#ifdef __cplusplus
}
#endif
/********************************************************************\
              Callback routines for system transitions

  These routines are called whenever a system transition like start/
  stop of a run occurs. The routines are called on the following
  occations:

  frontend_init:  When the frontend program is started. This routine
                  should initialize the hardware.

  frontend_exit:  When the frontend program is shut down. Can be used
                  to releas any locked resources like memory, commu-
                  nications ports etc.

  begin_of_run:   When a new run is started. Clear scalers, open
                  rungates, etc.

  end_of_run:     Called on a request to stop a run. Can send
                  end-of-run event and close run gates.

  pause_run:      When a run is paused. Should disable trigger events.

  resume_run:     When a run is resumed. Should enable trigger events.
\********************************************************************/

CaenDigitizer* gDigitizer;

/*-- Frontend Init -------------------------------------------------*/

INT frontend_init()
{
  setbuf(stdout, NULL);
  setbuf(stderr, NULL);

  delete gDigitizer;
  gDigitizer = new CaenDigitizer(hDB);

  return SUCCESS;
}

/*-- Frontend Exit -------------------------------------------------*/

INT frontend_exit()
{
  delete gDigitizer;

  return SUCCESS;
}

/*-- Begin of Run --------------------------------------------------*/

INT begin_of_run(INT run_number, char *error)
{
  printf("begin run %d\n",run_number);

  gDigitizer->StartAcquisition(hDB);

  return SUCCESS;
}

/*-- End of Run ----------------------------------------------------*/
INT end_of_run(INT run_number, char *error)
{
	static bool gInsideEndRun = false;

	if(gInsideEndRun) {
		printf("breaking recursive end_of_run()\n");
		return SUCCESS;
	}

	gInsideEndRun = true;

	printf("end run %d\n",run_number);
	gDigitizer->StopAcquisition();

	gInsideEndRun = false;

	return SUCCESS;
}

/*-- Pause Run -----------------------------------------------------*/
INT pause_run(INT run_number, char *error)
{
	//doesn't do anything right now as the cards have no pause/resume
	return SUCCESS;
}

/*-- Resume Run ----------------------------------------------------*/
INT resume_run(INT run_number, char *error)
{
	//doesn't do anything right now as the cards have no pause/resume
	return SUCCESS;
}

/*-- Frontend Loop -------------------------------------------------*/
INT frontend_loop()
{
	/* if frontend_call_loop is true, this routine gets called when
		the frontend is idle or once between every event */
	return SUCCESS;
}

/********************************************************************\

  Readout routines for different events

  \********************************************************************/

/*-- Trigger event routines ----------------------------------------*/
extern "C" INT poll_event(INT source, INT count, BOOL test)
/* Polling routine for events. Returns TRUE if event
	is available. If test equals TRUE, don't return. The test
	flag is used to time the polling */
{
	return gDigitizer->DataReady();
}

/*-- Interrupt configuration ---------------------------------------*/
extern "C" INT interrupt_configure(INT cmd, INT source, PTYPE adr)
{
	switch (cmd) {
		case CMD_INTERRUPT_ENABLE:
			break;
		case CMD_INTERRUPT_DISABLE:
			break;
		case CMD_INTERRUPT_ATTACH:
			break;
		case CMD_INTERRUPT_DETACH:
			break;
	}
	return SUCCESS;
}

/*-- Event readout -------------------------------------------------*/

INT read_event(char *pevent, INT off)
{
	//printf("read event!\n");

	/* init bank structure */
	bk_init32(pevent);

	gDigitizer->ReadData(pevent, "CAEN");

	return bk_size(pevent);
}

