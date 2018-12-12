# Semaphores
One producer, many consumers

There are M buffers, every buffer is of type A B or C ( and there are always buffers of each type)

# Processes, not threads

Problem is solved using processes - parent is a produer, children are customers.
Mmap and sh_oper are used in order to share variables, buffers and semaphores

# Posix 

I have decided to use posix semaphores instead of System V. They seem to be easier in use :)

# Running

To run add flags -lpthread -lrt
