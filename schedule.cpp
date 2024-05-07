//Brandon Skeens
//CS 480
//826416091

/*
Single Programmer Affidavit
I the undersigned promise that the submitted assignment is my own work. While I was
free to discuss ideas with others, the work contained is my own. I recognize that
should this not be the case; I will be subject to penalties as outlined in the course
syllabus.
Name: Brandon Skeens
Red ID: 826416091
*/

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <cstring>
#include "schedule.h"
#include "log.c"
#include <string.h>
#include <algorithm>

using namespace std;

//FCFS scheduling function
void fcfs(const vector<Process>& processes) {
    //Set variables
    int currentTime = 0;

    queue<Process> ready;
    vector<Process> blocked;
    vector<Process> done;
    Process currentProcess;

    for(int i = 0; i < static_cast<int>(processes.size()); ++i) {
        ready.push(processes[i]);
    }

    while (!ready.empty() || !blocked.empty()) {
        if (!ready.empty()) {
            //Load next ready process into currentProcess
            currentProcess = ready.front();
            ready.pop();

            //Update process wait time
            currentProcess.waitTime += currentTime - currentProcess.readyTime;

            //Update CPU burst time
            currentProcess.cpuBurst += currentProcess.bursts[currentProcess.currentBurst];

            //Set current time to add the current burst, then increment currentburst for process
            currentTime += currentProcess.bursts[currentProcess.currentBurst];
            currentProcess.currentBurst++;

            //Execute if the last burst in a process was just executed
            if (currentProcess.currentBurst >= static_cast<int>(currentProcess.bursts.size())) {
                //Set turnaround time to current time, log, and push to done queue
                log_cpuburst_execution(currentProcess.pid, currentProcess.cpuBurst, currentProcess.ioBurst, currentTime, COMPLETED);
                currentProcess.turnaround = currentTime;
                done.push_back(currentProcess);
            } //Execute if there are more bursts remaining in the process;
            else{
                //Set when the process will be ready, then log and push to blocked queue
                currentProcess.readyTime = currentTime + currentProcess.bursts[currentProcess.currentBurst];
                log_cpuburst_execution(currentProcess.pid, currentProcess.cpuBurst, currentProcess.ioBurst, currentTime, ENTER_IO);
                blocked.push_back(currentProcess);
            }

            //Sort blocked queue and search to see if any members can be put into the ready queue
            sort(blocked.begin(), blocked.end(), sortReadyTime);
            for (int i = 0; i < static_cast<int>(blocked.size()); ++i) {
                if (blocked[i].readyTime <= currentTime) {
                    blocked[i].ioBurst += blocked[i].bursts[blocked[i].currentBurst];
                    blocked[i].currentBurst++;

                    //Push to ready queue and remove from blocked queue
                    ready.push(blocked[i]);
                    blocked.erase(blocked.begin() + i);
                    --i;
                } else {break;}
            }

        } //if ready queue is empty, set current time to be the ready time for the next lowest ready time
        else {
            //Values to store lowest wait time and the index of the time
            int lowestWait = blocked[0].readyTime;
            int lowestIndex = 0;

            //iterate through blocked queue to find time and index
            for (int i = 1; i < static_cast<int>(blocked.size()); ++i) {
                if (blocked[i].readyTime < lowestWait) {
                    lowestWait = blocked[i].readyTime;
                    lowestIndex = 1;
                }
            }

            //set current time to the lowest wait, then push to ready queue and remove from blocked queue after updating necessary values
            currentTime = lowestWait;
            blocked[lowestIndex].ioBurst += blocked[lowestIndex].bursts[blocked[lowestIndex].currentBurst];
            blocked[lowestIndex].currentBurst++;
            ready.push(blocked[lowestIndex]);
            blocked.erase(blocked.begin() + lowestIndex);
        }
    }

    for (int i = 0; i < static_cast<int>(done.size()); ++i) {
        log_process_completion(done[i].pid, done[i].turnaround, done[i].waitTime);
    }

}

//Round robin scheduling function
void rr(vector<Process>& processes, int quantum) {
    int currentTime = 0;
    queue<Process> ready;
    vector<Process> done;
    vector<Process> blocked;
    Process currentProcess;

    //Add all processes to the queue
    for (int i = 0; i < static_cast<int>(processes.size()); ++i) {
        ready.push(processes[i]);
    }

    
    while (!ready.empty() || !blocked.empty()) {
        //Set current process to the front of ready queue
        if (!ready.empty()) {
            currentProcess = ready.front();
            ready.pop();
        
            //Add to process wait time based on how much time was spent in the ready queue
            currentProcess.waitTime += currentTime - currentProcess.readyTime;

            for(int i = 1; i <= quantum; ++i) {
                //Increment time, decrement the remaining time on current burst, increment cpuBurst figure
                currentTime++;
                currentProcess.bursts[currentProcess.currentBurst]--;
                currentProcess.cpuBurst++;

                //If current burst is finished
                if (currentProcess.bursts[currentProcess.currentBurst] < 1) {
                    //Increment current burst
                    currentProcess.currentBurst++;

                    //If there are no remaining process bursts
                    if (currentProcess.currentBurst >= currentProcess.bursts.size()) {
                        //Log burst execution, set turnaround time, push to done queue
                        log_cpuburst_execution(currentProcess.pid, currentProcess.cpuBurst, currentProcess.ioBurst, currentTime, COMPLETED);
                        currentProcess.turnaround = currentTime;
                        done.push_back(currentProcess);
                    } //if there are remaining bursts
                    else {
                        //Log burst execution, set ready time for next cpu burst, send to blocked queue
                        currentProcess.readyTime = currentProcess.bursts[currentProcess.currentBurst] + currentTime;
                        log_cpuburst_execution(currentProcess.pid, currentProcess.cpuBurst, currentProcess.ioBurst, currentTime, ENTER_IO);
                        blocked.push_back(currentProcess);
                    }
                    break;
                } //If quantum expires and there is still time left on the burst
                else if (i == quantum && currentProcess.bursts[currentProcess.currentBurst] > 0){
                    //Set readytime to the current time - 1 to compensate for the extra millisecond, then push to ready and log the burst
                    currentProcess.readyTime = currentTime;
                    blocked.push_back(currentProcess);
                    log_cpuburst_execution(currentProcess.pid, currentProcess.cpuBurst, currentProcess.ioBurst, currentTime, QUANTUM_EXPIRED);
                    break;
                }
            }
        }
        else {
            //If there are no ready processes, just increment the time then check blocked
            currentTime++;
        }

        //Temporary array
        vector<Process> newBlocked;

        //If blocked isn't empty, sort the vector by ready time then look for ready processes
        if (!blocked.empty()) {
            stable_sort(blocked.begin(), blocked.end(), [](const Process& p1, const Process& p2) {
                return p1.readyTime < p2.readyTime;
            });


            //For loop goes through sorted blocked vector looking for ready bursts
            for (int i = 0; i < blocked.size(); ++i) {
                if (blocked[i].readyTime <= currentTime) {
                    //if the burst is ready, check if it is an io or cpu burst
                    if (blocked[i].currentBurst % 2 == 1) {
                        //IO bursts will increment the io burst count and the current burst, then get pushed to ready
                        blocked[i].ioBurst += blocked[i].bursts[blocked[i].currentBurst];
                        blocked[i].currentBurst++;
                        ready.push(blocked[i]);
                    } else {
                        //CPU bursts from quantum expired will just be pushed to the back of ready
                        ready.push(blocked[i]);
                    }
                } else {
                    //If burst is not ready, return it to blocked
                    newBlocked.push_back(blocked[i]);
                }
            } 
            //Set blocked to the temporary blocked vector
            blocked = newBlocked;
        }
        
    }



    for (int i = 0; i < static_cast<int>(done.size()); ++i) {
        log_process_completion(done[i].pid, done[i].turnaround, done[i].waitTime);
    } 
} 

//Function to read in and parse input file into a vector of Processes
void parseInput(const string& filename, vector<Process>& processes) {
    //Create input filestream and check if it is open
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Unable to open <<" << filename << ">>" << endl;
        exit(1);
    }

    //Read file in line-by-line, assigning process ID and other variables
    string line;
    int processID = 0;

    while (getline(file, line)) {
        int burstNum = 1;
        Process process;
        process.pid = processID;
        stringstream ss(line);
        int burst;

        //while there is more to be read in on a line, read each burst into the process.bursts vector
        while (ss >> burst) {
            if (burst < 0) {
                cout << "A burst number must be bigger than 0" << endl;
                exit(1);
            }
            process.bursts.push_back(burst);
            if (burstNum % 2 == 1) {
                process.cpuBursts.push_back(burst);
            } else {
                process.ioBursts.push_back(burst);
            }
            burstNum++;
        }

        if (process.bursts.size() % 2 == 0) {
            cout << "There must be an odd number of bursts for each process" << endl;
            exit(1);
        }
        //push process to process vector, then increment processID
        processes.push_back(process);
        processID++;
    }
}

int main(int argc, char *argv[]) {
    //Create file and set default quantum and shceudling streategy
    const char* file = nullptr;
    int schedulingStrategy = 1;
    int quantum = 2;

    //exit if input is too long, too short, or is not divisible by 2
    if (argc < 2 || argc > 6 || argc % 2 == 1) {
        exit(1);
    } 
    else {
        file = argv[1];
        for (int i = 2; i < argc; ++i) {
            if (strcmp(argv[i], "-s") == 0) {
                //Check for scheduling strategy specifier
                if (i + 1 < argc) {
                    //if
                    if (strcmp(argv[i + 1], "rr") == 0) {
                        //Set to fcfs by default, so change to rr if input is rr
                        schedulingStrategy = 2;
                    }
                    else if (strcmp(argv[i + 1], "fcfs") != 0) {
                        //Exit if scheduling strategy is not rr or fcfs 
                        exit(1);
                    }
                } else {
                    //exit if there is no character after the "-s" to define scheduling strategy
                    exit(1);
                }
            }
            else if (strcmp(argv[i], "-q") == 0 && i + 1 < argc) {
                //Check for quantum specifier and convert input to quantum
                quantum = atoi(argv[i + 1]);
                if (quantum < 1) {
                    cout << "Time quantum must be a number and bigger than 0" << endl;
                    //If atoi fails, it returns 0, which means it would be an invalid input
                    exit(1);
                }
            }
        }
    }

    //Create vector to store processes from input file
    vector<Process> processes;
    parseInput(file, processes);

    //Print out processes and bursts read in
    for (int i = 0; i < processes.size(); ++i) {
        for (int j = 0; j < processes[i].bursts.size(); ++j) {
            cout << processes[i].bursts[j] << " " ;
        }
        cout << "\n" ;
    }

    //Run fcfs or rr based on scheduling strategy ready in
    if (schedulingStrategy == 1) {fcfs(processes);}
    else {rr(processes, quantum);}

    return 0;
}

//Helper function for sorting
bool sortReadyTime(const Process& p1, const Process& p2) {
    return p1.readyTime < p2.readyTime;
}
