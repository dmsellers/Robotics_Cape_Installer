/**
 * <rc/servo.h>
 *
 * The Robotics Cape has 8 3-pin headers for connecting hobby servos and ESCs.
 * These are driven by the PRU for extremely precise signaling with minimal CPU
 * use. Standard 3-pin servo connectors are not polarized so pay close attention
 * to the symbols printed in white silkscreen on the cape before plugging
 * anything in. The black/brown ground wire should always be closest to the cape
 * PCB. The pinnout for these standard 3 pin connectors is as follows.
 *
 * - 1 Ground
 * - 2 6V Power
 * - 3 Pulse width signal
 *
 * Both servos and Brushless ESCs expect pulse width signals corresponding to
 * the desired output position or speed. These pulses normally range from 900us
 * to 2100us which usually corresponds to +- 60 degrees of rotation from the
 * neutral position. 1500us usually corresponds to the center position. Many
 * servos work up to +- 90 degrees when given pulse widths in the extended range
 * from 600us to 2400us. Test the limits of your servos very carefully to avoid
 * stalling the servos motors.
 *
 * - Pulse Width     Pulse Width      Servo Angle
 * - Normalized     (Microseconds)    (Degrees)
 * - -1.5             600us           90 deg ccw
 * - -1.0             900us           60 deg ccw
 * - 0.0             1500us           centered
 * - 1.0             2100us           60 deg cw
 * - 1.5             2400us           90 deg cw
 *
 * Unlike PWM which is concerned with the ratio of pulse width to pulse
 * frequency, servos and ESCs are only concerned with the pulse width and can
 * tolerate a wide range of update frequencies. Servos can typically tolerate
 * update pulses from 5-50hz with more expensive digital models sometimes
 * capable of higher update rates. Brushless ESCs are much more tolerant and
 * typically accept update rates up to 200hz with some multirotor ESCs capable
 * of 400hz when using sufficiently short pulse widths.
 *
 * Since ESCs drive motor unidirectionally, it makes more sense to think of
 * their normalized throttle as ranging from 0.0 (stopped) to 1.0 (full power).
 * Thus, these functions translate a normalized value from 0.0 to 1.0 to a pulse
 * width between 1000us and 2000us which is a common factory-calibration range
 * for many ESCs. We suggest using the rc_calibrate_escs example program on all
 * ESCs used with the robotics cape to ensure they are calibrated to this exact
 * pulse range.
 *
 * We HIGHLY recommend the use of ESCs which use the BLHeli firmware because
 * this firmware allows the input pulse range to be programmed to exactly
 * 1000-2000us and the old fashioned calibration mode to be disabled. This
 * prevents accidental triggering of calibration mode during use and removes the
 * need to run rc_calibrate_escs. BLHeli includes a plethora of configurable
 * settings and features such as easily adjustable timing and sounds. More
 * information on the BLHeli open source project here.
 *
 * Unless calibration mode is disabled, most ESCs will go into a failsafe or
 * calibration mode if the first signals they receive when powered up are not
 * their calibrated minimum pulse width corresponding to the throttle-off
 * condition. Therefore it is necessary for your program to start sending pulses
 * with a normalized value of 0.0 for a second or more before sending any other
 * value to ensure expected operation.
 *
 * Some ESCs, including those running BLHeli firmware, will wake up but keep the
 * motor idle when receiving pulses slightly below the minimum. This is largely
 * undocumented but we call this "idle" mode. For this reason we allow inputs to
 * rc_send_esc_pulse_normalized and rc_send_esc_pulse_normalized_all to range
 * from -0.1 to 1.0 where 0.0 results in the lowest throttle the ESC allows and
 * -0.1 can be used for idle where the motor is entirely powered off but the ESC
 * is awake.
 *
 * A recent trend among ESCs is support of "One-Shot" mode which shrinks the
 * pulse range down to 125-250us for reduced latency. Like
 * rc_send_esc_pulse_normalized, these oneshot equivalents also take a range
 * from -0.1 to 1.0 to allow for idle signals.
 * @author     James Strawson
 * @date       3/7/2018
 *
 * @addtogroup Servo_and_ESC
 * @ingroup PRU
 * @{
 */


#ifndef RC_SERVO_H
#define RC_SERVO_H

#ifdef __cplusplus
extern "C" {
#endif

#define RC_SERVO_CH_MAX	8
#define RC_SERVO_CH_MIN	1
#define RC_SERVO_CH_ALL	0

/**
 * @brief      Configures the PRU to send servo pulses
 *
 *             Also leaves the servo power rail OFF, turn back on with
 *             rc_servo_power_rail_en(1) if you need to power servos off of the
 *             board.
 *
 * @return     0 on success, -1 on failure
 */
int rc_servo_init();

/**
 * @brief      Cleans up servo functionality and turns off the power rail.
 *
 * @return     0 on success, -1 on failure
 */
void rc_servo_cleanup();

/**
 * @brief      enables or disables the 6V power rail to drive servos.
 *
 *             The Robotics Cape has a 6V 4A high-efficiency switching regulator
 *             to power servos from the 2 cell LiPo battery. DO NOT enable this
 *             when using BEC-enabled brushless ESCs as it may damage them.
 *             Since brushless ESCs only need the ground and signal pins, it is
 *             safest to simply cut or disconnect the middle power wire. This
 *             will allow the use of servos and ESCs at the same time. Use the
 *             enable and disable functions above to control the power rail in
 *             software.
 *
 *             ALso use this to turn off power to the servos for example when
 *             the robot is in a paused state to save power or prevent noisy
 *             servos from buzzing.
 *
 * @param[in]  en    0 to disable, non-zero to enable
 *
 * @return     0 on success, -1 on failure
 */
int rc_servo_power_rail_en(int en);


/**
 * @brief      Sends a single pulse of desired width in microseconds (us) to one
 *             or all channels.
 *
 *             This function returns right away and the PRU manages the accurate
 *             timing of the pulse in the background. Therefore calling this
 *             function succesively for each channel will start the pulse for
 *             each channel at approximately the same time.
 *
 *             As described above, servos and ESCs require regular pulses of at
 *             least 5hz to function. Since these pulses do not have to be
 *             accurate in frequency, the user can use these functions to start
 *             pulses from a userspace program at convenient locations in their
 *             program, such as immediately when new positions are calculated
 *             from sensor values.
 *
 * @param[in]  ch    Channel to send signal to (1-8) or 0 to send to all
 *                   channels.
 * @param[in]  us    Pulse Width in microseconds
 *
 * @return     0 on success, -1 on failure
 */
int rc_servo_send_pulse_us(int ch, int us);


/**
 * @brief      Like rc_send_pulse_us but translates a desired servo position
 *             from -1.5 to 1.5 to a corresponding pulse width from 600 to 2400us.
 *
 *             We cannot gurantee all servos will operate over the full range
 *             from -1.5 to 1.5 as that is normally considered the extended
 *             range. -1.0 to 1.0 is a more typical safe range but may not
 *             utilize the full range of all servos.
 *
 * @param[in]  ch     Channel to send signal to (1-8) or 0 to send to all
 *                    channels.
 * @param[in]  input  normalized position from -1.5 to 1.5
 *
 * @return     0 on success, -1 on failure
 */
int rc_servo_send_pulse_normalized(int ch, float input);


/**
 * @brief      Like rc_send_pulse_normalized but translates a desired esc
 *             throttle position from 0 to 1.0 to a corresponding pulse width
 *             from 1000 to 2000us.
 *
 *             This only works as expected if your ESCs are calibrated to accept
 *             pulse widths from 1000-2000us. This is best done with an ESC
 *             programming tool but can also be done with the rc_calibrate_escs
 *             example program that comes installed with this package.
 *
 * @param[in]  ch     Channel to send signal to (1-8) or 0 to send to all
 *                    channels.
 * @param[in]  input  normalized position from 0 to 1.0
 *
 * @return     0 on success, -1 on failure
 */
int rc_servo_send_esc_pulse_normalized(int ch, float input);


/**
 * @brief      Like rc_send_pulse_normalized but translates a desired esc
 *             throttle position from 0 to 1.0 to a corresponding pulse width
 *             from 125 to 250us.
 *
 *             A recent trend among ESCs is support of "One-Shot" mode which
 *             shrinks the pulse range down to 125-250us for reduced latency. If
 *             you are sure your ESCs support this then you may try this
 *             function.
 *
 * @param[in]  ch     Channel to send signal to (1-8) or 0 to send to all
 *                    channels.
 * @param[in]  input  normalized position from 0 to 1.0
 *
 * @return     0 on success, -1 on failure
 */
int rc_servo_send_oneshot_pulse_normalized(int ch, float input);




#ifdef __cplusplus
}
#endif

#endif // RC_SERVO_H

/** @}  end group Servo */


/******************************************************************************
* SERVO AND ESC
*

*@ int rc_enable_servo_power_rail() @ int rc_disable_servo_power_rail()
 *
 *
 *
* @ int rc_servo_send_pulse_normalized(int ch, float input)
* @ int rc_servo_send_pulse_normalized_all(float input)
*
* The normal operating range of hobby servos is usually +- 60 degrees of
* rotation from the neutral position but they often work up to +- 90 degrees.
* rc_servo_send_pulse_normalized(int ch, float input) will send a single pulse to
* the selected channel. the normalized input should be between -1.5 and 1.5
* corresponding to the following pulse width range and angle.
*
* input     width   angle
* -1.5      600us  90 deg anticlockwise
* -1.0      900us  60 deg anticlockwise
*  0.0     1500us   0 deg neutral
*  1.0     2100us  60 deg clockwise
*  1.5     2400us  90 deg clockwise
*
* Note that all servos are different and do not necessarily allow the full
* range of motion past +-1.0. DO NOT STALL SERVOS.
*
* @ int rc_send_esc_pulse_normalized(int ch, float input)
* @ int rc_send_esc_pulse_normalized_all(float input)
*
* Brushless motor controllers (ESCs) for planes and multirotors are
* unidirectional and lend themselves better to a normalized range from 0 to 1.
* rc_send_esc_pulse_normalized(int ch, float input) also sends a single pulse
* but the range is set for common ESCs
*
* input    width       power
* -0.1      900      armed but idle
* 0.0      1000us       0%   off
* 0.5      1500us      50%  half-throttle
* 1.0      2000us      100% full-throttle
*
* This assumes the ESCs have been calibrated for the 1000-2000us range. Use the
* calibrate_escs example program to be sure.
*
* @ int rc_servo_send_pulse_us(int ch, int us)
* @ int rc_servo_send_pulse_us_all(int us)
*
* The user may also elect to manually specify the exact pulse width in
* in microseconds with rc_servo_send_pulse_us(int ch, int us). When using any of
* these functions, be aware that they only send a single pulse to the servo
* or ESC. Servos and ESCs typically require an update frequency of at least
* 10hz to prevent timing out. The timing accuracy of this loop is not critical
* and the user can choose to update at whatever frequency they wish.
*
* See the test_servos, sweep_servos, and calibrate_escs examples.
******************************************************************************/
