#ifndef server_h
#define server_h

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fstream>

#include <string.h>
#include <string>
#include <iostream>
#include <map>
#include <vector>
#include <bits/stdc++.h>

using namespace std;

#define PORT 5000;

class Server{
private:

  struct Client{
    int socket_id;
    int room_id;
    string name;
    int name_counter;
    Client(): name_counter(0){}
    void set_client(int _sock_id, int _room_id, string _name);
  };

  struct Room{
    int clients_size;
    int id;
    map<int, Client> clients;
    map<string, int> names;
    map<string, int> sock_finder;
    Room():clients_size(0){}
    Room(int _id): id(_id), clients_size(0){}
    void add_user(int sock, string& name);
  };

public:
  Server();
  void server_run();
  void create_connection(int sock);
  void handling_connection(char* buffer, int n, int sock);
  void handling_message(char* buffer, int n, int sock);
  void send_all_message(char* buffer, int n, int sock);
  void remove_client(int sock);
  //void send_connection_protocol();
  //void change_room(int sock, int room_num);
  bool send_message(int sock, int sock_recv, char* message);
  void send_message_timer(int sock, string ans, int tim);
  //void send_delayed_message(int sock, int sock, char* message, char* mes_len);
  ~Server();

private:
  vector<thread> threads;
  vector<int> allsockets;
  map<int, Room> room_finder;
  map<int, Client*> client_finder;
  mutex door_mutex;
  int sockfd;
  int newsockfd;
  int portno;
  int clilen;
  struct sockaddr_in serv_addr, cli_addr;
  int n;

};
#endif
