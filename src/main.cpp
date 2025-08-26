/*
load balancer act like a “middleman proxy.”
*/
#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include <mutex>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

std::vector<std::pair<std::string, int>> backends = {
    {"127.0.0.1", 8081},
    {"127.0.0.1", 8082},
    {"127.0.0.1", 8083}
};
/*
This is a list of backend servers (IP + port).
Currently, we have 3 backends running on localhost:8081, 8082, 8083.
The load balancer will forward requests to these servers in turn.
*/

int current = 0;
std::mutex mtx;

std::pair<std::string, int> get_next_backend() {
    std::lock_guard<std::mutex> lock(mtx); //insure concureny or multithreading
    auto backend = backends[current];
    current = (current + 1) % backends.size();
    return backend;
}
/*
current keeps track of which backend to use.
Each time a new request comes in, we return the next backend (8081 → 8082 → 8083 → back to 8081).
std::mutex ensures thread-safety so multiple clients don’t mess up the counter.
*/

void handle_client(int client_sock) {
    auto backend = get_next_backend();

    int backend_sock = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in backend_addr{};
    backend_addr.sin_family = AF_INET;
    backend_addr.sin_port = htons(backend.second);
    backend_addr.sin_addr.s_addr = inet_addr(backend.first.c_str());

    if (connect(backend_sock, (struct sockaddr*)&backend_addr, sizeof(backend_addr)) < 0) {
        std::cerr << "❌ Failed to connect to backend " 
                  << backend.first << ":" << backend.second << "\n";
        close(client_sock);
        return;
    }

    char buffer[4096];
    ssize_t bytes;
    while ((bytes = recv(client_sock, buffer, sizeof(buffer), 0)) > 0) {
        // Forward client → backend
        send(backend_sock, buffer, bytes, 0);

        // Forward backend → client
        bytes = recv(backend_sock, buffer, sizeof(buffer), 0);
        if (bytes > 0) send(client_sock, buffer, bytes, 0);
    }

    close(client_sock);
    close(backend_sock);
}
/*
Step 1: Pick a backend using round robin.
Step 2: Open a socket to the backend server.
Step 3: Relay data:
  Read request from client (recv).
  Forward it to backend (send).
  Read backend’s response (recv).
  Send it back to client (send).
Step 4: Close both connections after the client disconnects.
*/

int main() {
    int server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock < 0) {
        std::cerr << "❌ Failed to create socket\n";
        return 1;
    }

    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080);  // Load balancer listens on port 8080
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        std::cerr << "❌ Bind failed\n";
        return 1;
    }

    if (listen(server_sock, 10) < 0) {
        std::cerr << "❌ Listen failed\n";
        return 1;
    }

    std::cout << "🚀 Load Balancer running on port 8080\n";

    while (true) {
        int client_sock = accept(server_sock, nullptr, nullptr);
        if (client_sock >= 0) {
            std::thread(handle_client, client_sock).detach();
        }
    }

    close(server_sock);
    return 0;
}

/*
Creates a TCP server that listens on port 8080.
Accepts incoming client connections (accept).
For each client, spawns a new thread running handle_client().
This allows multiple clients simultaneously.
Runs in an infinite loop to handle requests forever.
*/