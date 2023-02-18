# You Spin Me Round Robin
This is the implementation for Round Robin Scheduling for a given workload and the quantum length.

## Building
To build this run "make" inside the lab1 directory.

## Running
Run this program by running './rr <workload> <quantum_length>'

Example:
>./rr processes.txt 3

Output:
>Average waiting time: 7.00
>Average response time: 2.75

## Cleaning up

To clean, run 'make clean'. If you used python test suite the run 'rm -r __pycache__' to remove unneccesary files. The pyhton test suite already runs 'make clean' for you.

