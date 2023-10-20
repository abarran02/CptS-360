#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

// Headers as needed
#define MAX_INPUT_LEN 256
typedef enum {false, true} bool;        // Allows boolean types in C

/* Defines a job struct */
typedef struct Process {
    uint32_t A;                         // A: Arrival time of the process
    uint32_t B;                         // B: Upper Bound of CPU burst times of the given random integer list
    uint32_t C;                         // C: Total CPU time required
    uint32_t M;                         // M: Multiplier of CPU burst time
    uint32_t processID;                 // The process ID given upon input read

    uint8_t status;                     // 0 is unstarted, 1 is ready, 2 is running, 3 is blocked, 4 is terminated

    int32_t finishingTime;              // The cycle when the the process finishes (initially -1)
    uint32_t currentCPUTimeRun;         // The amount of time the process has already run (time in running state)
    uint32_t currentIOBlockedTime;      // The amount of time the process has been IO blocked (time in blocked state)
    uint32_t currentWaitingTime;        // The amount of time spent waiting to be run (time in ready state)

    uint32_t IOBurst;                   // The amount of time until the process finishes being blocked
    uint32_t CPUBurst;                  // The CPU availability of the process (has to be > 1 to move to running)

    int32_t quantum;                    // Used for schedulers that utilise pre-emption

    bool isFirstTimeRunning;            // Used to check when to calculate the CPU burst when it hits running mode

    struct Process* nextInBlockedList;  // A pointer to the next process available in the blocked list
    struct Process* nextInReadyQueue;   // A pointer to the next process available in the ready queue
    struct Process* nextInReadySuspendedQueue; // A pointer to the next process available in the ready suspended queue
} _process;


uint32_t CURRENT_CYCLE = 0;             // The current cycle that each process is on
uint32_t TOTAL_CREATED_PROCESSES = 0;   // The total number of processes constructed
uint32_t TOTAL_STARTED_PROCESSES = 0;   // The total number of processes that have started being simulated
uint32_t TOTAL_FINISHED_PROCESSES = 0;  // The total number of processes that have finished running
uint32_t TOTAL_NUMBER_OF_CYCLES_SPENT_BLOCKED = 0; // The total cycles in the blocked state

const char* RANDOM_NUMBER_FILE_NAME= "random-numbers";
const uint32_t SEED_VALUE = 200;  // Seed value for reading from file

/**
 * Reads a random non-negative integer X from a file with a given line named random-numbers (in the current directory)
 */
uint32_t getRandNumFromFile(uint32_t line, FILE* random_num_file_ptr){
    uint32_t end, loop;
    char str[512];

    rewind(random_num_file_ptr); // reset to be beginning
    for(end = loop = 0;loop<line;++loop){
        if(0==fgets(str, sizeof(str), random_num_file_ptr)){ //include '\n'
            end = 1;  //can't input (EOF)
            break;
        }
    }
    if(!end) {
        return (uint32_t) atoi(str);
    }

    // fail-safe return
    return (uint32_t) 1804289383;
}

/**
 * Reads a random non-negative integer X from a file named random-numbers.
 * Returns the CPU Burst: : 1 + (random-number-from-file % upper_bound)
 */
uint32_t randomOS(uint32_t upper_bound, uint32_t process_indx, FILE* random_num_file_ptr)
{
    char str[20];

    uint32_t unsigned_rand_int = (uint32_t) getRandNumFromFile(SEED_VALUE+process_indx, random_num_file_ptr);
    uint32_t returnValue = 1 + (unsigned_rand_int % upper_bound);

    return returnValue;
}

/*
 Inititalize a new Process with the following properties:
    A: Arrival time of the process
    B: Upper Bound of CPU burst times of the given random integer list
    C: Total CPU time required
    M: Multiplier of CPU burst time
*/
_process init_process(int A, int B, int C, int M, int processId) {
    _process newprocess = {
        A = A,
        B = B,
        C = C,
        M = M
    };

    newprocess.processID = processId;
    newprocess.quantum = 2;  // only used by Round Robin

    return newprocess;
}


/********************* SOME PRINTING HELPERS *********************/

/**
 * Prints to standard output the original input
 * process_list is the original processes inputted (in array form)
 */
void printStart(_process process_list[])
{
    printf("The original input was: %i", TOTAL_CREATED_PROCESSES);

    uint32_t i = 0;
    for (; i < TOTAL_CREATED_PROCESSES; ++i)
    {
        printf(" ( %i %i %i %i)", process_list[i].A, process_list[i].B,
               process_list[i].C, process_list[i].M);
    }
    printf("\n");
}

/**
 * Prints to standard output the final output
 * finished_process_list is the terminated processes (in array form) in the order they each finished in.
 */
void printFinal(_process finished_process_list[])
{
    printf("The (sorted) input is: %i", TOTAL_CREATED_PROCESSES);

    uint32_t i = 0;
    for (; i < TOTAL_FINISHED_PROCESSES; ++i)
    {
        printf(" ( %i %i %i %i)", finished_process_list[i].A, finished_process_list[i].B,
               finished_process_list[i].C, finished_process_list[i].M);
    }
    printf("\n");
} // End of the print final function

/**
 * Prints out specifics for each process.
 * @param process_list The original processes inputted, in array form
 */
void printProcessSpecifics(_process process_list[])
{
    uint32_t i = 0;
    printf("\n");
    for (; i < TOTAL_CREATED_PROCESSES; ++i)
    {
        printf("Process %i:\n", process_list[i].processID);
        printf("\t(A,B,C,M) = (%i,%i,%i,%i)\n", process_list[i].A, process_list[i].B,
               process_list[i].C, process_list[i].M);
        printf("\tFinishing time: %i\n", process_list[i].finishingTime);
        printf("\tTurnaround time: %i\n", process_list[i].finishingTime - process_list[i].A);
        printf("\tI/O time: %i\n", process_list[i].currentIOBlockedTime);
        printf("\tWaiting time: %i\n", process_list[i].currentWaitingTime);
        printf("\n");
    }
} // End of the print process specifics function

/**
 * Prints out the summary data
 * process_list The original processes inputted, in array form
 */
void printSummaryData(_process process_list[])
{
    uint32_t i = 0;
    double total_amount_of_time_utilizing_cpu = 0.0;
    double total_amount_of_time_io_blocked = 0.0;
    double total_amount_of_time_spent_waiting = 0.0;
    double total_turnaround_time = 0.0;
    uint32_t final_finishing_time = CURRENT_CYCLE - 1;
    for (; i < TOTAL_CREATED_PROCESSES; ++i)
    {
        total_amount_of_time_utilizing_cpu += process_list[i].currentCPUTimeRun;
        total_amount_of_time_io_blocked += process_list[i].currentIOBlockedTime;
        total_amount_of_time_spent_waiting += process_list[i].currentWaitingTime;
        total_turnaround_time += (process_list[i].finishingTime - process_list[i].A);
    }

    // Calculates the CPU utilisation
    double cpu_util = total_amount_of_time_utilizing_cpu / final_finishing_time;

    // Calculates the IO utilisation
    double io_util = (double) TOTAL_NUMBER_OF_CYCLES_SPENT_BLOCKED / final_finishing_time;

    // Calculates the throughput (Number of processes over the final finishing time times 100)
    double throughput =  100 * ((double) TOTAL_CREATED_PROCESSES/ final_finishing_time);

    // Calculates the average turnaround time
    double avg_turnaround_time = total_turnaround_time / TOTAL_CREATED_PROCESSES;

    // Calculates the average waiting time
    double avg_waiting_time = total_amount_of_time_spent_waiting / TOTAL_CREATED_PROCESSES;

    printf("Summary Data:\n");
    printf("\tFinishing time: %i\n", CURRENT_CYCLE - 1);
    printf("\tCPU Utilisation: %6f\n", cpu_util);
    printf("\tI/O Utilisation: %6f\n", io_util);
    printf("\tThroughput: %6f processes per hundred cycles\n", throughput);
    printf("\tAverage turnaround time: %6f\n", avg_turnaround_time);
    printf("\tAverage waiting time: %6f\n", avg_waiting_time);
} // End of the print summary data function

/** Schedulers **/

/*
 * Reset all scheduler counters to zero
 */
void reset_counters() {
    CURRENT_CYCLE = 0;
    TOTAL_STARTED_PROCESSES = 0;
    TOTAL_FINISHED_PROCESSES = 0;
    TOTAL_NUMBER_OF_CYCLES_SPENT_BLOCKED = 0;
}

void reset_process_list(_process process_list[]) {
    for (int i = 0; i < TOTAL_CREATED_PROCESSES; i++) {
        process_list[i].currentCPUTimeRun = 0;
        process_list[i].currentIOBlockedTime = 0;
        process_list[i].currentWaitingTime = 0;
        process_list[i].finishingTime = -1;

        process_list[i].CPUBurst = 0;
        process_list[i].IOBurst = 0;

        process_list[i].status = 0;
        process_list[i].isFirstTimeRunning = true;

        process_list[i].nextInBlockedList = NULL;
        process_list[i].nextInReadyQueue = NULL;
    }
}

void add_to_ready(_process **ready, _process *newlyready) {
    _process *curprocess = *ready;
    newlyready->status = 1;

    // ready queue is empty
    if (curprocess == NULL) {
        *ready = newlyready;
        return;
    }

    // add to front of ready queue
    // prioritize arrival time, then process id
    if (newlyready->A < curprocess->A ||
        (newlyready->A == curprocess->A && newlyready->processID < curprocess->processID)) {
        newlyready->nextInReadyQueue = *ready;
        *ready = newlyready;
        return;
    }

    // iterate to end of ready queue
    while (curprocess->nextInReadyQueue != NULL) {
        // keep queue sorted by above criteria
        if (newlyready->A < curprocess->nextInReadyQueue->A ||
            (newlyready->A == curprocess->nextInReadyQueue->A && newlyready->processID < curprocess->nextInReadyQueue->processID)) {
            // insert newlyready between curprocess and curprocess->nextInReadyQueue
            newlyready->nextInReadyQueue = curprocess->nextInReadyQueue;
            curprocess->nextInReadyQueue = newlyready;
            return;
        }

        curprocess = curprocess->nextInReadyQueue;
    }

    // add current process to end of ready queue
    curprocess->nextInReadyQueue = newlyready;
}

void add_to_blocked(_process **blocked, _process *newlyblocked) {
    _process *curprocess = *blocked;
    newlyblocked->status = 3;

    // blocked list is empty
    if (*blocked == NULL) {
        *blocked = newlyblocked;
        return;
    }

    // add to front of blocked list
    if (newlyblocked->IOBurst < curprocess->IOBurst) {
        newlyblocked->nextInBlockedList = *blocked;
        *blocked = newlyblocked;
        return;
    }

    curprocess = *blocked;

    // iterate to end of blocked list
    while (curprocess->nextInBlockedList != NULL) {

        // keep list sorted by remaining IO burst
        if (newlyblocked->IOBurst < curprocess->nextInBlockedList->IOBurst) {
            // insert newlyblocked between curprocess and curprocess->nextInBlockedList
            newlyblocked->nextInBlockedList = curprocess->nextInBlockedList;
            curprocess->nextInBlockedList = newlyblocked;
            return;
        }

        curprocess = curprocess->nextInBlockedList;
    }

    // add new process to end of blocked queue
    curprocess->nextInBlockedList = newlyblocked;
}

void add_arrivals_to_ready(_process process_list[], _process **ready) {
    // iterate over process list
    for (int i = 0; i < TOTAL_CREATED_PROCESSES; i++) {
        if (process_list[i].A == CURRENT_CYCLE) {
            add_to_ready(ready, &process_list[i]);
        }
    }
}

void start_process(_process *proc, FILE *randfile) {
    uint32_t burst, time_left;

    // cold start (recalc burst) or starting from suspended (do nothing)
    if (proc->CPUBurst == 0) {
        time_left = proc->C - proc->currentCPUTimeRun;

        burst = randomOS(proc->B, proc->processID, randfile);
 
        // if the burst is greater than the CPU time remaining
        if (time_left < burst) {
            proc->CPUBurst = time_left;
        } else {
            proc->CPUBurst = burst;
            proc->IOBurst = burst * proc->M;
        }
    }

    // clear process knowledge of queues
    proc->nextInBlockedList = NULL;
    proc->nextInReadyQueue = NULL;
    proc->status = 2;

    if (proc->isFirstTimeRunning) {
        TOTAL_STARTED_PROCESSES++;
        proc->isFirstTimeRunning = false;
    }
}

/** First Come First Served (FCFS) **/

/*
 * Run the First Come First Served (FCFS) scheduler on process_list[]
 */
void fcfs_run(_process process_list[], _process finished_process_list[], FILE *randfile) {
    _process *running = NULL, *ready = NULL, *blocked = NULL, *curprocess;

    // ititialize ready queue
    add_arrivals_to_ready(process_list, &ready);

    // run first process
    running = ready;
    ready = running->nextInReadyQueue;
    start_process(running, randfile);

    // start cycle
    while (TOTAL_FINISHED_PROCESSES != TOTAL_CREATED_PROCESSES) {
        CURRENT_CYCLE++;

        // context switch
        if (running != NULL && (running->status == 3 || running->status == 4)) {
            if (ready != NULL) {
                // switch running process
                running = ready;
                ready = ready->nextInReadyQueue;
                start_process(running, randfile);
            } else {
                // no running process for this cycle
                running = NULL;
            }
        } else if (running == NULL && ready != NULL) {
            // no running process last cycle
            running = ready;
            ready = running->nextInReadyQueue;
            start_process(running, randfile);
        }

        // increment waiting time for each process in ready queue
        curprocess = ready;
        while (curprocess != NULL) {
            curprocess->currentWaitingTime++;
            curprocess = curprocess->nextInReadyQueue;
        }

        // operate on blocked list
        if (blocked != NULL) {
            TOTAL_NUMBER_OF_CYCLES_SPENT_BLOCKED++;
            curprocess = blocked;

            // decrement IO burst for all blocked processes
            while (curprocess != NULL) {
                curprocess->IOBurst--;
                curprocess = curprocess->nextInBlockedList;
            }

            // add finished IO processes to ready queue, remove from blocked list
            while (blocked != NULL && blocked->IOBurst == 0) {
                add_to_ready(&ready, blocked);
                blocked = blocked->nextInBlockedList;
            }
        }

        add_arrivals_to_ready(process_list, &ready);

        // operate on running process
        if (running == NULL) {
            continue;
        }

        running->currentCPUTimeRun++;
        running->CPUBurst--;

        if (running->currentCPUTimeRun == running->C) {
            // CPU task completes, terminate
            running->status = 4;
            running->finishingTime = CURRENT_CYCLE;
            finished_process_list[TOTAL_FINISHED_PROCESSES] = *running;
            TOTAL_FINISHED_PROCESSES++;
        } else if (running->CPUBurst == 0) {
            // CPU burst finishes, block for IO
            add_to_blocked(&blocked, running);
        }
    }

    CURRENT_CYCLE++;
}

/** Round Robin (RR) **/

void rr_run(_process process_list[], _process finished_process_list[], FILE *randfile) {
    _process *running = NULL, *ready = NULL, *blocked = NULL, *curprocess;
    uint32_t quantum, quantum_counter;  // overall quantum and counter to decrement

    // ititialize ready queue
    add_arrivals_to_ready(process_list, &ready);

    // run first process
    running = ready;
    ready = running->nextInReadyQueue;
    start_process(running, randfile);

    // quantum will be the same for all processes, can use first value
    quantum = running->quantum;
    quantum_counter = quantum;

    // start cycle
    while (TOTAL_FINISHED_PROCESSES != TOTAL_CREATED_PROCESSES) {
        CURRENT_CYCLE++;
        quantum_counter--;

        // increment waiting time for each process in ready queue
        curprocess = ready;
        while (curprocess != NULL) {
            curprocess->currentWaitingTime++;
            curprocess = curprocess->nextInReadyQueue;
        }

        add_arrivals_to_ready(process_list, &ready);

        // operate on blocked list
        if (blocked != NULL) {
            TOTAL_NUMBER_OF_CYCLES_SPENT_BLOCKED++;
            curprocess = blocked;

            // decrement IO burst for all blocked processes
            while (curprocess != NULL) {
                curprocess->IOBurst--;
                curprocess->currentIOBlockedTime++;
                curprocess = curprocess->nextInBlockedList;
            }

            // add finished IO processes to ready queue, remove from blocked list
            while (blocked != NULL && blocked->IOBurst == 0) {
                add_to_ready(&ready, blocked);
                blocked = blocked->nextInBlockedList;
            }
        }

        // operate on running process
        if (running != NULL) {
            running->currentCPUTimeRun++;
            running->CPUBurst--;

            if (running->currentCPUTimeRun == running->C) {
                // CPU task completes, terminate
                running->status = 4;
                running->finishingTime = CURRENT_CYCLE;
                finished_process_list[TOTAL_FINISHED_PROCESSES] = *running;
                running = NULL;

                TOTAL_FINISHED_PROCESSES++;
            } else if (running->CPUBurst == 0) {
                // CPU burst finishes, block for IO
                add_to_blocked(&blocked, running);
                running = NULL;
            }
        }

        // context switch every quantum cycle or running process stopped
        if (quantum_counter == 0 || running == NULL) {
            quantum_counter = quantum;

            // move running process to end of ready queue
            if (running != NULL) {
                add_to_ready(&ready, running);  // would have already terminated or blocked above
                running = NULL;
            }

            // move next ready process to running
            if (ready != NULL) {
                running = ready;
                ready = ready->nextInReadyQueue;
                start_process(running, randfile);
            }
        }
    }

    CURRENT_CYCLE++;
}

/** First Come First Served (FCFS) **/

void sjf_run(_process process_list[], _process finished_process_list[], FILE *randfile) {
    _process *running = NULL, *ready = NULL, *blocked = NULL, *curprocess, *shortestjob;

    // ititialize ready queue
    add_arrivals_to_ready(process_list, &ready);

    // run first process
    running = ready;
    ready = running->nextInReadyQueue;
    start_process(running, randfile);

    // start cycle
    while (TOTAL_FINISHED_PROCESSES != TOTAL_CREATED_PROCESSES) {
        CURRENT_CYCLE++;

        // increment waiting time for each process in ready queue
        curprocess = ready;
        while (curprocess != NULL) {
            curprocess->currentWaitingTime++;
            curprocess = curprocess->nextInReadyQueue;
        }

        add_arrivals_to_ready(process_list, &ready);

        // operate on blocked list
        if (blocked != NULL) {
            TOTAL_NUMBER_OF_CYCLES_SPENT_BLOCKED++;
            curprocess = blocked;

            // decrement IO burst for all blocked processes
            while (curprocess != NULL) {
                curprocess->IOBurst--;
                curprocess->currentIOBlockedTime++;
                curprocess = curprocess->nextInBlockedList;
            }

            // add finished IO processes to ready queue, remove from blocked list
            while (blocked != NULL && blocked->IOBurst == 0) {
                add_to_ready(&ready, blocked);
                blocked = blocked->nextInBlockedList;
            }
        }

        // operate on running process
        if (running != NULL) {
            running->currentCPUTimeRun++;
            running->CPUBurst--;

            if (running->currentCPUTimeRun == running->C) {
                // CPU task completes, terminate
                running->status = 4;
                running->finishingTime = CURRENT_CYCLE;
                finished_process_list[TOTAL_FINISHED_PROCESSES] = *running;
                running = NULL;

                TOTAL_FINISHED_PROCESSES++;
            } else if (running->CPUBurst == 0) {
                // CPU burst finishes, block for IO
                add_to_blocked(&blocked, running);
                running = NULL;
            }
        }

        // context switch
        if (running == NULL && ready != NULL) {
            // search ready queue for minimum job CPU burst, or arrival time for equal burst
            shortestjob = ready;
            curprocess = ready->nextInReadyQueue;

            while (curprocess != NULL) {
                if (curprocess->CPUBurst < shortestjob->CPUBurst || 
                    (curprocess->CPUBurst == shortestjob->CPUBurst && curprocess->A < shortestjob->A)) {
                    shortestjob = curprocess;
                }

                curprocess = curprocess->nextInReadyQueue;
            }
            
            if (shortestjob == ready) {
                // shortest job was first in queue, advance queue
                ready = ready->nextInReadyQueue;
            } else {
                // shortest job in middle of queue, remove and reconnect
                curprocess = ready;
                // inefficient but seek back to shortest job to remove from queue
                while (curprocess->nextInReadyQueue != shortestjob) {
                    curprocess = curprocess->nextInReadyQueue;
                }

                curprocess->nextInReadyQueue = shortestjob->nextInReadyQueue;
            }

            running = shortestjob;
            start_process(running, randfile);
        }
    }

    CURRENT_CYCLE++;
}

/** Input validation **/

int get_process_count(char *line) {
    char linecpy[MAX_INPUT_LEN], *token;
    const char delim[2] = " ";

    strcpy(linecpy, line);
    token = strtok(linecpy, delim);
    return atoi(token);
}

_process* parse_line(char *line, uint32_t *num_process) {
    const char delim[4] = "() ";
    char *token;
    _process *process_list;
    int val[4];

    // does not ingest process count prefix
    *num_process = get_process_count(line);
    process_list = (_process*)malloc(*num_process * sizeof(_process));

    // skip over count prefix
    token = strtok(line, " ");
    token = strtok(NULL, delim);

    // iterate over quadruples, ignore comment
    for (int quad = 0; quad < *num_process; quad++) {
        // iterate over values of current quadruple
        for (int n = 0; n < 4; n++) {
            if (token != NULL) {
                val[n] = atoi(token);
            } else {
                // malformed string
                return NULL;
            }
            token = strtok(NULL, delim);
        }

        process_list[quad] = init_process(val[0], val[1], val[2], val[3], quad);
    }

    return process_list;
}

_process* parse_file(char *filename, uint32_t *num_process) {
    char *line;
    size_t len = 0;
    FILE *processfile = fopen(filename, "r");

    if (processfile == NULL) {
        return NULL;
    }

    // get input from processfile
    getline(&line, &len, processfile);
    return parse_line(line, num_process);
}

int main(int argc, char *argv[])
{
    FILE *randfile = fopen(RANDOM_NUMBER_FILE_NAME, "r");
    _process *process_list, *finished_process_list;

    process_list = parse_file("sample_io/input/input-3", &TOTAL_CREATED_PROCESSES);
    finished_process_list = (_process*)malloc(TOTAL_CREATED_PROCESSES * sizeof(_process));

    // run each scheduler
    for (int scheduler = 0; scheduler < 3; scheduler++) {
        reset_counters();
        reset_process_list(process_list);  // can assume finished_process_list to contain "garbage" on each iteration

        if (scheduler == 0) {
            printf("START OF FIRST COME FIRST SERVE\n");
            fcfs_run(process_list, finished_process_list, randfile);
        } else if (scheduler == 1) {
            printf("\nSTART OF ROUND ROBIN\n");
            rr_run(process_list, finished_process_list, randfile);
        } else {
            printf("\nSTART OF SHORTEST JOB FIRST\n");
            sjf_run(process_list, finished_process_list, randfile);
        }
        
        printStart(process_list);
        printFinal(finished_process_list);
        printProcessSpecifics(process_list);
        printSummaryData(process_list);
    }

    fclose(randfile);
    return 0;
}
