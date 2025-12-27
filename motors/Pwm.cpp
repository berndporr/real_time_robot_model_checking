#include "Pwm.h"

int PWM::start(int channel, float low_time, float high_time, int chip)
{
    chippath = "/sys/class/pwm/pwmchip" + to_string(chip);
    pwmpath = chippath + "/pwm" + to_string(channel);
    string p = chippath + "/export";
    FILE *const fp = fopen(p.c_str(), "w");
    if (NULL == fp)
	{
	    fprintf(stderr, "PWM device: chip %d, channel %d does not exist.\n", chip, channel);
	    return -1;
	}
    for(int i = 0;; i++) {
	const int r = fprintf(fp, "%d", channel);
	usleep(retry_delay_us);
	if (r > 0) break;
	if (i > export_attempts) {
	    std::cerr << "Timeout enabling:" << pwmpath << std::endl;
	    return -1;
	}
    }
    per = calculateFre(low_time, high_time);
    setPeriod(per);
    setDutyCycleNS(DutyCycle);
    int r = enable();
    return r;
}
