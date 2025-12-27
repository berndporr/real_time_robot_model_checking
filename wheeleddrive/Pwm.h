#ifndef PWM_H
#define PWM_H

#include <iostream>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

using namespace std;

/**
 * @brief PWM control class, based on the sysfs interface to control the PWM output on the Rock 5B
 *
 * This class encapsulates writing to /sys/class/pwm/pwmchipX/pwmY,
 * including setting period, duty cycle, and enabling/disabling output.
 */
class PWM
{
public:
    /**
     * @brief Start the PWM output
     *
     * @param channel PWM channel (e.g., 0 for pwm0)
     * @param low_time Low time of the signal of one period
     * @param high_time High time of the signal of one period
     * @param chip PWM chip number
     * @return int 0 if success, <0 if failed
     */
    int start(int channel, float low_time, float high_time, int chip);

    /**
     * @brief Set duty cycle in nanoseconds
     */
    inline int setDutyCycleNS(int ns) const
    {
        return writeSYS(pwmpath + "/duty_cycle", ns);
    }

    /**
     * @brief Set PWM period in nanoseconds
     */
    inline int setPeriod(int ns) const
    {
        return writeSYS(pwmpath + "/period", ns);
    }

    /**
     * @brief Disable PWM output
     */
    inline int disable() const
    {
        return writeSYS(pwmpath + "/enable", 0);
    }

    ~PWM()
    {
        disable(); // Disable PWM on object destruction
    }
    int per = 0; // Period in nanoseconds

private:
    string chippath; // Path to pwmchip (e.g., /sys/class/pwm/pwmchip2)
    string pwmpath;  // Path to specific pwm channel (e.g., /pwm2)
    int highTimeToNs = 0;
    int duty_cycle = 0;

    /**
     * @brief Write an integer value to a sysfs file
     */
    inline int writeSYS(std::string filename, int value) const
    {
        FILE *const fp = fopen(filename.c_str(), "w");
        if (NULL == fp)
        {
	    std::cerr << "Could not write to: " << filename << std::endl;
            return -1;
        }
        const int r = fprintf(fp, "%d", value);
        fclose(fp);
        return r;
    }

    int calculateFre(float low_time, float high_time)
    {
        return (low_time + high_time) * 1000000;
    }

    /**
     * @brief Enable PWM output
     */
    int enable() const
    {
        return writeSYS(pwmpath + "/enable", 1);
    }

    static constexpr int export_attempts = 50;
    static constexpr int retry_delay_us = 10000;
    static constexpr int DefaultDutyCycle = 20000000;
};

#endif
