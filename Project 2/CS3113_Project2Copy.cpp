#include <iostream>
#include <queue>
#include <string>
#include <vector>
#include <tuple>
using namespace std;

struct PCB {
    // Define PCB fields as described earlier
    int processID; //The state field tracks the current status of the process, which can be one of the following: NEW, READY, RUNNING, or TERMINATED.
    int programCounter; //the index of the next instruction to be executed within the process's logical memory
    int instructionBase; //specifies the starting address of the instructions in the logical memory
    int dataBase; //points to the beginning of the data segment within the logical memory
    int memoryLimit; //defines the total size of logical memory allocated to the process
    int cpuCyclesUsed; //accumulates the total CPU cycles consumed by the process during its execution
    int registerValue; //simulated register used to store intermediate values during load and store operations
    int maxMemoryNeeded; //specifies the maximum memory required by the process as defined in the input file
    int mainMemoryBase; //denotes the starting address in main memory where the process, including its PCB and logical memory, is loaded
};

int globalClock;

/*void printRQueue(queue<tuple<int, int>>& readyQueue) {
    // Print Ready Queue
    for (int i = 0; i < readyQueue.size(); i++) {
        tuple<int, int> startAndDAddress = readyQueue.front();
        cout << get<0>(startAndDAddress) << "," << get<1>(startAndDAddress) << " ";
        readyQueue.push(startAndDAddress);
        readyQueue.pop();
    }
    cout << endl;
}

void printIOQueue(queue<vector<int>>& IOWaitingQueue) {
    // Print IO Waiting Queue
    for (int i = 0; i < IOWaitingQueue.size(); i++) {
        vector<int> processIOTime = IOWaitingQueue.front();
        cout << processIOTime[0] << "," << processIOTime[1] << "," << processIOTime[2] << "," << processIOTime[3] << " ";
        IOWaitingQueue.push(processIOTime);
        IOWaitingQueue.pop();
    }
    cout << endl;
}*/

void TimeOUT(PCB job, queue<tuple<int, int>>& readyQueue, vector<int>& mainMemory, int startAddress, int DAddress) {
    /*cout << "TimeOUT StartAddress: " << startAddress << endl;
    cout << "TimeOUT DAddress: " << DAddress << endl;
    cout << "TO R Queue Before Push:";
    printRQueue(readyQueue);*/
    //Add process to ReadyQueue
    readyQueue.push(make_tuple(startAddress, DAddress));
    /*cout << "TO R Queue After Push:";
    printRQueue(readyQueue);*/
    //Update PCB state to READY
    mainMemory[startAddress + 1] = 2; // state ("READY")
    //Print compute message
    //cout << "compute" << endl;
    //Print TimeOUT message
    cout << "Process " << job.processID << " has a TimeOUT interrupt and is moved to the ReadyQueue." << endl;
}

void IOInterrupt(PCB job, queue<vector<int>>& IOWaitingQueue, vector<int>& mainMemory, int startAddress, int cycles, int DAddress) {
    /*cout << "IO Queue Before Push: ";
    printIOQueue(IOWaitingQueue);*/
    //Combine process start address and IO time
    vector<int> processIOTime;
    processIOTime.push_back(startAddress);
    processIOTime.push_back(globalClock);
    processIOTime.push_back(cycles);
    processIOTime.push_back(DAddress);

    //--cout << "ProcessIOTime:" << processIOTime[0] << " " << processIOTime[1] << " " << processIOTime[2] << " " << processIOTime[3] << " " << endl;

    //Add process to IOWaitingQueue
    IOWaitingQueue.push(processIOTime);
    /*cout << "IO Queue After Push: ";
    printIOQueue(IOWaitingQueue);*/

    //Update PCB state to IOWaiting
    mainMemory[startAddress + 1] = 5; // state ("IOWaiting")
    //Print IOInterrupt message
    cout << "Process " << job.processID << " issued an IOInterrupt and moved to the IOWaitingQueue." << endl;
}

void checkIOWaitingQueue(queue<vector<int>>& IOWaitingQueue, vector<int>& mainMemory, queue<tuple<int, int>>& readyQueue, int contextSwitchTime) {
    //Check IO Waiting Queue for any processes that have completed IO
    if(!IOWaitingQueue.empty()) {
        int currentIOSize = IOWaitingQueue.size();
        //--cout << "IOC size:" << currentIOSize << endl;
        for(int i = 0; i < currentIOSize; i++) {
            vector<int> processIOTime = IOWaitingQueue.front();
            int processStartAddress = processIOTime[0];
            int processStartTime = processIOTime[1];
            int processCycles = processIOTime[2];
            int processDAddress = processIOTime[3];
            
            //--cout << "Global Clock E: " << globalClock << endl;
            //--cout << "Cycles to go: " << processCycles - (globalClock - processStartTime) << endl;
            if(globalClock - processStartTime >= processCycles) {
                /*cout << "IOCI R Queue Before: ";
                printRQueue(readyQueue);
                cout << "IOCI IO Queue Before: ";
                printIOQueue(IOWaitingQueue);*/
                //globalClock += processCycles;
                //Add process to ReadyQueue
                readyQueue.push(make_tuple(processStartAddress, processDAddress));
                //Update PCB state to READY
                mainMemory[processStartAddress + 1] = 2; // state ("READY")
                //Print and IOCompletion message
                cout << "print" << endl;
                cout << "Process " << mainMemory[processStartAddress] << " completed I/O and is moved to the ReadyQueue." << endl;
                //Remove process from IOWaitingQueue
                IOWaitingQueue.pop();
                /*cout << "IOCI R Queue After: ";
                printRQueue(readyQueue);
                cout << "IOCI IO Queue After: ";
                printIOQueue(IOWaitingQueue);*/
            }
            else {
                /*cout << "IOCE IO Queue Before: ";
                printIOQueue(IOWaitingQueue);*/
                //Move process to back of IOWaitingQueue
                IOWaitingQueue.push(processIOTime);
                IOWaitingQueue.pop();
                /*cout << "IOCE IO Queue After: ";
                printIOQueue(IOWaitingQueue);*/
            }
        }
    }
}

bool contextSwitch(queue<tuple<int, int>>& readyQueue, queue<vector<int>>& IOWaitingQueue, vector<int>& mainMemory, int startAddress, int contextSwitchTime, int DAddress) {
    bool executeNewProcess = true;

    /*cout << "CS R Queue Before: ";
    printRQueue(readyQueue);
    cout << "CS IO Queue Before: ";
    printIOQueue(IOWaitingQueue);*/
    
    //Check IO Waiting Queue for any processes that have completed IO
    checkIOWaitingQueue(IOWaitingQueue, mainMemory, readyQueue, contextSwitchTime);
    
    //Increment global clock
    //globalClock += contextSwitchTime;
    
    if(readyQueue.empty() && IOWaitingQueue.empty()) {
        //--cout << "ready empty and io empty " << endl;
        //If readyQueue is empty and IOWaitingQueue is empty, execute new process
        executeNewProcess = false;
        //--cout << "Empty CS Execute New Process: " << executeNewProcess << endl;
    }

    /*if(readyQueue.empty() && !IOWaitingQueue.empty()) {
        cout << "ready empty and io not empty " << endl;
        //If readyQueue is empty and IOWaitingQueue is not empty, increment global clock by context switch time
        globalClock += contextSwitchTime;
    }*/

    /*cout << "CS R Queue After: ";
    printRQueue(readyQueue);
    cout << "CS IO Queue After: ";
    printIOQueue(IOWaitingQueue);*/

    //--cout << "CS Execute New Process: " << executeNewProcess << endl;
    return executeNewProcess;
}

void loadJobsToMemory(queue<PCB>& newJobQueue, queue<tuple<int, int>>& readyQueue, vector<int>& mainMemory, int maxMemory, int numProcesses) {
    // TODO: Implement loading jobs into main memory

    // Initialize starting address and PCB size
    tuple <int, int> startAndDAddress = readyQueue.back();
    int startingAddress = get<0>(startAndDAddress);
    int DAddress = get<1>(startAndDAddress);
    int PCBSize = 10;
    
    // Pop the next job from the new job queue
    PCB newJob = newJobQueue.front();
    newJobQueue.pop();

    // Read number of instructions
    int numInstructions = 0;
    cin >> numInstructions;

    // Store PCB fields in main memory
    mainMemory[startingAddress] = newJob.processID; // processID
    mainMemory[startingAddress + 1] = 1; // state ("NEW")
    mainMemory[startingAddress + 2] = 0; // programCounter
    mainMemory[startingAddress + 3] = startingAddress + PCBSize; // instructionBase
    newJob.instructionBase = startingAddress + PCBSize;
    mainMemory[startingAddress + 4] = startingAddress + PCBSize + numInstructions; // dataBase
    newJob.dataBase = startingAddress + PCBSize + numInstructions;
    mainMemory[startingAddress + 5] = newJob.maxMemoryNeeded; // memoryLimit
    mainMemory[startingAddress + 6] = 0; // cpuCyclesUsed
    mainMemory[startingAddress + 7] = 0; // registerValue
    mainMemory[startingAddress + 8] = newJob.maxMemoryNeeded; // maxMemoryNeeded
    mainMemory[startingAddress + 9] = startingAddress; // mainMemoryBase

    // Set Address for next process if one exists
    if (readyQueue.size() < numProcesses) {
        startingAddress += PCBSize + newJob.maxMemoryNeeded;
        DAddress = 0;
        //--cout << "Ready DAddress: " << DAddress << endl;
        readyQueue.push(make_tuple(startingAddress, DAddress));
    }

    // Store instructions in main memory
    int currentIAddress = newJob.instructionBase;
    int currentDAddress = newJob.dataBase;

    // Loop through instructions and store them in main memory
    for (int i = 0; i < numInstructions; i++) {
        cin >> mainMemory[currentIAddress];

        // Store data for instructions based on their type
        switch(mainMemory[currentIAddress]) {
           case 1: {
                cin >> mainMemory[currentDAddress];
                cin >> mainMemory[currentDAddress + 1];
                currentDAddress += 2;
                break;
            }
            case 2: {
                cin >> mainMemory[currentDAddress];
                currentDAddress++;
                break;
            }
            case 3: {
                cin >> mainMemory[currentDAddress];
                cin >> mainMemory[currentDAddress + 1];
                currentDAddress += 2;
                break;
            }
            case 4: {
                cin >> mainMemory[currentDAddress];
                currentDAddress++;
                break;
            }
        }
        
        currentIAddress++; // Increment instruction address
    }
    
}

void executeCPU(int startAddress, vector<int>& mainMemory, queue<tuple<int, int>>& readyQueue, queue<vector<int>>& IOWaitingQueue, int CPUAllocated, int contextSwitchTime, int DAddress, vector<int>& startTimes) {
    // TODO: Implement CPU instruction execution
    /*cout << "Execute R Queue Before: ";
    printRQueue(readyQueue);
    cout << "Execute IO Queue Before: ";
    printIOQueue(IOWaitingQueue);*/
    bool executeNewProcess = false;
    //--cout << "Execute CS call" << endl;
    //executeNewProcess = contextSwitch(readyQueue, IOWaitingQueue, mainMemory, startAddress, contextSwitchTime, DAddress);

    // Initialize starting address and PCB size
    int PCBSize = 10;
    
    // Read PCB fields from main memory
    PCB newJob;
    newJob.processID = mainMemory[startAddress]; // processID
    mainMemory[startAddress + 1] = 3; // state ("RUNNING")
    newJob.programCounter = mainMemory[startAddress + 2]; // programCounter
    newJob.instructionBase = mainMemory[startAddress + 3]; // instructionBase
    newJob.dataBase = mainMemory[startAddress + 4]; // dataBase
    newJob.memoryLimit = mainMemory[startAddress + 5]; // memoryLimit
    newJob.cpuCyclesUsed = mainMemory[startAddress + 6]; // cpuCyclesUsed
    newJob.registerValue = mainMemory[startAddress + 7]; // registerValue
    newJob.maxMemoryNeeded = mainMemory[startAddress + 8]; // maxMemoryNeeded
    newJob.mainMemoryBase = mainMemory[startAddress + 9]; // mainMemoryBase

    // Print Running Message
    cout << "Process " << newJob.processID << " has moved to Running." << endl;
    globalClock += contextSwitchTime;
    //--cout << "Global Clock B: " << globalClock << endl;

    //Check if process is starting
    if(newJob.programCounter == 0) {
        //Log the start time of the process
        startTimes[newJob.processID - 1] = globalClock;
    }

    // Set starting address for instructions
    int IAddress = newJob.instructionBase;
    
    //--cout << "Cyles: " << newJob.cpuCyclesUsed << endl;
    if(newJob.cpuCyclesUsed == 0) {
        //newJob.programCounter = 0;
        DAddress = newJob.dataBase;
    }
    
    // Count Instructions
    int numInstructions = 0;
    for(int i = IAddress; i < mainMemory[startAddress + 4]; i++) {
        numInstructions++;
    }

    // Initialize variables for instruction execution
    int instruction;
    int iterations;
    int cycles;
    int totalCycles = 0;
    int value;
    int storeLoction;
    bool endProcess = false;

    //How do I save the progress of the process
    while(newJob.programCounter < numInstructions) {
        //cout << "P#" << newJob.processID << ", PC:" << mainMemory[startAddress + 2] << ", I#:" << numInstructions << ", DAd:" << DAddress << endl; 
            
        // Fetch instruction
        instruction = mainMemory[IAddress + newJob.programCounter];
        //--cout << "Instruction num: " << instruction << endl;
        newJob.programCounter++;
        mainMemory[startAddress + 2] = newJob.programCounter;
        //--cout << "Program Counter C: " << mainMemory[startAddress + 2] << endl;

        // Decode and execute instruction
        switch(instruction) {
            case 1: {
                // Compute instruction
                //--cout << "Compute DAddress Before:" << DAddress << endl;
                iterations = mainMemory[DAddress];
                cycles = mainMemory[DAddress + 1];
                DAddress += 2;
                newJob.cpuCyclesUsed += cycles;
                mainMemory[startAddress + 6] += cycles;
                globalClock += cycles;
                totalCycles += cycles;
                cout << "compute" << endl;
                //--cout << "Global Clock C: " << globalClock << endl;
                //--cout << "cycles:" << cycles << endl;
                /*if(newJob.programCounter < numInstructions) {
                    if(cycles >= CPUAllocated) {
                        TimeOUT(newJob, readyQueue, mainMemory, startAddress, DAddress);
                        //--cout << "Compute CS call" << endl;
                        executeNewProcess = contextSwitch(readyQueue, IOWaitingQueue, mainMemory, startAddress, contextSwitchTime, DAddress);
                        endProcess = executeNewProcess;
                    }
                }
                else {
                    cout << "compute" << endl;
                }*/
                //--cout << "C Process " << newJob.processID << " cycles used: " << newJob.cpuCyclesUsed << endl;
                //--cout << "Compute DAddress After:" << DAddress << endl;
                break;
            }
            case 2: {
                // Print instruction
                //--cout << "Print DAddress Before:" << DAddress << endl;
                cycles = mainMemory[DAddress];
                DAddress += 1;
                newJob.cpuCyclesUsed += cycles;
                mainMemory[startAddress + 6] += cycles;
                IOInterrupt(newJob, IOWaitingQueue, mainMemory, startAddress, cycles, DAddress);
                //--cout << "Print CS call" << endl;
                executeNewProcess = contextSwitch(readyQueue, IOWaitingQueue, mainMemory, startAddress, contextSwitchTime, DAddress);
                endProcess = executeNewProcess;
                //--cout << "P Process " << newJob.processID << " cycles used: " << newJob.cpuCyclesUsed << endl;
                //--cout << "Print DAddress After:" << DAddress << endl;
                break;
            }
            case 3: {
                // Store instruction
                //--cout << "Store DAddress Before:" << DAddress << endl;
                value = mainMemory[DAddress];
                //--cout << "value: " << value << endl;
                newJob.registerValue = value;
                //--cout << "registerValue: " << newJob.registerValue << endl;
                storeLoction = mainMemory[DAddress + 1];

                cycles = 1;
                if(storeLoction >= newJob.memoryLimit) {
                    cout << "store error!" << endl;
                } 
                else {
                    cout << "stored" << endl;
                    //--cout << "mem loc: " << startAddress + storeLoction << endl;
                    mainMemory[startAddress + storeLoction] = value;
                    mainMemory[startAddress + 7] = value;
                }
                DAddress += 2;
                newJob.cpuCyclesUsed += cycles;
                mainMemory[startAddress + 6] += 1;
                //--cout << "S Process " << newJob.processID << " cycles used: " << newJob.cpuCyclesUsed << endl;
                globalClock += 1;
                totalCycles += 1;
                //--cout << "Global Clock D: " << globalClock << endl;
                //--cout << "Store DAddress After:" << DAddress << endl;
                break;
            }
            case 4: {
                // Load instruction
                //--cout << "Load DAddress Before:" << DAddress << endl;
                storeLoction = mainMemory[DAddress];
                //--cout << "storeLoction: " << storeLoction << endl;
                cycles = 1;
                if(storeLoction >= newJob.memoryLimit) {
                    cout << "load error!" << endl;
                    newJob.registerValue = -1;
                } 
                else {
                    cout << "loaded" << endl;
                    //--cout << "mem loc: " << startAddress + storeLoction << endl;
                    newJob.registerValue = mainMemory[startAddress + storeLoction];
                    mainMemory[startAddress + 7] = newJob.registerValue;
                }
                DAddress += 1;
                newJob.cpuCyclesUsed += cycles;
                mainMemory[startAddress + 6] += 1;
                globalClock += 1;
                totalCycles += 1;
                //--cout << "Global Clock E: " << globalClock << endl;
                //--cout << "L Process " << newJob.processID << " cycles used: " << newJob.cpuCyclesUsed << endl;
                //--cout << "Load DAddress Before:" << DAddress << endl;
                break;
            }
        }
        if(newJob.programCounter != numInstructions && instruction != 2) {
            //Check if process has exceeded CPU cycles
            if (totalCycles >= CPUAllocated) {
                TimeOUT(newJob, readyQueue, mainMemory, startAddress, DAddress);
                executeNewProcess = contextSwitch(readyQueue, IOWaitingQueue, mainMemory, startAddress, contextSwitchTime, DAddress);
                endProcess = executeNewProcess;
            }
        }   

        //--cout << newJob.processID << " DAddress: " << DAddress << endl;
        //--cout << "End Process: " << endProcess << endl;

        if(endProcess) {
            break;
        }
    }

    if(endProcess)
    {
       return;
    }
    

    // Update PCB state to TERMINATED
    mainMemory[startAddress + 1] = 4; // state ("TERMINATED")

    // Determine State for Printing
    string state;
    switch(mainMemory[startAddress + 1]) {
        case 1: {
            state = "NEW";
            break;
        }
        case 2: {
            state = "READY";
            break;
        }
        case 3: {
            state = "RUNNING";
            break;
        }
        case 4: {
            state = "TERMINATED";
            break;
        }
        case 5: {
            state = "IOWaiting";
            break;
        }
    }

    // Output details of the executed instruction
    cout << "Process ID: " << newJob.processID << endl;
    cout << "State: " + state << endl;
    cout << "Program Counter: " << mainMemory[startAddress + 3] - 1 << endl;
    cout << "Instruction Base: " << newJob.instructionBase << endl;
    cout << "Data Base: " << newJob.dataBase << endl;
    cout << "Memory Limit: " << newJob.memoryLimit << endl;
    cout << "CPU Cycles Used: " << newJob.cpuCyclesUsed << endl;
    cout << "Register Value: " << newJob.registerValue << endl;
    cout << "Max Memory Needed: " << newJob.maxMemoryNeeded << endl;
    cout << "Main Memory Base: " << newJob.mainMemoryBase << endl;
    cout << "Total CPU Cycles Consumed: " << globalClock - startTimes[newJob.processID - 1] << endl;
    cout << "Process " << newJob.processID << " terminated. Entered running state at: " << startTimes[newJob.processID - 1] << ". Terminated at: " << globalClock << ". Total Execution Time: " << globalClock - startTimes[newJob.processID - 1] << "." << endl;

    //Check IO Waiting Queue for any processes that have completed IO
    checkIOWaitingQueue(IOWaitingQueue, mainMemory, readyQueue, contextSwitchTime);
}


int main() {
    int maxMemory;
    int numProcesses;
    queue<PCB> newJobQueue;
    queue<tuple<int, int>> readyQueue;
    queue<vector<int>> IOWaitingQueue;
    vector<int> mainMemory;
    int CPUAllocated;
    int contextSwitchTime;

    // Step 1: Read and parse input file
    // TODO: Implement input parsing and populate newJobQueue

    // Initialize main memory
    cin >> maxMemory;
    mainMemory = vector<int>(maxMemory, -1);

    // Read CPU allocated
    cin >> CPUAllocated;

    // Read context switch time
    cin >> contextSwitchTime;

    // Read number of processes
    cin >> numProcesses;

    // Initialize start times for each process
    vector<int> startTimes(numProcesses, 0);

    //Add starting address to readyQueue
    readyQueue.push(make_tuple(0, 0));
    
    // Step 2: Load jobs into main memory
    for (int i = 0; i < numProcesses; i++) {
        PCB newJob;
        cin >> newJob.processID >> newJob.maxMemoryNeeded;
        newJobQueue.push(newJob);
        loadJobsToMemory(newJobQueue, readyQueue, mainMemory, maxMemory, numProcesses);
    }
    
    // Step 3: After you load the jobs in the queue go over the main memory
    // and print the content of mainMemory. It will be in the table format
    // three columns as I had provided you earlier.
    for (int i = 0; i < mainMemory.size(); i++) {
        cout << i << " : " << mainMemory[i] << endl;
    }
    
    //Start global clock
    globalClock = 0;
    
    // Step 4: Process execution
    while (!readyQueue.empty() || !IOWaitingQueue.empty()) {
        /*cout << "Main R Queue Before: ";
        printRQueue(readyQueue);
        cout << "Main IO Queue Before: ";
        printIOQueue(IOWaitingQueue);*/

        //--cout << "Global Clock: " << globalClock << endl;
        //If readyQueue is empty and IOWaitingQueue is not empty, increment global clock by context switch time
        if(readyQueue.empty() && !IOWaitingQueue.empty()) {
            globalClock += contextSwitchTime;
            //--cout << "Global Clock A: " << globalClock << endl;
            checkIOWaitingQueue(IOWaitingQueue, mainMemory, readyQueue, contextSwitchTime);
            //globalClock += contextSwitchTime;
        }
        
        if(!readyQueue.empty()){
            tuple <int, int> startAndDAddress = readyQueue.front(); //readyQueue contains start addresses w.r.t main memory for jobs
            int startAddress = get<0>(startAndDAddress);
            int DAddress = get<1>(startAndDAddress);

            //--cout << "Start Address: " << startAddress << endl;
            //--cout << "Start DAddress: " << DAddress << endl;

            //Set the process state to RUNNING
            mainMemory[startAddress + 1] = 2; // state ("READY")
            
            readyQueue.pop();
            
            // Execute job
            executeCPU(startAddress, mainMemory, readyQueue, IOWaitingQueue, CPUAllocated, contextSwitchTime, DAddress, startTimes);
        }

        // Output Job that just completed execution – see example below
        /*if(globalClock > 300){
            break;
        }*/
    }

    cout << "Total CPU time used: " << globalClock + contextSwitchTime << "." << endl;

    return 0;
}