# Semaphores
One producer, many consumers

There are M buffers, every buffer is of type A B or C ( and there are always buffers of each type)

# Processes, not threads

Problem is solves using processes - parent is a produer, children are customers.
Mmap and sh_oper are used in order to share variables, buffers and semaphores

# Posix 

I have decided to use posix semaphores instead of System V.

# Running

To run add flags -lpthread -lrt
