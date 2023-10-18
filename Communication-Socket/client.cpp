#include <iostream>
#include <iomanip>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sstream>
#include <vector>
#include <pthread.h>
#include <netdb.h> //needed this shit for hostent

struct TaskData {
    int cpuNum;
    std::string input;
    std::vector<double> entropyValues;
    struct hostent* serverHost;
    struct sockaddr_in serverAddr;
    int serverPort;
};

void* handleServerResponse(void* arg) {
    TaskData* taskData = static_cast<TaskData*>(arg);

    int clientSocket;

    // Create socket
    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket < 0) {
        perror("Error creating socket");
        exit(1);
    }

    // Set up server address
    taskData->serverAddr.sin_family = AF_INET;
    taskData->serverAddr.sin_port = htons(taskData->serverPort);
    bcopy((char*)taskData->serverHost->h_addr, (char*)&taskData->serverAddr.sin_addr, taskData->serverHost->h_length);

    // Connect to the server
    if (connect(clientSocket, (struct sockaddr*)&taskData->serverAddr, sizeof(taskData->serverAddr)) < 0) {
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

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "usagee " << argv[0] << " hostname port" << std::endl;
        return 1;
    }

    char* serverName = argv[1];
    int serverPort = std::stoi(argv[2]);

    struct hostent* serverHost = gethostbyname(serverName);
    if (serverHost == nullptr) {
        std::cerr << "Error: Unknown host" << std::endl;
        return 1;
    }

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
        taskData.serverHost = serverHost;
        taskData.serverPort = serverPort;

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
