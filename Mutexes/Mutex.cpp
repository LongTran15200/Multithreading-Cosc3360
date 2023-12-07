#include <iostream>
#include <iomanip>
#include <cmath>
#include <sstream>
#include <map>
#include <vector>
#include <pthread.h>


//file scope similar to global scope
namespace {
    pthread_mutex_t mutex, hyperspace_mutex;
    pthread_cond_t condition;
    int turn = 1;
}

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

    //create local variable
    std::string input = threadData->input;
    int cpuNum = threadData->cpuNum;

    // Release the hyperspace_mutex
    pthread_mutex_unlock(&hyperspace_mutex);

    // Initialize local variables for entropy calculation
    std::map<char, int> freq;
    double currH = 0;
    int currFreq = 0;


    std::stringstream ss(input);
    char task;
    int freqVal;

    std::vector<double> entropyValues;

    // Collect the original input format
    std::stringstream originalInput;

    while (ss >> task >> freqVal) {
        // Calculate and update entropy
        double H = entropy(freq, currFreq, currH, task, freqVal);
        currH = H;
        currFreq += freqVal;
        
        // Collect entropy values
        entropyValues.push_back(currH);

        originalInput << task << "(" << freqVal << ")";
        if (ss.peek() != EOF) {
            originalInput << ", ";
        }
    }
    
    // Acquire the mutex before printing information
    pthread_mutex_lock(&mutex);

    // Wait until it's the thread's turn to print
    while (cpuNum != turn) {
        pthread_cond_wait(&condition, &mutex);
    }

    // Release the mutex after checking the condition
    pthread_mutex_unlock(&mutex);


    // Print CPU information and task scheduling details
    std::cout << "CPU " << cpuNum << std::endl;
    std::cout << "Task scheduling information: " << originalInput.str() << std::endl;

    // Print entropy values for the current CPU
    std::cout << "Entropy for CPU " << cpuNum << std::endl;
    for (int j = 0; j < static_cast<int>(entropyValues.size()); j++) {
        std::cout << std::fixed << std::setprecision(2) << entropyValues[j] << " ";
    }
    std::cout << std::endl;
    std::cout << std::endl;
    // Acquire the mutex before updating the turn
    pthread_mutex_lock(&mutex);
    
    // Increment the turn and signal other threads to wake up
    // This basically helped keep output in order
    ++turn;
    pthread_cond_broadcast(&condition);
    
    // Release the mutex after updating the turn and signaling
    pthread_mutex_unlock(&mutex);

    return nullptr;
}

int main() {
    int cpuNum = 1;
    std::string input;
    std::vector<std::string> inputs;

    //reading input
    while (std::getline(std::cin, input)) {
        if (input.empty()) {
            break;
        }

        inputs.push_back(input);

        cpuNum++;
    }

    // Once all the input is created, thread will be created and calculate the entropy
    // Create an array of pthreads
    std::vector<pthread_t> threads(inputs.size());
    
    //reconstruct only 1 variable
    ThreadData args;
    
    // Create and execute threads
    for (int i = 0; i < inputs.size(); i++) {

        // Acquire the hyperspace_mutex before creating the thread
        pthread_mutex_lock(&hyperspace_mutex);
        
        // Initialize data for current thread
        args.input = inputs[i];
        args.cpuNum = i + 1;
        pthread_create(&threads[i], nullptr, calculateEntropy, &args);
    }

    // Join all created threads
    for (int i = 0; i < inputs.size(); i++) {
        if (pthread_join(threads[i], nullptr) != 0) {
            std::cerr << "Error joining thread." << std::endl;
            return 1;
        }
    }


    return 0;
}
