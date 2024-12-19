#include <chrono>
#include <cstring>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <protocol.h>
#include <MessageServer.h>

MessageServer::MessageServer(uint16_t _listening_port) : server_sock(_listening_port) {
}

void MessageServer::run() {
  while (true) {
    boost::thread t(&MessageServer::handle_client, this, server_sock.accept_client()).detach();
  }
}

void handle_client(int client_fd) {
  UserSocket user_sock(client_fd);
  try {
    while (true) {
    
    }
  }
  catch (...) {

  } 
}

