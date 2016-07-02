# bHive
By David Bergeron 2016

A bare bones thread pool implementation.
------------------------------------------

Uses the pthreads library, have not tested on windows.

Use it to quickly set up thread tasks.


Currently the job Queue is implemented with a linked list.
Depending on usage this might be unacceptable. Consider using a
priority Queue instead.

TODO:
    Add bHive_forceJob() function to put a job at the head of the queue.
