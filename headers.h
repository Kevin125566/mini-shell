#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include <signal.h>
#include <string.h>
#include <vector>
#include <iomanip>
#include <math.h>
#include <fcntl.h>

using namespace std;

struct process {
    int pid;
    char state;
};

int parseInput(char* input, char** args);
string sliceString(char *toSlice);
void readPs(char* input, char** result);

void checkChildProcesses(vector<struct process*> &pcb);
void exitShell(vector<struct process *> &pcb);

void printJobs(vector<struct process*> &pcb);
void killJob(vector<struct process*> &pcb, int pid);
void resumeJob(int pid, vector<struct process*> &pcb);
void suspendJob(int pid, vector<struct process*> &pcb);

void pipedExec(char **args, char *fileToRead, char *fileToWrite, int readFile, int writeFile, vector<struct process *> &pcb, int argsSize);
void regularExec(char **args, vector<struct process *> &pcb, int argsSize);