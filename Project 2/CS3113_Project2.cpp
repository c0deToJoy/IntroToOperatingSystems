#include <iostream>
#include <queue>
#include <string>
#include <vector>
#include <tuple>
using namespace std;

struct PCB {
    // Define PCB fields as described earlier
    int processID; //The state field tracks the current status of the process, which can be one of the following: NEW, READY, RUNNING, or TERMINATED.
    int programCounter; //The index of the next instruction to be executed within the process's logical memory
    int instructionBase; //Specifies the starting address of the instructions in the logical memory
    int dataBase; //Points to the beginning of the data segment within the logical memory
    int memoryLimit; //Defines the total size of logical memory allocated to the process
    int cpuCyclesUsed; //Accumulates the total CPU cycles consumed by the process during its execution
    int registerValue; //Simulated register used to store intermediate values during load and store operations
    int maxMemoryNeeded; //Specifies the maximum memory required by the process as defined in the input file
    int mainMemoryBase; //Denotes the starting address in main memory where the process, including its PCB and logical memory, is loaded
};

//Define global clock
int globalClock;

//TimeOUT interrupt process by moving it to the ReadyQueue
void TimeOUT(PCB job, queue<tuple<int, int>>& readyQueue, vector<int>& mainMemory, int startAddress, int DAddress) {
    //Add process to ReadyQueue
    readyQueue.push(make_tuple(startAddress, DAddress));
    
    //Update PCB state to READY
    mainMemory[startAddress + 1] = 2; // state ("READY")

    //Print TimeOUT message
    cout << "Process " << job.processID << " has a TimeOUT interrupt and is moved to the ReadyQueue." << endl;
}

// IOInterrupt process by moving it to the IOWaitingQueue
void IOInterrupt(PCB job, queue<vector<int>>& IOWaitingQueue, vector<int>& mainMemory, int startAddress, int cycles, int DAddress) {
    //Combine process start address and IO time
    vector<int> processIOTime;
    processIOTime.push_back(startAddress);
    processIOTime.push_back(globalClock);
    processIOTime.push_back(cycles);
    processIOTime.push_back(DAddress);

    //Add process to IOWaitingQueue
    IOWaitingQueue.push(processIOTime);

    //Update PCB state to IOWaiting
    mainMemory[startAddress + 1] = 5; // state ("IOWaiting")

    //Print IOInterrupt message
    cout << "Process " << job.processID << " issued an IOInterrupt and moved to the IOWaitingQueue." << endl;
}

//Check IO Waiting Queue for any processes that have completed IO
void checkIOWQ(queue<vector<int>>& IOWaitingQueue, vector<int>& mainMemory, queue<tuple<int, int>>& readyQueue, int contextSwitchTime) {
    //If IOWaitingQueue is not empty
    if(!IOWaitingQueue.empty()) {
        int currentIOSize = IOWaitingQueue.size();
        //--cout << "IOC size:" << currentIOSize << endl;
        for(int i = 0; i < currentIOSize; i++) {
            //Get process and re-intialize variables
            vector<int> processIOTime = IOWaitingQueue.front();
            int processStartAddress = processIOTime[0];
            int processStartTime = processIOTime[1];
            int processCycles = processIOTime[2];
            int processDAddress = processIOTime[3];
            
            //If process has completed given number of cycles
            if(globalClock - processStartTime >= processCycles) {
                //Add process to ReadyQueue
                readyQueue.push(make_tuple(processStartAddress, processDAddress));

                //Update PCB state to READY
                mainMemory[processStartAddress + 1] = 2; // state ("READY")

                //Print and IOCompletion message
                cout << "print" << endl;
                cout << "Process " << mainMemory[processStartAddress] << " completed I/O and is moved to the ReadyQueue." << endl;

                //Remove process from IOWaitingQueue
                IOWaitingQueue.pop();
            }
            else {//If process has not completed given number of cycles
                //Move process to back of IOWaitingQueue
                IOWaitingQueue.push(processIOTime);
                IOWaitingQueue.pop();
            }
        }
    }
}

//Determinte if new process should execute through context switch
bool contextSwitch(queue<tuple<int, int>>& readyQueue, queue<vector<int>>& IOWaitingQueue, vector<int>& mainMemory, int startAddress, int contextSwitchTime, int DAddress) {
    //Initialize the decision to execute a new process
    bool executeNewProcess = true;
    
    //Check IO Waiting Queue for any processes that have completed IO
    checkIOWQ(IOWaitingQueue, mainMemory, readyQueue, contextSwitchTime);
    
    //If readyQueue is empty and IOWaitingQueue is not empty
    if(readyQueue.empty() && IOWaitingQueue.empty()) {
        //Set decision to execute new process to false
        executeNewProcess = false;
    }

    //Return decision to execute new process
    return executeNewProcess;
}

void loadJobsToMemory(queue<PCB>& newJobQueue, queue<tuple<int, int>>& readyQueue, vector<int>& mainMemory, int maxMemory, int numProcesses) {
    // TODO: Implement loading jobs into main memory

    //Initialize starting address and DAddress tuple as well as PCB size
    tuple <int, int> startAndDAddress = readyQueue.back();
    int startingAddress = get<0>(startAndDAddress);
    int DAddress = get<1>(startAndDAddress);
    int PCBSize = 10;
    
    //Pop the next job from the new job queue
    PCB newJob = newJobQueue.front();
    newJobQueue.pop();

    //Read number of instructions
    int numInstructions = 0;
    cin >> numInstructions;

    //Store PCB fields in main memory
    mainMemory[startingAddress] = newJob.processID; //processID
    mainMemory[startingAddress + 1] = 1; //state ("NEW")
    mainMemory[startingAddress + 2] = 0; //programCounter
    mainMemory[startingAddress + 3] = startingAddress + PCBSize; //instructionBase
    newJob.instructionBase = startingAddress + PCBSize; //Update instruction base in job
    mainMemory[startingAddress + 4] = startingAddress + PCBSize + numInstructions; //dataBase
    newJob.dataBase = startingAddress + PCBSize + numInstructions; //Update data base in job
    mainMemory[startingAddress + 5] = newJob.maxMemoryNeeded; //memoryLimit
    mainMemory[startingAddress + 6] = 0; //cpuCyclesUsed
    mainMemory[startingAddress + 7] = 0; //registerValue
    mainMemory[startingAddress + 8] = newJob.maxMemoryNeeded; //maxMemoryNeeded
    mainMemory[startingAddress + 9] = startingAddress; //mainMemoryBase

    //Set Address for next process if one exists
    if (readyQueue.size() < numProcesses) {
        startingAddress += PCBSize + newJob.maxMemoryNeeded;
        DAddress = 0;
        readyQueue.push(make_tuple(startingAddress, DAddress));
    }

    //Define current address starts for looping
    int currentIAddress = newJob.instructionBase;
    int currentDAddress = newJob.dataBase;

    //Loop through instructions and store them in main memory
    for (int i = 0; i < numInstructions; i++) {
        cin >> mainMemory[currentIAddress];

        //Store data for instructions based on their type
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
        
        currentIAddress++; //Increment instruction address
    }
    
}

void executeCPU(int startAddress, vector<int>& mainMemory, queue<tuple<int, int>>& readyQueue, queue<vector<int>>& IOWaitingQueue, int CPUAllocated, int contextSwitchTime, int DAddress, vector<int>& startTimes) {
    // TODO: Implement CPU instruction execution

    //Initialize decision to execute a new process to false
    bool executeNewProcess = false;

    //Initialize starting address and PCB size
    int PCBSize = 10;
    
    //Read PCB fields from main memory
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

    //Print Running Message and increment global clock by context switch time
    cout << "Process " << newJob.processID << " has moved to Running." << endl;
    globalClock += contextSwitchTime;

    //Check if process is starting
    if(newJob.programCounter == 0) {
        //Log the start time of the process
        startTimes[newJob.processID - 1] = globalClock;
    }

    //Set starting address for instructions
    int IAddress = newJob.instructionBase;
    
    //If process is starting
    if(newJob.cpuCyclesUsed == 0) {
        //Intialize DAddress to dataBase
        DAddress = newJob.dataBase;
    }
    
    //Count Instructions
    int numInstructions = 0;
    for(int i = IAddress; i < mainMemory[startAddress + 4]; i++) {
        numInstructions++;
    }

    //Initialize variables for instruction execution
    int instruction;
    int iterations;
    int cycles;
    int totalCycles = 0;
    int value;
    int storeLoction;
    bool endProcess = false;

    //While process has not completed all instructions
    while(newJob.programCounter < numInstructions) {
        //Fetch instruction
        instruction = mainMemory[IAddress + newJob.programCounter];
        
        //Increment program counter
        newJob.programCounter++;

        //Store current program count in main memory
        mainMemory[startAddress + 2] = newJob.programCounter;

        //Decode and execute instruction
        switch(instruction) {
            case 1: {
                //Compute instruction
                //Store iterations and cycles
                iterations = mainMemory[DAddress];
                cycles = mainMemory[DAddress + 1];

                //Increment DAddress
                DAddress += 2;

                //Increment cycles counters
                newJob.cpuCyclesUsed += cycles;
                mainMemory[startAddress + 6] += cycles;
                globalClock += cycles;
                totalCycles += cycles;

                //Print compute message
                cout << "compute" << endl;
                break;
            }
            case 2: {
                //Print instruction
                //Store cycles
                cycles = mainMemory[DAddress];

                //Increment DAddress
                DAddress += 1;

                //Increment cycles counters
                newJob.cpuCyclesUsed += cycles;
                mainMemory[startAddress + 6] += cycles;

                //Call IOInterrupt and context switch
                IOInterrupt(newJob, IOWaitingQueue, mainMemory, startAddress, cycles, DAddress);
                executeNewProcess = contextSwitch(readyQueue, IOWaitingQueue, mainMemory, startAddress, contextSwitchTime, DAddress);
                endProcess = executeNewProcess;
                break;
            }
            case 3: {
                //Store instruction
                //Store value and store location as well as update register value and intialize cycles
                value = mainMemory[DAddress];
                newJob.registerValue = value;
                storeLoction = mainMemory[DAddress + 1];
                cycles = 1;

                //If store location is greater than memory limit, print error message and update register value
                if(storeLoction >= newJob.memoryLimit) {
                    cout << "store error!" << endl;
                } 
                else { //Otherwise store value in main memory and update register value in main memory
                    cout << "stored" << endl;
                    mainMemory[startAddress + storeLoction] = value;
                    mainMemory[startAddress + 7] = value;
                }

                //Increment DAddress
                DAddress += 2;

                //Increment cycles counters
                newJob.cpuCyclesUsed += cycles;
                mainMemory[startAddress + 6] += 1;
                globalClock += 1;
                totalCycles += 1;
                break;
            }
            case 4: {
                //Load instruction
                //Store store location and  intialize cycles
                storeLoction = mainMemory[DAddress];
                cycles = 1;

                //If store location is greater than memory limit, print error message and update register value
                if(storeLoction >= newJob.memoryLimit) {
                    cout << "load error!" << endl;
                    newJob.registerValue = -1;
                } 
                else { //Otherwise load value from main memory and update register value in main memory
                    cout << "loaded" << endl;
                    newJob.registerValue = mainMemory[startAddress + storeLoction];
                    mainMemory[startAddress + 7] = newJob.registerValue;
                }

                //Increment DAddress
                DAddress += 1;

                //Increment cycles counters
                newJob.cpuCyclesUsed += cycles;
                mainMemory[startAddress + 6] += 1;
                globalClock += 1;
                totalCycles += 1;
                break;
            }
        }

        //If process has not completed all instructions and is not IO interrupted
        if(newJob.programCounter != numInstructions && instruction != 2) {
            //If process has exceeded CPU cycles, call TimeOUT and context switch
            if (totalCycles >= CPUAllocated) {
                TimeOUT(newJob, readyQueue, mainMemory, startAddress, DAddress);
                executeNewProcess = contextSwitch(readyQueue, IOWaitingQueue, mainMemory, startAddress, contextSwitchTime, DAddress);
                endProcess = executeNewProcess;
            }
        }   

        //If context switch has warranted a new process execution, break loop
        if(endProcess) {
            break;
        }
    }

    //If context switch has warranted a new process execution, return from function
    if(endProcess)
    {
       return;
    }
    
    //Update PCB state to TERMINATED
    mainMemory[startAddress + 1] = 4; //state ("TERMINATED")

    //Determine State for Printing
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

    //Output details of the executed instruction
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
    checkIOWQ(IOWaitingQueue, mainMemory, readyQueue, contextSwitchTime);
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

    //Initialize main memory
    cin >> maxMemory;
    mainMemory = vector<int>(maxMemory, -1);

    //Read CPU allocated
    cin >> CPUAllocated;

    //Read context switch time
    cin >> contextSwitchTime;

    //Read number of processes
    cin >> numProcesses;

    //Initialize start times for each process
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
        //If readyQueue is empty and IOWaitingQueue is not empty, increment global clock by context switch time
        if(readyQueue.empty() && !IOWaitingQueue.empty()) {
            globalClock += contextSwitchTime;
            checkIOWQ(IOWaitingQueue, mainMemory, readyQueue, contextSwitchTime);
        }
        
        //If readayQueue is not empty
        if(!readyQueue.empty()){
            //Get next process in ready queue
            tuple <int, int> startAndDAddress = readyQueue.front(); //readyQueue contains start addresses w.r.t main memory for jobs
            int startAddress = get<0>(startAndDAddress);
            int DAddress = get<1>(startAndDAddress);

            //Set the process state to RUNNING
            mainMemory[startAddress + 1] = 2; // state ("READY")
            
            //Remove process from readyQueue
            readyQueue.pop();
            
            // Execute job
            executeCPU(startAddress, mainMemory, readyQueue, IOWaitingQueue, CPUAllocated, contextSwitchTime, DAddress, startTimes);
        }

        // Output Job that just completed execution – see example below
    }

    //Print total CPU time
    cout << "Total CPU time used: " << globalClock + contextSwitchTime << "." << endl;

    return 0;
}