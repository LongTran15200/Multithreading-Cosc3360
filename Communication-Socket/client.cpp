#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sstream>
#include <vector>
#include <pthread.h>
#include <iomanip>


struct TaskData {
    int cpuNum;
    std::string input;
    std::vector<double> entropyValues;
};

void* handleServerResponse(void* arg) {
    const char* SERVER_IP = "127.0.0.1";  // Replace with your server's IP address
    const int SERVER_PORT = 1221;
    TaskData* taskData = static_cast<TaskData*>(arg);

    int clientSocket;
    struct sockaddr_in serverAddr;

    // Create socket
    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket < 0) {
        perror("Error creating socket");
        exit(1);
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SERVER_PORT);
    serverAddr.sin_addr.s_addr = inet_addr(SERVER_IP);

    // Connect to the server
    if (connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("Error connecting to server");
        close(clientSocket);
        pthread_exit(NULL);
    }

    // Send the input data to the server
    send(clientSocket, taskData->input.c_str(), taskData->input.size(), 0);

    char buffer[1024];
    memset(buffer, 0, sizeof(buffer));

    // Receive entropy data from the server
    ssize_t bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
    if (bytesReceived <= 0) {
        perror("Error receiving data from server");
        close(clientSocket);
        pthread_exit(NULL);
    }

    std::string entropyData(buffer);

    // Parse and store entropy values
    std::stringstream ss(entropyData);
    double entropyValue;
    taskData->entropyValues.clear();

    while (ss >> entropyValue) {
        taskData->entropyValues.push_back(entropyValue);
    }

    close(clientSocket);
    pthread_exit(NULL);
}

int main() {
    int cpuNum = 1;
    std::string input;
    std::vector<TaskData> taskDataVec;

    // Reading input
    while (std::getline(std::cin, input)) {
        if (input.empty()) {
            break;
        }

        TaskData taskData;
        taskData.input = input;
        taskData.cpuNum = cpuNum;

        taskDataVec.push_back(taskData);

        cpuNum++;
    }

    // Create pthreads for each CPU task
    pthread_t threads[taskDataVec.size()];

    // Create and execute threads for handling server responses
    for (size_t i = 0; i < taskDataVec.size(); i++) {
        if (pthread_create(&threads[i], nullptr, handleServerResponse, &taskDataVec[i]) != 0) {
            std::cerr << "Error creating thread for handling server response." << std::endl;
            return 1;
        }
    }

    // Wait for pthreads to finish handling server responses
    for (size_t i = 0; i < taskDataVec.size(); i++) {
        if (pthread_join(threads[i], nullptr) != 0) {
            std::cerr << "Error joining thread for handling server response." << std::endl;
            return 1;
        }
    }

    // This for loop is for outputting the information
    for (size_t i = 0; i < taskDataVec.size(); i++) {
        const TaskData& taskData = taskDataVec[i];
        std::cout << "CPU " << taskData.cpuNum << std::endl;
        std::cout << "Task scheduling information: ";

        // Creating sstream for output
        std::stringstream ss(taskData.input);
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

        std::cout << "Entropy for CPU " << taskData.cpuNum << std::endl;
        for (double entropyValue : taskData.entropyValues) {
            std::cout << std::fixed << std::setprecision(2) << entropyValue << " ";
        }
        std::cout << std::endl;
    }

    return 0;
}
