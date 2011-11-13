#ifndef OPTION_H
#define OPTION_H

#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

struct Option
{
    bool    arc_consistency;
    enum sort_t { SORT_DISABLED, SORT_DOMAIN_SIZE, SORT_FAILURES };
    sort_t  sort;

    Option();
    void GetOptions(int argc, char *argv[]);
};

Option::Option()
{
    arc_consistency = false;
    sort            = SORT_DISABLED;
}

void Option::GetOptions(int argc, char *argv[])
{
    char c;
    while ((c = getopt(argc, argv, "as:")) != -1) {
        switch (c) {
            case 'a': arc_consistency = true; break;
            case 's': 
                switch (optarg[0]) {
                    case 'd': sort = SORT_DOMAIN_SIZE; break;
                    case 'f': sort = SORT_FAILURES; break;
                    case 'D': sort = SORT_DISABLED; break;
                    default:
                              printf("Invalid sort option: %s\n", optarg);
                              exit(1);
                }
                break;
        }
    }
}

#endif
