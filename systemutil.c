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

  printf("%0.2lf GB / %0.2lf GB  -- %0.2lf GB / %0.2lf GB \t",
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
  
  double usage = 100 * (double)effective_total_change / (double)total_change;

  printf("Total cpu usage: %0.2lf %%\n", usage);
  
   
}


void cpu_graphics(CpuValues cpu_current1, CpuValues cpu_current2){

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
  
  double percentage = 100 * (double)effective_total_change / (double)total_change;
  int increment = (int)percentage;

  printf("\t");
  for (int i = 0; i <= increment; i++ )
  {
    printf("|");
  }
    printf(" %0.2lf", percentage);
    printf("\n");
  }

void memory_graphics(MemoryValue memory1, MemoryValue memory2){
  
  /*the base sample*/
  double Used_ram_1 = memory1.totalram - memory1.freeram;
  double Used_swap_1 = memory1.totalswap - memory1.freeswap;

  /*the delayed sample*/
  double Used_ram_2 = memory2.totalram - memory2.freeram;
  double Used_swap_2 = memory2.totalswap - memory2.freeswap;

  double change = (Used_swap_2 + Used_ram_2 - Used_swap_1 - Used_ram_1);

  int increment = (int)(change/0.001);
  int absincr = abs(increment);

  if (change < 0){
    for (int i = 0; i <= absincr; i++){
      printf(":");
    }
    printf("@ ");
    printf("%0.3lf change", change);
    printf("\n");
  } else {
    for (int i = 0; i <= absincr; i++){
      printf("#");
    }
    printf("* ");
    printf("%0.3lf change", change);
    printf("\n");

  }
}
int abs(int value){
  int final;
  if (value < 0){
    final = -1 * value;
  } else{
    final = value;
  }
  return final;
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
  flag_arr[5] = 10; //Default value for time delay built in
  flag_arr[6] = 1;////Default value for sample size built in

  while ((opt = getopt_long(argc, argv, "g", long_options, &option_index)) !=
         -1) {
    /* Detect the end of the options. */
    switch (opt) {

      case 'g':
        flag_arr[2] = 1;
        break;
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
          printf("ERROR: --samples argument must be only a positive integer, entered '%s'\n", optarg);
          exit(1);
        }
      case 5:
        if ((strtol(optarg, NULL, 10) != 0)) {
          flag_arr[4] = 1;
          flag_arr[6] = strtol(optarg, NULL, 10);
          arg_given = 1;
          break;
        } else {
          printf("ERROR: --tdelay argument argument must be only a positive integer, entered %s\n", optarg);
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
               "manual for usage\n");
        exit(1);
    }
  }
  /* Accessing any remaining command line arguments */
  
  if (optind < argc && argc - optind == 2) {
    int temp = optind;
    
    if ((strtol(argv[temp++], NULL, 10) != 0) && arg_given == 0) {
      flag_arr[5] = strtol(argv[optind++], NULL, 10);
    } else {
          printf("ERROR: Positional arguments not used properly, please refer to manual\n");
          exit(1);}
    if ((strtol(argv[temp++], NULL, 10) != 0)) {
      flag_arr[6] = strtol(argv[optind++], NULL, 10);
    } else {
          printf("ERROR: Positional arguments not used properly, please refer to manual\n");
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
  int sample_size = cmd_flags[5];
  int time_delay = cmd_flags[6];

  /*Set flags into human friendly variables*/
  int sys = cmd_flags[0];
  int uzer = cmd_flags[1];
  int graf = cmd_flags[2];
  int samp = cmd_flags[3]; 
  int tdlay = cmd_flags[4];

  /* Arrays that will remember the past for us*/
  CpuValues cpu_array[sample_size + 1]; // to Store n = sample_size samples and 1 base sample
  MemoryValue memory_array[sample_size + 1];

  /* Base variables used for memory and cpu */
  curr_memory_sample = get_memory_usage();
  curr_cpu_sample = get_cpu_values();

  cpu_array[0] = curr_cpu_sample;
  memory_array[0] = curr_memory_sample;

  for (int i = 0; i < sample_size; i++) {
    /*Terminal escape codes*/
    printf("\033[2J"); //Clear screen
    printf("\e[1;1H"); //position curser to 1,1 row and column

    /*delayed samples*/
    sleep(time_delay);

    next_memory_sample = get_memory_usage();
    next_cpu_sample = get_cpu_values();

    cpu_array[i+1] = next_cpu_sample;
    memory_array[i+1] = next_memory_sample;


    if ((sys == 1 && uzer == 1) || (sys == 0 && uzer == 0)){
      if (graf == 1){
        printf("---------------------------------------\n");
        printf("### Memory ### (Phys.Used/Tot -- Virtual Used/Tot\n");
        for (int j = 0; j <= i ; j++){
          fflush(stdout);
          print_memory_usage(memory_array[j]); 
          memory_graphics(memory_array[j], memory_array[j + 1]);
        }
        print_newline((sample_size-1) - i);
        printf("---------------------------------------\n");
        printf("### CPU information ###\n");
        print_cpu_usage(cpu_array[i], cpu_array[i + 1]);
        for (int j = 0; j <= i ; j++){
            cpu_graphics(cpu_array[j], cpu_array[j + 1]);
        }
        print_newline((sample_size-1) - i); 
        printf("---------------------------------------\n");
        printf("### System Information ###\n");
        print_sysinfo();
        printf("---------------------------------------\n");
        printf("### Sessions/Users ###\n");
        print_user_session();
        printf("---------------------------------------\n");
      }
      else{
        printf("---------------------------------------\n");
        printf("### Memory ### (Phys.Used/Tot -- Virtual Used/Tot\n");
        for (int j = 0; j <= i ; j++){
          fflush(stdout);
          print_memory_usage(memory_array[j]);
          printf("\n");
        }
        print_newline((sample_size-1) - i);
        
        printf("---------------------------------------\n");
        printf("### CPU information ###\n");
        print_cpu_usage(cpu_array[i], cpu_array[i + 1]);
        printf("---------------------------------------\n");
        printf("### System Information ###\n");
        print_sysinfo();
        printf("---------------------------------------\n");
        printf("### Sessions/Users ###\n");
        print_user_session();
        printf("---------------------------------------\n");
      }
    } 
    
    else if (sys == 1){
      if (graf == 1){
        printf("---------------------------------------\n");
        printf("### Memory ### (Phys.Used/Tot -- Virtual Used/Tot\n");
        for (int j = 0; j <= i ; j++){
          fflush(stdout);
          print_memory_usage(memory_array[j]); 
          memory_graphics(memory_array[j], memory_array[j + 1]);
        }
        print_newline((sample_size-1) - i);
        printf("---------------------------------------\n");
        printf("### CPU information ###\n");
        print_cpu_usage(cpu_array[i], cpu_array[i + 1]);
        for (int j = 0; j <= i ; j++){
            cpu_graphics(cpu_array[j], cpu_array[j + 1]);
        }
        print_newline((sample_size-1) - i); 
        printf("---------------------------------------\n");
        printf("### System Information ###\n");
        print_sysinfo();
      }
      else{
        printf("---------------------------------------\n");
        printf("### Memory ### (Phys.Used/Tot -- Virtual Used/Tot\n");
        for (int j = 0; j <= i ; j++){
          fflush(stdout);
          print_memory_usage(memory_array[j]);
          printf("\n");
        }
        print_newline((sample_size-1) - i);
        printf("---------------------------------------\n");
        printf("### CPU information ###\n");
        print_cpu_usage(cpu_array[i], cpu_array[i + 1]);
        printf("---------------------------------------\n");
        printf("### System Information ###\n");
        print_sysinfo();
        printf("---------------------------------------\n");
      }
      
    } 
    else if (uzer == 1){
      if (graf == 1){
        printf("Graphics flag cannot be passed for User only\n");
        exit(1);
      } else{
        printf("---------------------------------------\n");
      printf("### System Information ###\n");
      print_sysinfo();
      printf("---------------------------------------\n");
      printf("### Sessions/Users ###\n");
      print_user_session();
      printf("---------------------------------------\n");
      }
    }
    fflush(stdout);

  }
  
  exit(0);
}
