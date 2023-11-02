#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/wait.h>
#include <vector>
#include <map>
#include <sstream>
#include <cmath>
#include <pthread.h>

//calculating entropy function
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

//this is needed for the server (fork)
void fireman(int) {
    while (waitpid(-1, nullptr, WNOHANG) > 0) {}
}

void handleClient(int clientSocket) {
    char buffer[1024];
    memset(buffer, 0, sizeof(buffer));

    // Receive scheduling information from the client
    ssize_t bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
    if (bytesReceived <= 0) {
        perror("Error receiving data from client");
        close(clientSocket);
        return;
    }

    std::string inputData(buffer);

    // Parse input data and calculate entropy
    std::map<char, int> freq;
    double currentH = 0.0;
    int currFreq = 0;

    std::vector<double> entropyValues;

    std::stringstream ss(inputData);
    char task;
    int freqVal;

    while (ss >> task >> freqVal) {
        // Calculate and update entropy
        double H = entropy(freq, currFreq, currentH, task, freqVal);
        currentH = H;
        currFreq += freqVal;

        // Collect entropy values
        entropyValues.push_back(currentH);
    }

    // Send entropy values back to the client
    std::stringstream entropyStream;
    for (double entropyValue : entropyValues) {
        entropyStream << entropyValue << " ";
    }

    std::string entropyData = entropyStream.str();
    send(clientSocket, entropyData.c_str(), entropyData.size(), 0);

    close(clientSocket);
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <Port>" << std::endl;
        return 1;
    }

    int serverSocket, clientSocket;
    socklen_t cli_len;
    struct sockaddr_in serv_addr, cli_addr;

    int serverPort = std::stoi(argv[1]);

    // Create socket
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) {
        perror("Error opening socket");
        exit(1);
    }

    // Set socket options
    int opt = 1;
    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
        perror("Error setting socket options");
        close(serverSocket);
        exit(1);
    }

    // Bind socket
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(serverPort);
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    if (bind(serverSocket, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Error binding socket");
        close(serverSocket);
        exit(1);
    }

    // Listen for incoming connections which is from the client
    listen(serverSocket, 5);

    // Install signal handler for reaping child processes
    signal(SIGCHLD, fireman);
    cli_len = sizeof(cli_addr);
    while (true) {
        clientSocket = accept(serverSocket, (struct sockaddr*)&cli_addr, &cli_len);
        if (clientSocket < 0) {
            perror("Error accepting connection");
            continue;
        }

        // Fork a child process to handle the client
        if (fork() == 0) {
            // In child process, close listening socket
            close(serverSocket);
            handleClient(clientSocket);
            exit(0);
        } else if (fork() < 0) {
            perror("Fork failed.");
            close(clientSocket);
            close(serverSocket);
            return 1;
        }

        // In parent process, close client socket
        close(clientSocket);
    }

    // Close the listening socket (which will not reach here)
    close(serverSocket);
    return 0;
}
