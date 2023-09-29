This code demonstrates how to implement entropy calculation using multithreading. It was taught in the course COSC3360 as an example of multithreading in C++.

# Overview
The code calculates entropy for a series of CPU task schedules. It takes input for each CPU, where each input line consists of task and frequency information. The code then calculates the entropy of the task schedule for each CPU using multithreading and displays the results.

# Code Structure
ThreadData struct: Stores specific information for each thread, including the input data, CPU number, and entropy values.

entropy function: Calculates the entropy using the given formula and updates the frequency map.

calculateEntropy function: The thread function responsible for calculating entropy for a single CPU.

main function: Reads input data for multiple CPUs, creates threads for each CPU to calculate entropy, and displays the results.

# Multithreading
The code demonstrates multithreading by creating a thread for each CPU's entropy calculation. Threads are created and executed using the POSIX thread library (pthread). Each thread operates independently, allowing for parallel execution and improved performance.

# Input and Output
The code reads input data for multiple CPUs, where each line contains task and frequency information. It then calculates the entropy for each CPU's task schedule and displays the results, including the CPU number, task scheduling information, and entropy values.

# Usage
To use this code:

Compile it using a C++ compiler with support for POSIX threads (e.g., g++ -pthread).
Provide input for each CPU's task schedule.
