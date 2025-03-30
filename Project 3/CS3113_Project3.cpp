#include <iostream>
#include <queue>
#include <string>
#include <vector>
#include <tuple>
#include <list>
#include <iterator>
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

void loadJobsToMemory(list<tuple<int, int, int>>& newJobQueue, queue<tuple<int, int>>& readyQueue, vector<int>& mainMemory, int numProcesses, list<tuple<int, int, int>>::iterator& itr) {
    // TODO: Implement loading jobs into main memory
    
    //Get process to be loaded from newJobQueue
    tuple<int, int, int> processToBeLoaded = *itr;
    PCB newJob;
    newJob.processID = get<0>(processToBeLoaded);
    newJob.maxMemoryNeeded = get<2>(processToBeLoaded) - 10; //Subtract PCB size from total memory needed
    
    //Initialize starting address and DAddress tuple as well as PCB size
    tuple <int, int> startAndDAddress;
    int startingAddress = get<1>(processToBeLoaded);
    int DAddress = 0;
    int PCBSize = 10;
    
    //Print loading message
    cout << "Process " << newJob.processID << " loaded into memory at address " << startingAddress << " with size " << (10 + newJob.maxMemoryNeeded) << "." << endl;
    
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
    
    //Push process to readyQueue
    readyQueue.push(make_tuple(startingAddress, DAddress));

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
        
        //Increment instruction address
        currentIAddress++;
    }

    
}

//Attempt to coalesce memory blocks
bool memoryCoalescing(list<tuple<int, int, int>>& newJobQueue, tuple<int, int>& newJobQueueHold, list<tuple<int, int, int>>::iterator& itr) {
    //Initialize feedback for whether coalescing was successful and created block large enough to hold new job
    bool coalesce = false;

    //Reset iterator and define variables
    itr = newJobQueue.begin();
    tuple<int, int, int> memoryBlock1;
    tuple<int, int, int> memoryBlock2;
    int blockProcessID1;
    int blockProcessID2;
    int blockStartingAddress;
    int blockSize;
    bool loaded;

    //Iterate through newJobQueue to find adjacent empty blocks and combine them
    for(int j = 0; j < newJobQueue.size() - 1; j++) {
        //Get current memory block and next memory block
        memoryBlock1 = *itr;
        itr++;
        memoryBlock2 = *itr;
        itr--;
        blockProcessID1 = get<0>(memoryBlock1);
        blockProcessID2 = get<0>(memoryBlock2);
        blockStartingAddress = get<1>(memoryBlock1);
        blockSize = get<2>(memoryBlock1) + get<2>(memoryBlock2);

        //If both memory blocks are empty
        if(blockProcessID1 == -1 && blockProcessID2 == -1) {
            //Combine memory blocks
            *itr = make_tuple(blockProcessID1, blockStartingAddress, blockSize);
            //Check if new job can fit in combined block
            if(newJobQueueHold != make_tuple(-1, -1) && ((get<1>(newJobQueueHold) + 10) <= blockSize)) {
                //Set coalesce to true
                coalesce = true;
            }
            //Remove next memory block
            itr++;
            itr = newJobQueue.erase(itr);
            itr--;
            //If block following combined block is not empty
            if(get<0>(*itr) != -1) {
                //Stop searching for memory blocks to coalesce
                return coalesce;
            }
            //If block following combined block is empty, continue loop to coalesce adjacent empty blocks
        }
        else { //If both memory blocks are not empty
            //Move to next memory block
            itr++;
        }
    }
    //If no adjacent empty blocks were found, return coalesce
    return coalesce;
}

void executeCPU(int startAddress, vector<int>& mainMemory, queue<tuple<int, int>>& readyQueue, queue<vector<int>>& IOWaitingQueue, int CPUAllocated, int contextSwitchTime, int DAddress, vector<int>& startTimes, list<tuple<int, int, int>>& newJobQueue, list<tuple<int, int, int>>::iterator& itr, int& numProcesses, tuple<int, int>& newJobQueueHold) {
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

    //Remove process from newJobQueue and main memory
    //Reset iterator and define variables
    itr = newJobQueue.begin();
    tuple<int, int, int> memoryBlock;
    int blockProcessID;
    int blockStartingAddress;
    int blockSize;

    //Iterate through newJobQueue to find process and remove it
    for(int j = 0; j < newJobQueue.size(); j++) {
        //Get current memory block
        memoryBlock = *itr;
        blockProcessID = get<0>(memoryBlock);
        blockStartingAddress = get<1>(memoryBlock);
        blockSize = get<2>(memoryBlock);
        //If the current memory block is the one to be removed
        if(blockProcessID == newJob.processID) {
            //Remove process from newJobQueue
            cout << "Process " << newJob.processID << " terminated and released memory from " << blockStartingAddress << " to " << blockStartingAddress + blockSize - 1 << "." << endl;
            *itr = make_tuple(-1, blockStartingAddress, blockSize);
            //Stop searching for memory block
            break;
        }
        else { //If the current memory block is not the one to be removed
            //Move to next memory block
            itr++;
        }
    }
    //Clear the memory for the terminated process from start address to end address
    for (int i = blockStartingAddress; i < blockStartingAddress + blockSize; i++) {
        mainMemory[i] = -1;
    }

    //Attempt to load remaining jobs into memory
    //If there are any processes left to load
    if(numProcesses > 0) {
        bool loaded;
        bool success = false;
        //Load processes into newJobQueue
        //Iterate through number of processes
        for (numProcesses; numProcesses > 0; numProcesses--) {
            loaded = false;

            //If there is a process waiting in newJobQueueHold
            if(newJobQueueHold != make_tuple(-1, -1)) {
                //Load process from newJobQueueHold
                newJob.processID = get<0>(newJobQueueHold);
                newJob.maxMemoryNeeded = get<1>(newJobQueueHold);
            }
            else { //Otherwise read in new process
                //Clear processID and maxMemoryNeeded and load in next values
                newJob.processID = 0;
                newJob.maxMemoryNeeded = 0;
                cin >> newJob.processID >> newJob.maxMemoryNeeded;
            }

            //Reset iterator
            itr = newJobQueue.begin();
            //Iterate through newJobQueue to find a block of memory large enough to hold the new job
            for(int j = 0; j < newJobQueue.size(); j++) {
                //Get current memory block
                memoryBlock = *itr;
                blockProcessID = get<0>(memoryBlock);
                blockStartingAddress = get<1>(memoryBlock);
                blockSize = get<2>(memoryBlock);
                //If the current memory block is empty and large enough to hold the new job
                if(blockProcessID == -1 && blockSize >= 10 + newJob.maxMemoryNeeded) {
                    //Insert new job into newJobQueue
                    newJobQueue.insert(itr, make_tuple(newJob.processID, blockStartingAddress, 10 + newJob.maxMemoryNeeded));
                    //Update starting address and block size
                    blockStartingAddress += (10 + newJob.maxMemoryNeeded);
                    blockSize -= (10 + newJob.maxMemoryNeeded);
                    //Update current memory block in newJobQueue
                    *itr = make_tuple(blockProcessID, blockStartingAddress, blockSize);
                    itr--;
                    //Load job into main memory
                    loadJobsToMemory(newJobQueue, readyQueue, mainMemory, numProcesses, itr);
                    loaded = true;
                    //Clear newJobQueueHold
                    newJobQueueHold = make_tuple(-1, -1);
                    //Stop searching for memory block
                    break;
                }
                else { //If the current memory block is not empty or not large enough to hold the new job
                    //Move to next memory block
                    itr++;
                }
            }
            //Reset iterator to beginning of newJobQueue
            itr = newJobQueue.begin();
            //If no memory block was found, hold job, and attempt memory coalescing
            if(!loaded) {
                newJobQueueHold = make_tuple(newJob.processID, newJob.maxMemoryNeeded);
                cout << "Insufficient memory for Process " << newJob.processID << ". Attempting memory coalescing." << endl;
                success = memoryCoalescing(newJobQueue, newJobQueueHold, itr);
            }
            //If memory coalescing was unsuccessful
            if(!loaded && !success){
                //Print message and stop loading new jobs
                cout << "Process " << newJob.processID << " waiting in NewJobQueue due to insufficient memory." << endl;
                break;
            }
            else if(!loaded && success) { //If memory coalescing was successful
                //Load job into main memory
                cout << "Memory coalesced. Process " << newJob.processID << " can now be loaded." << endl;
                //Reset iterator
                itr = newJobQueue.begin();
                bool openMemory = false;
                //Iterate through newJobQueue to find the block of memory large enough to hold the new job
                while(!openMemory) {
                    //Get current memory block
                    memoryBlock = *itr;
                    //If the current memory block is empty and large enough to hold the new job
                    if(get<0>(memoryBlock) == -1 && get<2>(memoryBlock) >= (10 + newJob.maxMemoryNeeded)) {
                        //Set openMemory to true
                        openMemory = true;
                    }
                    else { //If the current memory block is not empty or not large enough to hold the new job
                        //Move to next memory block
                        itr++;
                    }
                }

                //One iterator is set to correct memory block, get current memory block
                memoryBlock = *itr;
                blockProcessID = get<0>(memoryBlock);
                blockStartingAddress = get<1>(memoryBlock);
                blockSize = get<2>(memoryBlock);
                
                //Insert new job into newJobQueue
                newJobQueue.insert(itr, make_tuple(newJob.processID, blockStartingAddress, 10 + newJob.maxMemoryNeeded));
                //Update starting address and block size
                blockStartingAddress += (10 + newJob.maxMemoryNeeded);
                blockSize -= (10 + newJob.maxMemoryNeeded);
                //Update current memory block in newJobQueue
                *itr = make_tuple(blockProcessID, blockStartingAddress, blockSize); 
                itr--;
                //Load job into main memory
                loadJobsToMemory(newJobQueue, readyQueue, mainMemory, numProcesses, itr);
                //Clear newJobQueueHold
                newJobQueueHold = make_tuple(-1, -1);
            }
        }
    }

    //Check IO Waiting Queue for any processes that have completed IO
    checkIOWQ(IOWaitingQueue, mainMemory, readyQueue, contextSwitchTime);
}


int main() {
    int maxMemory;
    int numProcesses;
    list<tuple<int, int, int>> newJobQueue;
    list<tuple<int, int, int>>::iterator itr;
    tuple<int, int> newJobQueueHold;
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

    //Add initial memory block and set iterator starting point
    newJobQueue.push_back(tuple(-1, 0, maxMemory));
    itr = newJobQueue.begin();
    
    // Step 2: Load jobs into main memory
    //Define variables for loading jobs
    PCB newJob;
    tuple<int, int, int> memoryBlock;
    int blockProcessID;
    int blockStartingAddress;
    int blockSize;
    bool loaded;

    //Load processes into newJobQueue
    //Iterate through number of processes
    for (numProcesses; numProcesses > 0; numProcesses--) {
        //Initialize loaded to false and clear processID and maxMemoryNeeded before loading in values
        loaded = false;
        newJob.processID = 0;
        newJob.maxMemoryNeeded = 0;
        cin >> newJob.processID >> newJob.maxMemoryNeeded;

        //Iterate through newJobQueue to find a block of memory large enough to hold the new job
        for(int j = 0; j < newJobQueue.size(); j++) {
            //Get current memory block
            memoryBlock = *itr;
            blockProcessID = get<0>(memoryBlock);
            blockStartingAddress = get<1>(memoryBlock);
            blockSize = get<2>(memoryBlock);

            //If the current memory block is empty and large enough to hold the new job
            if(blockProcessID == -1 && blockSize >= 10 + newJob.maxMemoryNeeded) {
                //Insert new job into newJobQueue
                newJobQueue.insert(itr, make_tuple(newJob.processID, blockStartingAddress, 10 + newJob.maxMemoryNeeded));
                //Update starting address and block size
                blockStartingAddress += (10 + newJob.maxMemoryNeeded);
                blockSize -= (10 + newJob.maxMemoryNeeded);
                //Update current memory block in newJobQueue
                *itr = make_tuple(blockProcessID, blockStartingAddress, blockSize);
                itr--;
                //Load job into main memory
                loadJobsToMemory(newJobQueue, readyQueue, mainMemory, numProcesses, itr);
                loaded = true;
                //Stop searching for memory block
                break;
            }
            else { //If the current memory block is not empty or not large enough to hold the new job
                //Move to next memory block
                itr++;
            }
        }
        //Reset iterator to beginning of newJobQueue
        itr = newJobQueue.begin();
        //If no memory block was found, "attempt" memory coalescing (unnecessary for intial loading loop) and print message
        if(!loaded) {
            //Save current job in newJobQueueHold
            newJobQueueHold = make_tuple(newJob.processID, newJob.maxMemoryNeeded);
            cout << "Insufficient memory for Process " << newJob.processID << ". Attempting memory coalescing." << endl;
            cout << "Process " << newJob.processID << " waiting in NewJobQueue due to insufficient memory." << endl;
            //Stop loading new jobs
            break;
        }
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
            executeCPU(startAddress, mainMemory, readyQueue, IOWaitingQueue, CPUAllocated, contextSwitchTime, DAddress, startTimes, newJobQueue, itr, numProcesses, newJobQueueHold);
        }

        // Output Job that just completed execution – see example below
    }

    //Print total CPU time
    cout << "Total CPU time used: " << globalClock + contextSwitchTime << "." << endl;
    return 0;
}