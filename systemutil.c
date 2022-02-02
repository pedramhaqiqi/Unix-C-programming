#include <getopt.h>
#include <sys/resource.h>
#include <sys/sysinfo.h>
#include <sys/types.h>
#include <sys/utsname.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <utmp.h>

struct sysinfo systeminfo;
struct utsname unamept;


typedef struct memory_value {
  double freeram;
  double totalram;
  double freeswap;
  double totalswap;
} MemoryValue;

typedef struct cpu_value {
  unsigned long long user;
  unsigned long long nice;
  unsigned long long system;
  unsigned long long idle;
  unsigned long long iowait;
  unsigned long long irq;
  unsigned long long softirq;

} CpuValues;

/* Array of long options that getopt_long() uses for command line parcing*/
static struct option long_options[] = {
    /* These are the input options from user. */
    {"system", no_argument, 0, 1},
    {"user", no_argument, 0, 2},
    {"graphics", no_argument, 0, 3},
    {"samples", required_argument, 0, 4},
    {"tdelay", required_argument, 0, 5},
    {0, 0, 0, 0}  // the struct option requires a the last array to be zeros
};

CpuValues get_cpu_values() {
  FILE *fp;
  CpuValues cpu_values;
  fp = fopen("/proc/stat", "r");
  fscanf(fp, "cpu %llu %llu %llu %llu %llu %llu %llu ", &cpu_values.user,
         &cpu_values.nice, &cpu_values.system, &cpu_values.idle,
         &cpu_values.iowait, &cpu_values.irq, &cpu_values.softirq);
  fclose(fp);
  return cpu_values;
}

MemoryValue get_memory_usage() {
  sysinfo(&systeminfo);
  MemoryValue memory_values;
  memory_values.freeram =
      (double)(systeminfo.freeram * systeminfo.mem_unit) / ((double)(1 << 30));
  memory_values.totalram =
      (double)(systeminfo.totalram * systeminfo.mem_unit) / ((double)(1 << 30));
  memory_values.freeswap =
      (double)(systeminfo.freeswap * systeminfo.mem_unit) / ((double)(1 << 30));
  memory_values.totalswap =
      (double)(systeminfo.totalswap * systeminfo.mem_unit) /
      ((double)(1 << 30));

  return memory_values;
}

void print_user_session(){
    utmpname(UTMP_FILE);
    struct utmp *utmp_info;
    setutent();
    utmp_info = getutent();
    while(utmp_info != NULL){
      if (utmp_info->ut_type == USER_PROCESS){
          printf("%9s%12s (%s)\n", utmp_info->ut_user, utmp_info->ut_line, utmp_info->ut_host);
      }
      utmp_info = getutent();
    } 
    endutent();
  
}

void print_memory_usage(MemoryValue memory) {
  double total_ram = memory.totalram;
  double free_ram = memory.freeram;
  double total_swap = memory.totalswap;
  double free_swap = memory.freeswap;

  printf("%0.2lf GB / %0.2lf GB  -- %0.2lf GB / %0.2lf GB\n",
         total_ram - free_ram, total_ram,
         total_ram + total_swap - free_swap - free_ram, total_ram + total_swap);
}

void print_sysinfo() {
  uname(&unamept);
  char *systemname = unamept.sysname;
  char *machinename = unamept.nodename;
  char *releaseinfo = unamept.release;
  char *versioninfo = unamept.version;
  char *machinearch = unamept.machine;
  printf("System Name: %s\n", systemname);
  printf("Machine Name: %s\n", machinename);
  printf("Version: %s\n", releaseinfo);
  printf("Release: %s\n", versioninfo);
  printf("Architecture: %s\n", machinearch);
}

void print_cpu_usage(CpuValues cpu_current1, CpuValues cpu_current2) {
  /*Base cpu sample*/
  unsigned long long total_1 = cpu_current1.user + cpu_current1.nice +
                               cpu_current1.system + cpu_current1.idle +
                               cpu_current1.iowait + cpu_current1.irq +
                               cpu_current1.softirq;

  /*delayed cpu sample*/
  unsigned long long total_2 = cpu_current2.user + cpu_current2.nice +
                               cpu_current2.system + cpu_current2.idle +
                               cpu_current2.iowait + cpu_current2.irq +
                               cpu_current2.softirq;
  /*change calculation*/
  unsigned long long effective_total_1 = total_1 - cpu_current1.idle;
  unsigned long long effective_total_2 = total_2 - cpu_current2.idle;

  unsigned long long effective_total_change =
      effective_total_2 - effective_total_1;
  unsigned long long total_change = total_2 - total_1;
  printf("Total cpu usage: %0.2lf %%\n ",
         100 * (double)effective_total_change / (double)total_change);
}

void memory_usage_change(MemoryValue memory1, MemoryValue memory2) {
  /*the base sample*/
  double Used_ram_1 = memory1.totalram - memory1.freeram;
  double Used_swap_1 = memory1.totalswap - memory1.freeswap;

  /*the delayed sample*/
  double Used_ram_2 = memory2.totalram - memory2.freeram;
  double Used_swap_2 = memory2.totalswap - memory2.freeswap;

  double difference = (Used_swap_2 + Used_ram_2 - Used_swap_1 - Used_ram_1);
  if (difference < 0) {
    printf("memory change is 0.00 GB\n");
  } else {
    printf("memory change is %0.5lf GB\n", difference);
  }
}

void clear_line()
{
    printf("\e[2K");
}

void print_newline(int lines){
  for (int i = 0; i < lines; i++){
    printf("\n");
  }
}

int *cmd_parsing_function(int argc, char *argv[]) {
  int opt;
  int option_index = 0;
  int *flag_arr;
  int arg_given = 0;
  flag_arr = (int *)calloc(7, sizeof(int));
  flag_arr[5] = 1; //Default value for time delay built in
  flag_arr[6] = 10;////Default value for sample size built in

  while ((opt = getopt_long(argc, argv, "g", long_options, &option_index)) !=
         -1) {
    /* Detect the end of the options. */
    switch (opt) {
      case 1:
        flag_arr[0] = 1;
        break;

      case 2:
        flag_arr[1] = 1;
        break;

      case 3:
        flag_arr[2] = 1;
        break;

      case 4:
        if ((strtol(optarg, NULL, 10) != 0)) {
          flag_arr[3] = 1;
          flag_arr[5] = strtol(optarg, NULL, 10);
          arg_given = 1;
          break;
        } else {
          printf("ERROR: --samples argument must be a positive integer, entered '%s'\n", optarg);
          exit(1);
        }
      case 5:
        if ((strtol(optarg, NULL, 10) != 0)) {
          flag_arr[4] = 1;
          flag_arr[6] = strtol(optarg, NULL, 10);
          arg_given = 1;
          break;
        } else {
          printf("ERROR: --tdelay argument must be a positive integer %s\n", optarg);
          exit(1);
        }
      case '?':
        /* getopt_long prints and error message on its own. */
        printf(
            "%s\n",
            "Usage is [--system | --user ] *[--samples=] *[--tdelay=]");  // Usage
                                                                          // comment
                                                                          // to
                                                                          // use
        exit(1);

      default:
        printf("%s",
               "ERROR in identifiying command line command, pass read user "
               "manual for usage");
        exit(1);
    }
  }
  /* Accessing any remaining command line arguments */
  
  if (optind < argc && argc - optind == 2) {
    int temp = optind;
    
    if ((strtol(argv[temp++], NULL, 10) != 0) && arg_given == 0) {
      flag_arr[5] = strtol(argv[optind++], NULL, 10);
    } else {
          printf("ERROR: Positional arguments must be a positive integer\n");
          exit(1);}
    if ((strtol(argv[temp++], NULL, 10) != 0)) {
      flag_arr[6] = strtol(argv[optind++], NULL, 10);
    } else {
          printf("ERROR: Positional arguments must be a positive integer\n");
          exit(1);}  
    }
     else if(optind < argc && argc - optind != 2){
      printf("%s", "ERROR: Incorrect Positinal arguments\n");
      exit(1);
    }

  return flag_arr;
}


int main(int argc, char **argv) {
  /*initializaitons*/
  int *cmd_flags;
  CpuValues curr_cpu_sample;
  CpuValues next_cpu_sample;
  MemoryValue curr_memory_sample;
  MemoryValue next_memory_sample;


  /*reading Cmd line flag array*/
  cmd_flags = cmd_parsing_function(argc, argv);


  /*Set sample size and time delay of */
  int sample_size = cmd_flags[6];
  int time_delay = cmd_flags[5];

  /*Set flags into human friendly variables*/
  int sys = cmd_flags[0];
  int uzer = cmd_flags[1];
  int graf = cmd_flags[2];
  int samp = cmd_flags[3]; 
  int tdlay = cmd_flags[4];

  /* Base variables used for memory and cpu */
  curr_memory_sample = get_memory_usage();
  curr_cpu_sample = get_cpu_values();

  /* Arrays that will remember the past for us*/
  CpuValues cpu_array[sample_size];
  MemoryValue memory_array[sample_size];

  for (int i = 0; i <= sample_size; i++) {
    /*Terminal escape codes*/
    printf("\033[2J"); //Clear screen
    printf("\e[1;1H"); //position curser to 1,1 row and column
    /*delayed samples*/
    sleep(time_delay);
    next_memory_sample = get_memory_usage();
    next_cpu_sample = get_cpu_values();
    

    /*function calls*/
    printf("### Memory ### (Phys.Used/Tot -- Virtual Used/Tot\n");
    print_memory_usage(curr_memory_sample);
    memory_usage_change(curr_memory_sample, next_memory_sample);
    printf("---------------------------------------\n");
    printf("### CPU information ###\n");
    print_cpu_usage(curr_cpu_sample, next_cpu_sample);
    printf("---------------------------------------\n");
    printf("### System Information ###\n");
    print_sysinfo();
    printf("---------------------------------------\n");
    printf("### Sessions/Users ###\n");
    print_user_session();
    

    if (sys == 1){
      
    }

    if (uzer == 1){

    }

    if (sys == 1 and uzer == 1){

    }

    













    curr_cpu_sample = next_cpu_sample;
    curr_memory_sample = next_memory_sample;
  }
  exit(0);
}
