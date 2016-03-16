#include <iostream>
#include <strings.h>
#include <algorithm>

bool UniqueIdx(int x, int y, int *pos, int n)
{
    for (int i=0; i<n; i++)
        if (*((pos+i*2)+0) == x && *((pos+i*2)+1) == y)
            return false;
    return true;
}

bool atob(const char *str)
{
    if (!strcasecmp(str, "true"))
        return true;
    return false;
}
