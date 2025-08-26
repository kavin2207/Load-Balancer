Load Balancer (C++)

A simple TCP load balancer in C++ that distributes client requests to multiple backend servers using Round-Robin.

Features:
- Round-robin request distribution
- Multi-threaded with mutex protection
- TCP sockets for communication
- Configurable backend pool
- Example HTTP test setup

Project Structure:
src/main.cpp             -> Load balancer implementation
include/load_balancer.h  -> Header file
tests/test_round_robin.cpp -> Round-robin tests
docs/PROJECT_DOCS.txt    -> Documentation

Build & Run:
1. Compile Load Balancer
   g++ src/main.cpp -o load_balancer -pthread

2. Run backend servers
   python3 -m http.server 9001
   python3 -m http.server 9002

3. Run load balancer
   ./load_balancer

4. Test with curl
   curl http://localhost:8080
   curl http://localhost:8080

Tests:
   g++ tests/test_round_robin.cpp -o test_round_robin -pthread
   ./test_round_robin

Future Improvements:
- Least connections, IP hash algorithms
- Health checks
- Logging and monitoring
- Config file support

License: MIT
