#include "mysync.h"
#include "patterns.h"
#include <regex.h>
bool matches_pattern(const char *filename, regex_t *regex_patterns, int num_patterns) {
    int ret = regexec(&regex_patterns[1], filename, 0, NULL, 0);

    for (int i = 0; i < num_patterns; i++)
    {
        if(regexec(&regex_patterns[i], filename, 0, NULL, 0) == 0){
            return true;
        }
    }
    return false;
}