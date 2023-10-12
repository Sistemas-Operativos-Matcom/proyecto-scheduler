#if !defined(JOB)

struct job
{
    int pid;
    int ellapsed_slice_time;
};
typedef struct job* job;

job new_job(int);

#endif