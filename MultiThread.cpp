#include <iostream>
#include <iomanip>
#include <cmath>
#include <sstream>
#include <map>
#include <vector>
#include <pthread.h>

//storing specific information 
struct ThreadData {
    std::string input;
    int cpuNum;
    std::vector<double> entropyValues;
};

// Function to calculate entropy
double entropy(std::map<char, int>& freq, int currFreq, double currH, char selectedTask, int extraFreq) {
    double H = 0;
    int Nfreq = currFreq + extraFreq;
    double currentTerm;

    if (Nfreq == extraFreq) {
        H = 0;
    } else {
        if (freq[selectedTask] == 0) {
            currentTerm = 0;
        } else {
            currentTerm = freq[selectedTask] * log2(freq[selectedTask]);
        }

        double newTerm = (freq[selectedTask] + extraFreq) * log2(freq[selectedTask] + extraFreq);
        H = log2(Nfreq) - ((log2(currFreq) - currH) * currFreq - currentTerm + newTerm) / Nfreq;
    }

    // Store the extraFreq with the same selectedTask for the next entropy calculation 
    freq[selectedTask] += extraFreq;
    return H;
}


// Function to calculate entropy and produce output for a single CPU
// Function must have void parameter for thread
void* calculateEntropy(void* arg) {
    ThreadData* threadData = static_cast<ThreadData*>(arg);

    // Initialize variables
    std::map<char, int> freq;
    double currH = 0;
    int currFreq = 0;

    // Create a stringstream to split the input string
    std::stringstream ss(threadData->input);
    char task;
    int freqVal;

    std::vector<double> entropyValues; // Collect entropy values

    while (ss >> task >> freqVal) {
        // Calculate and update entropy
        double H = entropy(freq, currFreq, currH, task, freqVal);
        currH = H;
        currFreq += freqVal;

        // Collect entropy values
        entropyValues.push_back(currH);
    }

    // Store the entropy values and CPU number in threadData
    threadData->entropyValues = entropyValues;
    return nullptr;
}

int main() {
    int cpuNum = 1;
    std::string input;
    std::vector<ThreadData> threadDataVec;

    //reading input
    while (std::getline(std::cin, input)) {
        if (input.empty()) {
            break;
        }

        ThreadData threadData;
        threadData.input = input;
        threadData.cpuNum = cpuNum;

        threadDataVec.push_back(threadData);

        cpuNum++;
    }

    // Once all the input is created, thread will be created and calculate the entropy
    // Create an array of pthreads
    pthread_t threads[threadDataVec.size()];

    // Create and execute threads
    for (int i = 0; i < static_cast<int>(threadDataVec.size()); i++) {
        if (pthread_create(&threads[i], nullptr, calculateEntropy, &threadDataVec[i]) != 0) {
            std::cerr << "Error creating thread." << std::endl;
            return 1;
        }
    }

    // Join all created threads
    for (int i = 0; i < static_cast<int>(threadDataVec.size()); i++) {
        if (pthread_join(threads[i], nullptr) != 0) {
            std::cerr << "Error joining thread." << std::endl;
            return 1;
        }
    }

    // This for loop is for outputting the information
    for (int i = 0; i < static_cast<int>(threadDataVec.size()); i++) {
        const ThreadData& threadData = threadDataVec[i];
        std::cout << "CPU " << threadData.cpuNum << std::endl;
        std::cout << "Task scheduling information: ";

        //creating sstream for output
        std::stringstream ss(threadData.input);
        char task;
        int freqVal;
        
        // Output task and frequency information with parenthesis and commas
        while (ss >> task >> freqVal) {
            std::cout << task << "(" << freqVal << ")";
            if (ss.peek() != EOF) {
                std::cout << ", ";
            }
        }

        std::cout << std::endl;

        std::cout << "Entropy for CPU " << threadData.cpuNum << std::endl;
        for (int j = 0; j < static_cast<int>(threadData.entropyValues.size()); j++) {
            std::cout << std::fixed << std::setprecision(2) << threadData.entropyValues[j] << " ";
        }
        std::cout << std::endl;
    }

    return 0;
}
/*
input-
A 2 B 4 C 3 A 7
B 3 A 3 C 3 A 1 B 1 C 1 
Output-
CPU 1
Task scheduling information: A(2), B(4), C(3), A(7)
Entropy for CPU 1
0.00 0.92 1.53 1.42

CPU 2
Task scheduling information: B(3), A(3), C(3), A(1), B(1), C(1)
Entropy for CPU 2
0.00 1.00 1.58 1.57 1.57 1.58
*/
