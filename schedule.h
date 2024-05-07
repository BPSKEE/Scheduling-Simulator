#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <queue>
#include <algorithm>
#include <iomanip>

using namespace std;

struct Process {
    vector<int> cpuBursts;
    vector<int> ioBursts;
    vector<int> bursts;
    int pid;
    int currentBurst = 0;
    int waitTime = 0;
    int turnaround = 0;
    int readyTime = 0;
    int cpuBurst = 0;
    int ioBurst = 0;
};


void parseInput(const string& filename, vector<Process>& processes);
void fcfs(const vector<Process>& processes);
void rr(vector<Process>& processes, int quantum);
bool sortReadyTime(const Process& p1, const Process& p2);











/*
#ifndef SCHEDULE_H
#define SCHEDULE_H
#define MAX_BURSTS 100
#include <vector>






typedef struct Process {
    int processID;
    int numBursts;
    int bursts[MAX_BURSTS];
    int cpuBursts = 0;
    int ioBursts = 0;
    int turnaround = 0;
    struct Process *next;
} Process;

void fcfs(Process *processes, int numProcesses);
void rr(Process *processes, int numProcesses, int quantum);
Process* parseInput(const char* filename, int* numProcesses);

#endif

*/
