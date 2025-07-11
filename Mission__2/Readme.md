--First install make command
1. sudo apt update
2. sudo apt install build-essential

--Building the scripts/Codebase
3. make all

--Running the codebase
4. taskset -c 0 ./sender.o & taskset -c 1 ./receiver.o
