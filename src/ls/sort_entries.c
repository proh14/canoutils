#include <stdlib.h>
#include <string.h>

#include "ls.h"

static
int compare_names(entry_t const *leftp, entry_t const *rightp)
{
    return strcoll(leftp->name, rightp->name);
}

static
int compare_times(entry_t const *leftp, entry_t const *rightp)
{
    return rightp->stat.st_mtim.tv_sec - leftp->stat.st_mtim.tv_sec;
}

void sort_entries(entry_t *entries, int count)
{
    qsort(entries, count, sizeof(*entries), (__compar_fn_t)&compare_names);
}

void sort_entries_by_time(entry_t *entries, int count)
{
    qsort(entries, count, sizeof(*entries), (__compar_fn_t)&compare_times);
}
