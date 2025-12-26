#include "ServoMotorSetting.h"
#include "stdio.h"
// This program sets the speed of two servo motors based on command line arguments.

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        std::cout << "Usage: " << argv[0] << " <left_speed> <right_speed>(speed range from -10 to 10)" << std::endl;
        return 1;
    }

    const int PWM_servo_right = 1;
    const int PWM_servo_left = 2;

    ServoMotorSetting servo_right(0, 20, 1.52, PWM_servo_right); // 1.52ms
    ServoMotorSetting servo_left(0, 20, 1.52, PWM_servo_left); // 1.52ms

    int left_speed = std::atoi(argv[1]);
    int right_speed = std::atoi(argv[2]);

    std::cout << "Setting speeds - Left: " << left_speed << ", Right: " << right_speed << std::endl;

    servo_left.ChangeLeftSpeed(left_speed);
    servo_right.ChangeRightSpeed(right_speed);

    std::cout << "Press any key to stop" << std::endl;

    getchar();

    servo_left.ChangeLeftSpeed(0);
    servo_right.ChangeRightSpeed(0);

    return 0;
}
