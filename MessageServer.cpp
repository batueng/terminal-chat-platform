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

#include "protocol.h"
#include "MessageServer.h"

MessageServer::MessageServer(uint16_t _listening_port) : server_sock(_listening_port) {
}

void MessageServer::run() {
  while (true) {
    boost::thread t(&MessageServer::handle_client, this, server_sock.accept_client()).detach();
  }
}

void handle_client(int client_fd) {
  UserSocket user_sock(client_fd);
  while (true) {
    try {
      std::string buf = user_sock.recv_len(sizeof(tcp_hdr_t));
      tcp_hdr_t* tcp_hdr = reinterpret_cast<tcp_hdr_t*> (buf.c_str());

      std::string data = user_sock.recv_len(tcp_hdr->data_len);
      
      switch (tcp_hdr->method) {
        case WHERE:
          break;

        case JOIN:
          break;

        case CREATE:
          break;

        case CHAT:
          Message message{tcp_hdr->user_name, data};
          sessions[tcp_hdr->session_id]->queue_message(message);
        
        default:
          break;
        }
    }
    catch (...) {

    }
  }
}

