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
//     $Date: 2012-04-26 15:35:04 $
//
//--------------------------------------------------------------------
//
// Product: RT-Tester template for test configuration / initialisation
//
//--------------------------------------------------------------------
//
// File Identification: 
//
// $RCSfile: RTTcontext_private.h,v $
//
// $Header: /home/repository/VVTCVS/CVS/rtt-swi/tpl/RTTcontext_private.h,v 1.4 2012-04-26 15:35:04 oli Exp $
//
// $Revision: 1.4 $
//
// First edition by: Stefan Bisanz
// Last update by    $Author: oli $
//
//--------------------------------------------------------------------
//
// Description: 
//   Private context parts for test procedures, for use with RTTcontext.c.
//
// -------------------------------------------------------------------
// @TABLE OF CONTENTS:                 [TOCD: 16:23 16 Mär 2009]
//
//      [0.1] DEFINITIONS
//      [0.2] global variables
//      [0.3] Function declarations
// -------------------------------------------------------------------*/


#ifndef _RTTCONTEXT_PRIVATE_H
#define _RTTCONTEXT_PRIVATE_H

#ifdef __cplusplus
extern "C" {
#endif

// ///////////////////////////////////////////////
// [0.1] DEFINITIONS
// ///////////////////////////////////////////////

/**
 * The following defined are substituded by test case specific values during
 * the test case generation.
 */

/** @def RTT_CONFIG_NUM_CHANNEL_FILES 
  * The number of channel files relevant for a test case.
  * The value of this define is created during the test case generation. */
#define RTT_CONFIG_NUM_CHANNEL_FILES 0

/** @def RTT_CONFIG_NUM_CHANNEL_PER_LWP 
  * The maximum number of channels of an LWP of a test case.
  * The value of this define is created during the test case generation. */
#define RTT_CONFIG_NUM_CHANNEL_PER_LWP 0

// ///////////////////////////////////////////////
// [0.2] global variables
// ///////////////////////////////////////////////

/** @var rttCnlSpec_t *cnlPtr[RTT_CONFIG_NUM_LWP][RTT_CONFIG_NUM_CHANNEL_FILES+1]
 *  This array contains the channel spcification structures of all channel
 *  files.
 */
extern rttCnlSpec_t *cnlPtr[RTT_CONFIG_NUM_LWP][RTT_CONFIG_NUM_CHANNEL_FILES+1];

/** @var const char *rttAllUsedChannels[RTT_CONFIG_NUM_LWP+1][RTT_CONFIG_NUM_CHANNEL_PER_LWP+1];
 *  This global variable holds a list of all channels stored in each LWP.
 */
extern const char *rttAllUsedChannels[RTT_CONFIG_NUM_LWP+1][RTT_CONFIG_NUM_CHANNEL_PER_LWP+1];

// -------------------------------------------------------------------------

#ifdef __cplusplus
} // terminate extern "C" 
#endif

// ///////////////////////////////////////////////
// [0.3] Function declarations
// ///////////////////////////////////////////////

// declaration of init, process and finit functions for AMs
extern void test_am_timetick_INIT(void *);
extern void test_am_timetick_PROCESS(void *);
extern void test_am_timetick_FINIT(void *);
extern void test_am_stimulator_INIT(void *);
extern void test_am_stimulator_PROCESS(void *);
extern void test_am_stimulator_FINIT(void *);
extern void test_am__ora_End1I_INIT(void *);
extern void test_am__ora_End1I_PROCESS(void *);
extern void test_am__ora_End1I_FINIT(void *);
extern void test_am__ora_End2I_INIT(void *);
extern void test_am__ora_End2I_PROCESS(void *);
extern void test_am__ora_End2I_FINIT(void *);
extern void test_am__ora_EtherI_INIT(void *);
extern void test_am__ora_EtherI_PROCESS(void *);
extern void test_am__ora_EtherI_FINIT(void *);


#endif

/*//////////////////////////////////////////////////////////////
//
// Change History:
//
// $Log: not supported by cvs2svn $
// Revision 1.3  2012/02/16 17:11:36  oli
// adjusted company postal address (to Am_Fallturm_1)
//
// Revision 1.2  2009/03/16 18:50:42  bisanz
// Declarations of UNIT_AM_FUNCTIONS (i.e. <my_am>_INIT etc.) are removed
// from RTTcontext.h, again in order to remove unnecessary make dependencies
// and are contained in RTTcontext_private.h, now.
// Note: They are not extern "C" anymore, which is unnecessary.
//
// Revision 1.1  2009/03/13 11:54:06  bisanz
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
//
//
///////////////////////////////////////////////////////////////
*/
