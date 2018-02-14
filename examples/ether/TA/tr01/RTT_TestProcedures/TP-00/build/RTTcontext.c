/** @file
/////////////////////////////////////////////////////////////////////
//
// Verified Systems International GmbH
// Am Fallturm 1 
// D-28359 Bremen
// Germany
// Tel.  : +49 421 57204 0
// Fax   : +49 421 57204 22
// e-mail: info@verified.de
//
//--------------------------------------------------------------------
//
// (C) Copyright Verified Systems International GmbH 
//     $Date: 2017-01-25 16:47:07 $
//
//--------------------------------------------------------------------
//
// Product: RT-Tester template for test configuration / initialisation
//
//--------------------------------------------------------------------
//
// File Identification: 
//
// $RCSfile: RTTcontext.c,v $
//
// $Header: /home/repository/VVTCVS/CVS/rtt-swi/tpl/RTTcontext.c,v 1.69.2.4 2017-01-25 16:47:07 oli Exp $
//
// $Revision: 1.69.2.4 $
//
// First edition by: Marcus Dahlweid
// Last update by    $Author: oli $
//
//--------------------------------------------------------------------
//
// Description: 
//   Implementation of default context for unit tests
//   RTT 6.x STL Preprocessor, parsing .tpl .conf .rts files
//
//--------------------------------------------------------------------
// @TABLE OF CONTENTS:                 [TOCD: 20:24 11 Mär 2009]
//
//      [0.1] global variables
//  [1] Global Variable Declarations for signal handling
//  [2] global functions
//      [2.1] Stub test integration context init
//      [2.2] AM_CONTEXT: INIT and FINIT
//      [2.3] LWP_CONTEXT: INIT and FINIT; rtt_checkForNodeTermination
//      [2.4] CNODE_CONTEXT: INIT and FINIT
//      [2.5] CHANNEL: INIT
//  [3] Standard Format
//  [4] compute test verdict
//--------------------------------------------------------------------*/


#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#include "RTTcontext.h"
#include "RTTcontext_private.h"
#include "RTTverdict.h"
#include "RTTstubs_local.h"
#include "RTTstubs_global.h"

#include <stdlib.h>
#include <stdio.h>
#include "rttExternDecl.h"
#include "rttModelVarDecl.h"
#include "rttInterface.h"
#include "rttSignalChecker.h"


// ///////////////////////////////////////////////
// [0.1] global variables
// ///////////////////////////////////////////////

/** @var  RTTam_context_t AM_context[RTT_CONFIG_NUM_AM];
 *  A global variable containing the AM configuration of all activates AMs of
 *  a test case.
 */
volatile RTTam_context_t   AM_context[RTT_CONFIG_NUM_AM];

/** @var RTTlwp_context_t  LWP_context[RTT_CONFIG_NUM_LWP];
 *  A global variable containing th LWP configuration of a test case. 
 */
volatile RTTlwp_context_t  LWP_context[RTT_CONFIG_NUM_LWP];

/** @var RTTcnode_context_t  CNODE_context[RTT_CONFIG_NUM_CNODE];
 *  A global variable containing th CNODE configuration of a test case. 
 */
#ifndef _CADUL
volatile RTTcnode_context_t  CNODE_context[RTT_CONFIG_NUM_CNODE];
#else
volatile RTTcnode_context_t  CNODE_context[1];
#endif

#ifdef RTT_CLUSTER_SOE
// local table for monitoring of last closed cycle for local AMs
volatile ccetsTables_t timeStampTable;
#endif

/** @var RTTglobal_setup_t RTTgs;
 *  A global variable containing the global setup of a test case.
 */
volatile RTTglobal_setup_t RTTgs;


/** @var rtt_uctx_t rtt_lwp_uctx[RTT_CONFIG_NUM_LWP][RTT_CONFIG_NUM_AM];
 *  A global array of user space context structures for the AMs of each LWP.
 */
rtt_uctx_t rtt_lwp_uctx[RTT_CONFIG_NUM_LWP][RTT_CONFIG_NUM_AM];

/** @var rtt_scheduler_uctx[RTT_CONFIG_NUM_LWP];
 *  A global array of user space context structures of the scheduler process
 *  for each LWP.
 */
rtt_uctx_t rtt_scheduler_uctx[RTT_CONFIG_NUM_LWP];

/** @var int32_t rttContext_rttTestRunning;
 *  The global test status Flag. Possible values are:
 *  0 = test case not running (not startetd or terminated)
 *  1 = test case is initialising
 *  2 = test running
 */
volatile int32_t rttContext_rttTestRunning  = 0;

/** @var uint64_t rttStartOfTestTic;
 *  The global timetick at which the test did start.
 */
volatile uint64_t rttStartOfTestTic;

/** @var rttStartOfTestEpoch
 *  The global time epoch used for the time tick calculation.
 */
volatile time_t rttStartOfTestEpoch;

/** @var uint64_t rttTestDurationSec;
 *  A max test duration boundary (This value is set only if the executeble of
 *  the test case is called with the duration time as its argument. If the
 *  script rtt-run-test is used, this variable is not used and the test
 *  duration is checked by a seperate watchdog process.
 */
uint64_t rttTestDurationSec;

/** @var rttMajorFrameTick
 *  Logical time stamp (rttMajorFrameTick,rttSubframeTick)
 *  consisting of the actual major frame number.
 *
 *  The rttMajorFrameTick is zero at start of test
 *  and is incremented every cycletime time units,
 *  where cycletime is the smalles cycle time defined
 *  for any of the LWPs. The rttSubframeTick is set
 *  to zero at start of test and reset at every increment
 *  of rttMajorFrameTick. The rttSubframeTick is set to the actual
 *  sub-frame number whenever an abstract machine is 
 *  activated at a scheduled sub-frame.
 */
volatile extern uint32_t rttMajorFrameTick;

/** @var rttSubframeTick
 *  See explanations given for
 *  rttMajorFrameTick.
 */
volatile extern uint32_t rttSubframeTick;


/** @var uint32_t rtt_config_num_am;
 *  A global variables storing the number of activated AMs. It is set at
 *  runtime according to the value of RTT_CONFIG_NUM_AM.
 */
uint32_t rtt_config_num_am = RTT_CONFIG_NUM_AM;

/** @var uint32_t rtt_config_num_lwp;
 *  A global variables storing the number of LWPs of a test case. It is set at
 *  runtime according to the value of RTT_CONFIG_NUM_LWP.
 */
uint32_t rtt_config_num_lwp = RTT_CONFIG_NUM_LWP;

/** @var uint32_t rtt_config_num_cnode; A global variables storing the number
 *  of CNODEs of a test case. It is set at runtime according to the value of
 *  RTT_CONFIG_NUM_CNODE.
 */
uint32_t rtt_config_num_cnode = RTT_CONFIG_NUM_CNODE;

/** @var rttCnlSpec_t *cnlPtr[RTT_CONFIG_NUM_LWP][RTT_CONFIG_NUM_CHANNEL_FILES+1]
 *  This array contains the channel specification structures of all channel
 *  files.
 */
rttCnlSpec_t  *cnlPtr[RTT_CONFIG_NUM_LWP][RTT_CONFIG_NUM_CHANNEL_FILES+1];

/** @var const char *rttAllUsedChannels[RTT_CONFIG_NUM_LWP+1][RTT_CONFIG_NUM_CHANNEL_PER_LWP+1];
 *  This global variable holds a list of all channels stored in each LWP.
 */
const char *rttAllUsedChannels[RTT_CONFIG_NUM_LWP+1][RTT_CONFIG_NUM_CHANNEL_PER_LWP+1] = 
{};


/** @var rttVecSpec_t *vecPtr[RTT_CONFIG_NUM_LWP][RTT_CONFIG_NUM_CHANNEL_FILES+1]
 *  This array contains the vector specification structures of all vector
 *  files.
 */
// todo-vec: 
rttVecSpec_t  *vecPtr[RTT_CONFIG_NUM_LWP][RTT_CONFIG_NUM_VECTORS+1] =
{};

// {};



/** @var rtt_ti_level_t rttTiLevel;
 *  The global test integration level of a test case.
 */
rtt_ti_level_t rttTiLevel = rtt_ti_level_undef;

/** @var long int rttWarnings[RTT_CONFIG_NUM_AM];
 *  A global variables contain an array with the number of warnings caused by
 *  each AM during the test execution.
 */
long int rttWarnings[RTT_CONFIG_NUM_AM];

/** @var long int rttFailures[RTT_CONFIG_NUM_AM];
 *  A global variables contain an array with the number of errors caused by
 *  each AM during the test execution.
 */
long int rttFailures[RTT_CONFIG_NUM_AM];


/** @var uint32_t rtt_SUT_am_id
 *  Re-Introduced this variable for backward compatibility reasons:
 *   @li available to all stubs (i.e. generated before 1.6.0, and then
 *       modified)
 *   @li allocated in the RTTcontext
 *   @li contains the number of the AM marked as SUT, and 0 if no such SUT
 *       is declared.
 * For tests based on version 1.6.0 or later, this global variable has no effect
 */
uint32_t rtt_SUT_am_id = 0;

/** @var unique ids for channel specs.
 */
int32_t rtt_max_channel_id = 0;

/** @var maximum size of channel data
 */
uint32_t rtt_max_channel_size = 0;

/** @var local cluster node id 
 */
uint32_t rtt_local_cnode_id = 0;

/** @var local cluster node timing offset 
 */
uint64_t rtt_local_cnode_offset = 0L;

/** @var rtt_scratchpad_signal_size
 * This size must be big enough to accomodate the committing of any signal
 * to the test log. 
 * The output starts with a headline.
 * Since a signal consisist of max. RTTSIG_MAX_SIG_SIZE bytes, the numeric 
 * interpretation printing one byte as "0xXX " + line breaks
 * can be over-estimated by factor           6
 */
const uint32_t rtt_scratchpad_signal_size = 
#if RTT_CONFIG_HAVE_SIGNALSET
                                            256 + (RTTSIG_MAX_SIG_SIZE)*6;
#else
                                            512; /* A small scratch pad to be
						    able to print error
						    messages */
#endif

/** @var rtt_rttselect_semantics
 *  Defines the configured @rttSelect() and @rttSelectX() semantics.
 */
const rttctx_rttselect_semantics_t rtt_rttselect_semantics
= (rttctx_rttselect_semantics_t)(RTTCTX_RTTSELECT_PROFILE_LEGACY_6_0_4_8_2);

// ///////////////////////////////////////////////////////////////////
// [1] Global Variable Declarations for signal handling
// ///////////////////////////////////////////////////////////////////

#ifdef _LINUX_DEBUG
const bool_t rttHaveDebugMode = TRUE;
// Additional disable CFLAGS
#  define _DISABLE_SIGINT_HANDLER_
#  define _DISABLE_SIGTERM_HANDLER_
#  define _DISABLE_SIGSEGV_HANDLER_
#else
const bool_t rttHaveDebugMode = FALSE;
#endif

/** Install signal handler for SIGHUP */
#ifdef _DISABLE_SIGHUP_HANDLER_
const bool_t rttIgnoreSignal_SIGHUP = TRUE;
#else
const bool_t rttIgnoreSignal_SIGHUP = FALSE;
#endif

/** Install signal handler for SIGINT */
#ifdef _DISABLE_SIGINT_HANDLER_
const bool_t rttIgnoreSignal_SIGINT = TRUE;
#else
const bool_t rttIgnoreSignal_SIGINT = FALSE;
#endif

/** Install signal handler for SIGQUIT */
#ifdef _DISABLE_SIGQUIT_HANDLER_
const bool_t rttIgnoreSignal_SIGQUIT = TRUE;
#else
const bool_t rttIgnoreSignal_SIGQUIT = FALSE;
#endif

/** Install signal handler for SIGILL */
#ifdef _DISABLE_SIGILL_HANDLER_
const bool_t rttIgnoreSignal_SIGILL = TRUE;
#else
const bool_t rttIgnoreSignal_SIGILL = FALSE;
#endif

/** Install signal handler for SIGABRT */
#ifdef _DISABLE_SIGABRT_HANDLER_
const bool_t rttIgnoreSignal_SIGABRT = TRUE;
#else
const bool_t rttIgnoreSignal_SIGABRT = FALSE;
#endif

/** Install signal handler for SIGFPE */
#ifdef _DISABLE_SIGFPE_HANDLER_
const bool_t rttIgnoreSignal_SIGFPE = TRUE;
#else
const bool_t rttIgnoreSignal_SIGFPE = FALSE;
#endif

/** Install signal handler for SIGSEGV */
#ifdef _DISABLE_SIGSEGV_HANDLER_
const bool_t rttIgnoreSignal_SIGSEGV = TRUE;
#else
const bool_t rttIgnoreSignal_SIGSEGV = FALSE;
#endif

/** Install signal handler for SIGPIPE */
#ifdef _DISABLE_SIGPIPE_HANDLER_
const bool_t rttIgnoreSignal_SIGPIPE = TRUE;
#else
const bool_t rttIgnoreSignal_SIGPIPE = FALSE;
#endif

/** Install signal handler for SIGALRM */
#ifdef _DISABLE_SIGALRM_HANDLER_
const bool_t rttIgnoreSignal_SIGALRM = TRUE;
#else
const bool_t rttIgnoreSignal_SIGALRM = FALSE;
#endif

/** Install signal handler for SIGTERM */
#ifdef _DISABLE_SIGTERM_HANDLER_
const bool_t rttIgnoreSignal_SIGTERM = TRUE;
#else
const bool_t rttIgnoreSignal_SIGTERM = FALSE;
#endif

/** Install signal handler for SIGUSR1 */
#ifdef _DISABLE_SIGUSR1_HANDLER_
const bool_t rttIgnoreSignal_SIGUSR1 = TRUE;
#else
const bool_t rttIgnoreSignal_SIGUSR1 = FALSE;
#endif

/** Install signal handler for SIGUSR2 */
#ifdef _DISABLE_SIGUSR2_HANDLER_
const bool_t rttIgnoreSignal_SIGUSR2 = TRUE;
#else
const bool_t rttIgnoreSignal_SIGUSR2 = FALSE;
#endif

/** Install signal handler for SIGCHLD */
#ifdef _DISABLE_SIGCHLD_HANDLER_
const bool_t rttIgnoreSignal_SIGCHLD = TRUE;
#else
const bool_t rttIgnoreSignal_SIGCHLD = FALSE;
#endif

/** Install signal handler for SIGCONT */
#ifdef _DISABLE_SIGCONT_HANDLER_
const bool_t rttIgnoreSignal_SIGCONT = TRUE;
#else
const bool_t rttIgnoreSignal_SIGCONT = FALSE;
#endif

/** Install signal handler for SIGSTOP */
#ifdef _DISABLE_SIGSTOP_HANDLER_
const bool_t rttIgnoreSignal_SIGSTOP = TRUE;
#else
const bool_t rttIgnoreSignal_SIGSTOP = FALSE;
#endif

/** Install signal handler for SIGTSTP */
#ifdef _DISABLE_SIGTSTP_HANDLER_
const bool_t rttIgnoreSignal_SIGTSTP = TRUE;
#else
const bool_t rttIgnoreSignal_SIGTSTP = FALSE;
#endif

/** Install signal handler for SIGTTIN */
#ifdef _DISABLE_SIGTTIN_HANDLER_
const bool_t rttIgnoreSignal_SIGTTIN = TRUE;
#else
const bool_t rttIgnoreSignal_SIGTTIN = FALSE;
#endif

/** Install signal handler for SIGTTOU */
#ifdef _DISABLE_SIGTTOU_HANDLER_
const bool_t rttIgnoreSignal_SIGTTOU = TRUE;
#else
const bool_t rttIgnoreSignal_SIGTTOU = FALSE;
#endif


// ///////////////////////////////////////////////////////////////////
// [2] global functions
// ///////////////////////////////////////////////////////////////////

// ///////////////////////////////////////////////
// [2.1] Stub test integration context init
// ///////////////////////////////////////////////

/**
 * Function stublevel_context_INIT.
 * Initialisation of stub test integration levels
 */

void stublevel_context_INIT() {

// automatically created initialisation code for stub TI level


// automatically created stub initialisation calls


}

/**
 * Function stublevel_context_FINIT.
 * Finalisation of stub test integration levels
 */

void stublevel_context_FINIT() {

// cleanup allocated stub ports


}

// ///////////////////////////////////////////////
// [2.2] AM_CONTEXT: INIT and FINIT
// ///////////////////////////////////////////////

/**
 * Function am_context_INIT.
 * Initialisation of AM context.
 */
void am_context_INIT(){
  //FILE*    fp;
  time_t   startup_time_epoch;
  int32_t  am_counter;

  /** derive global setup */
  RTTgs.stop_on_fail           = FALSE;

  RTTgs.timers_in_microseconds = (bool_t)RTT_TIMERS_IN_MICROSECONDS;
  RTTgs.use_relative_time      = (bool_t)RTT_USE_RELATIVE_TIME;
  RTTgs.use_local_time         = (bool_t)RTT_USE_LOCAL_TIME;

  time(&startup_time_epoch);
  RTTgs.start_time_epoch = startup_time_epoch;
  RTTgs.start_time       = rttGetTimeTick(RTT_TIMERS_IN_MICROSECONDS  ? 
					  rttTimeScale_micro_seconds  :
					  rttTimeScale_milli_seconds
					  );
#if (defined RTT_LOGSIG_TO_SOCKET)
#ifdef    RTT_BLOCK_SIGNAL_SOCKET
  RTTgs.rtt_sig_socket_ipaddr = NULL;
  RTTgs.rtt_sig_socket_port   = 0;
#else
  RTTgs.rtt_sig_socket_ipaddr = "127.0.0.1";
  RTTgs.rtt_sig_socket_port   = 20020;
#endif /* RTT_BLOCK_SIGNAL_SOCKET */
#endif /* RTT_LOGSIG_TO_SOCKET */

  RTTgs.fmi2_tags_output_stream = stdout;

#ifdef RTT_CLUSTER  
  RTTgs.cluster_mode =  (rtt_config_num_cnode > 0);
#else
  RTTgs.cluster_mode =  FALSE;
#endif

/** @todo: move this depenency to the config file */ 
#ifndef XXX_RTT_CONTEXT_STUB_DEFAULT_PRINTOFF_YYY
#define XXX_RTT_CONTEXT_STUB_DEFAULT_PRINTOFF_YYY FALSE
#endif

/** 
 * Lower limit on scratchpad size (in order to avoid a configuration pitfall)
 *
 * @todo: move this adjustment to the config/.rtp file 
 */ 
#ifndef XXX_RTT_CONTEXT_MIN_SCRATCHPAD_SIZE_YYY
#define XXX_RTT_CONTEXT_MIN_SCRATCHPAD_SIZE_YYY  40960
#endif


  RTTgs.stub_default_printOff = XXX_RTT_CONTEXT_STUB_DEFAULT_PRINTOFF_YYY;

  // -----------------------------------------------------------------------

  /** initialize the one AM for unit tests */

  am_counter = 0;

  // -----------------------------------------------------------------------
  // Context settings for debugging / additional output, that may be set 
  // via CFLAGS within a testbed or test config file.
  // -----------------------------------------------------------------------
#ifdef RTTSIG_WARNINGS
  RTTgs.siglib_warnings = RTTSIG_WARNINGS;
#else
  RTTgs.siglib_warnings = 0;
#endif  


#ifdef RTT_CLUSTER_SOE
  // initialize the local table for monitoring of last closed cycle for local AMs
  memset(&timeStampTable, 0, sizeof(timeStampTable));
#endif

  // automatically created initialisation code for AMs
  // -- NOTE: following pattern required to be sole entry in line
rttctx_initialize_AM(am_counter 
                    , "am_timetick" 
                    , rttAmType_AbstractMachine
                    , 0 
                    , RTT_CONTEXT_NUM_MAX(40960, XXX_RTT_CONTEXT_MIN_SCRATCHPAD_SIZE_YYY )
                    , RTT_CTXLIB_STDOUT_MASK | RTT_CTXLIB_FILE_MASK | 0 | 0 | 0
                    , "0" 
                    , 0
                    , -1
                    , 10000000
                    , 0
                    , FALSE
                    , 23
                    , RTT_TAG_TRACE_UNDEFINED
                    , ""
                    );

AM_context[am_counter].init_function    = test_am_timetick_INIT;
AM_context[am_counter].process_function = test_am_timetick_PROCESS;
AM_context[am_counter].finit_function   = test_am_timetick_FINIT;
am_counter++; 
 // --------------------------------------------------
rttctx_initialize_AM(am_counter 
                    , "am_stimulator" 
                    , rttAmType_AbstractMachine
                    , 0 
                    , RTT_CONTEXT_NUM_MAX(40960, XXX_RTT_CONTEXT_MIN_SCRATCHPAD_SIZE_YYY )
                    , RTT_CTXLIB_STDOUT_MASK | RTT_CTXLIB_FILE_MASK | 0 | 0 | 0
                    , "0" 
                    , 0
                    , -1
                    , 10000000
                    , 0
                    , FALSE
                    , 23
                    , RTT_TAG_TRACE_UNDEFINED
                    , ""
                    );

AM_context[am_counter].init_function    = test_am_stimulator_INIT;
AM_context[am_counter].process_function = test_am_stimulator_PROCESS;
AM_context[am_counter].finit_function   = test_am_stimulator_FINIT;
am_counter++; 
 // --------------------------------------------------
rttctx_initialize_AM(am_counter 
                    , "am__ora_End1I" 
                    , rttAmType_AbstractMachine
                    , 0 
                    , RTT_CONTEXT_NUM_MAX(40960, XXX_RTT_CONTEXT_MIN_SCRATCHPAD_SIZE_YYY )
                    , 0 | RTT_CTXLIB_FILE_MASK | 0 | 0 | 0
                    , "0" 
                    , 0
                    , -1
                    , 10000000
                    , 0
                    , FALSE
                    , 23
                    , RTT_TAG_TRACE_UNDEFINED
                    , ""
                    );

AM_context[am_counter].init_function    = test_am__ora_End1I_INIT;
AM_context[am_counter].process_function = test_am__ora_End1I_PROCESS;
AM_context[am_counter].finit_function   = test_am__ora_End1I_FINIT;
am_counter++; 
 // --------------------------------------------------
rttctx_initialize_AM(am_counter 
                    , "am__ora_End2I" 
                    , rttAmType_AbstractMachine
                    , 0 
                    , RTT_CONTEXT_NUM_MAX(40960, XXX_RTT_CONTEXT_MIN_SCRATCHPAD_SIZE_YYY )
                    , 0 | RTT_CTXLIB_FILE_MASK | 0 | 0 | 0
                    , "0" 
                    , 0
                    , -1
                    , 10000000
                    , 0
                    , FALSE
                    , 23
                    , RTT_TAG_TRACE_UNDEFINED
                    , ""
                    );

AM_context[am_counter].init_function    = test_am__ora_End2I_INIT;
AM_context[am_counter].process_function = test_am__ora_End2I_PROCESS;
AM_context[am_counter].finit_function   = test_am__ora_End2I_FINIT;
am_counter++; 
 // --------------------------------------------------
rttctx_initialize_AM(am_counter 
                    , "am__ora_EtherI" 
                    , rttAmType_AbstractMachine
                    , 0 
                    , RTT_CONTEXT_NUM_MAX(40960, XXX_RTT_CONTEXT_MIN_SCRATCHPAD_SIZE_YYY )
                    , 0 | RTT_CTXLIB_FILE_MASK | 0 | 0 | 0
                    , "0" 
                    , 0
                    , -1
                    , 10000000
                    , 0
                    , FALSE
                    , 23
                    , RTT_TAG_TRACE_UNDEFINED
                    , ""
                    );

AM_context[am_counter].init_function    = test_am__ora_EtherI_INIT;
AM_context[am_counter].process_function = test_am__ora_EtherI_PROCESS;
AM_context[am_counter].finit_function   = test_am__ora_EtherI_FINIT;
am_counter++; 
 // --------------------------------------------------

  // --

}

/**
 * Function am_context_FINIT.
 * Finalisation of AM context.
 */

void am_context_FINIT(){
#ifdef RTT_MEMORY_DEALLOCATION
    uint64_t i;
    for(i=0; i<RTT_CONFIG_NUM_AM; i++) {
	if (AM_context[i].used) {
	    rttctx_finalize_AM(i);
	}
    }
#endif
}

// ///////////////////////////////////////////////
// [2.3] LWP_CONTEXT: INIT and FINIT; rtt_checkForNodeTermination
// ///////////////////////////////////////////////

/**
 * Function lwp_context_INIT.
 * Initialisation of LWP context.
 */

void lwp_context_INIT(){
    int32_t lwp_counter = 0;

    // automatically created initialisation code for LWPs
    // -- NOTE: following pattern required to be sole entry in line
#if (! defined _LINUX_DEBUG)

rttctx_initialize_LWP(lwp_counter
                      , "lwp"
                      , RTT_CONFIG_GLOBAL_CYCLETIME
                      , 0
                      , 5
                      , "am_timetick am_stimulator am__ora_End1I am__ora_End2I am__ora_EtherI"
                      , "-1 -1 -1 -1 -1"
                      , "0 0 0 0 0"
                      , "0 0 0 0 0"
                      , FALSE);
lwp_counter++;
     

#else /* defined _LINUX_DEBUG */

/* default lwp */
rttctx_initialize_LWP(lwp_counter
                      , "lwp_1"
                      , RTT_CONFIG_GLOBAL_CYCLETIME
                      , 0
                      , 5
                      , "am_timetick am_stimulator am__ora_End1I am__ora_End2I am__ora_EtherI"
                      , "-1"
                      , "0"
                      , "0"
                      , FALSE);
lwp_counter++;


#endif /* defined _LINUX_DEBUG */


    // -- 

}

/**
 * Function lwp_context_FINIT.
 * Finalisation of LWP context.
 */

void lwp_context_FINIT(){
    int i;
    for(i=0; i<RTT_CONFIG_NUM_LWP; i++) {
	rttctx_finalize_LWP(i);
    }
}

/**
 * Determine, whether the current cluster node has any LWP with at least one AM
 *
 * If the count drops to 0, then the test is terminated by
 * setting rttContext_rttTestRunning = 0
 */
void rtt_checkForNodeTermination(){
  int idx;

  for(idx = 0; idx < RTT_CONFIG_NUM_LWP; idx++){
    if( ! LWP_context[idx].lwp_terminated){
      return;
    }
  }
#if (RTT_CONFIG_NUM_CNODE > 1)
  fprintf(stderr, "** No LWP active on this node. Terminating Test.\n");
#elif (RTT_CONFIG_NUM_LWP > 1)
  fprintf(stderr, "** No active LWP left. Terminating Test.\n");
#else
  fprintf(stderr, "** All AMs have terminated. Terminating Test.\n");
#endif

  rttContext_rttTestRunning = 0;
}


// ///////////////////////////////////////////////
// [2.4] CNODE_CONTEXT: INIT and FINIT
// ///////////////////////////////////////////////

/**
 * Function cnode_context_INIT.
 * Initialisation of CNODE context.
 */

void cnode_context_INIT(){
    int32_t cnode_counter = 0L;

    // automatically created initialisation code for CNODEs
    // -- NOTE: following pattern required to be sole entry in line
// NOTE: generation of this code was re-written in 6.0-4.6.1 [scan_conf.y:2072]

    // -- 

    if(cnode_counter < 0L){ // -- this is to avoid compiler warnings --
      return;
    }
}


/**
 * Function cnode_context_FINIT.
 * Finalisation of CNODE context.
 */

void cnode_context_FINIT(){
    int i;
    for(i=0; i<RTT_CONFIG_NUM_CNODE; i++) {
	rttctx_finalize_CNODE(i);
    }
}


// ///////////////////////////////////////////////
// [2.5] CHANNEL: INIT
// ///////////////////////////////////////////////

/**
 * Function channel_context_init_LWP. Initialisation of channel data
 * structures specific for LWP with id lwp_id.
 * 
 * @param lwp_id internal id of the LWP, which whose structures are
 *               initialised.
 */

void channel_context_init_LWP(uint32_t lwp_id) {

    const char   *cnl = rttAllUsedChannels[lwp_id][0];
    rttCnlSpec_t *cnlSpec;
    uint32_t      cnlCnt = 0;

    while ( cnl ) {

	if ( (cnlSpec = rttGetCnlSpec(cnl,lwp_id)) ) {

	    /* assign unique channel od */
	    if (cnlSpec->global_cnl_id == -1) {
#if (VERBOSE_LEVEL > 0)
		printf("# assigning id %d to channel %s.\n",
		       rtt_max_channel_id,cnl);
#endif
		cnlSpec->global_cnl_id = rtt_max_channel_id;
		rtt_max_channel_id++;
	    }

            /* allocate the RB allocation table (one slot per LWP) */
	    if (cnlSpec->rbTablePtr == NULL) {
		cnlSpec->rbTablePtr 
		    = (rttRb_t **)calloc(RTT_CONFIG_NUM_LWP,sizeof(rttRb_t *));
	    }
	    
	    /* allocate one ring buffer structure for this LWP */
	    cnlSpec->rbTablePtr[lwp_id] 
		= (rttRb_t *)calloc(1,sizeof(rttRb_t));
	    
	    /* allocate data buffer for this LWP's ring buffer */
	    cnlSpec->rbTablePtr[lwp_id]->b
		= calloc(cnlSpec->cnlBufferSize,cnlSpec->cnlMsgSize);
	    
	    if(NULL == cnlSpec->rbTablePtr[lwp_id]->b){
	      fprintf(stderr, "WARNING: LWP %d: allocating memory for channel '%s' failed!\n"
		      "  I/O access to this channel from this LWP will fail.\n"
		      "  Please consider reducing the channel buffer size in file '%s' (currently: %lu).\n"
		      , (int)lwp_id, cnlSpec->cnlName, cnlSpec->cnlFile, (long unsigned int)cnlSpec->cnlBufferSize);
	    }

	    /* initialise write index -*/
	    (cnlSpec->rbTablePtr[lwp_id])->wIdx = 0;
	    
	    /* insert number of buffer entries into RB data structure */
	    (cnlSpec->rbTablePtr[lwp_id])->bufferSize 
		= cnlSpec->cnlBufferSize;
	    
	    /* record associated LWP id */
	    /** @todo internal/external LWP id handling */
	    (cnlSpec->rbTablePtr[lwp_id])->lwpId = lwp_id;

	    /* record largest channel for cluster communication */
	    if (rtt_max_channel_size < cnlSpec->cnlMsgSize)
		rtt_max_channel_size = cnlSpec->cnlMsgSize;
	}

	cnl = rttAllUsedChannels[lwp_id][++cnlCnt];
    }
}

/**
 * Function channel_context_INIT.
 * Initialisation of channel context.
 *
 * @note The channel context is also created and initialised
 *       for LWPs which do not run on the local cluster node.
 *       This is because data written by an AM on LWP l, cluster node n,
 *       is sent to other cluster nodes m via cluster communication
 *       and placed there into the channel ring buffer associated again
 *       with LWP l.
 */

void channel_context_INIT(){
    uint32_t lwp_index = 0;
    uint32_t channel_index = 0;

    // automatically created initialisation code for channel entry
    // -- NOTE: following pattern required to be sole entry in line

    // -- 

    for(lwp_index = 0; lwp_index < RTT_CONFIG_NUM_LWP; lwp_index++) {
	channel_context_init_LWP(lwp_index);
    }

    if(lwp_index < channel_index){ // -- avoid compiler warnings --
      return;
    }
}

/**
 * Function channel_context_FINIT.
 * Finalisation of channel context.
 */

void channel_context_FINIT(){
    const char   *cnl = NULL;
    rttCnlSpec_t *cnlSpec;
    uint32_t      cnlCnt = 0;
    uint32_t      lwp_index = 0;

    for(lwp_index = 0; lwp_index < RTT_CONFIG_NUM_LWP; lwp_index++) {
	cnl = rttAllUsedChannels[lwp_index][0];
	cnlCnt = 0;
	while ( cnl ) {
	    
	    if ( (cnlSpec = rttGetCnlSpec(cnl,lwp_index)) ) {
		if (cnlSpec->rbTablePtr[lwp_index] != NULL) {

		    if (cnlSpec->rbTablePtr[lwp_index]->b != NULL) {
			free(cnlSpec->rbTablePtr[lwp_index]->b);
			cnlSpec->rbTablePtr[lwp_index]->b = NULL;
		    }

		    free(cnlSpec->rbTablePtr[lwp_index]);
		    cnlSpec->rbTablePtr[lwp_index] = NULL;
		}
	    }

	    cnl = rttAllUsedChannels[lwp_index][++cnlCnt];
	}
    }

    for(lwp_index = 0; lwp_index < RTT_CONFIG_NUM_LWP; lwp_index++) {
	cnl = rttAllUsedChannels[lwp_index][0];
	cnlCnt = 0;
	while ( cnl ) {
	    
	    if ( (cnlSpec = rttGetCnlSpec(cnl,lwp_index)) ) {
		if (cnlSpec->rbTablePtr != NULL) {
		    free(cnlSpec->rbTablePtr);
		    cnlSpec->rbTablePtr = NULL;
		}
	    }

	    cnl = rttAllUsedChannels[lwp_index][++cnlCnt];
	}
    }
}


/**
 * Function rttGetCnlSpec. Retrieves the channel specification
 * setup for a given channel name on an LWP.
 *
 * @param cnl Name of the channel
 * @param lwp_id The number of the LWP.
 *
 * @TODO missing documentation
 * @return
 */
rttCnlSpec_t *rttGetCnlSpec(const char *cnl, uint32_t lwp_id) {

#if (RTT_CONFIG_NUM_CHANNEL_FILES > 0)

    uint32_t cnlFileCnt;
    rttCnlSpec_t *cPtr;
    
    for ( cnlFileCnt = 0; 
	  cnlFileCnt < RTT_CONFIG_NUM_CHANNEL_FILES; 
	  cnlFileCnt++ ) {

	cPtr = cnlPtr[lwp_id][cnlFileCnt];
        while ( cPtr && cPtr->cnlName ) {

	    if ( strcmp(cPtr->cnlName,cnl) == 0 ) {
		return cPtr;
	    }
	    cPtr++;
	}
    }
#endif

    fprintf(stderr, "Could not find channel setup for %s [lwp %ld].\n",cnl,(long int)lwp_id);
    exit(1);
    return NULL;
}


/**
 * Function rttLogReplayGetCnlSpec. Retrieves the channel specification
 * setup for a given channel name on an LWP.  Special version of rttGetCnlSpec
 * that does not terminate with exit() if channel is not found.
 *
 * @param cnl Name of the channel
 *
 * @TODO missing documentation
 * @return
 */
rttCnlSpec_t *rttLogReplayGetCnlSpec(const char *cnlName) {

#if (RTT_CONFIG_NUM_CHANNEL_FILES > 0)
    uint32_t lwp_id;
    uint32_t cnlFileCnt;
    rttCnlSpec_t *cPtr;

    for (lwp_id = 0; lwp_id < RTT_CONFIG_NUM_LWP; lwp_id++) {
	for (cnlFileCnt = 0; cnlFileCnt < RTT_CONFIG_NUM_CHANNEL_FILES; cnlFileCnt++) {
	    cPtr = cnlPtr[lwp_id][cnlFileCnt];
	    while (cPtr && cPtr->cnlName) {
		if (strcmp(cPtr->cnlName, cnlName) == 0) {
		    return cPtr;
		}
		cPtr++;
	    }
	}
    }
#endif

    return NULL;
}



/**
 * Function rttPruneReadIndex. Remove duplicate read index array
 * initialisation. All ports of a channel of an abstract machine
 * should use the same read index.
 *
 * @param am_id Internal number of an abstract machine.
 */
void rttPruneReadIndex(uint32_t am_id) 
{
    int numIdPorts = 0;
    int port_idx;
    
    while (AM_context[am_id].input_ports[numIdPorts] != NULL) {
	// Start comparing input_port[numIdPorts] with input_port[numIdPorts+1] 
	port_idx = numIdPorts+1;
	while (AM_context[am_id].input_ports[port_idx] != NULL) {
	    if (AM_context[am_id].input_ports[numIdPorts]->cnlSpec
		== AM_context[am_id].input_ports[port_idx]->cnlSpec) {
		// Both ports are using the same channel
		if (AM_context[am_id].input_ports[numIdPorts]->rIdxPtr
		    !=  AM_context[am_id].input_ports[port_idx]->rIdxPtr) {
		    // Point to the same ringbuffer
		    free(AM_context[am_id].input_ports[port_idx]->rIdxPtr);
		    free(AM_context[am_id].input_ports[port_idx]->readTimeTicPtr);
		    
		    AM_context[am_id].input_ports[port_idx]->rIdxPtr
			= AM_context[am_id].input_ports[numIdPorts]->rIdxPtr;
		    AM_context[am_id].input_ports[port_idx]->readTimeTicPtr
			= AM_context[am_id].input_ports[numIdPorts]->readTimeTicPtr;
		}
	    }
	    port_idx++;
	}
	numIdPorts++;
    }
}

/**
 * Function rttFreeReadIndex. Since read indices may be used by
 * multiple ports, the allocated memory must not be freed for each
 * port, but only once. This function frees the allocated memory once
 * and sets all referenced the rIdxPtr to NULL.
 *
 * @param am_id Internal number of an abstract machine.
 * @param rIdxPtr Read index pointer
 */
void rttFreeReadIndex(uint32_t am_id, uint32_t *rIdxPtr) 
{
    int numIdPorts = 0;
    int isFound = 0;
    
    if (rIdxPtr == NULL) return;
    
    while (AM_context[am_id].input_ports[numIdPorts] != NULL) {
	if (AM_context[am_id].input_ports[numIdPorts]->rIdxPtr == rIdxPtr) {
	    if (!isFound) {
		free (AM_context[am_id].input_ports[numIdPorts]->rIdxPtr);
		free (AM_context[am_id].input_ports[numIdPorts]->readTimeTicPtr);
		isFound = 1;
	    }
	    
	    AM_context[am_id].input_ports[numIdPorts]->rIdxPtr = NULL;
	    AM_context[am_id].input_ports[numIdPorts]->readTimeTicPtr = NULL;
	}
	numIdPorts++;
    }
}

void vector_context_INIT(){



}


void vector_context_FINIT(){

// @todo ?

}




/**
 * Function rttAllWarnings. 
 * Returns the sum of all warnings of all defined abstract machines.
 * Warning: this function is not thread safe.
 * @return the sum of all warnings of all defined abstract machines
 */
long int rttAllWarnings() {
    int amid;
    long int retval = 0;
    for (amid=0; amid<RTT_CONFIG_NUM_AM; amid++) {
	retval += rttWarnings[amid];
    }
    return retval;
}

/**
 * Function rttAllFailures. 
 * Returns the sum of all failures of all defined abstract machines.
 * Warning: this function is not thread safe.
 * @return the sum of all failures of all defined abstract machines
 */
long int rttAllFailures() {
    int amid;
    long int retval = 0;
    for (amid=0; amid<RTT_CONFIG_NUM_AM; amid++) {
	retval += rttFailures[amid];
    }
    return retval;
}

// ///////////////////////////////////////////////////////////////////
// [3] Standard Format
// ///////////////////////////////////////////////////////////////////

/**
 * Function rttStandardFormat. Standard logging function for unit test AMs.
 * 
 * @param amId       internal number of the abstract machine
 * @param eventFlag  condition/event/action flag
 * @param string     text string to be logged
 */
void rttStandardFormat(unsigned long amId, 
		       unsigned long eventFlag, 
		       char* string){
  rttLogFormat(amId, eventFlag, rttEventType_output, string);
}

/**
 * Function rttStandardLogAM. Standard logging function for unit test AMs:
 * Always prints scratchpad (with safe terminating zero) with standard flags
 * 
 * @param amId       internal number of the abstract machine
 */
void rttStandardLogAM(unsigned long amId){
  rttLogAM(amId, rttEventType_normal_output);
}

// ///////////////////////////////////////////////////////////////////
// [4] compute test verdict
// ///////////////////////////////////////////////////////////////////


/**
 * Create a test verdict according to context informations.
 * 
 * @param amNumber  internal number of the abstract machine
 * @param nWarnings number of warnings (fails of  @rttCheck)
 * @param nErrors   number of errors   (fails of  @rttAssert)
 */
void rttAMContext_compute_verdict(unsigned long int amNumber, 
				  unsigned long int nWarnings, 
				  unsigned long int nErrors){
  if(     0L != nErrors){
    rttctx_set_verdict_AM(amNumber, RTTverdict_fail, NULL);
  }
}

/** dummy value for unreachable return statement (after exit) */
static rtt_uctx_t rtt_uctx_empty_val;

rtt_uctx_t rtt_get_lwp_uctx(uint32_t lwp_id, uint32_t am_id) {

    if ( lwp_id < RTT_CONFIG_NUM_LWP && am_id < RTT_CONFIG_NUM_AM )
	return rtt_lwp_uctx[lwp_id][am_id];
    else {

	fprintf(stderr, "Trouble in rtt_get_lwp_uctx() - exit.\n");
	exit(1);
	{ // -- unreachable: this is to avoid complier warnings --
          return rtt_uctx_empty_val;
	}
    }

}


rtt_uctx_t *rtt_get_lwp_uctxPtr(uint32_t lwp_id, uint32_t am_id) {

    if ( lwp_id < RTT_CONFIG_NUM_LWP && am_id < RTT_CONFIG_NUM_AM )
	return (rtt_uctx_t *)&(rtt_lwp_uctx[lwp_id][am_id]);
    else {

	fprintf(stderr, "Trouble in rtt_get_lwp_uctx() - exit.\n");
	exit(1);
	// -- unreachable: this is to avoid complier warnings --
	return NULL;
    }
}




/*//////////////////////////////////////////////////////////////
//
// Change History:
//
// $Log: not supported by cvs2svn $
// Revision 1.69.2.3  2017/01/10 16:07:52  oli
// - included support for writing signal data to a UDP socket (configured via SIGNAL_SOCKET)
// - added python utility rtt_live_sigplot.py for visualisation
//
// Revision 1.69.2.2  2016/02/26 13:17:09  oli
// tag output at test @FINIT:
//     - per default, write to the stream at RTTgs.fmi2_tags_output_stream
//       (per default <stdout> - which may be manipulated after startup)
//     - only if RTT_FMI_TAGS2RTTINTERFACE_LOG_TAGS is defined, call
//       void fmi2rttInterface_log_tags(const char* line)
//     - set DEBUG_RTT_FMI_TAGS2STDERR to redirect to <stderr> in any case
//
// Revision 1.69.2.1  2015/11/25 16:27:12  oli
// initialise var
//
// Revision 1.69  2015/06/09 09:26:44  oli
// implement upper limit on channel buffer size (RTTCNLLIB_MAX_BUFFER_SIZE)
//    also added warning on startup (RTTcontext.c) if allocating memory fails
//    (implements #5539, see ~c23905)
//
// Revision 1.68  2015/02/20 12:24:38  hjficker
// Fix for Mantis-8077: Create a small signal scratchpad even if signals
// are not available, to be able to print error messages
//
// Revision 1.67  2014/04/14 15:33:04  emm
// signal library:
//     * if @s is called on a signal not subscribed locally, it does not
//       longer return arbitrary stack values, but returns 0.
//     * new debugging feature for signal access:
//         CFLAGS ; "-DRTTSIG_WARNINGS=0x00000001"
//       activates signal access warnings (in .conf file, or project.rtp)
//       If this is activated, access to signals which are not subscribed
//       locally will trigger a warning in the test log. This holds
//       for @s, @sigGet*, @sigWaitForVal, @sigWaitForAllVal.
//
//       RTTSIG_WARNINGS is designed as a bit vector which will allow to
//       activate various warnings in the future.
//
// Revision 1.66  2012/10/31 08:49:11  emm
// fix log in comment of previous change, since it produced
// warnings at compile time (comment within comment)
//
// Revision 1.65  2012/10/29 13:01:01  oli
// re-organised TRUE/FALSE definition:
// - attempt to correct faulty definitions
//   (FALSE must always be 0, TRUE != 0)
// - re-arranged src/ *.c template files, such that RTTcontext.h is included
//   before other rtt* header files
//   (this allows for true/false corrections)
//   Note that this does not affect the *.rts file compilation.
//
// Revision 1.64  2012/04/26 15:35:04  oli
// type refectoring
//  changed field type for string contants from 'char*' to 'const char*'
//  this is more robust and avoids several compile warnings concerning
//  the generated code (of stubs/channel files)
// Note: this changes RT-Tester internal types rttCnlSpec_t, rttParmSpec_t
//       in rttcnllib.h
//
// Revision 1.63  2012/02/16 17:11:36  oli
// adjusted company postal address (to Am_Fallturm_1)
//
// Revision 1.62  2011/03/03 09:46:25  bisanz
// Merged all modifications of of branch rscholz_newbuildsystem into the main
// trunk.
// Since it has been ensured that in turn the branch already contains the
// modifications of the trunk, only the differences between the heads of the main
// trunk (sync__pre__2011-03-03__2__trunk--from-rscholz_newbuildsystem) and the
// branch (sync__2011-03-03__2__rscholz_newbuildsystem--to--trunk) are merged.
// The updated main trunk is going to be tagged as:
//   sync__2011-03-03__2__trunk--from-rscholz_newbuildsystem
//
// Revision 1.59.2.1  2010/11/24 11:34:28  bisanz
// Merged modifications of the main trunk:
//   rscholz_build_20100427 --> sync__2010-11-24__trunk--to--rscholz_newbuildsystem
// into branch:
//   rscholz_newbuildsystem
// The updated branch is going to be tagged as:
//   sync__2010-11-24__rscholz_newbuildsystem--from--trunk
//
// Revision 1.61  2010/09/21 17:04:08  oli
// implemented semantic change:
// * an AM without PASS/FAIL has not verdict "NOT TESTED"
// * if all AMs have verdict "NOT TESTED", the test procedure has verdict "INCONC"
// This is easier to motivate and allows the operation @rttSetVerict(INCONC) to have the effect of rendering a test-procedure INCONC (if not FAIL/TESTERROR for other reasons)
//
// Revision 1.60  2010/09/17 15:17:43  oli
// Changes in VERDICT-Handling
// - the verdict of an AM with no pass/fail/testerror is  INCONC
// - a testprocedure is INCONC, if all the associated AMs are INCONC
// - the initial "state" of a test procedure is "NOT TESTED", which is
//   now properly added to the PASS/FAIL/INCOC/TESTERROR diagram
// See: (rt-tester/417)
//
// Revision 1.59  2010/03/03 19:30:38  bisanz
// Contains now a constant rtt_rttselect_semantics which provides what is
// configured via new configuration option RTTSELECT_SEMANTICS.
// This contributes to the fix wrt. rt-tester/202, rt-tester/414.
//
// Revision 1.58  2010/02/24 21:33:49  oli
// Signal extension:
// - for log entries, do not use the usual scratchpad but
//   instead a special (relatively small) reserved amount of
//   scratchpad memory.
// - Added configuration variable rtt_scratchpad_signal_size
// - This resolves conflicts with RTTL macros that use
//   signal extension functions inside arguments (fixes rt-tester/611)
//
// Revision 1.57  2009/05/07 18:10:22  bisanz
// According to PR rt-tester/264, some interface function signatures have been
// changed to use parameters of type "const char *" instead of "char *". This
// should be perfectly compatible with all using code, since the additional
// "const" is an additional requirement for the function itself which guarantees
// that the memory pointed to by the parameter will not be modified.
// Additionally, rttThrow() and rttCatch() handle the "exception data" now as
// const void * (had been void *) in the same fashion. Here, rttCatch() actually
// returns const void *, which means that the using code must not modify the
// memory. This _is_ an additional requirement to the using code, but modifying
// the data of a caught exception is real bad design, and it is not very likely
// that existing test procedures do so. Otherwise, of course, they must be
// adjusted.
//
// Revision 1.56  2009/03/13 11:54:29  bisanz
// In addition to RTTcontext.h, there is RTTcontext_private.h now; the latter
// shall contain declarations that users shall not use directly.
// This corresponds to previous changes on RTTcontext.h and RTTcontext.c, which
// had attempted to remove the
//   #define`s RTT_CONFIG_NUM_CHANNEL_FILES,
//             RTT_CONFIG_NUM_CHANNEL_PER_LWP
// from RTTcontext.h in order to have less re-compilation dependencies for stub
// compilation:
//   Both are always updated on addition/removal of stub declarations within
//   some .stubs file; and each stub dependes on RTTcontext.h; therefore always
//   _all_ stubs had to be re-compiled.
//
// Further definitions may be moved in the same fashion, if more dependencies
// shall be removed.
//
// Note that "power users" may include RTTcontext_private.h anyway; all
// definitions here stay available for them.
//
// Revision 1.55  2009/03/12 14:09:05  bisanz
// temporarily reverted last change; RTT_CONFIG_NUM_CHANNEL* have to be
// accessible;
// @todo: probably add seperate header file
//
// Revision 1.54  2009/03/11 21:21:46  bisanz
// Changed: #define`s RTT_CONFIG_NUM_CHANNEL_FILES,
//                    RTT_CONFIG_NUM_CHANNEL_PER_LWP
//          have been moved to RTTcontext.c (was RTTcontext.h before);
//   this is necessary to minimise dependencies for stub compilation.
//   Both are always updated on addition/removal of stub declarations within
//   some .stubs file; and each stub dependes on RTTcontext.h; therefore always
//   _all_ stubs had to be re-compiled.
//   Since a.m. #define`s are only used within RTTcontext.c, this dependency is
//   removed in this way.
//
// Revision 1.53  2009/03/06 15:59:48  tatiana
// modified structure of timeStampTable
//
// Revision 1.52  2009/02/05 18:05:43  tatiana
// use for timeStampTable double buffering
//
// Revision 1.51  2009/01/08 13:39:34  tatiana
// added time stamp table for SOE cluster
//
// Revision 1.50  2008/07/03 14:02:02  oli
// added: if license server answers ARC then an 'archive version marker' head the test log
// added: the tool version is printed to each test log
//
// Revision 1.49  2008/05/29 16:08:34  oli
// Logging: extended distinction of test log class markers:
// 	B C D E F I L N M P R S T U V W X O
// see rtt-swi-baseline-information.txt for details
//
// rtt-get-verdict: use new class marker "R" in order to recognise the results
//   section also in wrapped-around output files
//   (improves e.g. unit-tests/test3)
//
// Revision 1.48  2008/04/28 07:47:08  oli
// changed message
//   "** No LWP active on this node. Terminating Test."
// to be more situation-depended, e.g.:
//   "** All AMs have terminated. Terminating Test.\n"
// (for test procedures with just one LWP)
//
// Revision 1.47  2007/04/26 14:13:26  oli
// added conditional compilation (RTT_CONFIG_NUM_CHANNEL_FILES >0)
// in order to avoid compiler warnings for surplus comparisons
//
// Revision 1.46  2007/04/26 08:32:16  oli
// removed type-depended initialisation of dummy variable rtt_uctx_empty_val
//
// Revision 1.45  2007/04/26 07:00:03  oli
// added (silly, but harmless) code in order to avoid Cadul compiler warnings
//
// Revision 1.44  2007/03/16 13:09:50  oli
// fixed (jkrueger, l.2746): subframe timing: use context->cycletime, not cycletime
//
// changed:
// - LWPs no longer terminate, if the last AM is gone
//   (some AM may be resumed later)
// - instead, if no AM is running on one cluster node, the test terminates
//   (this is neccessary, since many 1-LWP tests do not use @rttStopTest)
//   see: RTTcontext.c:rtt_checkForNodeTermination()
//   (yes, it is ugly to put it here, but ctxlib does not maintain the list of
//   LWPs dynamically)
// - handleResumeAmCmd will be ineffective, if the AM is still running;
//   if it is suspended on the same LWP, then the remaining suspend-time is set to 0
// - the field .am_suspended is used to denote 'indefinite suspension', i.d.,
//   handleSuspendAmCmd(numSuspendCycles=0) or handleKillAmCmd()
//
// Revision 1.43  2006/10/13 15:33:32  chref
// adjustments necessary for dynamic AM handling:
// - always have 512 entries in AM_context
// - used-flag in AM_context entries (in order to find free entries)
// - always have 512 entries in am_id_list, am_start_list and am_prio_list
// - keep "volatile" attribute when passing pointers to LWP_context entries
//   to functions
//
// Revision 1.42  2006/04/19 10:29:57  oli
// Introduced (adjustable) minimum scratchpad size,
// default is 40 KB.
// Currently this can be set with the CFLAG
// "-DXXX_RTT_CONTEXT_MIN_SCRATCHPAD_SIZE_YYY=<NUM>"
// This should migrate into the .rtp file setup.
//
// Revision 1.41  2005/12/09 18:49:31  walsen
// - support suspend/resume with MX
// - support channels > 32KB with MX
//
// Revision 1.40  2005/11/09 15:54:44  dahlweid
// - Added variables rttIgnoreSignal_<signal>, which are initialised from
//   RTTcontext.c and allow to disable signal handlers for the specified signals
//   with compile switches for each test procedure.
// - Fixed C++ compile problem of rttHaveDebugMode.
//
// Revision 1.39  2005/11/03 09:15:39  dahlweid
// Fix for PR: rt-tester/76
// Added an additional iteration over all vectors of a .vec file in
// rtt-gen-test. This requires a new #define RTT_CONFIG_NUM_VECTORS in
// RTTcontext.c/.h, which keeps the number of used vectors instead of the number
// of vector files.
//
// Revision 1.38  2005/10/07 13:40:56  chref
// added logreplay library implementing the capture/replay feature
//
// Revision 1.37  2005/09/21 14:47:25  chref
// marked global variables that get changed in different threads as "volatile"
//
// Revision 1.36  2005/08/17 18:00:25  jp
// RT-Tester User Threads, first version
//
// Revision 1.35  2005/08/12 15:55:02  walsen
// further cad-ul adjustments
//
// Revision 1.34  2005/06/09 12:35:29  oli
// restored head revision status (RTT_REV_6_0_REL_3_2_7), after
// release RTT_REV_6_0_REL_3_2_6 has been constructed ex posterior:
// snapshot date: 2005-05-18, with removal of vector dependencies from RTTcontext.c
//                [tagged 2005-06-09]
//
// Revision 1.31  2005/05/27 13:52:55  oli
// added: VECTOR setup
//
// Revision 1.30  2005/05/20 18:22:53  jp
// First changes in conf parser for vectors
//
// Revision 1.29  2005/05/16 13:55:07  jp
// Define empty vector initialisation functions
//
// Revision 1.28  2005/05/16 13:45:46  jp
// Typos corrected and comments changed
//
// Revision 1.27  2005/04/17 21:12:19  jp
// Moved code from RTTschedule.c and RTTcontext.c templates
// to ctx-lib. This reduces the amount of code which has to
// be re-compiled for every test. Moreover, we prefer to hide
// our scheduling algorithms inside the ctx-lib, instead of showing
// the code in the templates.
//
// Revision 1.26  2005/04/08 08:46:28  jp
// Multi-threaded cluster nodes - first extensions
//
// Revision 1.25  2004/10/18 09:36:48  oli
// introduced XXX_RTT_CONTEXT_STUB_DEFAULT_PRINTOFF_YYY for stub initialisation
//   (printOff field of ports are initialised by this via the global variable
//     RTTgs.stub_default_printOff
//    )
// this is preparing a later replacement via a  *.conf field and already
// operative, if you compile with
// CFLAGS ; "-DXXX_RTT_CONTEXT_STUB_DEFAULT_PRINTOFF_YYY=TRUE"
//
// Revision 1.24  2004/09/08 08:57:29  oli
// fixed field name "stub_default_LogOff" to "stub_default_printOff"
//       (in accordance with port field .printOff)
//
// Revision 1.23  2004/09/08 08:38:05  oli
// - added global bool field RTTgs.stub_default_LogOff
//   to be able to switch on/off stub logging in one
//   place
//
// Revision 1.22  2004/08/04 14:53:21  marlex
// functions comments inserted/changed to coding standard
// for more information (and TODO) see in TODO_for_coding_standard.txt
//
// Revision 1.21  2004/08/02 14:43:01  marlex
// header added/changed for coding standard in *.l, *.y, *.c, *.h files
//
//
//
///////////////////////////////////////////////////////////////
*/
