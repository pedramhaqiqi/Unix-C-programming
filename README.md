System Utility Monitor:

Program file name: systemutil.c

compiling command: gcc -o systemutil systemutil.c

Overview:

This program will output system utilization of the machine the user is currently working on, it runs from the command line and outputs to it. It will report different metrics of the utilization of a given system as described below. Accepted command line argument formats will be discussed later.

--system
        to indicate that only the system usage should be generated


--user

        to indicate that only the users usage should be generated


--graphics or -g

        to include graphical output in the cases where a graphical outcome is possible as indicated below.


--samples=N

        if used the value N will indicate how many times the statistics are going to be collected and results will be average and reported based on the N number of repetitions.
If not value is indicated the default value will be 10.

--tdelay=T

        to indicate how frequently to sample in seconds.
If not value is indicated the default value will be 1 sec.


The last two arguments can also be considered as positional arguments. Meaning the user can either:

a) Input ./systemutil [--system | --user] --sample=N --tdelay=T 

b) Input ./systemutil [--system | --user] N T (values only)


The following are the standard C libraries used to gather/implement the program:

#include <getopt.h>

This Library was used to parse the command line arguments, it houses the commonly used getopt() function, which can parse short , i.e "-" single dash, arguments, and it houses the more advanced getopt_long() function which will be discussed later. 

#include <sys/sysinfo.h>

sysinfo() returns certain statistics on memory and swap usage, as
well as the load average.

#include <sys/types.h>

Includes some important types and constant that will be used in the program

#include <sys/utsname.h>

The struct of sysinfo that was used with the sysinfo function was defined here

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Standard Input output and strings library

#include <unistd.h>

The <unistd.h> header defines miscellaneous symbolic constants
and types, and declares miscellaneous functions. The actual
values of the constants are unspecified except as shown. The
contents of this header are shown below.

#include <utmp.h>

The utmp file allows one to discover information about who is
currently using the system.  There may be more users currently
using the system, because not all programs use utmp logging. The struct utmp was populated to get the information regarding users. 


HOW TO USE: 

You can use the program after compilation on your command line in the directory you stored the exec file. The command line arguments have been worked on heavily so you have a robust experience. 

The following is are guidelines for command line arguments

. Permutation of the arguments are allowed 

. Using --system --user is equivalent to not putting any arguments (default)

. User should either use positional arguments with their flags or only with values:
    ex: ./systemutil --system --tdelay=5 4 will give input error

. The values set for samples and tdelay should be positive integers.

.The graphics flag alone can be used by itself

. The user can input no flag for system and user and follow it by positional arguments for samples and Tdelay

. Positional arguments ORDER IS IMPORTANT, as in, the first integer will be samples and the second will be time. 

Example of possible command line arguments:

./systemutil --system

./systemutil --system --user

./systemutil 

./systemutil --user --system --graphics

./systemutil --system 5 1

./systemutil --system --samples=10 

./systemutil 5 1 --system

Examples of invalid command line arguments:

./systemutil --samples

./systemutil --tdelay

./systemutil abcd

./systemutil --system --samples = 5 (due to spacing)



Overview of important problems in implementation:
 

Formatting:

One of the hardest parts in this project was to give the visual of an output that is changing dynamically, for this I used a strategy of printing out new lines using the print_newlines(), together with storing the past memory and cpu samples inside of an array of CPU types and Memory types, this gave me access to past information about the system which I could just reprint each iteration.

The program takes a base samples 0, which then it uses together with sample 1 to generate the first "stat", and then goes to generate a sample 2 and use it with the previous sample for a new stats and so on.

Command line parsing:

Command line parsing was another major issue and one of the key logical units of the program, the function cmd_parsing_function() returns an array of integers, of length 7. The first 5 elements store bit wise values (keys) that tell the brain (main) which stats the user has requested. WHile the last two entries store the arguments that come with --samples and --tdlay, which are set by default to [0,0,0,0,0,10,1] using this I can streamline the process of printing out different statistics

Print functions used to print out information: 

self-explanatory, using memory samples created in main with delays of sleep(tdlay), and instances of the CpuValues and MemoryValues to 
to print out information when requested.

CPUVALUES:

the conversion of CPU values from the /proc/stats file was one of the difficult parts of this process, as I realized the numbers were in bytes, and to convert a division by 1024 * 1024 * 1024 or eqv wit bitshifting of (1<<30), was needed to output GB values

MEMORY VALUES:

Not so difficult, the important part was to signify the difference bewtween virtual and physical memory, and by adding total swap + total ram one can find their total capacity. 


The other fucntions can be found with further documentation inside the source code.


Thank you for reading, enjoy.











