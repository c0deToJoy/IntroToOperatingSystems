#include <iostream>
#include <queue>
#include <string>
#include <vector>
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

void loadJobsToMemory(queue<PCB>& newJobQueue, queue<int>& readyQueue, vector<int>& mainMemory, int maxMemory, int numProcesses) {
    // TODO: Implement loading jobs into main memory

    // Initialize starting address and PCB size
    int startingAddress = readyQueue.back();
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
        readyQueue.push(startingAddress);
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

void executeCPU(int startAddress, vector<int>& mainMemory) {
    // TODO: Implement CPU instruction execution

    // Initialize starting address and PCB size
    int PCBSize = 10;

    // Read PCB fields from main memory
    PCB newJob;
    newJob.processID = mainMemory[startAddress]; // processID
    mainMemory[startAddress + 1] = 2; // state ("READY")
    newJob.programCounter = mainMemory[startAddress + 3] - 1; // programCounter
    newJob.instructionBase = mainMemory[startAddress + 3]; // instructionBase
    newJob.dataBase = mainMemory[startAddress + 4]; // dataBase
    newJob.memoryLimit = mainMemory[startAddress + 5]; // memoryLimit
    newJob.cpuCyclesUsed = mainMemory[startAddress + 6]; // cpuCyclesUsed
    newJob.registerValue = mainMemory[startAddress + 7]; // registerValue
    newJob.maxMemoryNeeded = mainMemory[startAddress + 8]; // maxMemoryNeeded
    newJob.mainMemoryBase = mainMemory[startAddress + 9]; // mainMemoryBase

    // Set starting address for instructions
    int IAddress = newJob.instructionBase;

    // Set starting address for data
    int DAddress = newJob.dataBase;

    // Count Instructions
    int numInstructions = 0;
    for(int i = IAddress; i < DAddress; i++) {
        numInstructions++;
    }

    // Initialize variables for instruction execution
    int instruction;
    int iterations;
    int cycles;
    int value;
    int storeLoction;
    int offset = newJob.instructionBase + newJob.maxMemoryNeeded;
    int num;
    int processes = 0;

    for(int j = 0; j < numInstructions; j++) {    
        // Fetch instruction
        instruction = mainMemory[IAddress + j];

        // Decode and execute instruction
        switch(instruction) {
            case 1: {
                // Compute instruction
                cout << "compute" << endl;
                iterations = mainMemory[DAddress];
                cycles = mainMemory[DAddress + 1];
                DAddress += 2;
                newJob.cpuCyclesUsed += cycles;
                break;
            }
            case 2: {
                // Print instruction
                cout << "print" << endl;
                cycles = mainMemory[DAddress];
                DAddress += 1;
                newJob.cpuCyclesUsed += cycles;
                break;
            }
            case 3: {
                // Store instruction
                value = mainMemory[DAddress];
                newJob.registerValue = value;
                storeLoction = mainMemory[DAddress + 1];
                if(storeLoction >= newJob.memoryLimit) {
                    cout << "store error!" << endl;
                } 
                else {
                    cout << "stored" << endl;
                    mainMemory[startAddress + storeLoction] = value;
                }
                DAddress += 2;
                newJob.cpuCyclesUsed += 1;
                break;
            }
            case 4: {
                // Load instruction
                storeLoction = mainMemory[DAddress];

                if(storeLoction >= newJob.memoryLimit) {
                    cout << "load error!" << endl;
                    newJob.registerValue = -1;
                } 
                else {
                    cout << "loaded" << endl;
                    newJob.registerValue = mainMemory[startAddress + storeLoction];
                }

                DAddress += 1;
                newJob.cpuCyclesUsed += 1;
                break;
            }
        }
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
    }

    // Output details of the executed instruction
    cout << "Process ID: " << newJob.processID << endl;
    cout << "State: " + state << endl;
    cout << "Program Counter: " << newJob.programCounter << endl;
    cout << "Instruction Base: " << newJob.instructionBase << endl;
    cout << "Data Base: " << newJob.dataBase << endl;
    cout << "Memory Limit: " << newJob.memoryLimit << endl;
    cout << "CPU Cycles Used: " << newJob.cpuCyclesUsed << endl;
    cout << "Register Value: " << newJob.registerValue << endl;
    cout << "Max Memory Needed: " << newJob.maxMemoryNeeded << endl;
    cout << "Main Memory Base: " << newJob.mainMemoryBase << endl;
    cout << "Total CPU Cycles Consumed: " << newJob.cpuCyclesUsed << endl;
}


int main() {
    int maxMemory;
    int numProcesses;
    queue<PCB> newJobQueue;
    queue<int> readyQueue;
    vector<int> mainMemory;

    // Step 1: Read and parse input file
    // TODO: Implement input parsing and populate newJobQueue

    // Initialize main memory
    cin >> maxMemory;
    mainMemory = vector<int>(maxMemory, -1);

    // Read number of processes
    cin >> numProcesses;

    //Add starting address to readyQueue
    readyQueue.push(0);
    
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
    
    // Step 4: Process execution
    while (!readyQueue.empty()) {
        int startAddress = readyQueue.front();
        //readyQueue contains start addresses w.r.t main memory for jobs
        
        readyQueue.pop();
        
        // Execute job
        executeCPU(startAddress, mainMemory);
        
        // Output Job that just completed execution – see example below
    }

    return 0;
}