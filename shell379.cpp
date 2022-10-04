#include "headers.h"

int main() {
    vector<struct process *> pcb;

    while (1) {
        char input[200];
        char *args[200];
        int argsSize = 0;

        memset(args, '\0', 200);

        cout << "SHELL379: ";
        cin.getline(input, 200);
        
        argsSize = parseInput(input, args);
        checkChildProcesses(pcb);

        if (input[0] == '\0') {
            continue;
        }
        else if (strcmp(args[0], "exit") == 0) {
            exitShell(pcb);
            break;
        }
        else if (strcmp(args[0], "jobs") == 0) {
            printJobs(pcb);
        }
        else if (strcmp(args[0], "kill") == 0) {
            int pidToKill = stoi(args[1]);

            killJob(pcb, pidToKill);
        }
        else if (strcmp(args[0], "resume") == 0) {
            int pidToResume = stoi(args[1]);

            resumeJob(pidToResume, pcb);
        }
        else if (strcmp(args[0], "sleep") == 0) {
            int sleepDuration = stoi(args[1]);

            sleep(sleepDuration);
        }
        else if (strcmp(args[0], "suspend") == 0) {
            int pidToSuspend = stoi(args[1]);
            
            suspendJob(pidToSuspend, pcb);
        }
        else if (strcmp(args[0], "wait") == 0) {
            int c_pid = stoi(args[1]);
            int status;

            waitpid(c_pid, &status, 0);
        }
        else {
            int readFile = 0, writeFile = 0;
            char fileToRead[200];
            char fileToWrite[200];
            string resultRead;
            string resultWrite;
            
            // separate the filenames from < and >
            for (int i = 0; i < argsSize; i++) {
                if (strchr(args[i], '<') != NULL) {
                    readFile = 1;
                    resultRead = sliceString(args[i]);
                    strcpy(fileToRead, resultRead.c_str());
                }
                else if (strchr(args[i], '>') != NULL) {
                    writeFile = 1;
                    resultWrite = sliceString(args[i]);
                    strcpy(fileToWrite, resultWrite.c_str());
                }
            }

            if (readFile || writeFile) {
                pipedExec(args, fileToRead, fileToWrite, readFile, writeFile, pcb, argsSize);
            }
            else regularExec(args, pcb, argsSize);
        }
    }
}