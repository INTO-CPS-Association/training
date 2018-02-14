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
//     $Date: 2012-02-16 17:11:37 $
//
//--------------------------------------------------------------------
//
// Produkt: RTT - Abstract Machine Library
//
//--------------------------------------------------------------------
//
// File Identification:
//
// $RCSfile: RTTschedule.h,v $
//
// $Header: /home/repository/VVTCVS/CVS/rtt-swi/tpl/RTTschedule.h,v 1.8 2012-02-16 17:11:37 oli Exp $
//
// $Revision: 1.8 $
//
// First edition by: Uwe Schulze
// Last update by    $Author: oli $
//
//--------------------------------------------------------------------
//
// Description: 
//
//--------------------------------------------------------------------*/

#ifndef _RTT_SCHEDULE_H_
#define _RTT_SCHEDULE_H_

#ifdef __cplusplus
extern "C" {
#endif


#include "RTTcontext.h"

/************************************************************
 *  Defines and Type definitions
 ************************************************************/

// maximum suspend time of scheduler loop in nano seconds
#define RTT_SCHEDULING_SUSPEND_TIME 0 // 50 ms

/************************************************************
 *  functions
 ************************************************************/

/**
 * create clones for LWPs
 *
 * @TODO missing documentation
 * @return
 */
int rttCreateLWPs();

#ifndef _CYGWIN

/**
 * wait for LWPs to terminate
 *
 * @TODO missing documentation
 * @return
 */
int rttWaitLWPs();

#endif

/**
 * yield function for AMs
 *
 * @TODO missing documentation
 * @param am_id
 * @return
 */
int rtt_yield(uint32_t am_id);


#ifdef __cplusplus
          } // terminate extern "C" 
#endif

#endif /* _RTT_SCHEDULE_H_ */

/*//////////////////////////////////////////////////////////////
//
// Change History:
//
// $Log: not supported by cvs2svn $
// Revision 1.7  2009/08/04 13:55:44  jp
// New company address inserted into headers
//
// Revision 1.6  2005/08/17 18:00:25  jp
// RT-Tester User Threads, first version
//
// Revision 1.5  2005/08/04 16:11:31  oli
// guarded header files with the block
//    #ifdef __cplusplus
//     extern "C" {
//     #endif
// in order to allow for linking with C++ Style name mangling.
// (changed typename -> type_name)
//
// Revision 1.4  2004/08/04 14:53:21  marlex
// functions comments inserted/changed to coding standard
// for more information (and TODO) see in TODO_for_coding_standard.txt
//
// Revision 1.3  2004/08/02 14:43:01  marlex
// header added/changed for coding standard in *.l, *.y, *.c, *.h files
//
// Revision 1.2  2003/12/01 13:27:27  oli
// - fixed "unknown conversion type character `]' in format" problem (./swi-tests/test5)
// - removed shift/reduce and shift/shift conflicts in .y files
// - fixed or commented (in case of unmatched .l rules) warnings during make
//
// Revision 1.1  2003/11/20 11:34:37  dahlweid
// - Replaced -unit by -test in scripts.
// - Removed _swi suffix in templates.
// - Renamed director_unit to rtt-test-case.
// - Renamed rtt-unit-types.h/-config.h to rtt-test-types.h/-config.h
//
// Revision 1.7  2003/11/11 14:29:49  uwe
// moved rtt_lwp_status_t to ctxlib
//
// Revision 1.6  2003/11/04 13:17:14  uwe
// - Added commandline paraqmeter -h (helpmessage) and test duration time.
// - Added duration time handling and nanosleep function.
//
// Revision 1.5  2003/11/04 08:45:19  uwe
// added _CYGWIN support
//
// Revision 1.4  2003/11/03 13:57:22  uwe
// Added LWP status information.
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
