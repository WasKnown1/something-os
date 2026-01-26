#include <smthng_os.h>
#include <string.h>

b8 compare_ending(const i8 *string, const i8 *ending) {
    u32 ending_length = strlen(ending);
    string += strlen(string) - ending_length;
    for (int i = 0; i < ending_length; i++)
        if (string[i] != ending[i])
            return false;
    return true;
}