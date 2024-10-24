#include <iostream>
#include <string>
#include <cstring>
#include <vector>
#include <sstream>

#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>

class Client {
private:
    int client_fd;
    int listening_port;
    
public:

    Client(int listening_port_in);

    int start_client(std::string log_file);
};

Client::Client(int listenting_port_in) : listening_port(listenting_port_in) {

}