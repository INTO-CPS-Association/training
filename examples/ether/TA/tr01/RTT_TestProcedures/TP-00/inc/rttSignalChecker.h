    /** @file
    //////////////////////////////////////////////////////////////////////
    //
    // Verified Systems International GmbH
    //
    // http://www.verified.de/
    //
    // Tel.  : +49 421 57204 0
    // Fax   : +49 421 57204 22
    // e-mail: info@verified.de
    //
    //--------------------------------------------------------------------
    //
    // (C) Copyright Verified Systems International GmbH
    //     $Date: $
    //
    //--------------------------------------------------------------------
    //
    // Product: RTT-MBT signal checker for RTTL projects
    //
    //--------------------------------------------------------------------
    //
    // File Identification:
    //
    // $RCSfile:$
    //
    // $Header:$
    //
    // $Revision:$
    //
    // First edition by: Uwe Schulze
    // Last update by    $Author:$
    //
    //--------------------------------------------------------------------
    //
    // Description:
    // This checker function is part of the project templates for RTT-MBT
    // projects. It is an implementation of a checker state machine for a
    // single SUT2TE signal. The generated checker abstract machines use
    // this function for each of the signals that they are checking, to
    // calculate the verdict of the respective signal.
    //
    // This is version 1.3 of this checker function. It is compliant with
    // RTT-MBT versions 9.0-1.4.0 or later.
    //
    //--------------------------------------------------------------------*/

    #ifndef _RTT_SIGNAL_CHECKER_H_
    #define _RTT_SIGNAL_CHECKER_H_

    typedef enum rttmbt_signal_types_t {
        rttmbt_bool,
        rttmbt_short,
        rttmbt_signed_short,
        rttmbt_unsigned_short,
        rttmbt_int,
        rttmbt_signed_int,
        rttmbt_unsigned_int,
        rttmbt_char,
        rttmbt_signed_char,
        rttmbt_unsigned_char,
        rttmbt_long,
        rttmbt_signed_long,
        rttmbt_unsigned_long,
        rttmbt_float,
        rttmbt_double
    } rttmbt_signal_types_t;

    /**
     * This function implements the checker state machine for a single SUT2TE signal of a checker AM.
     * Checker AMs are generated for each component of the SUT that contains a defined behaviour
     * (normally a state machine). When the checker AMs reach a stable state, this checker function
     * is called for each SUT2TE signal that the respective model component writes to.
     *
     * Note: signals can have different typed of values. To make the checker function generic,
     *       pointers are used for all signal values and type casts are performed according to
     *       the type information given by the parameter 'type'.
     *
     * @param current_state         The current state of the checker. This is a pointer
     *                              to an integer that contains the rttmbt_checker_state_t
     *                              value for the checker state for the respective signal.
     * @param counter               This is a pointer to the integer holding the current
     *                              timeout counter. If the counter is less then 0, the
     *                              timeout is violated. The counter is decreased once per
     *                              checker function call by the value of the global
     *                              variable _timeTickDiff.
     * @param expected              A pointer to the variable with the current expected value.
     *                              The expected value is changed by the checker AM.
     * @param expected_old          A pointer to the last known expected value. This value
     *                              is written by the checker function and is used to detect
     *                              new expected values.
     * @param observed              A pointer to the variable with the current observed value.
     *                              The observed value is changed by the checker AM.
     * @param observed_old          A pointer to the last known observed value. This value
     *                              is written by the checker function and is used to detect
     *                              new observed values.
     * @param admissibleError       The admissible deviation for the signal value as specified
     *                              in the signal map. Note: the admissible error is always
     *                              given as a double value, independent of the actual value
     *                              type of the signal.
     * @param latencyTimeout        The latency value as given in the signal map. The timeout
     *                              counter is set to this value whenever a new expected value
     *                              is detected.
     * @param earlyChangeTimeout    The value for early SUT indications. The timeout counter
     *                              is set to this value whenever the checker function is not
     *                              expecting a SUT indication change and a new observed value
     *                              is detected.
     * @param type                  The type information for observed, observed_old, expected
     *                              and expected old.
     * @param signalName            The name of the signal as a character pointer.
     *                              This information is used for logging.
     * @param tags                  The currently tested test case tags. If this parameter is
     *                              NULL, no tag information is logged during the checks.
     */
    void check_sut2te_indication(int *current_state,
                                 int *counter,
                                 void *expected,
                                 void *expected_old,
                                 void *observed,
                                 void *observed_old,
                                 double admissibleError,
                                 int latencyTimeout,
                                 int earlyChangeTimeout,
                                 rttmbt_signal_types_t type,
                                 const char *signalName,
                                 const char *tags);

    /**
     * This function is used to check the signal values at the end of a test procedure
     *
     * @param current_state         The current state of the checker. This is a pointer
     *                              to an integer that contains the rttmbt_checker_state_t
     *                              value for the checker state for the respective signal.
     * @param expected              A pointer to the variable with the current expected value.
     *                              The expected value is changed by the checker AM.
     * @param observed              A pointer to the variable with the current observed value.
     *                              The observed value is changed by the checker AM.
     * @param observed_old          A pointer to the last known observed value. This value
     *                              is written by the checker function and is used to detect
     *                              new observed values.
     * @param admissibleError       The admissible deviation for the signal value as specified
     *                              in the signal map. Note: the admissible error is always
     *                              given as a double value, independent of the actual value
     *                              type of the signal.
     * @param type                  The type information for observed, observed_old, expected
     *                              and expected old.
     * @param signalName            The name of the signal as a character pointer.
     *                              This information is used for logging.
     * @param tags                  The currently tested test case tags. If this parameter is
     *                              NULL, no tag information is logged during the checks.
     */
    void check_sut2te_indication_finit(int *current_state,
                                       void *expected,
                                       void *observed,
                                       void *observed_old,
                                       double admissibleError,
                                       rttmbt_signal_types_t type,
                                       const char *signalName,
                                       const char *tags);

    #endif // _RTT_SIGNAL_CHECKER_H_

