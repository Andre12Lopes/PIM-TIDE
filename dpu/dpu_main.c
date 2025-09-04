#include <defs.h>

#include "scheduler.h"

__host uint32_t dpu_id;

int
main()
{
    int tid;

    tid = me();

    if (dpu_id > 0)
    {
        if (tid == 0)
        {
            scheduler_init(dpu_id);
            dpu_id = 0;
        }
    }
    else
    {
        scheduler_process_sub_transactions(tid);
    }

    return 0;
}
