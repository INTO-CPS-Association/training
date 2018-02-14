/** @file
//////////////////////////////////////////////////////////////////////
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
// Produkt: RTT - Abstract Machine Library
//
//--------------------------------------------------------------------
//
// File Identification:
//
// $RCSfile: RTTschedule.c,v $
//
// $Header: /home/repository/VVTCVS/CVS/rtt-swi/tpl/RTTschedule.c,v 1.74 2015-08-28 15:07:25 oli Exp $
//
// $Revision: 1.74 $
//
// First edition by: Uwe Schulze
// Last update by    $Author: oli $
//
//--------------------------------------------------------------------
//
// Description: 
//
--------------------------------------------------------------------*/

/************************************************************
 *  includes
 ************************************************************/

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include "rtt_test_config.h"

#ifndef _CADUL
#include <sys/types.h>
#include <unistd.h>
#ifndef _MINGW
#include <sys/wait.h>
#include <sched.h>
#endif /* _MINGW */
#endif /* _CADUL */

#include <stdio.h>
#include <time.h>
#include <errno.h>
#include <signal.h>

#ifdef _LINUX
#include <ucontext.h>
#endif

#include "RTTcontext.h"
#include "RTTschedule.h"

#include "rtt_test_config.h"
#include "rtt_test_types.h"

#include "rttctxlib.h"

#ifdef _RTTHLRT
#include "rtthlrt.h"
#endif

/************************************************************
 *  Defines and Type definitions
 ************************************************************/

// stack size for clone processes
#define CHILD_STACK_SIZE 4*1024*1024


/************************************************************
 *  Global variables
 ************************************************************/
extern const bool_t rttHaveDebugMode;
bool_t   rttActiveWaitForLwps = TRUE;

/************************************************************
 *  functions
 ************************************************************/

/**
 * Function getClusterNode. (defined in RTTstartup.c)
 */
uint32_t getClusterNode();

/**
 * declaration of LWP_main
 */
#ifdef RTT_PTHREAD
void *
#else
int
#endif
LWP_main(void *args);


/**
 * Function rttSignalHandler, implemented in RTTstartup.c.
 * This function defines the actions for the different signals that can be
 * sent to the test process.
 *
 * @TODO missing documentation of parameters
 * @param sig
 */
void rttSignalHandler( int sig );


/**
 * Signal handler that prints a backtrace
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
    ) {
    RTT_TID_t lwp_pid = RTT_GETTID();
    int   lwp_id;
    long int current_am_id = -1L;

    /* find LWP an AM */
    for (lwp_id = 0; lwp_id < RTT_CONFIG_NUM_LWP; lwp_id++) {
	if (RTT_TID_EQUAL(LWP_context[lwp_id].lwp_pid, lwp_pid))
	    break;
    }

    if (lwp_id == RTT_CONFIG_NUM_LWP) {
	// Check if it is the control thread
	if (RTT_TID_EQUAL(rttControlThreadId, lwp_pid))	{
	    fprintf(stderr,
		    "Control thread: exiting on signal %d\n",
		    sig);
	} else {
	    // unable to determine LWP id
	    fprintf(stderr,
		    "lwpBacktraceSignalHandler: cannot determine LWP context for PID %lu.\n"
		    "Exiting with signal %d.\n",
		    (unsigned long)lwp_pid, sig);
	}
    } else {
	current_am_id = LWP_context[lwp_id].current_am;
	if (RTT_CONFIG_NUM_CNODE > 0) {
	    fprintf(stderr, "CNODE %lu LWP %ld AM %ld: exiting on signal %d\n",
		    (long unsigned int)getClusterNode(), (long int)lwp_id, (long int)current_am_id, sig);
	} else {
	    fprintf(stderr, "LWP %ld AM %ld: exiting on signal %d\n",
		    (long int)lwp_id, (long int)current_am_id, sig);
	}
    }

    // print backtrace
#ifdef _LINUX
#if defined(__i386__)
    rttctx_print_backtrace(TRUE, (void *)((ucontext_t *)ctx)->uc_mcontext.gregs[REG_EIP]);
#elif defined(__x86_64__)
    rttctx_print_backtrace(TRUE, (void *)((ucontext_t *)ctx)->uc_mcontext.gregs[REG_RIP]);
#elif defined(__PPC__)
    printf("*** TODO: backtrace ***\n");
#else
#error Missing support for this architecture, please fix
#endif
#else
    rttctx_print_backtrace(TRUE, NULL);
#endif

    // set test running flag to 0 (stop test)
    rttContext_rttTestRunning = 0;
	    
    // set lwp status to aborted
    if (lwp_id != RTT_CONFIG_NUM_LWP)
	LWP_context[lwp_id].lwp_status = rtt_lwp_aborted;

    // exit
    exit(sig);
}



/**
 * Signal handler ignoring signal INT
 *
 * @TODO missing documentation
 *
 * @param sig
 */
void lwpSignalHandler(int sig) {
    RTT_TID_t lwp_pid = RTT_GETTID();
    int   lwp_id, am_id, i;
    long int current_am_id = -1L;
    char *tmp;

    /* Bugfix for RT Tester bug report 207: When using pthreads, all
       threads (that includes the control thread) share a single
       signal handler. So this signal handler gets also called when
       the control thread receives a signal. So if the thread ID
       belongs to the control thread, switch to the signal handler for
       the control thread. This feels a bit "hackish", but it seems to
       work. */
    if (RTT_TID_EQUAL(rttControlThreadId, lwp_pid)) {
	rttSignalHandler(sig);
	return;
    }

    //printf("lwpSignalHandler: searching for lwp_pid %d\n",lwp_pid);
    for (lwp_id=0; lwp_id<RTT_CONFIG_NUM_LWP; lwp_id++) {
	if (RTT_TID_EQUAL(LWP_context[lwp_id].lwp_pid, lwp_pid))
	    break;
    }

    if (lwp_id == RTT_CONFIG_NUM_LWP) {
	// unable to determine LWP id
	fprintf(stderr,
		"lwpSignalHandler: cannot determine LWP context for PID %lu.\n"
		"Exiting with signal %d.\n",
		(unsigned long)lwp_pid,
		sig);
	exit(sig);
    } else {
	current_am_id = LWP_context[lwp_id].current_am;
    }

    signal(sig, &lwpSignalHandler);

    switch (sig) {
#if (! defined _CADUL) && (! defined _MINGW)
	case SIGHUP:
	case SIGQUIT:
	case SIGPIPE:
	case SIGALRM:
#endif
	case SIGABRT:
	case SIGTERM:
	    // set test running flag to 0 (stop test)
	    rttContext_rttTestRunning = 0;

	    // output message on stderr and in log of current AM.
	    fprintf(stderr, "LWP %d: caught signal %d\n", lwp_id, sig);
	    tmp = (char*)malloc(1024);

	    for (i=0; i<LWP_context[lwp_id].num_of_ams; i++) {
		am_id = LWP_context[lwp_id].am_id_list[i];
		sprintf(tmp,
			"ABORTED IN TEST STEP: %ld\n"
			"CAUSE               : CAUGHT SIGNAL %d\n",
			AM_context[am_id].test_step,
			sig);
		rttctx_set_verdict_AM(am_id, RTTverdict_error, tmp);
		fflush(0);
	    }
	    // set lwp status to aborted
	    LWP_context[lwp_id].lwp_status = rtt_lwp_aborted;
	    break;

	case SIGSEGV:
	case SIGILL:
	case SIGFPE:
	    // print error message to standard error
	    fprintf(stderr, "LWP %d: exiting on signal %d\n", lwp_id, sig);

	    // set test running flag to 0 (stop test)
	    rttContext_rttTestRunning = 0;

	    // set lwp status to aborted
	    LWP_context[lwp_id].lwp_status = rtt_lwp_aborted;

	    // exit
	    exit(sig);
	    break;
	default:
	    fprintf(stderr, "LWP %d: ignoring signal %d\n", lwp_id, sig);
	    break;
    }
    
    if(current_am_id < 0){ // -- this is to avoid compiler warnings ---
      return;
    }
}

/**
 * create LWPs for AM scheduling
 *
 * @return Always 0
 *
 */
int rttCreateLWPs() {

#if ( (defined (_LINUX) || defined (RTT_PTHREAD) ) && ! defined (_LINUX_DEBUG)  )
    uint32_t idx,cnode_id;
    int32_t lwp_idx;
#ifndef RTT_PTHREAD
    void *lwp_stack_mem;
    void *lwp_stack;
    uint32_t flags;
#endif

    uint32_t numLwpsOnThisNode = 1;
    uint32_t numCpusOnThisNode = rttctxGetAvailableCpus();


    // check for cluster node configuration
    if (RTT_CONFIG_NUM_CNODE > 0) {
	// calculate cluster node id
	cnode_id = getClusterNode();
	numLwpsOnThisNode = CNODE_context[cnode_id].num_of_lwps;

	// Use active wait in LWP schedulers with positive cycletime,
	// if we have at least as many CPUs as LWPs
	rttActiveWaitForLwps = (numLwpsOnThisNode <= numCpusOnThisNode);

	// create LWPs
	for (lwp_idx=0; 
	     lwp_idx<CNODE_context[cnode_id].num_of_lwps;
	     lwp_idx++) {
	    idx = CNODE_context[cnode_id].lwp_id_list[lwp_idx];
	    // set lwp status to rtt_lwp_initialising
	    LWP_context[idx].lwp_status = rtt_lwp_initialising;
            // The first LWP on this cluster node shall perform
            // the cluster communication for this node:
	    LWP_context[idx].doClusterCommunication = ( lwp_idx == 0 );

            // The first LWP on this cluster node shall perform
            // the writing of captured data
	    LWP_context[idx].doWriteCaptureData = ( lwp_idx == 0 );

            // Create a cluster tx buffer, to be used for storing
            // references to channel data to be communicated to other
            // cluster nodes by the LWP responsible for cluster
            // communication
	    LWP_context[idx].rttCnlClusterTxBuffer 
		= (rttCnlClusterTxBuffer_t *)calloc(1,sizeof(rttCnlClusterTxBuffer_t));

	    // initialise capture buffers
	    LWP_context[idx].rttCnlCaptureBuffer =
		LWP_context[idx].rttVecCaptureBuffer = NULL;

	    // creating lwp
	    LWP_context[idx].lwp_pid = (RTT_TID_t)-1L;

#if defined(RTT_PTHREAD)
	    pthread_create((pthread_t *)&LWP_context[idx].lwp_pid,
			   NULL, LWP_main, (void*)&(LWP_context[idx]));
#else
	    // define flag for cloning
	    flags=CLONE_VM | CLONE_FILES; //| CLONE_SIGHAND ; //| CLONE_THREAD;
	    // allocate stack memory for lwp
	    lwp_stack_mem = (void*) malloc(CHILD_STACK_SIZE);
	    lwp_stack
              = (uint8_t*)lwp_stack_mem + CHILD_STACK_SIZE - sizeof(void *);

	    LWP_context[idx].lwp_pid = 
		clone(&LWP_main,lwp_stack,flags,
		      (void*)&(LWP_context[idx]));
#endif
	}
    } else {
	// create LWPs
	numLwpsOnThisNode = RTT_CONFIG_NUM_LWP;


	// Use active wait in LWP schedulers with positive cycletime,
	// if we have at least as many CPUs as LWPs
	rttActiveWaitForLwps = (numLwpsOnThisNode <= numCpusOnThisNode);


	for (idx=0; idx<RTT_CONFIG_NUM_LWP; idx++) {
	    // set lwp status to rtt_lwp_initialising
	    LWP_context[idx].lwp_status = rtt_lwp_initialising;

            // The first LWP shall perform the writing of captured data
	    LWP_context[idx].doWriteCaptureData = ( idx == 0 );

	    // initialise capture buffers
	    LWP_context[idx].rttCnlCaptureBuffer =
		LWP_context[idx].rttVecCaptureBuffer = NULL;

	    // creating lwp
	    LWP_context[idx].lwp_pid = (RTT_TID_t)-1L;

#if defined(RTT_PTHREAD)
	    pthread_create((pthread_t *)&LWP_context[idx].lwp_pid,
			   NULL, LWP_main, (void*)&(LWP_context[idx]));
#else
	    // define flag for cloning
	    flags=CLONE_VM | CLONE_FILES; //| CLONE_SIGHAND ; //| CLONE_THREAD;
	    // allocate stack memory for lwp
	    lwp_stack_mem = (void*) malloc(CHILD_STACK_SIZE);
	    lwp_stack
              = (uint8_t*)lwp_stack_mem + CHILD_STACK_SIZE - sizeof(void *);

	    LWP_context[idx].lwp_pid = 
		clone(&LWP_main,lwp_stack,flags,
		      (void*)&(LWP_context[idx]));
#endif
	}
    }

#else

    uint32_t numLwpsOnThisNode = 1;
    uint32_t numCpusOnThisNode = 1;


    // Use active wait in LWP schedulers with positive cycletime,
    // if we have at least as many CPUs as LWPs
    rttActiveWaitForLwps = (numLwpsOnThisNode <= numCpusOnThisNode);


    LWP_context[0].lwp_pid = RTT_GETTID();

    // The first LWP shall perform the writing of captured data
    LWP_context[0].doWriteCaptureData = TRUE;

    // initialise capture buffers
    LWP_context[0].rttCnlCaptureBuffer =
	LWP_context[0].rttVecCaptureBuffer = NULL;
    
    // clone function not supported for WIN32!
    // only one process is running
    LWP_main((void*)&(LWP_context[0]));

#endif

    return 0;
}

#if ( (defined(_LINUX) || defined(RTT_PTHREAD)) && ! defined (_LINUX_DEBUG) )

/**
 * @TODO missing documentation
 *
 * @return
 */
int rttWaitLWPs() {
    RTT_TID_t lwp_pid;
#ifndef RTT_PTHREAD
    pid_t pid;
    int status;
#endif
    uint32_t idx,cnode_id;
    int32_t lwp_idx;

    // check for cluster node configuration
    if (RTT_CONFIG_NUM_CNODE > 0) {
	// calculate cluster node id
	cnode_id = getClusterNode();
	// wait for child processes to terminate
	for (lwp_idx=0; 
	     lwp_idx<CNODE_context[cnode_id].num_of_lwps;
	     lwp_idx++) {
	    idx = CNODE_context[cnode_id].lwp_id_list[lwp_idx];
	    lwp_pid = LWP_context[idx].lwp_pid;
	    //printf("waiting for pid %d\n",lwp_pid);
#ifdef RTT_PTHREAD
	    pthread_join(lwp_pid, NULL);
#else
#ifdef _LINUX
	    pid = waitpid(lwp_pid,&status,__WALL);
#else
	    pid = waitpid(lwp_pid,&status,0);
#endif
	    if (pid == -1)
		perror("rttWaitLWPs: ");
	    else {
		if ((WIFEXITED(status)) && (WEXITSTATUS(status) == 0)) 
		    printf("clone %d exited normally\n",pid);
		else if (WIFEXITED(status))
		    printf("clone %d exit status:%d\n",
			   pid,WEXITSTATUS(status));
	    
		if (WIFSIGNALED(status)) {
		    printf("clone %d signal %d was not caught\n",
			   pid,WTERMSIG(status));
		}
	    }
#endif
	}
    } else {
	// wait for child processes to terminate
	for (idx=0; idx<RTT_CONFIG_NUM_LWP; idx++) {
	    lwp_pid = LWP_context[idx].lwp_pid;
	    //printf("waiting for pid %d\n",lwp_pid);
#ifdef RTT_PTHREAD
	    pthread_join(lwp_pid, NULL);
#else
#ifdef _LINUX
	    pid = waitpid(lwp_pid,&status,__WALL);
#else
	    pid = waitpid(lwp_pid,&status,0);
#endif
	    if (pid == -1)
		perror("rttWaitLWPs: ");
	    else {
		if ((WIFEXITED(status)) && (WEXITSTATUS(status) == 0)) 
		    printf("clone %d exited normally\n",pid);
		else if (WIFEXITED(status))
		    printf("clone %d exit status:%d\n",
			   pid,WEXITSTATUS(status));
	    
		if (WIFSIGNALED(status)) {
		    printf("clone %d signal %d was not caught\n",
			   pid,WTERMSIG(status));
		}
	    }
#endif
	}
    }
    return 0;
}

#endif

/*//////////////////////////////////////////////////////////////
//
// Change History:
//
// $Log: not supported by cvs2svn $
// Revision 1.73  2012/10/31 08:49:11  emm
// fix log in comment of previous change, since it produced
// warnings at compile time (comment within comment)
//
// Revision 1.72  2012/10/29 13:01:01  oli
// re-organised TRUE/FALSE definition:
// - attempt to correct faulty definitions
//   (FALSE must always be 0, TRUE != 0)
// - re-arranged src/ *.c template files, such that RTTcontext.h is included
//   before other rtt* header files
//   (this allows for true/false corrections)
//   Note that this does not affect the *.rts file compilation.
//
// Revision 1.71  2012/02/16 17:11:37  oli
// adjusted company postal address (to Am_Fallturm_1)
//
// Revision 1.70  2011/03/23 07:48:46  oli
// cosmetics: align set_verdict_TESTERROR() output string (terminating newline)
// avoid using the scratchpad of the AM (again)
//
// Revision 1.69  2011/03/07 18:26:44  bisanz
// minor: fixed typo
//
// Revision 1.68  2009/08/04 13:55:44  jp
// New company address inserted into headers
//
// Revision 1.67  2009/06/30 13:27:26  bisanz
// Using explicitly "uint8_t*" instead of "void*" for pointer arithmetics.
// This should fix PR rt-tester/429, parts (4,5,6).
//
// Revision 1.66  2007/08/09 09:25:30  oli
// guarded _GNU_SOURCE: only define, if not defined already
//
// Revision 1.65  2007/08/06 14:10:22  oli
// prepared introduction of (compiler) configuration field FAMILY
// new: mingw family
// prepared build on Win32/Cygwin that compiles executables with mingw
// (without reference to cygwin1.dll)
//
// Revision 1.64  2007/07/18 09:16:06  chref
// PS3 fixes
//
// Revision 1.63  2007/06/26 08:29:27  oli
// revoked previous change, reverted to V-1.61
//
// Revision 1.61  2007/04/26 07:00:03  oli
// added (silly, but harmless) code in order to avoid Cadul compiler warnings
//
// Revision 1.60  2007/04/20 08:17:24  oli
// robustness: use type cast in order to avoid compiler warnings
//
// Revision 1.59  2007/04/18 12:09:53  oli
// removed compile warnings by int casts for format strings;
// declare rttWaitLWPs, if appropriate
//
// Revision 1.58  2007/02/21 17:28:51  hjficker
// Bugfix: Added missing declaration of numLwpsOnThisNode and
// numCpusOnThisNode when both RTT_PTHREAD and _LINUX are not defined
// (e.g. on CAD-UL)
//
// Revision 1.57  2007/02/19 16:38:44  jp
// Fixes for synchronised wait for initial timetick
//
// Revision 1.56  2007/02/18 09:05:00  jp
// Function rttCreateLWPs(): set rttActiveWaitForLwps-flag to true or false,
// depending on whether numLwpsOnThisNode <= numCpusOnThisNode holds or not.
//
// Revision 1.55  2007/01/11 15:56:06  hjficker
// Use macro RTT_TID_EQUAL(x, y)
//
// Revision 1.54  2007/01/11 15:03:28  hjficker
// Fixed compiler warnings
//
// Revision 1.53  2007/01/10 14:28:19  hjficker
// Added more comments why the other signal handler is called from within
// the signal handler
//
// Revision 1.52  2007/01/10 13:54:43  hjficker
// Fix for RTT problem report 207: When the lwpSignalHandler is called from
// the control thread, it calls rttSignalHandler instead of executing the LWP
// related signal handling code.
//
// Revision 1.51  2006/12/14 08:20:46  dahlweid
// Move CLONE flags into the correct source code section.
//
// Revision 1.50  2006/12/13 16:56:36  chref
// pthread patch:
// - if pthread library is available, use pthread_create to create new LWPs
//   instead of clone
// - should also work on cygwin (not yet tested)
// - use configure switch --enable-pthread/--disable-pthread to
//   override automatic detection
//
// Revision 1.49  2006/09/15 16:13:12  chref
// make backtraces work on x86_64 platform
//
// Revision 1.48  2006/08/02 13:07:53  chref
// output cluster node only if cluster nodes available in config
//
// Revision 1.47  2006/08/02 12:53:00  chref
// also print cluster node in signal handler
//
// Revision 1.46  2006/08/02 10:10:27  chref
// make sure the LWP context can be determined from signal handlers
// in single-LWP mode
//
// Revision 1.45  2006/08/02 09:21:59  chref
// completed backtrace printing: resolve symbol and line number
// information on Cygwin and Linux
//
// Revision 1.44  2006/07/28 15:16:37  chref
// added a new signal handler that prints a backtrace on
// "bad" signals like SIGSEGV etc. (currently Linux only)
//
// Revision 1.43  2005/11/09 17:30:30  walsen
// initialize lwp_pid so that ctrl-c works on darwin
//
// Revision 1.42  2005/11/09 15:54:44  dahlweid
// - Added variables rttIgnoreSignal_<signal>, which are initialised from
//   RTTcontext.c and allow to disable signal handlers for the specified signals
//   with compile switches for each test procedure.
// - Fixed C++ compile problem of rttHaveDebugMode.
//
// Revision 1.41  2005/10/07 13:40:56  chref
// added logreplay library implementing the capture/replay feature
//
// Revision 1.40  2005/08/25 15:04:15  walsen
// revert unintentional check-in
//
// Revision 1.39  2005/08/25 14:47:38  walsen
// fix cygpath call
//
// Revision 1.38  2005/08/18 19:37:55  walsen
// make rtt-test-case templates compile with cadul
//
// Revision 1.37  2005/08/17 18:00:25  jp
// RT-Tester User Threads, first version
//
// Revision 1.36  2005/08/15 07:41:59  bruening
//
// Fix VERIFIED-RTT-0061.Initialize LWP_context[idx].lwp_pid to -1 also
// for the case cluster node configuration
//
// Revision 1.35  2005/08/10 15:13:30  bruening
// Fix VERIFIED-RTT-0061.Initialize LWP_context[idx].lwp_pid to -1
// for detecting successfull return from function clone
//
// Revision 1.34  2005/08/05 17:23:55  oli
// adjusted: source files for compilation with C++ compiler
//           (added type casts, removed warnings)
//
// Revision 1.33  2005/04/19 16:52:09  jp
// Make debug mode visible via globa constant
//
// Revision 1.32  2005/04/17 21:12:20  jp
// Moved code from RTTschedule.c and RTTcontext.c templates
// to ctx-lib. This reduces the amount of code which has to
// be re-compiled for every test. Moreover, we prefer to hide
// our scheduling algorithms inside the ctx-lib, instead of showing
// the code in the templates.
//
// Revision 1.31  2005/04/17 16:26:47  jp
// - Perform frame cycle handling with active wait instead
//   of nanosleep().
// - Flush stdout only if at least one AM on the LWP writes
//   to stdout.
// - Store remaining frame cycle time in LWP context
// - Produce error message if frame cycle overrun occurs.
//
// Revision 1.30  2005/04/17 14:24:28  jp
// rttReceiveFromCnodes() to be called without lwp_id parameter
//
// Revision 1.29  2005/04/08 17:10:46  jp
// Write to console when start-of-test signal is there
//
// Revision 1.28  2005/04/08 16:58:29  jp
// After AM yields, LWP performs rttTxClusterData()
//
// Revision 1.27  2005/04/08 08:46:28  jp
// Multi-threaded cluster nodes - first extensions
//
// Revision 1.26  2005/02/08 11:22:06  dahlweid
// - Included patches from Stefan Walsen, who ported RT-Tester 6.0 MacOSX
//   (Darwin).
// - Use autoconf macros for pth checks.
//
// Revision 1.25  2004/11/30 17:01:13  chref
// Fixed determination of AM ID for Linux 2.6:  The PID can be identical
// for all cloned threads and therefore cannot be used to identify
// different LWPs.  Instead, the thread ID must be used.
//
// Revision 1.24  2004/10/25 08:16:53  chref
// removed race condition when using Linux 2.6.x with NPTL threads
//
// Revision 1.23  2004/08/04 14:53:21  marlex
// functions comments inserted/changed to coding standard
// for more information (and TODO) see in TODO_for_coding_standard.txt
//
// Revision 1.22  2004/08/02 14:43:01  marlex
// header added/changed for coding standard in *.l, *.y, *.c, *.h files
//
// Revision 1.21  2004/03/25 12:30:57  chref
// corrected check of return value of rtthlrt_{begin,end}_rt_process
//
// Revision 1.20  2004/03/24 15:28:21  dahlweid
// Add libhlrt to link flags, if hlrt extension is configured.
//
// Revision 1.19  2004/03/19 16:07:40  oli
// added CLONE_FILES to the flags of clone()
//       (fixes trouble with corrupted .log files in cluster mode)
//
// Revision 1.18  2004/03/06 17:28:33  jp
// Trigger rttTxTerminateTest() whenever an LWP is
// ready for termination.
//
// Revision 1.17  2004/03/06 17:11:06  jp
// New function rttTxTerminateTest() distributes test termination command in cluster
//
// Revision 1.16  2004/03/06 14:54:06  jp
// Receive data from other cluster nodes
//
// Revision 1.15  2004/03/05 16:03:58  uwe
// Adjustments to signal handling in startup process and clones.
//
// Revision 1.14  2004/03/05 15:40:33  oli
// removed: surplus arguments in AM intialisation (startup time is given via
//          RTTgs.start_time_epoch)
// fixed:   initialisation and finalisation of an abstract machine now happens
//          WHILE the clone is active:
//          (via functions rttctx_initialize_AM_runtime,
//                         rttctx_finalize_AM_runtime
//           that are called in the generated _INIT / _FINIT procedure of the
//           abstact machine, see scan_rts.y)
//
// Revision 1.13  2004/03/03 16:04:21  uwe
// Added initialisation and startup of cluster node LWPs.
//
// Revision 1.12  2004/02/16 15:52:44  dahlweid
// Damn Windows: uint32_t != unsigned int
//
// Revision 1.11  2004/02/16 15:40:44  dahlweid
// Removed compiler warning under Windows.
//
// Revision 1.10  2004/02/13 13:47:27  dahlweid
// Bugfix: context->current_am needs also to be set during
// initialisation. Otherwise all output will be generated for AM 0.
//
// Revision 1.9  2004/02/06 10:18:23  dahlweid
// Suppressed debugging output generated during test run.
//
// Revision 1.8  2004/02/05 16:20:44  dahlweid
// Install SIGTERM signal handler only for non-Windows systems.
//
// Revision 1.7  2004/02/05 15:11:54  dahlweid
// Flush data sent to stdout, if scheduler still has enough time to sleep.
//
// Revision 1.6  2004/01/22 12:49:19  dahlweid
// Added initialisation of local variable to avoid compiler warnings.
//
// Revision 1.5  2003/12/19 13:13:56  oli
// cleaned type of time_used (and calculations with it)
//
// Revision 1.4  2003/12/01 13:27:27  oli
// - fixed "unknown conversion type character `]' in format" problem (./swi-tests/test5)
// - removed shift/reduce and shift/shift conflicts in .y files
// - fixed or commented (in case of unmatched .l rules) warnings during make
//
// Revision 1.3  2003/11/26 09:35:23  uwe
// Bugfix in lwp_id calculation
//
// Revision 1.2  2003/11/24 09:58:06  dahlweid
// Disabled backtrace debugging code for Cygwin platform and enable it
// only if -D_LINUX_DEBUG is specified.
//
// Revision 1.1  2003/11/20 11:34:37  dahlweid
// - Replaced -unit by -test in scripts.
// - Removed _swi suffix in templates.
// - Renamed director_unit to rtt-test-case.
// - Renamed rtt-unit-types.h/-config.h to rtt-test-types.h/-config.h
//
// Revision 1.25  2003/11/18 14:40:40  rscholz
// Improve debugging information in lwpSignalHandler().
//
// Revision 1.24  2003/11/17 16:23:32  uwe
// printf format adjusted
//
// Revision 1.23  2003/11/17 16:18:35  dahlweid
// Changed output for maximum cycle time violation.
//
// Revision 1.22  2003/11/17 14:59:33  uwe
// Removed memory leak.
//
// Revision 1.21  2003/11/12 14:11:15  uwe
// - Added debugging support (single LWP) with compile flag -D_LINUX_DEBUG.
// - Bugfix in LWP_process.
//
// Revision 1.20  2003/11/12 10:28:44  uwe
// Cycletimes now use the global timescale definition.
//
// Revision 1.19  2003/11/11 16:07:29  uwe
// max. cycle time violation output changed from stderr to logFormat.
//
// Revision 1.18  2003/11/11 14:29:23  uwe
// Added setting of current AM during scheduling of an LWP.
//
// Revision 1.17  2003/11/11 13:00:36  dahlweid
// Corrected spelling mistake.
//
// Revision 1.16  2003/11/05 10:45:23  dahlweid
// removed debugging output.
//
// Revision 1.15  2003/11/05 08:07:26  uwe
// Added LWP cycletime to scheduling algorithm.
// Added checking of max. cycletime for AMs.
//
// Revision 1.14  2003/11/04 13:23:20  uwe
// removed debug output
//
// Revision 1.13  2003/11/04 13:17:14  uwe
// - Added commandline paraqmeter -h (helpmessage) and test duration time.
// - Added duration time handling and nanosleep function.
//
// Revision 1.12  2003/11/04 08:48:32  dahlweid
// Tracking nanosleep threading problem.
//
// Revision 1.11  2003/11/04 08:45:19  uwe
// added _CYGWIN support
//
// Revision 1.10  2003/11/03 14:26:26  uwe
// exchanged sleep(1) in scheduler startup by rttWaitMilliseconds(50)
//
// Revision 1.9  2003/11/03 13:57:22  uwe
// Added LWP status information.
//
// Revision 1.8  2003/10/31 16:45:45  uwe
// added termination of AMs
//
// Revision 1.7  2003/10/31 11:50:23  uwe
// adjusted output of signal handler
//
// Revision 1.6  2003/10/31 11:11:46  uwe
// bugfix in finit function.
//
// Revision 1.5  2003/10/31 10:00:20  dahlweid
// Elimimated compiler warning.
//
// Revision 1.4  2003/10/30 13:23:38  uwe
// - removed rtttypes.h include
// - INT64 changed to uint64_t
//
// Revision 1.3  2003/10/30 06:39:34  uwe
// try to cleanup director_swi.c and RTTschedule_swi.{c,h} files to create a
// library for rttproc.
//
// Revision 1.2  2003/10/29 11:23:33  uwe
// added rtt_yield(uint32_t am_id) function
//
// Revision 1.1  2003/10/29 07:02:43  uwe
// added new _swi templates
//
//
///////////////////////////////////////////////////////////////
*/
