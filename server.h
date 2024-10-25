// server.h
#ifndef server_h
#define server_h

#include <iostream>
#include <iomanip>
#include <string>
#include <cstring>
#include <ctime>
#include <chrono>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <fstream>

#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>

class MessageServer {
private:

    int socketfd;

    // Stores all client fds
    std::unordered_set<int> master;

    // Stores client fds who are in the listening stage
    std::unordered_set<int> listening_clients;

    // Map clients
    std::unordered_map<int,int> matched_clients;

    void make_sockaddr(struct sockaddr_in *addr, int listening_port);

    // Handle new connection from client to server
    void handle_new_connection();

    /*  Handle client request
        These can be:
          - Chosing listening or connecting
          - Request to connect to client
          - Message client
    */
    void handle_client_request(int clientfd);

    void handle_client_listen(int clientfd);

    void handle_client_match(int clientfd);
    
    void handle_client_message(int clientfd);

    void handle_error_message(int clientfd);

    void create_listener_lists(std::string list_str);

public:

    MessageServer();

    void start_server(int listening_port);

};

#endif
