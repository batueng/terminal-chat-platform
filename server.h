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

    class Session {
    private:
        uint64_t id;
        std::unordered_set<int> session_fds;    
    public:
        Session() : id(MessageServer::glob_session_id++) {}
    };

    static uint64_t glob_session_id;

    int socketfd;
    int listening_port;
    // Stores all client fds
    std::unordered_set<int> master;
    std::unordered_set<Session> sessions;
    std::unordered_map<int, Session*> fd_session;
    std::unordered_map<std::string, Session*> name_session;
    std::unordered_map<int, Session*> id_session;

    // Stores client fds who are in the listening stage
    std::unordered_set<int> listening_clients;

    void make_socket();

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

    MessageServer(int _listening_port);

    void start();

};

#endif
