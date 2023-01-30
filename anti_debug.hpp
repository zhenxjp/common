#pragma once
#include "xcom.hpp"


static int get_tracer_pid()
{
    const char *path = "/proc/self/status";
    FILE *f = fopen(path, "r");
    if (NULL == f)
    {
        return 0;
    }

    const int line_len = 1024;
    const char* find = "TracerPid:";
    const int key_len = strlen(find);

    char line_buff[line_len] = {0};
    while (fgets(line_buff, line_len, f))
    {
        if (strncmp(line_buff, find, key_len) == 0)
        {
            int tracer_id = atoi(line_buff+key_len);
            return tracer_id;
        }
    }
    return 0;
}