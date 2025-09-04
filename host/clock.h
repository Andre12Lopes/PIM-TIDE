#pragma once

#include <sys/time.h>

#include <ctime>
#include <iostream>

class Clock
{
  public:
    Clock()
    {
    }

    void
    getDateTimestamp(char *now)
    {
        time_t t;
        struct tm my_time;

        t = time(NULL);

        localtime_r(&t, &my_time);
        my_time.tm_isdst = -1;

        if (!strftime(now, 15, "%Y%m%d%H%M%S", &my_time))
        {
            exit(1);
        }
    }
};
