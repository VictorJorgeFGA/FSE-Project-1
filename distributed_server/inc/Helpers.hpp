#ifndef _HELPERS_HPP_
#define _HELPERS_HPP_

#include <iostream>
#include <cstdlib>

namespace Helpers
{
    int gpio_bcm_to_wiringPi_pin(int gpio_bcm)
    {
        switch (gpio_bcm)
        {
        case 4:
            return 7;
            break;
        case 17:
            return 0;
            break;
        case 22:
            return 3;
            break;
        case 10:
            return 12;
            break;
        case 9:
            return 13;
            break;
        case 11:
            return 14;
            break;
        case 28:
            return 17;
            break;
        case 30:
            return 19;
            break;
        case 14:
            return 15;
            break;
        case 15:
            return 16;
            break;
        case 18:
            return 1;
            break;
        case 23:
            return 4;
            break;
        case 24:
            return 5;
            break;
        case 25:
            return 6;
            break;
        case 8:
            return 10;
            break;
        case 7:
            return 11;
            break;
        case 29:
            return 18;
            break;
        case 31:
            return 20;
            break;

        default:
            break;
        }
        std::cerr << "No wiringPi pin exists for bcm pin " << gpio_bcm << std::endl;
        exit(EXIT_FAILURE);
        return 0;
    }
}

#endif