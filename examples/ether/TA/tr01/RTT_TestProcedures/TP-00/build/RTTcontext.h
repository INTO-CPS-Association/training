/** @file
/////////////////////////////////////////////////////////////////////
//
// Verified Systems International GmbH
// Am Fallturm 1 
// D-28359 Bremen
// Germany
// Tel.  : +49 421 57204-0
// Fax   : +49 421 57204-22
// e-mail: info@verified.de
//
//--------------------------------------------------------------------
//
// (C) Copyright Verified Systems International GmbH 
//     $Date: 2015-08-28 15:07:25 $
//
//--------------------------------------------------------------------
//
// Product: RT-Tester template for test configuration / initialisation
//
//--------------------------------------------------------------------
//
// File Identification: 
//
// $RCSfile: RTTcontext.h,v $
//
// $Header: /home/repository/VVTCVS/CVS/rtt-swi/tpl/RTTcontext.h,v 1.58 2015-08-28 15:07:25 oli Exp $
//
// $Revision: 1.58 $
//
// First edition by: Markus Dahlweid
// Last update by    $Author: oli $
//
//--------------------------------------------------------------------
//
// Description: 
//   Default contex for unit tests
//   RTT 6.x STL Preprocessor, parsing .tpl .conf .rts files
//
// -------------------------------------------------------------------
// @TABLE OF CONTENTS:		       [TOCD: 09:50 19 Apr 2006]
//
//      [0.1] MACROS
//      [0.2] DEFINITIONS
//      [0.3] global variables
//  [1] Global Variable Declarations for signal handling
//  [2] Global Function Declarations
//      [2.1] Service Functions
//      [2.2] CNODE, LWP and AM context functions
//      [2.3] AM context functions
//      [2.4] LWP context functions
//      [2.5] CNODE context functions
//      [2.6] channel context functions
//      [2.7] Service Functions
// -------------------------------------------------------------------*/


#ifndef _RTTCONTEXT_H
#define _RTTCONTEXT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "rtt_test_config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>


// -- INCLUDES FROM THE PROJECT CONFIGURATION FILES ------------------------
#include <stdlib.h>
#include <stdio.h>
#include "rttExternDecl.h"
#include "rttModelVarDecl.h"
#include "rttInterface.h"
#include "rttSignalChecker.h"


#ifdef __cplusplus
}
#endif

// -- INCLUDES FROM THE CHANNEL DECLARATIONS -------------------------------


// -- INCLUDES FROM THE VECTOR DECLARATIONS --------------------------------



// -- CPP INCLUDES FROM THE PROJECT CONFIGURATION FILES --------------------


#ifdef __cplusplus
extern "C" {
#endif

// -- RTT specific includes ------------------------------------------------
#include "rtt_test_version.h"
#include "rtt_test_types.h"
#include "RTTverdict.h"

#include "rttstrlib.h"
#include "rttcnllib.h"
#include "rttisetlib.h"
#include "rttxserlib.h"
#include "rttctxlib.h"
#include "rttservicelib.h"
#include "rttbooleancontractlib.h"

#include "rtt_prj_time_scale.h"

#ifdef __cplusplus
}
#endif

// -- USER-DEFINED PRINT FUNCTION SIGNATURES -------------------------------
#ifndef RTT_NOINC_USER_PRINTFUNS
#include "RTT_user_printfuns.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

// ///////////////////////////////////////////////
// [0.1] MACROS
// ///////////////////////////////////////////////

/** Numerical Maximum of 2 numbers */
#define RTT_CONTEXT_NUM_MAX(a,b)  ((a) > (b) ? (a) : (b))

/** Numerical Minimum of 2 numbers */
#define RTT_CONTEXT_NUM_MIN(a,b)  ((a) < (b) ? (a) : (b))

// ///////////////////////////////////////////////
// [0.2] DEFINITIONS
// ///////////////////////////////////////////////

/**
 * The following defined are substituded by test case specific values during
 * the test case generation.
 */

/** @def RTT_CONST_TESTCASE_AUTHOR_STRING 
  * The author of the test case. 
  * The value of this define is created during the test case generation. */
#define RTT_CONST_TESTCASE_AUTHOR_STRING    "intocps"

/** @def RTT_CONST_TESTCASE_COMPONENT_STRING 
  * The name of the component, that is tested.  
  * The value of this define is created during the test case generation. */
#define RTT_CONST_TESTCASE_COMPONENT_STRING "RTT_TestProcedures"

/** @def RTT_CONST_TESTCASE_TESTCASE_STRING The name of the testcase. 
  * The value of this define is created during the test case generation. */
#define RTT_CONST_TESTCASE_TESTCASE_STRING  "TP-00"

/** @def RTT_TIMERS_IN_MICROSECONDS 
  * This define is used to decide if the timescale of the test case is
  * microseconds or milliseconds.
  * timescale 'ticks' is treated like microseconds
  * The value of this define is derived from rtt_prj_time_scale.h. */
#define RTT_TIMERS_IN_MICROSECONDS  (1 == 1 _us)

/** @def RTT_PROJECT_NAME_STRING
  * This define contains the string form of the current project name
  * The value of this define is created during the test case generation. */
#define RTT_PROJECT_NAME_STRING       "INTO-CPS-Demo"

/** @def RTT_TESTCONTEXT_STRING
  * This define contains the string form of the current test context
  * (absolute path) */
#define RTT_TESTCONTEXT_STRING        "C:/INTO-CPS/INTO_CPS_Pilot_Studies/case-study_ether/Test-Data-Generation/Ether_Indirect_Test_Model_v0.41"

/** @def RTT_CONST_RTTASSERT_TAG_SYNTAX
 * Accepted tag string syntax in @rttAssert(<epr>, <string>)
 * @see rttctx_rttassert_tag_syntax_t 
 */
#define RTT_CONST_RTTASSERT_TAG_SYNTAX  ((rttctx_rttassert_tag_syntax_t)(RTTCTX_RTTASSERT_DEFAULT))

/** @def RTT_CONST_RANDOM_GEN
 * Kind of random number generator used for this test procedure
 * @see rttctx_random_gen_t
 */
#define  RTT_CONST_RANDOM_GEN  RTTCTX_RANDOM_GEN_DEFAULT

/** @def RTT_CATCH_CPP_EXCEPTIONS
 * Configures, whether c++ exceptions are automatically caught by @try-@catch
 * blocks (value: 1) or not (value: 0).
 */
#define RTT_CATCH_CPP_EXCEPTIONS RTT_EXCEPTIONS_INCLUDE_CPP

/** @def RTT_CONFIG_NUM_AM 
  * The number of activated AMs of a test case.
  * The value of this define is created during the test case generation. */
#define RTT_CONFIG_NUM_AM  512

/** @def RTT_CONFIG_NUM_LWP 
  * The number of defined LWPs of a test case.
  * The value of this define is created during the test case generation. 
  * @note: after a 'make debug', this value always equals '1'
  */
#ifdef _LINUX_DEBUG
#define RTT_CONFIG_NUM_LWP 1
#else
#define RTT_CONFIG_NUM_LWP 1
#endif

/** @def RTT_CONFIG_NUM_CNODE 
  * The number of defined CNODEs of a test case.
  * The value of this define is created during the test case generation. */
#define RTT_CONFIG_NUM_CNODE 0

/** @def RTT_CONFIG_NUM_VEC_FILES
  * The number of vector files referenced in the *.conf file
  * The value of this define is created during the test case generation. */
#define RTT_CONFIG_NUM_VEC_FILES  0

/** @def RTT_CONFIG_NUM_VECTORS
 * The number of vectors defined in vector files referenced in the *.conf
 * file. The value of this define is created during the test case
 * generation. */
#define RTT_CONFIG_NUM_VECTORS  0

/** @def RTT_CONFIG_GLOBAL_CYCLETIME 
  * The global global cycletime as defined in the test configuration file.
  * The value of this define is created during the test case generation. */
#define RTT_CONFIG_GLOBAL_CYCLETIME 0


/** @def RTT_CONFIG_HAVE_SIGNALSET
 * 0: the SIGNALSET extension is not used
 * 1: at least one active AM defines a SIGNALSET, which implies that
 *    the SIGNALSET extension is used
 */
#define RTT_CONFIG_HAVE_SIGNALSET   0

// ///////////////////////////////////////////////
// [0.3] global variables
// ///////////////////////////////////////////////

/** @var  RTTam_context_t AM_context[RTT_CONFIG_NUM_AM];
 *  A global variable containing the AM configuration of all activates AMs of
 *  a test case.
 */
extern volatile RTTam_context_t   AM_context[RTT_CONFIG_NUM_AM];

/** @var RTTlwp_context_t  LWP_context[RTT_CONFIG_NUM_LWP];
 *  A global variable containing th LWP configuration of a test case. 
 */
extern volatile RTTlwp_context_t  LWP_context[RTT_CONFIG_NUM_LWP];

/** @var RTTcnode_context_t  CNODE_context[RTT_CONFIG_NUM_CNODE];
 *  A global variable containing th CNODE configuration of a test case. 
 */
#ifndef _CADUL
extern volatile RTTcnode_context_t  CNODE_context[RTT_CONFIG_NUM_CNODE];
#else
extern volatile RTTcnode_context_t  CNODE_context[1];
#endif

#ifdef RTT_CLUSTER_SOE
// local table for monitoring of last closed cycle for local AMs
extern volatile ccetsTables_t timeStampTable;
#endif

/** @var RTTglobal_setup_t RTTgs;
 *  A global variable containing the global setup of a test case.
 */
extern volatile RTTglobal_setup_t RTTgs;

/** @var uint32_t rtt_SUT_am_id;
 *  A global variable containing the am id of the abstract machine that is
 *  specified as the system under test (SUT) in the configuration file.
 */
//extern uint32_t rtt_SUT_am_id;

/** @var int32_t rttContext_rttTestRunning;
 *  The global test status Flag. Possible values are:
 *  0 = test case not running (not startetd or terminated)
 *  1 = test case is initialising
 *  2 = test running
 */
extern volatile int32_t rttContext_rttTestRunning;

/** @var uint64_t rttStartOfTestTic;
 *  The global timetick at which the test did start.
 */
extern volatile uint64_t rttStartOfTestTic;

/** @var rttStartOfTestEpoch
 *  The global time epoch used for the time tick calculation.
 */
extern volatile time_t rttStartOfTestEpoch;

/** @var uint64_t rttTestDurationSec;
 *  A max test duration boundary (This value is set only if the executeble of
 *  the test case is called with the duration time as its argument. If the
 *  script rtt-run-test is used, this variable is not used and the test
 *  duration is checked by a seperate watchdog process.
 */
extern uint64_t rttTestDurationSec;

/** @var uint32_t rtt_config_num_am;
 *  A global variables storing the number of activated AMs. It is set at
 *  runtime according to the value of RTT_CONFIG_NUM_AM.
 */
extern uint32_t rtt_config_num_am;

/** @var uint32_t rtt_config_num_lwp;
 *  A global variables storing the number of LWPs of a test case. It is set at
 *  runtime according to the value of RTT_CONFIG_NUM_LWP.
 */
extern uint32_t rtt_config_num_lwp;

/** @var uint32_t rtt_config_num_cnode; A global variables storing the number
 *  of CNODEs of a test case. It is set at runtime according to the value of
 *  RTT_CONFIG_NUM_CNODE.
 */
extern uint32_t rtt_config_num_cnode;

/** @var rtt_ti_level_t rttTiLevel;
 *  The global test integration level of a test case.
 */
extern rtt_ti_level_t rttTiLevel;

/** @var uint32_t rtt_SUT_am_id
 *  Re-Introduced this variable for backward compatibility reasons:
 *   @li available to all stubs (i.e. generated before 1.6.0, and then
 *       modified)
 *   @li allocated in the RTTcontext
 *   @li contains the number of the AM marked as SUT, and 0 if no such SUT
 *       is declared.
 * For tests based on version 1.6.0 or later, this global variable has no effect
 */
extern uint32_t rtt_SUT_am_id;

/** @var rttHaveDebugMode
 *  Indicate to rttctxlib, that the test is compiled in debug mode.
 */
extern const bool_t rttHaveDebugMode;

/** @var rtt_rttselect_semantics
 *  Defines the configured @rttSelect() and @rttSelectX() semantics.
 */
extern const rttctx_rttselect_semantics_t rtt_rttselect_semantics;

// ///////////////////////////////////////////////////////////////////
// [1] Global Variable Declarations for signal handling
// ///////////////////////////////////////////////////////////////////

/** 
 * These variables are initialised from RTTcontext.c and allow to disable
 * signal handlers for the specified signals with compile switches for each
 * test procedure.
 *
 * The compile switches used for disabling signals are of the form:
 *    _DISABLE_<Signal>_HANDLER_
 *
 * Signal     Value     Action   Comment
 * -------------------------------------------------------------------------
 * SIGHUP        1       Term    Hangup detected on controlling terminal
 *                               or death of controlling process
 * SIGINT        2       Term    Interrupt from keyboard
 * SIGQUIT       3       Core    Quit from keyboard
 * SIGILL        4       Core    Illegal Instruction
 * SIGABRT       6       Core    Abort signal from abort(3)
 * SIGFPE        8       Core    Floating point exception
 * SIGKILL       9       Term    Kill signal
 * SIGSEGV      11       Core    Invalid memory reference
 * SIGPIPE      13       Term    Broken pipe: write to pipe with no readers
 * SIGALRM      14       Term    Timer signal from alarm(2)
 * SIGTERM      15       Term    Termination signal
 * SIGUSR1   30,10,16    Term    User-defined signal 1
 * SIGUSR2   31,12,17    Term    User-defined signal 2
 * SIGCHLD   20,17,18    Ign     Child stopped or terminated
 * SIGCONT   19,18,25            Continue if stopped
 * SIGSTOP   17,19,23    Stop    Stop process
 * SIGTSTP   18,20,24    Stop    Stop typed at tty
 * SIGTTIN   21,21,26    Stop    tty input for background process
 * SIGTTOU   22,22,27    Stop    tty output for background process
 */
extern const bool_t rttIgnoreSignal_SIGHUP;
extern const bool_t rttIgnoreSignal_SIGINT;
extern const bool_t rttIgnoreSignal_SIGQUIT;
extern const bool_t rttIgnoreSignal_SIGILL;
extern const bool_t rttIgnoreSignal_SIGABRT;
extern const bool_t rttIgnoreSignal_SIGFPE;
extern const bool_t rttIgnoreSignal_SIGSEGV;
extern const bool_t rttIgnoreSignal_SIGPIPE;
extern const bool_t rttIgnoreSignal_SIGALRM;
extern const bool_t rttIgnoreSignal_SIGTERM;
extern const bool_t rttIgnoreSignal_SIGUSR1;
extern const bool_t rttIgnoreSignal_SIGUSR2;
extern const bool_t rttIgnoreSignal_SIGCHLD;
extern const bool_t rttIgnoreSignal_SIGCONT;
extern const bool_t rttIgnoreSignal_SIGSTOP;
extern const bool_t rttIgnoreSignal_SIGTSTP;
extern const bool_t rttIgnoreSignal_SIGTTIN;
extern const bool_t rttIgnoreSignal_SIGTTOU;

// ///////////////////////////////////////////////////////////////////
// [2] Global Function Declarations
// ///////////////////////////////////////////////////////////////////

/** @var long int rttWarnings[RTT_CONFIG_NUM_AM];
 *  A global variables contain an array with the number of warnings caused by
 *  each AM during the test execution.
 */
extern long int rttWarnings[RTT_CONFIG_NUM_AM];

/** @var long int rttFailures[RTT_CONFIG_NUM_AM];
 *  A global variables contain an array with the number of errors caused by
 *  each AM during the test execution.
 */
extern long int rttFailures[RTT_CONFIG_NUM_AM];

/** @var local cluster node id 
 */
extern uint32_t rtt_local_cnode_id;

/** @var local cluster node timing offset 
 */
extern uint64_t rtt_local_cnode_offset;

// ///////////////////////////////////////////////
// [2.1] Service Functions
// ///////////////////////////////////////////////

/**
 * Function rttAllWarnings. 
 * Returns the sum of all warnings of all defined abstract machines.
 * @return the sum of all warnings of all defined abstract machines.
 * Warning: this function is not thread safe.
 */
extern long int rttAllWarnings();

/**
 * Function rttAllFailures. 
 * Returns the sum of all failures of all defined abstract machines.
 * @return the sum of all failures of all defined abstract machines.
 * Warning: this function is not thread safe.
 */
extern long int rttAllFailures();

/**
 * Function rttStandardFormat. Standard logging function for unit test AMs.
 * 
 * @param amId       internal number of the abstract machine
 * @param eventFlag  condition/event/action flag
 * @param string     text string to be logged
 */
extern void rttStandardFormat(unsigned long, unsigned long, char*);


/**
 * Function rttStandardLogAM. Standard logging function for unit test AMs:
 * Always prints scratchpad (with safe terminating zero) with standard flags
 * 
 * @param amId       internal number of the abstract machine
 */
extern void rttStandardLogAM(unsigned long amId);

/**
 * Function rttAMContext_compute_verdict.Create a test verdict according
 * to context informations.
 * 
 * @param amNumber  internal number of the abstract machine
 * @param nWarnings number of warnings (fails of  @rttCheck)
 * @param nErrors   number of errors   (fails of  @rttAssert)
 * 
 */
extern void rttAMContext_compute_verdict(unsigned long, 
					 unsigned long,
					 unsigned long);

// ///////////////////////////////////////////////
// [2.2] CNODE, LWP and AM context functions
// ///////////////////////////////////////////////

/**
 * Function stublevel_context_INIT.
 * Initialisation of stub test integration levels
 */
extern void stublevel_context_INIT();

/**
 * Function stublevel_context_FINIT.
 * Finalisation of stub test integration levels
 */
extern void stublevel_context_FINIT();

// @todo-vec: document
extern void vector_context_INIT();

// @todo-vec: document
extern void vector_context_FINIT();


// ///////////////////////////////////////////////
// [2.3] AM context functions
// ///////////////////////////////////////////////

/**
 * Function am_context_INIT.
 * Initialisation of AM context.
 */
extern void am_context_INIT();

/**
 * Function am_context_FINIT.
 * Finalisation of AM context.
 */
extern void am_context_FINIT();

// ///////////////////////////////////////////////
// [2.4] LWP context functions
// ///////////////////////////////////////////////

/**
 * Function lwp_context_INIT.
 * Initialisation of LWP context.
 */
extern void lwp_context_INIT();

/**
 * Function lwp_context_FINIT.
 * Finalisation of LWP context.
 */
extern void lwp_context_FINIT();

// ///////////////////////////////////////////////
// [2.5] CNODE context functions
// ///////////////////////////////////////////////

/**
 * Function cnode_context_INIT.
 * Initialisation of CNODE context.
 */
extern void cnode_context_INIT();

/**
 * Function cnode_context_FINIT.
 * Finalisation of CNODE context.
 */
extern void cnode_context_FINIT();

// ///////////////////////////////////////////////
// [2.6] channel context functions
// ///////////////////////////////////////////////

/**
 * Function channel_context_INIT.
 * Initialisation of channel context.
 */
void channel_context_INIT();

/**
 * Function channel_context_FINIT.
 * Finalisation of channel context.
 */
void channel_context_FINIT();

/**
 * Function rttGetCnlSpec. Retrieves the channel specification
 * setup for a given channel name on an LWP.
 *
 * @param cnl Name of the channel
 * @param lwp_id The number of the LWP.
 */
rttCnlSpec_t *rttGetCnlSpec(const char *cnl, uint32_t lwp_id);

/**
 * Function rttPruneReadIndex. Remove duplicate read index array
 * initialisation. All ports of a channel of an abstract machine
 * should use the same read index.
 *
 * @param am_id Internal number of an abstract machine.
 */
void rttPruneReadIndex(uint32_t am_id);

/**
 * Function rttFreeReadIndex. Since read indices may be used by
 * multiple ports, the allocated memory must not be freed for each
 * port, but only once. This function frees the allocated memory once
 * and sets all referenced the rIdxPtr to NULL.
 *
 * @param am_id Internal number of an abstract machine.
 * @param rIdxPtr Read index pointer
 */
void rttFreeReadIndex(uint32_t am_id, uint32_t *rIdxPtr);

// ///////////////////////////////////////////////
// [2.7] Service Functions
// ///////////////////////////////////////////////

/**
 * Function rttStandardFormat. Standard logging function for unit test AMs.
 * 
 * @param amId       internal number of the abstract machine
 * @param eventFlag  condition/event/action flag
 * @param string     text string to be logged
 */
extern void rttStandardFormat(unsigned long, unsigned long, char*);

/**
 * Function rttAMContext_compute_verdict.Create a test verdict according
 * to context informations.
 * 
 * @param amNumber  internal number of the abstract machine
 * @param nWarnings number of warnings (fails of  @rttCheck)
 * @param nErrors   number of errors   (fails of  @rttAssert)
 * 
 */
extern void rttAMContext_compute_verdict(unsigned long, 
					 unsigned long,
					 unsigned long);

// -------------------------------------------------------------------------

#ifdef __cplusplus
          } // terminate extern "C" 
#endif


#endif

/*//////////////////////////////////////////////////////////////
//
// Change History:
//
// $Log: not supported by cvs2svn $
// Revision 1.57  2015/07/24 15:30:57  oli
// allow overwrite of TIMERS field by local setting;
// - this entails that settings are now stored in
//       ./stubsrc/ rtt_prj_time_scale.h
// - added dependency to this header to both RTTcontext.h
// - added compile-test logic to force 'big' update when TIMERS have changed
//
// Revision 1.56  2015/02/25 08:58:26  oli
// removed obsolete SIGNAL_AUTO_SUBSCRIPTION; OFF | ON | WARNING (no longer required for #7170)
//
// Revision 1.55  2014/04/23 15:58:49  oli
// added configuration field
//          SIGNAL_AUTO_SUBSCRIPTION; ON | OFF | WARNING
//       to influence the auto-subscription behaviour (default: OFF)
// see #7170
//
// Revision 1.54  2012/07/17 07:31:02  oli
// added clarification
//
// Revision 1.53  2012/07/16 15:03:10  oli
// project configuration:
// allow to use own version of gettimeofday() function by setting
//          TIMERS; ticks; <my_gettimeofday>
// which reads time via <my_gettimeofday> instead of gettimeofday();
// one microsecond is interpreted as one tick and does denote a logical
// time rather than system time;
// time values are denoted by ticks in the test log instead of milli/microseconds; (implements feature request #6834)
//
// Revision 1.52  2012/07/11 09:06:45  oli
// added compile-time macro RTT_TESTCONTEXT_STRING
// in ./src/ directory to allow use of RTT_TESTCONTEXT in *.rts files
// (implements feature request PR #6810)
//
// Revision 1.51  2012/04/26 15:35:04  oli
// type refectoring
//  changed field type for string contants from 'char*' to 'const char*'
//  this is more robust and avoids several compile warnings concerning
//  the generated code (of stubs/channel files)
// Note: this changes RT-Tester internal types rttCnlSpec_t, rttParmSpec_t
//       in rttcnllib.h
//
// Revision 1.50  2012/02/16 17:11:36  oli
// adjusted company postal address (to Am_Fallturm_1)
//
// Revision 1.49  2010/03/03 19:30:38  bisanz
// Contains now a constant rtt_rttselect_semantics which provides what is
// configured via new configuration option RTTSELECT_SEMANTICS.
// This contributes to the fix wrt. rt-tester/202, rt-tester/414.
//
// Revision 1.48  2010/02/15 16:26:11  oli
// use type-cast for RTT_CONST_RTTASSERT_TAG_SYNTAX definition.
// fixes (rt-tester/608)
//
// Revision 1.47  2009/08/04 13:55:44  jp
// New company address inserted into headers
//
// Revision 1.46  2009/06/17 10:15:14  oli
// added intialisation of global/local signals,
// if one of the activated AMs uses SIGNALSET field
//
// Revision 1.45  2009/06/10 08:59:07  bisanz
// cosmetics
//
// Revision 1.44  2009/06/10 08:50:14  bisanz
// Re-added inclusion of "RTT_user_printfuns.h" for backwards compatibility.
// This is guarded by a pre-processor macro, i.e. by default, inclusion is done
// but may be de-activated by defining RTT_NOINC_USER_PRINTFUNS.
//
// Revision 1.43  2009/03/16 18:50:42  bisanz
// Declarations of UNIT_AM_FUNCTIONS (i.e. <my_am>_INIT etc.) are removed
// from RTTcontext.h, again in order to remove unnecessary make dependencies
// and are contained in RTTcontext_private.h, now.
// Note: They are not extern "C" anymore, which is unnecessary.
//
// Revision 1.42  2009/03/13 11:59:06  bisanz
// (1) In addition to RTTcontext.h, there is RTTcontext_private.h now; the latter
//     shall contain declarations that users shall not use directly.
//     This corresponds to previous changes on RTTcontext.h and RTTcontext.c,
//     which had attempted to remove the
//       #define`s RTT_CONFIG_NUM_CHANNEL_FILES,
//                 RTT_CONFIG_NUM_CHANNEL_PER_LWP
//     from RTTcontext.h in order to have less re-compilation dependencies for
//     stub compilation:
//       Both are always updated on addition/removal of stub declarations within
//       some .stubs file; and each stub dependes on RTTcontext.h; therefore
//       always _all_ stubs had to be re-compiled.
//
//     Further definitions may be moved in the same fashion, if more dependencies
//     shall be removed.
//
//     Note that "power users" may include RTTcontext_private.h anyway; all
//     definitions here stay available for them.
// (2) Removed inclusion of "RTT_user_printfuns.h" from RTTcontext.h and
//     RTTcontext_global.h, for removal dependencies on stub-compilation.
//     For "make enum", the respective header is used directly now.
//     For stub and channel code compilation, it is not needed since the
//     declarations are given by generated channel headers.
//     For compilation of .rts.c files, the declarations are now inserted into
//     each .rts.c itself by rtt-gen-test and rtt-update-test, respectively.
//
// Revision 1.41  2009/03/12 14:09:05  bisanz
// temporarily reverted last change; RTT_CONFIG_NUM_CHANNEL* have to be
// accessible;
// @todo: probably add seperate header file
//
// Revision 1.40  2009/03/11 21:21:46  bisanz
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
// Revision 1.39  2009/03/06 15:59:48  tatiana
// modified structure of timeStampTable
//
// Revision 1.38  2009/03/04 15:05:19  bisanz
// - added: configuration field EXCEPTION_HANDLING
// - modified exception handling, such that @catch also
//   catches C++ exceptions (rt-tester/337)
//   To have the previous behaviour (separate handling),
//   use configuration option
//        EXCEPTION_HANDLING; SEPARATE
//
// Revision 1.37  2009/02/05 18:05:43  tatiana
// use for timeStampTable double buffering
//
// Revision 1.36  2009/01/08 13:39:34  tatiana
// added time stamp table for SOE cluster
//
// Revision 1.35  2008/12/11 10:01:14  oli
// added: configuration field RANDOM (and entry of its value in the RTTgs variable)
//
// Revision 1.34  2008/11/05 16:48:51  oli
// prepared configuration of allowed @rttAssert tag syntax (rt-tester/353)
//
// Revision 1.33  2007/08/02 09:26:51  oli
// added: inclusion of project name to test log
//
// Revision 1.32  2007/01/22 13:13:16  oli
// fixed cpp-name-mangling for cnl.h files and user defined
// print functions:
// declarations are outside 'extern C' name-mangling
//
// Revision 1.31  2006/10/13 15:33:32  chref
// adjustments necessary for dynamic AM handling:
// - always have 512 entries in AM_context
// - used-flag in AM_context entries (in order to find free entries)
// - always have 512 entries in am_id_list, am_start_list and am_prio_list
// - keep "volatile" attribute when passing pointers to LWP_context entries
//   to functions
//
// Revision 1.30  2006/04/19 10:29:57  oli
// Introduced (adjustable) minimum scratchpad size,
// default is 40 KB.
// Currently this can be set with the CFLAG
// "-DXXX_RTT_CONTEXT_MIN_SCRATCHPAD_SIZE_YYY=<NUM>"
// This should migrate into the .rtp file setup.
//
// Revision 1.29  2005/11/09 15:54:44  dahlweid
// - Added variables rttIgnoreSignal_<signal>, which are initialised from
//   RTTcontext.c and allow to disable signal handlers for the specified signals
//   with compile switches for each test procedure.
// - Fixed C++ compile problem of rttHaveDebugMode.
//
// Revision 1.28  2005/11/03 09:15:39  dahlweid
// Fix for PR: rt-tester/76
// Added an additional iteration over all vectors of a .vec file in
// rtt-gen-test. This requires a new #define RTT_CONFIG_NUM_VECTORS in
// RTTcontext.c/.h, which keeps the number of used vectors instead of the number
// of vector files.
//
// Revision 1.27  2005/10/10 12:03:19  oli
// fixed: catering for 'make debug':
//        in presence of compile flag _LINUX_DEBUG, the value of
//        RTT_CONFIG_NUM_LWP is set to one
//
// Revision 1.26  2005/10/07 12:52:11  oli
// added processing of .rtp/.conf file option
// CPP_INCLUDE ; "cpp_header.hh"
//
// Revision 1.25  2005/09/21 14:47:25  chref
// marked global variables that get changed in different threads as "volatile"
//
// Revision 1.24  2005/08/19 16:50:39  jp
// User thread scheduling for init and finit processes
//
// Revision 1.23  2005/08/17 18:00:25  jp
// RT-Tester User Threads, first version
//
// Revision 1.22  2005/08/12 15:55:02  walsen
// further cad-ul adjustments
//
// Revision 1.21  2005/08/04 16:11:31  oli
// guarded header files with the block
//    #ifdef __cplusplus
//     extern "C" {
//     #endif
// in order to allow for linking with C++ Style name mangling.
// (changed typename -> type_name)
//
// Revision 1.20  2005/05/27 13:52:55  oli
// added: VECTOR setup
//
// Revision 1.19  2004/08/04 14:53:21  marlex
// functions comments inserted/changed to coding standard
// for more information (and TODO) see in TODO_for_coding_standard.txt
//
// Revision 1.18  2004/08/02 14:43:01  marlex
// header added/changed for coding standard in *.l, *.y, *.c, *.h files
//
//
//
///////////////////////////////////////////////////////////////
*/
