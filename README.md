# Virtual Memory Manager Simulation

This program simulates a Virtual Memory Manager system.
Main memory is represented as a vector while secondary disk storage is represented as an fstream object (txt file).

Inputs are described as follows:
processes.txt - First line int represents N processes, followed by N lines containing process ready time and service time (in ms).

commands.txt - List of tasks to be run by processes followed by a variable string ID and variable unsigned int value if needed by task.

memconfig.txt - Max number of variables allowed by main memory, disk memory has unlimited variable storage size.

vm.txt - Simulated disk storage area (Read/Write).


HOW TO RUN FROM COMMAND LINE:

[Path to executible(.exe) file] [Directory where inputs are located] [Directory where you want output to be generated]

Ex: C:\Users\Public\files\Scheduler.exe C:\Users\Public\files\inputs\ C:\Users\Public\files\outputs
