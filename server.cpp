#include <iostream>
#include <iomanip>
#include <string>
#include <cstring>
#include <ctime>
#include <chrono>
#include <vector>
#include <unordered_map>
#include <sstream>

#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>

#include "server.h"

MessageServer::MessageServer(int listening_port_in) : listening_port(listening_port_in) {}

void MessageServer::make_sockaddr(struct sockaddr_in *addr) {
    addr->sin_family = AF_INET;
    addr->sin_addr.s_addr = htonl(INADDR_ANY);
    addr->sin_port = htons(listening_port);    
}

void MessageServer::start_server() {

    socketfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    struct sockaddr_in server_addr;
    make_sockaddr(&server_addr);

    if (bind(socketfd, (sockaddr*) &server_addr, sizeof(server_addr)) < 0) {
        std::cerr << "Could not bind to socket." << std::endl;
        exit(1);
    }

    if (listen(socketfd, 10) < -1) {
        std::cerr << "Could not listen." << std::endl;
        exit(1);
    }
    
    fd_set read_fds;
    while (true) {
        FD_ZERO(&read_fds);
        FD_SET(socketfd, &read_fds);
        
        for (const auto& clientfd: master) FD_SET(clientfd, &read_fds);

        if (select(FD_SETSIZE, &read_fds, nullptr, nullptr, nullptr) < 0) {
            std::cerr << "select error" << std::endl;
        }
        
        if (FD_ISSET(socketfd, &read_fds)) {
            handle_new_connection();
        }

        for (auto it = master.begin(); it != master.end();) {
            int clientfd = *it;
            // Is fd ready for reading?
            if (FD_ISSET(clientfd, &read_fds)) {
                handle_client_request(clientfd);
            }
            ++it;
        }
    }
}

void MessageServer::handle_new_connection() {
    int clientfd = accept(socketfd, nullptr, nullptr);
    if (clientfd < 0) {
        std::cerr << "Connection failed" << std::endl;
        return;
    }
    master.insert(clientfd);
}

void MessageServer::handle_client_request(int clientfd) {
    // Receive opcode
    uint32_t network_order;
    ssize_t bytes = recv(socketfd, &network_order, sizeof(network_order), MSG_WAITALL);

    uint32_t opcode = ntohl(network_order);
    std::cout << "Opcode received: " << opcode << std::endl;
    
    switch (opcode) {
        // Chose to listen
        case 0:
            handle_client_listen(clientfd);
            break;
        // Chose to connect
        case 1:
            handle_client_connect(clientfd);
            break;
        // Request to connect to client
        case 2:
            handle_client_match(clientfd);
            break;
        // Message client
        case 3:
            handle_client_message(clientfd);
            break;
        // Send client an error message
        default:
            handle_error_message(clientfd);
    }
}

void MessageServer::handle_client_listen(int clientfd) {
    listening_clients.insert(clientfd);
}

void MessageServer::handle_client_connect(int clientfd) {
    std::string list_str;
    create_listener_lists(list_str);
}

void MessageServer::create_listener_lists(std::string list_str) {
    std::stringstream ss;
    for (auto it = listening_clients.begin(); it != listening_clients.end();) {
        ss << *it << ".";
    }
    list_str = ss.str();
}
