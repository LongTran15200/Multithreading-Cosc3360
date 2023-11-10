#include <iostream>
#include <iomanip>
#include <cmath>
#include <sstream>
#include <map>
#include <vector>
#include <pthread.h>

namespace {
    pthread_mutex_t mutex, hyperspace_mutex;
    pthread_cond_t condition;
    int turn = 1;
}

struct ThreadData {
    std::string input;
    int cpuNum;
    std::vector<double> entropyValues;
};

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

    freq[selectedTask] += extraFreq;
    return H;
}

void* calculateEntropy(void* arg) {
    ThreadData* threadData = static_cast<ThreadData*>(arg);

    //create local variable
    std::string input = threadData->input;
    int cpuNum = threadData->cpuNum;

    pthread_mutex_unlock(&hyperspace_mutex);
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
        double H = entropy(freq, currFreq, currH, task, freqVal);
        currH = H;
        currFreq += freqVal;
        entropyValues.push_back(currH);

        originalInput << task << "(" << freqVal << ")";
        if (ss.peek() != EOF) {
            originalInput << ", ";
        }
    }

    pthread_mutex_lock(&mutex);
    while (cpuNum != turn) {
        pthread_cond_wait(&condition, &mutex);
    }
    pthread_mutex_unlock(&mutex);

    std::cout << "CPU " << cpuNum << std::endl;
    std::cout << "Task scheduling information: " << originalInput.str() << std::endl;

    std::cout << "Entropy for CPU " << cpuNum << std::endl;
    for (int j = 0; j < static_cast<int>(entropyValues.size()); j++) {
        std::cout << std::fixed << std::setprecision(2) << entropyValues[j] << " ";
    }
    std::cout << std::endl;

    pthread_mutex_lock(&mutex);
    ++turn;
    pthread_cond_broadcast(&condition);
    pthread_mutex_unlock(&mutex);

    return nullptr;
}

int main() {
    int cpuNum = 1;
    std::string input;
    std::vector<std::string> inputs;

    while (std::getline(std::cin, input)) {
        if (input.empty()) {
            break;
        }

        inputs.push_back(input);

        cpuNum++;
    }
    std::vector<pthread_t> threads(inputs.size());
    ThreadData args;
    
    for (int i = 0; i < inputs.size(); i++) {
        pthread_mutex_lock(&hyperspace_mutex);
        args.input = inputs[i];
        args.cpuNum = i + 1;
        pthread_create(&threads[i], nullptr, calculateEntropy, &args);
    }

    for (int i = 0; i < inputs.size(); i++) {
        if (pthread_join(threads[i], nullptr) != 0) {
            std::cerr << "Error joining thread." << std::endl;
            return 1;
        }
    }

    return 0;
}
