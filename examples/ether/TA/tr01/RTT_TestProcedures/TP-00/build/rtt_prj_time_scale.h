/*:"millisecond; fmi2_gettimeofday":fmi2_gettimeofday:1:1:*/
/* BASED on compile of TP RTT_TestProcedures/TP-00 */
#ifndef _RTT_PRJ_TIME_SCALE_H_
#define _RTT_PRJ_TIME_SCALE_H_

#define _ms *1
#define _us /1000

/** @def RTT_TIME_TICK_STRING
 * Contains the scaling of one time tick ("ms", "us", or just "tick") */
#define RTT_TIME_TICK_STRING  "ms"

/** @def RTT_TIME_SCALE_STRING
 * This define contains the string form of the timescale.
 * The value of this define is created during the test procedure generation. */
#define RTT_TIME_SCALE_STRING "millisecond; fmi2_gettimeofday"

/** @def RTT_GETTIMEOFDAY_FUNCTION
 * Contains the name of the used gettimeofday() function */
#define RTT_GETTIMEOFDAY_FUNCTION   fmi2_gettimeofday
#define RTT_GETTIMEOFDAY_CUSTOMIZED 1

/** @def RTT_USE_RELATIVE_TIME
 * This define is used to decided if relative or absolute timeing is used for
 * logging.
 * The value of this define is created during the test procedure generation. */
#define RTT_USE_RELATIVE_TIME 1

/** @def RTT_USE_LOCAL_TIME
 * This define is used to decided if local or global time is used for
 * logging.
 * The value of this define is created during the test procedure generation. */
#define RTT_USE_LOCAL_TIME    1
#endif /* _RTT_PRJ_TIME_SCALE_H_ */
