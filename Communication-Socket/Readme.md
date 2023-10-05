# Multi-Threaded Client-Server Application for Entropy Calculation

This C++ application demonstrates a multi-threaded client-server architecture for calculating entropy values of CPU task scheduling. The server calculates entropy values, and the client handles input and output using threads and socket communication.

## How It Works

### Server

- The server listens for incoming client connections on a specified IP address and port (default: 127.0.0.1:1221).
- Upon client connection, the server forks a child process to handle the client separately.
- The server receives task scheduling information from the client.
- Using the received information, the server calculates entropy values for each CPU.
- Entropy values are sent back to the client.
- The server efficiently manages multiple clients using a forked process for each connection.

### Client

- The client reads task scheduling information from the standard input (stdin).
- It creates a thread for each set of input data to calculate entropy values.
- Each thread sends the input data to the server using sockets for entropy calculation.
- The client receives entropy values from the server.
- After all threads have completed, the client outputs the results including CPU information, task scheduling details, and entropy values.

## How to Use

1. Compile and run the server application on a machine with a static IP address.
2. Compile and run the client application on the same machine or a different machine, specifying the server's IP address if different.
3. Input the task scheduling information on the client side.
4. The client sends input data to the server for entropy calculation.
5. The server calculates entropy values and sends them back to the client.
6. The client displays the entropy values for each CPU.

## Project Overview

This project demonstrates the use of sockets and multi-threading in a client-server architecture to efficiently compute entropy values for CPU task scheduling. It can be extended for various other distributed computing tasks.
