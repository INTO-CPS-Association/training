//-*- mode: c++; c-basic-offset: 4; -*-
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
//     $Date: 2016-07-26 15:10:03 $
//
//--------------------------------------------------------------------
//
// Product: RT-Tester template for test startup operations
//
//--------------------------------------------------------------------
//
// File Identification: 
//
// $RCSfile: RTTstartup.c,v $
//
// $Header: /home/repository/VVTCVS/CVS/rtt-swi/tpl/RTTstartup.c,v 1.93.2.2 2016-07-26 15:10:03 oli Exp $
//
// $Revision: 1.93.2.2 $
//
// First edition by: Markus Dahlweid
// Last update by    $Author: oli $
//
//--------------------------------------------------------------------
//
// Description: 
//   
//
//--------------------------------------------------------------------*/
/** startup file for RT-Tester */ 

/************************************************************
  Includes
*************************************************************/

#include <stdio.h> 
#include <signal.h> 
#ifndef _CADUL
#include <unistd.h>
#endif

#include "RTTcontext.h"
#include "RTTschedule.h"

#include "rtt_test_config.h"
#include "rtt_test_types.h"

#include "rttservicelib.h"
#ifdef RTT_GSMI_EXTENSION
#include "rttvlclib.h"
#endif

#if (defined _CADUL) && (! defined RTT_CADUL_USE_CORRIDOR) /* CPU AFFINITY */
extern "C" int LoadLibrary(const char *Name);
extern "C" void *GetProcAddress(int Handle, const char *Name);
extern "C" int CallProcAddress(void *, int, ...);
#endif

#if RTT_CONFIG_HAVE_SIGNALSET
#include "RTTsignal.h"
#endif

#ifdef RTT_CLUSTER
#include "rttclusterdlib.h"
#endif

/** RTTschedule.c */
#if ( (defined(_LINUX) || defined(RTT_PTHREAD)) && ! defined (_LINUX_DEBUG) )
extern int rttWaitLWPs();
#endif

//extern rtt_gettimeofday_function_hook_t RTT_GETTIMEOFDAY_FUNCTION;
#if (defined _CADUL) || (defined RTT_GETTIMEOFDAY_CUSTOMIZED)
extern int RTT_GETTIMEOFDAY_FUNCTION(struct timeval *tv, struct timezone *tz);
#endif



/************************************************************
 *  Defines
 ************************************************************/

#define PID_FILENAME_TPL "rtt-test-case.%s.pid"

#define RTT_GET_TIC (rttGetTimeTick(RTTgs.timers_in_microseconds  ? \
			            rttTimeScale_micro_seconds  :   \
			            rttTimeScale_milli_seconds))

/************************************************************
 *  Global variables
 ************************************************************/

static int32_t rttContext_timeGeneration  = 0;

static char *prog_name = NULL;

/*--- identification of this RT-Tester Process ---*/
//static int32_t rttContext_rttProcId       = 0;

// ///////////////////////////////////////////////////////////////////
// Signal Handler
// ///////////////////////////////////////////////////////////////////

/**
 * Function rttSignalHandler.
 * This function defines the actions for the different signals that can be
 * sent to the test process.
 *
 * @TODO missing documentation of parameters
 * @param sig
 */
void rttSignalHandler( int sig ) {
    uint32_t am_id;
    rttTimeTic_t cnode_timetic;
    char* tmp;

#ifdef RTT_CLUSTER
    uint32_t rtt_local_cnode_id = getClusterNode();
#else
    uint32_t rtt_local_cnode_id = 0;
#endif

    cnode_timetic = RTT_GET_TIC - rttStartOfTestTic;

    signal(sig, &rttSignalHandler);

    switch (sig ) {

	case 10: 
	    rttContext_timeGeneration = 1;
#if (VERBOSE_LEVEL > 0) 
	    printf("TM %011" PRIu64 " CN%4d O Received signal 10 - starting test.\n",
		   (uint64_t)cnode_timetic, rtt_local_cnode_id);
#endif
	    break;

#if (! defined _CADUL) && (! defined _MINGW)
	case SIGPIPE:
	case SIGALRM:
	case SIGQUIT:
	case SIGHUP:
#endif
	case SIGABRT:
	case SIGTERM:
	case SIGINT:
	    if ( rttContext_rttTestRunning ) {
		// stop test execution
		rttContext_rttTestRunning = 0;

		// print output message
		fprintf(stderr,
		       "TM %011" PRIu64 " CN%4lu O Caught termination signal %d, "
		       "stop test execution.\n",
		       (uint64_t)cnode_timetic, (long unsigned int)rtt_local_cnode_id, sig);
                fflush(0);

		// output message to log of current AM.
		tmp = (char*)malloc(1024);
		for(am_id = 0; am_id < RTT_CONFIG_NUM_AM; am_id++) {
		    if (AM_context[am_id].used) {
			sprintf(tmp,
				"ABORTED IN TEST STEP: %ld\n"
				"CAUSE               : CAUGHT SIGNAL %d\n",
				AM_context[am_id].test_step,
				sig);
			rttctx_set_verdict_AM(am_id, RTTverdict_error, tmp);
			fflush(0);
		    }
		}
	    }
	    break;

	case SIGILL:
	case SIGFPE:
	case SIGSEGV:
	    // stop test execution
	    rttContext_rttTestRunning = 0;

	    fprintf(stderr,
		   "TM %011" PRIu64 " CN%4lu O Exiting on signal %d.\n",
		   (uint64_t)cnode_timetic, (long unsigned int)rtt_local_cnode_id, sig);
	    fflush(0);

#ifdef RTT_CLUSTER
	    rttClusterdTestStop();
#endif 
	    // exit
	    exit(sig);
	    break;

	default:
	    if ( rttContext_rttTestRunning ) {
		// stop test execution
		rttContext_rttTestRunning = 0;

		// print output message
		fprintf(stderr,
		       "TM %011" PRIu64 " CN%4lu O Caught unexpected signal %d, exiting!\n",
		       (uint64_t)cnode_timetic, (long unsigned int)rtt_local_cnode_id, sig);
                fflush(0);

#ifdef RTT_CLUSTER
		rttClusterdTestStop();
#endif 
		// exit
		exit(sig);
	    }
	    break;

    }

}


/**
 * Signal handler that prints a backtrace, implemented in RTTschedule.c
 *
 * @TODO missing documentation
 *
 * @param sig
 * @param si
 * @param ctx
 */
void lwpBacktraceSignalHandler(int sig
#ifdef _LINUX
			       , siginfo_t *si
			       , void *ctx
#endif
			       );


#if (! defined _CADUL) && (! defined _MINGW)
/**
 * Function rtt_nanosleep.
 * This function is used during the test run to suspend the control process.
 */
void rtt_nanosleep() {
    int    dummy;
    struct timespec dur, rem;
    uint64_t duration = RTT_CONFIG_GLOBAL_CYCLETIME;
    uint64_t timescale = RTTgs.timers_in_microseconds  ? 1000000  : 1000;

    dur.tv_sec = (time_t)(duration/timescale);
    dur.tv_nsec = (long)((1000000000/timescale)*(duration%timescale));

    dummy = nanosleep(&dur,&rem);
    rttctx_warn_if_nonzero(dummy, "call to nanosleep() failed; return value was: %d.");
    return;
}
#endif


#if (! defined _CADUL) && (! defined _MINGW)
/**
 * Function rttGetHostName()
 *
 * This function returns the hostname of the cluster node the test is
 * currently running. If the hostname cannot be determined the test is stopped
 * automatically.
 */
static const char *rttGetHostName() {
    static char hostname[1024];
    int ret = gethostname(hostname, 1024);
    if (ret == -1) {
	fprintf(stderr, "Error: Cannot determine hostname.");
#ifdef RTT_CLUSTER
	rttClusterdTestStop();
#endif 
	exit(10);
    }
    return hostname;
}
#endif

#if (defined _CADUL) 
extern "C" int tolower (int);
/**
 * Function rttGetHostName()
 *
 * This function returns the hostname or "localhost",
 * if the hostname cannot be determined.
 */
static const char *rttGetHostName() {
  static char hostname[1024];

  const char *computername = getenv("COMPUTERNAME");
  if (computername != 0) {
    const unsigned int length = strlen(computername);
    unsigned int index = 0;
    for (;
         ((index < length)
          && (index < (sizeof(hostname)/sizeof(hostname[0]) - 1)));
         ++index) {
      hostname[index] = (char)tolower((int)computername[index]);
    }
    hostname[index] = 0;
  }
  else {
    sprintf(hostname, "localhost");
  }
  return hostname;
}
#endif

#if (defined _CADUL) 
#if (defined RTT_CADUL_USE_CORRIDOR)
extern "C" int getpid();
#else
static int getpid () {
  // load native library
  const char libName[] = "MSVCRT.DLL";
  int libDesc = LoadLibrary(libName);
  if(libDesc == 0) {
    fprintf(stderr, "Unable to load library \"%s\".\n", libName);
  }
  else {
    void *f__getpid = GetProcAddress(libDesc, "_getpid");
    if (f__getpid == 0) {
      fprintf(stderr, "Unable to load function \"_getpid\"!\n");
    }
    else {
      // just call _getpid() from native library
      return CallProcAddress(f__getpid, 0);
    }
  }
  return -1;
}
#endif
#endif

/**
 * Functiom createPidReferenceFile.  
 *
 * This function creates a file "rtt-test-case.pid" in the current directory,
 * which contains the PID of the process. This PID can be used to terminate
 * the process manually.
 */
void createPidReferenceFile() {
#if (! defined _MINGW)
    FILE              *pidfile;
    char PID_FILENAME[1024];

    sprintf(PID_FILENAME, PID_FILENAME_TPL, rttGetHostName());
    pidfile = fopen(PID_FILENAME, "wb");
    if (pidfile != NULL) {
	fprintf(pidfile, "%d", getpid());
	fclose(pidfile);
    } else {
	fprintf(stderr, "Cannot create PID file \"%s\" in current directory.", 
		PID_FILENAME);
	exit(10);
    }
#endif
#ifdef RTT_CLUSTER
    rttClusterdTestStart();
#endif 
}

/**
 * Function removePidReferenceFile.  
 *
 * @note this function could return the value of 'unlink', but does not
 *
 * This function removes a file "rtt-test-case.pid" in the current directory,
 * which contains the PID of the process. 
 */
void removePidReferenceFile() {
#if (! defined _MINGW)
    char PID_FILENAME[1024];

    sprintf(PID_FILENAME, PID_FILENAME_TPL, rttGetHostName());
#ifdef _CADUL
#ifdef remove
#undef remove
#endif
    remove(PID_FILENAME);
#else
    unlink(PID_FILENAME);
#endif /*_CADUL*/
#endif
#ifdef RTT_CLUSTER
    rttClusterdTestStop();
#endif 
}

#if (defined _CADUL) || (defined _MINGW)
/** Constant Implementation: Always return 0 */
uint32_t getClusterNode() { 
  return 0; 
}
#else
/**
 * Function getClusterNode.  
 *
 * This function returns the cluster node id of the cluster node, the test is
 * started on. The cluster node id can be used to get the CNODE_context entry
 * of this cluster node. The CNODE_context contains definitions about the LWPs
 * that are running on this node.
 * 
 * @return Cluster node id of the cluster node. Its range is in
 *         0..RTT_CONFIG_NUM_CNODE-1. The id is an index so that
 *         CNODE_context[id] contains the cluster node attributes
 *         of the local node where getClusterNode() has been called.
 */
uint32_t getClusterNode() {
    char hostname[1024];
    uint32_t cnode_idx;
    int ret = gethostname(hostname, 1024);
    if (ret == -1) {
	fprintf(stderr,
		"Error: Cannot determine hostname."
		" (needed for cluster node id)\n");
#ifdef RTT_CLUSTER
	rttClusterdTestStop();
#endif 
	exit(10);
    }
    for (cnode_idx = 0; cnode_idx < RTT_CONFIG_NUM_CNODE; cnode_idx++) {
	if (strcmp(hostname,CNODE_context[cnode_idx].hostname) == 0) {
#if (VERBOSE_LEVEL > 0)
	    fprintf(stdout,
		    "cluster node '%s' has id %d.\n",
		    hostname,cnode_idx);
#endif
	    return cnode_idx;
	}
    }
    fprintf(stderr,
	    "Error: Cluster node '%s' is not defined "
	    "in the test configuration.\n",hostname);
#ifdef RTT_CLUSTER
    rttClusterdTestStop();
#endif 
    exit(10);
}
#endif

// ///////////////////////////////////////////////////////////////////
// parse parameters
// ///////////////////////////////////////////////////////////////////
/**
 * Function parseCommandline.
 * The only allowed commandline so far is an optional max. duration time for
 * the test process.
 * @param argc the number of arguments, including the name of the executable
 * @param argv an array of char*, pointing to the different argument strings
 */
static void  parseCommandline(int argc, char *argv[]) {
    long int parsed_val ; // parsed integer value

#ifndef _CADUL
    prog_name = strdup(argv[0]);
#else
    prog_name = "rtt-test-case.exe";
#endif
    
    if ( argc >= 2 ) {
	if (strcmp(argv[1],"-h") == 0) {
	    fprintf(stderr,
		    "Usage: %s [<TEST-DURATION(SEC)>]\n",
		    prog_name);
#ifdef RTT_CLUSTER
	    rttClusterdTestStop();
#endif 
	    exit(10);
	}
	parsed_val = atol(argv[1]);
	if(parsed_val >= 0L){
	  rttTestDurationSec = (uint64_t)parsed_val;
	}
	else {
	  rttTestDurationSec = 0LL;
	}
    } else {
	// no argument given => endless duration time
	rttTestDurationSec = 0LL;
    }

}


/**
 * rttAllCnodesReady()
 *
 * @return TRUE iff all cluster nodes are in state readyToRun.
 */
#ifdef RTT_CLUSTER
static bool_t rttAllCnodesReady(void) {

    uint32_t cnodeId;

    for (cnodeId = 0; cnodeId < rtt_config_num_cnode; cnodeId++) {
	if ( ! CNODE_context[cnodeId].readyToRun ) return FALSE;
    }

    return TRUE;

}
#endif

// ///////////////////////////////////////////////////////////////////
// MAIN
// ///////////////////////////////////////////////////////////////////


/**
   The main routine starts up the unit test.

   The following signals are caught and yield a test termination
   with finializing log entry:

   <PRE> 
   Signals described in the original POSIX.1 standard.


   Signal     Value     Action   Comment
   -------------------------------------------------------------------------
   SIGHUP        1       Term    Hangup detected on controlling terminal
   or death of controlling process
   SIGINT        2       Term    Interrupt from keyboard
   SIGQUIT       3       Core    Quit from keyboard
   SIGILL        4       Core    Illegal Instruction
   SIGABRT       6       Core    Abort signal from abort(3)
   SIGFPE        8       Core    Floating point exception
   SIGKILL       9       Term    Kill signal
   SIGSEGV      11       Core    Invalid memory reference
   SIGPIPE      13       Term    Broken pipe: write to pipe with no readers
   SIGALRM      14       Term    Timer signal from alarm(2)
   SIGTERM      15       Term    Termination signal
   SIGUSR1   30,10,16    Term    User-defined signal 1
   SIGUSR2   31,12,17    Term    User-defined signal 2
   SIGCHLD   20,17,18    Ign     Child stopped or terminated
   SIGCONT   19,18,25            Continue if stopped
   SIGSTOP   17,19,23    Stop    Stop process
   SIGTSTP   18,20,24    Stop    Stop typed at tty
   SIGTTIN   21,21,26    Stop    tty input for background process
   SIGTTOU   22,22,27    Stop    tty output for background process

   The signals SIGKILL and SIGSTOP cannot be caught, blocked, or ignored.
   </PRE>
*/

/** ensure that stdout and stderr are opened in binary mode */
RTT_INCLUDE_THIS_LINE_BEFORE_MAIN;


/**
 * Function main.
 * The main function of each test case. This function controls the startup and
 * the test termination.
 * @param argc the number of arguments, including the name of the executable
 * @param argv an array of char*, pointing to the different argument strings
 */
int main (int argc, char *argv[]) {
#if RTT_CONFIG_HAVE_SIGNALSET
    rttSigDef_t** sigGlobalDef;
    rttSigDef_t** sigLocalDef;
    uint32_t numGlobalSig = 0;
    uint32_t numLocalSig = 0;
#endif /* RTT_CONFIG_HAVE_SIGNALSET */
    time_t t_val;
    rtt_lwp_status_t status_of_lwps = rtt_lwp_initialising;

#if ( (defined (_LINUX) || defined (RTT_PTHREAD) ) && ! defined (_LINUX_DEBUG)  )
    uint32_t lwp_id, lwp_idx;
    uint32_t cnode_id;
#if (VERBOSE_LEVEL > 0) || defined (RTT_CLUSTER)
    rttTimeTic_t cnode_timetic;
#endif
#endif

#if ((defined _LINUX) || (defined _CYGWIN) || (defined _DARWIN)) && (! defined RTT_LINGW)
    struct sigaction sa;
    {
      sigset_t sigset;
      sigemptyset(&sigset);
#if (defined SIGRTMIN) && (defined SIGRTMAX)
      // by default, block all user-defined signals;
      // this will be inherited by all spawned pthreads (if used)
      int signo;
      for (signo = SIGRTMIN; signo <= SIGRTMAX; ++signo) {
        sigaddset(&sigset, signo);
      }
#endif
#ifdef RTT_BLOCK_SIGCHLD
      sigaddset(&sigset, SIGCHLD);
#endif
#ifdef RTT_PTHREAD
      pthread_sigmask( SIG_BLOCK, &sigset, NULL );
#else
      sigprocmask( SIG_BLOCK, &sigset, NULL );
#endif
    }
#endif

    if(! TRUE){
	fprintf(stderr,
"WARNING: DETECTED mis-definition of constant TRUE (==%d)\n"
"         -> must not be 0\n"
"         ** test procedure execution may be defective **\n"
"         PLEASE CORRECT THIS IN YOUR PROJECT HEADER FILES!\n"
		, (int)TRUE);
    }
    if(FALSE){
	fprintf(stderr,
"WARNING: DETECTED mis-definition of constant FALSE (==%d)\n"
"         -> must be 0\n"
"         ** test procedure execution may be defective **\n"
"         PLEASE CORRECT THIS IN YOUR PROJECT HEADER FILES!\n"
		, (int)FALSE);
    }

#if (defined _CADUL) && ( ! defined RTT_CADUL_USE_CORRIDOR ) /* CPU AFFINITY */
    {
        /* for use on multicore processors, set affinity of this
           process to one single core */
        const char * const RTT_KERNEL32_LIB="kernel32.dll";
        fprintf(stderr,
                "Setting Affinity of Cadul Code (requires %s)\n",
                RTT_KERNEL32_LIB);
        //
        typedef void* HANDLE;
        typedef unsigned long int __RTTstartup__DWORD;
        //
        int libDesc = LoadLibrary(RTT_KERNEL32_LIB);
        bool_t ok = (libDesc != 0);
        if (!ok) {
            fprintf(stderr, "Unable to load library \"%s\".\n",
                    RTT_KERNEL32_LIB);
        }
        if (ok) {
            void *f_GetCurrentProcess
                = GetProcAddress(libDesc, "GetCurrentProcess");
            void *f_GetProcessAffinityMask
                = GetProcAddress(libDesc, "GetProcessAffinityMask");
            void *f_SetProcessAffinityMask
                = GetProcAddress(libDesc, "SetProcessAffinityMask");
            if (f_GetCurrentProcess == 0) {
                fprintf(stderr,
                        "Unable to load function \"GetCurrentProcess\"!\n");
                ok = FALSE;
            }
            if (f_GetProcessAffinityMask == 0) {
                fprintf(stderr, "Unable to load function "
                        "\"GetProcessAffinityMask\"!\n");
                ok = FALSE;
            }
            if (f_SetProcessAffinityMask == 0) {
                fprintf(stderr, "Unable to load function "
                        "\"SetProcessAffinityMask\"!\n");
                ok = FALSE;
            }
            __RTTstartup__DWORD processAffinityMask = 0;
            HANDLE hProc = 0;
            if (ok) {
                /* get handle of current process */
                hProc=(HANDLE)CallProcAddress(f_GetCurrentProcess, 0);
                ok = (hProc != 0);
                if (!ok) {
                    fprintf(stderr, "\"GetCurrentProcess\" fails\n");
                }
            }
            if (ok) {
                /* get affinity mask */
                __RTTstartup__DWORD systemAffinityMask = 0;
                ok = (bool_t)CallProcAddress(f_GetProcessAffinityMask, 3,
                                             hProc,
                                             &processAffinityMask,
                                             &systemAffinityMask);
                if (!ok) {
                    fprintf(stderr, "\"GetProcessAffinityMask\" fails\n");
                }
            }
            if (ok) {
                /* find a bit that is set */
                __RTTstartup__DWORD mask = 1;
                while ((mask != 0) && (mask & processAffinityMask) == 0) {
                    mask = mask << 1;
                }
                /* set mask */
                ok = (bool_t)CallProcAddress(f_SetProcessAffinityMask, 2,
                                             hProc,
                                             mask);
                if (!ok) {
                    fprintf(stderr, "\"SetProcessAffinityMask\" fails\n");
                }
            }
            if (ok) {
                fprintf(stderr, "Succeeded to set Affinity.\n");
            }
            else {
                fprintf(stderr,
                        "Failed to set Affinity due to previous errors.\n");
            }
        }
    }
#endif  /* (defined _CADUL) CPU AFFINITY */


    // load symbols for backtrace
    rttctx_init_symbols();

    // parse commandline parameters
    parseCommandline(argc, argv);

    // Get the thread ID for the control thread
    rttControlThreadId = RTT_GETTID();

    // initialise Context for 
    // stub test integration levels, AMs, LWPs and channels
    createPidReferenceFile();

#ifdef RTT_EXTERNAL_CLOCK_SOURCE
    /* Synchronize with external clock source as soon as possible, at
       least before any call op rttGetTimeTick().
       Reason: rttGetTimeTick() make sure to return only monotonic
       values. When the clock is synchronized later, rttGetTimeTick()
       will return the same value until the external clock reaches the
       local clock. */
    rttctx_initialSyncWithExternalClock();
#endif /* RTT_EXTERNAL_CLOCK_SOURCE */

    RTTgs.project_name  = strdup(RTT_PROJECT_NAME_STRING);
    RTTgs.is_archive_version = FALSE;
    RTTgs.tag_syntax   = (rttctx_rttassert_tag_syntax_t) RTT_CONST_RTTASSERT_TAG_SYNTAX;
    RTTgs.random_gen   = (rttctx_random_gen_t) RTT_CONST_RANDOM_GEN;
    RTTgs.have_signal_extension = (0 != RTT_CONFIG_HAVE_SIGNALSET);
    RTTgs.testproc_name = rttstr_zero_string(strlen(RTT_CONST_TESTCASE_COMPONENT_STRING) + strlen(RTT_CONST_TESTCASE_TESTCASE_STRING) + 2);
    sprintf(RTTgs.testproc_name, "%s/%s", RTT_CONST_TESTCASE_COMPONENT_STRING, RTT_CONST_TESTCASE_TESTCASE_STRING);
    RTTgs.rtt_testcontext  = strdup(RTT_TESTCONTEXT_STRING);
    RTTgs.timetick_string  = strdup(RTT_TIME_TICK_STRING);
    RTTgs.gettimeofday     = (rtt_gettimeofday_function_hook_t)RTT_GETTIMEOFDAY_FUNCTION;
#if (defined RTT_DEBUG_RTTYIELD)
    RTTgs.debug_rttyield = TRUE;
#else
    RTTgs.debug_rttyield = FALSE;
#endif

    am_context_INIT();
    lwp_context_INIT();
    cnode_context_INIT();
    channel_context_INIT();
    vector_context_INIT();
    stublevel_context_INIT();

    // register signal handlers
    if (!rttIgnoreSignal_SIGINT)  signal(SIGINT,   &rttSignalHandler);
    if (!rttIgnoreSignal_SIGTERM) signal(SIGTERM,  &rttSignalHandler);
#if (! defined _CADUL) && (! defined _MINGW)
    if (!rttIgnoreSignal_SIGHUP)  signal(SIGHUP,   &rttSignalHandler);
    if (!rttIgnoreSignal_SIGUSR1) signal(SIGUSR1,  &rttSignalHandler);
    if (!rttIgnoreSignal_SIGUSR2) signal(SIGUSR2,  &rttSignalHandler);
    if (!rttIgnoreSignal_SIGCHLD) signal(SIGCHLD,  &rttSignalHandler);
    if (!rttIgnoreSignal_SIGCONT) signal(SIGCONT,  &rttSignalHandler);
    if (!rttIgnoreSignal_SIGSTOP) signal(SIGSTOP,  &rttSignalHandler);
    if (!rttIgnoreSignal_SIGTSTP) signal(SIGTSTP,  &rttSignalHandler);
    if (!rttIgnoreSignal_SIGTTIN) signal(SIGTTIN,  &rttSignalHandler);
    if (!rttIgnoreSignal_SIGTTOU) signal(SIGTTOU,  &rttSignalHandler);
#endif

    // exit on error signals
#ifdef _LINUX
    /* use 3-parameter signal handler on linux */
    sa.sa_sigaction = lwpBacktraceSignalHandler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_SIGINFO;

    if (!rttIgnoreSignal_SIGILL)  sigaction(SIGILL,  &sa, NULL);
    if (!rttIgnoreSignal_SIGABRT) sigaction(SIGABRT, &sa, NULL);
    if (!rttIgnoreSignal_SIGFPE)  sigaction(SIGFPE,  &sa, NULL);
    if (!rttIgnoreSignal_SIGPIPE) sigaction(SIGPIPE, &sa, NULL);
    if (!rttIgnoreSignal_SIGALRM) sigaction(SIGALRM, &sa, NULL);
    if (!rttIgnoreSignal_SIGQUIT) sigaction(SIGQUIT, &sa, NULL);
#if ( ! defined (_LINUX_DEBUG) )
    if (!rttIgnoreSignal_SIGSEGV) sigaction(SIGSEGV, &sa, NULL);
#endif
#else
    if (!rttIgnoreSignal_SIGILL)  signal(SIGILL,  lwpBacktraceSignalHandler);
    if (!rttIgnoreSignal_SIGABRT) signal(SIGABRT, lwpBacktraceSignalHandler);
    if (!rttIgnoreSignal_SIGFPE)  signal(SIGFPE,  lwpBacktraceSignalHandler);
    if (!rttIgnoreSignal_SIGSEGV) signal(SIGSEGV, lwpBacktraceSignalHandler);
#if (! defined _CADUL) && (! defined _MINGW)
    if (!rttIgnoreSignal_SIGPIPE) signal(SIGPIPE, lwpBacktraceSignalHandler);
    if (!rttIgnoreSignal_SIGQUIT) signal(SIGQUIT, lwpBacktraceSignalHandler);
    if (!rttIgnoreSignal_SIGALRM) signal(SIGALRM, lwpBacktraceSignalHandler);
#endif
#endif

    // switch to initialisation phase
    rttContext_rttTestRunning = 1;
    status_of_lwps = rtt_lwp_initialising;

    // calculate global start time and epoch

    rttStartOfTestTic = rttGetTimeTick(RTTgs.timers_in_microseconds  ? 
				       rttTimeScale_micro_seconds  :
				       rttTimeScale_milli_seconds);
    t_val = time((time_t *)&rttStartOfTestEpoch);
    if ( t_val == ((time_t)-1) )
	perror("error calculating absolute startup time:");


#ifdef RTT_CLUSTER
    rtt_local_cnode_id = getClusterNode();
    // setup Myrinet or TCP/IP communications with other nodes
    rttClusterInit(rtt_local_cnode_id, RTT_CONFIG_NUM_CNODE);
#endif

    // signalset initialisation (global and local)
#if RTT_CONFIG_HAVE_SIGNALSET
    {
	rttSigRet_t rttSigRet;

	rtt_init_testbed_signals(&sigGlobalDef, &numGlobalSig);
	rtt_init_testproc_signals(&sigLocalDef, &numLocalSig);
	init_signal_set(sigGlobalDef, numGlobalSig,
			sigLocalDef, numLocalSig);
	rttVlcInit();
#ifdef RTT_GSMI_EXTENSION
        rttVlcGSMIInit(numGlobalSig, numLocalSig);
#endif
	rttSigRet = rttSigInit(sigGlobalDef, numGlobalSig,
			       sigLocalDef, numLocalSig);
	
	if (rttSigRet != rttSigOk) {
	    fprintf(stderr, "Signal initialisation failed\n");
	    fflush(NULL);
#ifdef RTT_CLUSTER
	    rttClusterdTestStop();
#endif 
	    exit(10);
	}
    }
#endif /* RTT_CONFIG_HAVE_SIGNALSET */

    // create test LWPs for AMs
    rttCreateLWPs();


#if ( (defined (_LINUX) || defined (RTT_PTHREAD) ) && ! defined (_LINUX_DEBUG)  )

    // check for cluster node configuration
    if (RTT_CONFIG_NUM_CNODE > 0) {
	// calculate cluster node id
	cnode_id = getClusterNode();
	// wait for LWPs to be ready to run
	while (status_of_lwps == rtt_lwp_initialising) {
	    status_of_lwps = rtt_lwp_ready_to_run;
	    for (lwp_idx=0; 
		 lwp_idx<(uint32_t)CNODE_context[cnode_id].num_of_lwps;
		 lwp_idx++) {
		lwp_id = CNODE_context[cnode_id].lwp_id_list[lwp_idx];
		if (LWP_context[lwp_id].lwp_status == rtt_lwp_initialising)
		    status_of_lwps = rtt_lwp_initialising;
	    }
	}
    } else {
	// wait for LWPs to be ready to run
	while (status_of_lwps == rtt_lwp_initialising) {
	    status_of_lwps = rtt_lwp_ready_to_run;
	    for (lwp_id=0; lwp_id<RTT_CONFIG_NUM_LWP; lwp_id++) {
		if (LWP_context[lwp_id].lwp_status == rtt_lwp_initialising)
		    status_of_lwps = rtt_lwp_initialising;
	    }
	}
    }

    if (rttContext_rttTestRunning != 0) {
      // create map of channels which are written to / read from 
      // on specific cluster nodes
#ifdef RTT_CLUSTER
      rttCreateChannelMap();
#endif
    }
    
    // startup test now, unless @INIT already perfored '@rttStopTest'
    if (rttContext_rttTestRunning != 0) {

#if (VERBOSE_LEVEL > 0)
	cnode_timetic = RTT_GET_TIC - rttStartOfTestTic;
#ifdef RTT_CLUSTER
	printf("TM %011" PRIu64 " CN%4d O Starting test on cluster node.\n",
	       (uint64_t)cnode_timetic, rtt_local_cnode_id);
#else
	printf("TM %011" PRIu64 " CN%4d O Starting test on cluster node.\n",
	       (uint64_t)RTT_GET_TIC, 0);
#endif
	fflush(NULL);
#endif

	/** Cluster startup synchronisation */
#ifdef RTT_CLUSTER
	{
	  uint32_t remoteCnodeId;
	  uint32_t cmd;
	  uint32_t cn;
	  uint32_t cnlId = 0;
	    
	  /*--- send my channel subscriptions to other nodes ---*/
	  for ( cnlId = 0; cnlId < RTT_NUM_CHANNEL_MAP; cnlId++) {

	    if ( rttReadChannelMap[cnlId] )
	      for ( cn = 0; cn < rtt_config_num_cnode; cn++) {
		if ( cn != rtt_local_cnode_id )
		  rttcluster_tx_subscribecnlcmd(cn,cnlId);
		rttRxClusterCmd(&cmd,&remoteCnodeId);
		if (cmd == RTT_CLUSTER_INITIALISED_TEST) {
		  CNODE_context[remoteCnodeId].readyToRun = TRUE;   
		  cnode_timetic = RTT_GET_TIC - rttStartOfTestTic;
		  printf("TM %011" PRIu64 " CN%4d O RTT_CLUSTER_INITIALISED_TEST FROM CN %d\n",
			 (uint64_t)cnode_timetic, rtt_local_cnode_id, remoteCnodeId);
		  fflush(NULL);
		}
	      }
	  }
		
	  /*--- Send my "end of initialisation" message to all nodes ---*/
	  rttTxClusterCmd(RTT_CLUSTER_INITIALISED_TEST);
	  cnode_timetic = RTT_GET_TIC - rttStartOfTestTic;
	  printf("TM %011" PRIu64 " CN%4d O RTT_CLUSTER_INITIALISED_TEST\n",
		 (uint64_t)cnode_timetic, rtt_local_cnode_id);
	  fflush(NULL);
	  CNODE_context[rtt_local_cnode_id].readyToRun = TRUE;

	  do {

	    rttRxClusterCmd(&cmd,&remoteCnodeId);
	    if (cmd == RTT_CLUSTER_INITIALISED_TEST) {
	      CNODE_context[remoteCnodeId].readyToRun = TRUE;   
	      cnode_timetic = RTT_GET_TIC - rttStartOfTestTic;
	      printf("TM %011" PRIu64 " CN%4d O RTT_CLUSTER_INITIALISED_TEST FROM CN %d\n",
		     (uint64_t)cnode_timetic, rtt_local_cnode_id, remoteCnodeId);
	    }

	  } while ( ! rttAllCnodesReady() );

	}
    
	printf("TM %011" PRIu64 " CN%4d O Cluster node initialisation completed\n",
	       (uint64_t)cnode_timetic, rtt_local_cnode_id);
	fflush(NULL);
	
#endif
	rttContext_rttTestRunning = 2;
    }

    status_of_lwps = rtt_lwp_running;
    // check for cluster node configuration
    // @todo remove this busy wait stuff - 
    // instead wait for all LWPs to terminate
    if (RTT_CONFIG_NUM_CNODE > 0) {
	// check for running LWPs
	while ( rttContext_rttTestRunning &&
	        (status_of_lwps == rtt_lwp_running)) {
	    status_of_lwps = rtt_lwp_test_completed;
	    for (lwp_idx=0; 
		 lwp_idx<(uint32_t)CNODE_context[cnode_id].num_of_lwps;
		 lwp_idx++) {
		lwp_id = CNODE_context[cnode_id].lwp_id_list[lwp_idx];
		if ((LWP_context[lwp_id].lwp_status == rtt_lwp_ready_to_run) ||
		    (LWP_context[lwp_id].lwp_status == rtt_lwp_running))
		    status_of_lwps = rtt_lwp_running;
	    }
	    // wait for LWP
	    rtt_nanosleep();
	}
    } else {
	// check for running LWPs
	while (rttContext_rttTestRunning &&
	       (status_of_lwps == rtt_lwp_running)) {
	    status_of_lwps = rtt_lwp_test_completed;
	    for (lwp_id=0; lwp_id<RTT_CONFIG_NUM_LWP; lwp_id++) {
		if ((LWP_context[lwp_id].lwp_status == rtt_lwp_ready_to_run) ||
		    (LWP_context[lwp_id].lwp_status == rtt_lwp_running))
		    status_of_lwps = rtt_lwp_running;
	    }
	    // wait for LWP
	    rtt_nanosleep();
	}
    }

    // set global test termination flag
    rttContext_rttTestRunning = 0;

    // wait for child processes to terminate
#ifndef _LINUX_DEBUG
    rttWaitLWPs();
#endif
#else
    if(status_of_lwps == rtt_lwp_initialising){
        status_of_lwps = rtt_lwp_test_completed;
    }
#endif /* defined _LINUX || defined RTT_PTHREAD */

    am_context_FINIT();
    lwp_context_FINIT();
    cnode_context_FINIT();
    channel_context_FINIT();
    vector_context_FINIT();
    stublevel_context_FINIT();

    removePidReferenceFile();

#ifdef RTT_CLUSTER
    rttClusterFinit();
#endif 

#ifdef _CADUL
    // -- this is to avoid compiler warnings ----------------------------
    if(rtt_lwp_initialising == status_of_lwps){
      return 0;
    }
#endif

    return 0;
}


/*//////////////////////////////////////////////////////////////
//
// Change History:
//
// $Log: not supported by cvs2svn $
// Revision 1.93.2.1  2015/10/22 12:34:02  oli
// do not need channel map
//
// Revision 1.93  2015/09/15 13:15:37  uwe
// Enabled signal handlers for Mac OS X.
//
// Revision 1.92  2015/09/04 08:26:07  oli
// block sigaction for RTT_LINGW
//
// Revision 1.91  2015/06/09 12:40:21  oli
// robusness / diagnostics:
// - added (flooding-controlled) function rttctx_warn_if_nonzero()
//   for diagnostic output
// - do not ignore fails calls to nanosleep(),  issue warning
//   (but allow for flooding control) - implements #12180
//
// Revision 1.90  2015/05/26 13:46:32  oli
// for GSMI, added include of rttvlclib.h
// (required for C++ compile, since rttVlcGSMIInit() is used then and has to be declared)
// thanks to chref for finding this one
//
// Revision 1.89  2015/02/25 08:58:26  oli
// removed obsolete SIGNAL_AUTO_SUBSCRIPTION; OFF | ON | WARNING (no longer required for #7170)
//
// Revision 1.88  2014/10/10 11:08:01  oli
// robustness: for [u]int64_t integers use consequently the
// PRIu64, PRId64 format directives instead of llu, lld
// (implements #11558)
//
// Revision 1.87  2014/04/23 15:58:49  oli
// added configuration field
//          SIGNAL_AUTO_SUBSCRIPTION; ON | OFF | WARNING
//       to influence the auto-subscription behaviour (default: OFF)
// see #7170
//
// Revision 1.86  2014/04/23 09:20:17  oli
// fixed ifdef syntax
//
// Revision 1.85  2014/04/15 06:46:29  oli
// Adjustments for execution in RunCadul (virtual machine):
// * remove dynamic loading of DLLs
// * deactivate setting of CPU affinity (handled by RunCadul)
// * supplement implementations of still required functions
// * provided run-time environment setting
// * change socket communication to corridor callbacks
// * add latency in license daemon before closing socket afte reply
//   (required to be functional with corridor callbacks)
// * add utility 'rtt_aux_user' for user identification
//
// Revision 1.84  2012/12/07 16:50:47  oli
// reworked fix of PR #7202, since it may do more harm than good;
// now only a (compile-time) WARNING is issues (unless for Cadul, where #warning is not supported).
// On startup a warning is issued to <stderr>, if  a misdefinition is detected.
// This fixes the problem described in (PR #7370)
//
// Revision 1.83  2012/10/31 08:49:12  emm
// fix log in comment of previous change, since it produced
// warnings at compile time (comment within comment)
//
// Revision 1.82  2012/10/29 13:01:01  oli
// re-organised TRUE/FALSE definition:
// - attempt to correct faulty definitions
//   (FALSE must always be 0, TRUE != 0)
// - re-arranged src/ *.c template files, such that RTTcontext.h is included
//   before other rtt* header files
//   (this allows for true/false corrections)
//   Note that this does not affect the *.rts file compilation.
//
// Revision 1.81  2012/08/22 12:40:45  oli
// added: debugging provision for user-space scheduling:
//   if at compile-time of test-procedure the flag
//      -DRTT_DEBUG_RTTYIELD
//   is set (or RTTgs.debug_rttyield is set to TRUE explicitly), then
//   every call to rtt_yield() will generate an output of the following
//   form to stderr:
// 	  DEBUG: RTT_YIELD (lwpId=<n>, amId=<n>)
//   This can be helpful to identify a crashing AM in cases where no
//   backtrace is available (e.g., CADUL);
//   it also can identify hanging LWPs
//
// Revision 1.80  2012/07/17 06:24:26  oli
// changed gettimeofday function pointer assignement so that it also works with Cadul
//
// Revision 1.79  2012/07/16 15:03:10  oli
// project configuration:
// allow to use own version of gettimeofday() function by setting
//          TIMERS; ticks; <my_gettimeofday>
// which reads time via <my_gettimeofday> instead of gettimeofday();
// one microsecond is interpreted as one tick and does denote a logical
// time rather than system time;
// time values are denoted by ticks in the test log instead of milli/microseconds; (implements feature request #6834)
//
// Revision 1.78  2012/07/11 12:17:04  oli
// extended global setting by char* field: RTTgs.rtt_testcontext
//
// Revision 1.77  2012/02/16 17:11:37  oli
// adjusted company postal address (to Am_Fallturm_1)
//
// Revision 1.76  2011/03/23 07:48:47  oli
// cosmetics: align set_verdict_TESTERROR() output string (terminating newline)
// avoid using the scratchpad of the AM (again)
//
// Revision 1.75  2011/03/22 20:19:40  oli
//  use scratchpad, when calling rttctx_set_verdict_AM() - fixes (rt-tester/709)
//
// Revision 1.74  2011/03/03 09:46:25  bisanz
// Merged all modifications of of branch rscholz_newbuildsystem into the main
// trunk.
// Since it has been ensured that in turn the branch already contains the
// modifications of the trunk, only the differences between the heads of the main
// trunk (sync__pre__2011-03-03__2__trunk--from-rscholz_newbuildsystem) and the
// branch (sync__2011-03-03__2__rscholz_newbuildsystem--to--trunk) are merged.
// The updated main trunk is going to be tagged as:
//   sync__2011-03-03__2__trunk--from-rscholz_newbuildsystem
//
// Revision 1.69.2.2  2011/01/17 18:45:33  bisanz
// Merged modifications of the main trunk:
//   sync__2010-11-24__trunk--to--rscholz_newbuildsystem --> RTT_REV_6_0_REL_4_8_6
// into branch:
//   rscholz_newbuildsystem
// The updated branch is going to be tagged as:
//   sync__2011-01-17__rscholz_newbuildsystem--from--RTT_REV_6_0_REL_4_8_6
//
// Revision 1.69.2.1  2010/11/24 11:34:28  bisanz
// Merged modifications of the main trunk:
//   rscholz_build_20100427 --> sync__2010-11-24__trunk--to--rscholz_newbuildsystem
// into branch:
//   rscholz_newbuildsystem
// The updated branch is going to be tagged as:
//   sync__2010-11-24__rscholz_newbuildsystem--from--trunk
//
// Revision 1.73  2010/12/20 14:02:11  bisanz
// Renamed local type definition of DWORD (for CAD-UL) to __RTTstartup__DWORD, in
// order to prevent from name clashes.
//
// Revision 1.72  2010/08/20 09:16:29  bisanz
// Fix on modification of rev. 1.70: Moved sigemptyset() up a bit, such that it
// will be used if SIGRTMIN or SIGRTMAX are undefined, too.
//
// Revision 1.71  2010/08/16 15:22:31  mwege
// - GSMI: Change of Master-> Clear signal subscribtion masks.
// - VLC-Lib: Error message inserted for out of read buffer case in VLC library.
//
// Revision 1.70  2010/08/13 15:29:15  bisanz
// For cygwin, the pre-processor define RTT_BLOCK_SIGCHILD is now evaluated. If
// defined, sigprocmask() is called in the same fashion as for Linux. Further,
// the signals [ SIGRTMIN .. SIGRTMAX ] are blocked, too.
// This fixes PR rt-tester/652.
//
// Revision 1.69  2010/03/18 16:07:47  oli
// Added: associated the generic tag ANONYMOUS##<tp> to @rttAssert(),
// if no test cases are given (rt-tester/627)
// For this, the name of the test procedure was added to the global settings (as string).
//
// Revision 1.68  2010/03/03 13:51:35  mwege
// Fixed some issues that made the signal extension inoperative if CAD-UL
// compiler was used for test compilation:
// - Removed declaration of two constants within an expression of a for-loop.
//   (rttsiglib_list.h)
// - Added casts for code generated by signal set parser (scan_sigset_yacc.y).
// - Added cast for malloc of global signal data structure in RTTsignal.c.
// - Added casts for RTTgs.tag_syntax and RTTgs.random_gen assignments in
//   RTTstartup.c.
//
// Revision 1.67  2009/11/04 09:18:09  emm
// fix handling of signals which are too large for siglib/vlclib.
// This fixes GNATS PR 576.
//
// Revision 1.66  2009/08/04 13:55:44  jp
// New company address inserted into headers
//
// Revision 1.65  2009/06/25 14:06:27  mwege
// signalset initialisation after rtt_local_cnode_id has been set
//
// Revision 1.64  2009/06/23 15:29:04  tatiana
// changed signature of init_signal_set()
//
// Revision 1.63  2009/06/23 14:13:26  emm
// - fix types of signal definition variables
// - temporarily remove signal cnode assignment setup
//
// Revision 1.62  2009/06/22 15:39:07  tatiana
// call init_signal_set()
//
// Revision 1.61  2009/06/17 12:52:00  oli
// added: global configuration field have_signal_extension
//
// Revision 1.60  2009/06/17 10:15:14  oli
// added intialisation of global/local signals,
// if one of the activated AMs uses SIGNALSET field
//
// Revision 1.59  2009/05/25 18:38:11  bisanz
// fix for CADUL: added #undef remove, which is needed for COOPI applications,
// because there, remove is hidden by some #define because it must not be used
// for the COOPI SUT
//
// Revision 1.58  2009/04/14 12:16:29  bisanz
// fix: added declaration of tolower for CADUL-rttGetHostName
//
// Revision 1.57  2009/04/02 13:54:03  bisanz
// enabled pid-reference-file handling for CADUL
//
// Revision 1.56  2009/01/30 21:13:34  hjficker
// - Added generic support for synchronization with externel clocks (currently
//   implemented only for SOE clusters)
// - do initial synchronization with external clocks much earlier in test run
//
// Revision 1.55  2008/12/11 10:01:14  oli
// added: configuration field RANDOM (and entry of its value in the RTTgs variable)
//
// Revision 1.54  2008/11/05 16:48:52  oli
// prepared configuration of allowed @rttAssert tag syntax (rt-tester/353)
//
// Revision 1.53  2008/07/03 14:02:02  oli
// added: if license server answers ARC then an 'archive version marker' head the test log
// added: the tool version is printed to each test log
//
// Revision 1.52  2008/06/30 12:40:19  oli
// included: license check mechanism (based on communal included files)
//
// Revision 1.51  2008/05/16 08:30:36  emm
// fix syntax error which occurred in case of (VERBOSE_LEVEL>0) and (!defined RTT_CLUSTER)
//
// Revision 1.50  2008/04/07 16:02:51  bisanz
// completed and activated code parts for setting CPU affinity (CADUL)
// to be tested
//
// Revision 1.49  2008/04/07 13:22:23  oli
// added (guarded) experimental code parts for setting CPU affinity
//
// Revision 1.48  2007/11/22 18:49:54  bisanz
// added blocking of signal SIGCHLD, guarded by pre-processor define
// RTT_BLOCK_SIGCHLD
// useful on spawning processes from test procedure, e.g. by system() call
// --> otherwise, test procedure would exit
//
// Revision 1.47  2007/08/21 08:02:04  oli
// added macro RTT_INCLUDE_THIS_LINE_BEFORE_MAIN:
// ensure that RT-Tester executables open stdout, stderr as binary streams
// (no DOS line breaks in MinGW build)
//
// Revision 1.46  2007/08/06 14:10:22  oli
// prepared introduction of (compiler) configuration field FAMILY
// new: mingw family
// prepared build on Win32/Cygwin that compiles executables with mingw
// (without reference to cygwin1.dll)
//
// Revision 1.45  2007/05/29 08:09:59  oli
// RTTstartup.c: added uint32_t getClusterNode(){ return 0; }
// this fixes (rt-tester/240)
//
// Revision 1.44  2007/04/26 07:00:03  oli
// added (silly, but harmless) code in order to avoid Cadul compiler warnings
//
// Revision 1.43  2007/04/20 08:54:10  oli
// use type cast to avoid compiler warnings (g++ (GCC) 4.1.2 20061115)
//
// Revision 1.42  2007/04/19 11:53:16  oli
// added int casts in order to avoid compile warnings
//
// Revision 1.41  2007/04/19 11:50:56  oli
// robustness: do not declare variable inside for() construct
// (not allowed with gcc (GCC) 4.0.2 20050901)
// added int casts in order to avoid compile warnings
//
// Revision 1.40  2007/04/19 09:32:05  jp
// Mask RT-Signals SIGRTMIN..SIGRTMAX for RT-Tester
//
// Revision 1.39  2007/04/18 12:09:53  oli
// removed compile warnings by int casts for format strings;
// declare rttWaitLWPs, if appropriate
//
// Revision 1.38  2007/03/20 09:18:56  oli
// robustness: also check for rttContext_rttTestRunning != 0
// after call to rttCreateChannelMap() (rt-tester/220,223)
//
// Revision 1.37  2007/03/19 18:03:52  oli
// fixed @init:@rttstopTest (rt-tester/220):
//
// If after the @INIT phase, rttContext_rttTestRunning == 0,
// then do not call rttCreateChannelMap(), for it might crash.
//
// Revision 1.36  2007/01/11 15:03:28  hjficker
// Fixed compiler warnings
//
// Revision 1.35  2007/01/10 14:55:50  hjficker
// Install lwpBacktraceSignalHandler() for error signals also for the
// control thread.
//
// Revision 1.34  2007/01/10 13:54:43  hjficker
// Fix for RTT problem report 207: When the lwpSignalHandler is called from
// the control thread, it calls rttSignalHandler instead of executing the LWP
// related signal handling code.
//
// Revision 1.33  2006/12/13 17:29:33  dahlweid
// Fixed initialisation of LWPs in pthread configuration
//
// Revision 1.32  2006/10/13 15:33:32  chref
// adjustments necessary for dynamic AM handling:
// - always have 512 entries in AM_context
// - used-flag in AM_context entries (in order to find free entries)
// - always have 512 entries in am_id_list, am_start_list and am_prio_list
// - keep "volatile" attribute when passing pointers to LWP_context entries
//   to functions
//
// Revision 1.31  2006/08/08 11:07:40  chref
// load symbols for backtrace on test startup, because loading can fail
// if performed when memory/stack has already been corrupted
//
// Revision 1.30  2006/07/03 13:14:34  oli
// fixed: faulty uses of printf(stderr, "..." ) --> fprintf(stderr, "...")
//
// Revision 1.29  2006/06/27 07:57:49  dahlweid
// Logging to console is now generated in the RT-Tester typical timestamp format.
//
// Revision 1.28  2006/03/03 16:27:23  oli
// fixed: "gethostname" for cygwin (disabled for _CADUL)
//
// Revision 1.27  2006/02/23 09:21:49  dahlweid
// Replaced rtt-test-case.pid by rtt-test-case.<hostname>.pid
//
// Revision 1.26  2006/01/16 15:51:05  oli
// replaced all occurrences of fopen: "w", "r":
//    fopen(..., "r")   --> fopen(..., "rb")
//    fopen(..., "w")   --> fopen(..., "wb")
//
// Revision 1.25  2005/11/09 15:54:44  dahlweid
// - Added variables rttIgnoreSignal_<signal>, which are initialised from
//   RTTcontext.c and allow to disable signal handlers for the specified signals
//   with compile switches for each test procedure.
// - Fixed C++ compile problem of rttHaveDebugMode.
//
// Revision 1.24  2005/09/21 14:47:25  chref
// marked global variables that get changed in different threads as "volatile"
//
// Revision 1.23  2005/08/18 19:37:55  walsen
// make rtt-test-case templates compile with cadul
//
// Revision 1.22  2005/08/12 15:55:02  walsen
// further cad-ul adjustments
//
// Revision 1.21  2005/08/10 06:59:29  jp
// First version of the TCP/IP cluster communication,
// as an configurable alternative. Not yet tested, but note that
// changes will only become visible if you configure with
//     --enable-tcpcluster
//
// Revision 1.20  2005/07/14 16:17:18  jp
// Completion of startup protocol with channel subscription in cluster
//
// Revision 1.19  2005/06/24 08:44:01  dahlweid
// Replaced mring references by generic cluster function calls
//
// Revision 1.18  2005/05/16 13:45:46  jp
// Typos corrected and comments changed
//
// Revision 1.17  2005/04/08 08:46:28  jp
// Multi-threaded cluster nodes - first extensions
//
// Revision 1.16  2005/02/08 11:22:06  dahlweid
// - Included patches from Stefan Walsen, who ported RT-Tester 6.0 MacOSX
//   (Darwin).
// - Use autoconf macros for pth checks.
//
// Revision 1.15  2004/11/18 08:50:04  dahlweid
// Support Verified Start-Stop-Controller from RT-Tester 6.x. On test start the
// clusterd is informed about test startup; on test completion the flashing
// indication is reset via clusterd.
//
// Revision 1.14  2004/08/04 14:53:21  marlex
// functions comments inserted/changed to coding standard
// for more information (and TODO) see in TODO_for_coding_standard.txt
//
// Revision 1.13  2004/08/02 14:43:01  marlex
// header added/changed for coding standard in *.l, *.y, *.c, *.h files
//
//
//
///////////////////////////////////////////////////////////////
*/
