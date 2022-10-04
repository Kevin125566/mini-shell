#include "headers.h"

void getProcessStat(int pid, char *type) {
    char buf[200];
    int status, fd[2];

    if (pipe(fd) < 0) perror("pipe error");

    pid_t c_pid = fork();

    if (c_pid < 0) {
        perror("Could not fork");
    }
    else if (!c_pid) {
        // child process
        close(fd[0]);
        dup2(fd[1], STDOUT_FILENO);
        close(fd[1]);

        string tmp = to_string(pid);
        char numChar[tmp.length() + 1];
        strcpy(numChar, tmp.c_str());

        char *args[] = {"ps", "-p", numChar, "-o", type, NULL};

        if (execvp(args[0], args) < 0) {
            perror("execvp");
            exit(127);
        }
    }
    else {
        // parent process
        char *result[200];
        close(fd[1]);
        read(fd[0], buf, 200);
        close(fd[0]);

        waitpid(c_pid, &status, 0);

        readPs(buf, result); // separate "TIME" and actual time
        cout << result[1] << " ";
    }
}

void checkChildProcesses(vector<struct process*> &pcb) {
    for (vector<struct process *>::iterator it = pcb.begin(); it != pcb.end();) {
        int status = -1;

        if (waitpid((*it)->pid, &status, WNOHANG) && WIFEXITED(status)) {
            // child process has exited -- remove from pcb
            free(*it);
            it = pcb.erase(it);
        }
        else it++;
    }
}

void exitShell(vector<struct process *> &pcb) {
    int userTime, sysTime;
    struct rusage usage;

    while (pcb.size() != 0) {
        int status = -1;

        if (kill(pcb.back()->pid, SIGKILL)) perror("Could not kill process");

        waitpid(pcb.back()->pid, &status, 0);
        free(pcb.back());
        pcb.pop_back();
    }

    getrusage(RUSAGE_CHILDREN, &usage);
    userTime = usage.ru_utime.tv_sec;
    sysTime = usage.ru_stime.tv_sec;

    cout << "Resources used:\n";
    cout << "User time = " << right << setw(6) << userTime << " seconds\n";
    cout << "Sys  time = " << right << setw(6) << sysTime << " seconds\n";
}

void printJobs(vector<struct process*> &pcb) {
    int userTime, sysTime;
    struct rusage usage;

    getrusage(RUSAGE_CHILDREN, &usage);
    userTime = usage.ru_utime.tv_sec;
    sysTime = usage.ru_stime.tv_sec;

    cout << "Running processes:\n";
    if (pcb.size() > 0) {
        cout << "#    PID S SEC COMMAND\n";
        for (int i = 0; i < pcb.size(); i++) {
            cout << i << ":";
            cout << right << setw(6) << pcb[i]->pid << " "; // right << setw() is right justification
            cout << pcb[i]->state << " " << right << setw(3);
            // output both process time and command
            getProcessStat(pcb[i]->pid, "time");
            getProcessStat(pcb[i]->pid, "command");
            cout << endl;
        }
    }
    cout << "Processes = " << right << setw(6) << pcb.size() << " active\n";
    cout << "Completed processes:\n";
    cout << "User time = " << right << setw(6) << userTime << " seconds\n";
    cout << "Sys  time = " << right << setw(6) << sysTime << " seconds\n";
}

void killJob(vector<struct process*> &pcb, int pid) {
    int indexToRemove = -1;
    int status;

    for (int i = 0; i < pcb.size(); i++) {
        if (pcb[i]->pid == pid) indexToRemove = i;
    }
    
    if (indexToRemove != -1) {
        if (!kill(pid, SIGKILL)) {
            waitpid(pid, &status, 0);
            free(pcb[indexToRemove]);
            pcb.erase(pcb.begin()+indexToRemove);
        }
        else perror("Could not kill process");
    }
    else cout << "No such process\n";
}

void resumeJob(int pid, vector<struct process *> &pcb) {
    if(kill(pid, SIGCONT)) perror("Could not resume process");
    else {
        for (int i = 0; i < pcb.size(); i++) {
            if (pcb[i]->pid == pid) {
                pcb[i]->state = 'R';
                break;
            }
        }
    }
}

void suspendJob(int pid, vector<struct process *> &pcb) {
    if (kill(pid, SIGSTOP)) perror("Could not suspend process");
    else {
        for (int i = 0; i < pcb.size(); i++) {
            if (pcb[i]->pid == pid) {
                pcb[i]->state = 'T';
                break;
            }
        }
    }
}

void pipedExec(char **args, char *fileToRead, char *fileToWrite, int readFile, int writeFile, vector<struct process *> &pcb, int argsSize) {
    int ffd, fdd;
    pid_t pid;

    if ((pid = fork()) < 0) {
        perror("Could not fork");
    }
    else if(!pid) {
        if (readFile) {
            //https://stackoverflow.com/questions/11515399/implementing-shell-in-c-and-need-help-handling-input-output-redirection
            if ((fdd = open(fileToRead, O_RDONLY)) < 0) {
                perror("open failed");
                exit(127);
            }
            dup2(fdd, STDIN_FILENO);
            close(fdd);
        }
        if (writeFile) {
            // open file --> if not exists then create it
            if ((ffd = open(fileToWrite, O_CREAT | O_WRONLY, 0644)) < 0) {
                perror("open failed");
                exit(127);
            }
            dup2(ffd, STDOUT_FILENO);
            close(ffd);
        }
        if (execvp(args[0], args) < 0) {
            perror("execvp error");
            exit(127);
        }
    }
    else {
        int backgroundProcess = 0;
                
        if (strcmp(args[argsSize - 1], "&") == 0) backgroundProcess = 1;

        if (backgroundProcess) {
            struct process *newProc = (struct process*) malloc(sizeof(struct process));
            newProc->pid = pid;
            newProc->state = 'R';
            pcb.push_back(newProc);
        }
        else {
            int status;
            waitpid(pid, &status, 0);
        }
    }
}

void regularExec(char **args, vector<struct process *> &pcb, int argsSize) {
    pid_t pid = fork();

    if (pid < 0) {
        perror("Could not fork");
    }
    else if (!pid) {
        if (execvp(args[0], args) == -1) {
            perror("execvp");
            exit(127);
        }
    }
    else {
        int backgroundProcess = 0;
                
        if (strcmp(args[argsSize - 1], "&") == 0) backgroundProcess = 1;

        if (backgroundProcess) {
            struct process *newProc = (struct process*) malloc(sizeof(struct process));
            newProc->pid = pid;
            newProc->state = 'R';
            pcb.push_back(newProc);
        }
        else {
            int status;
            waitpid(pid, &status, 0);
        }
    }
}