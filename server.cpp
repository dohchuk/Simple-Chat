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
#include "server.h"
//#include <bits/stdc++.h>
#include <mutex>
#include <vector>
#include <map>



using namespace std;

#define PORT 5000;

string readword(int& pos, char* buffer){
  string ans;
  while(buffer[pos] != '\n' && buffer[pos] !=  ' '){
    ans += buffer[pos];
    pos++;
  }
  pos++;
  return ans;
}

void buffer_cpy(string& s, char *buf){
  bzero(buf, 256);
  int len = s.length();
  for(int i = 0; i < len; i++)
    buf[i] = s[i];
}

void Server::Client::set_client(int _sock_id, int _room_id, string _name){
  socket_id = _sock_id;
  room_id = _room_id;
  name = _name;
}

Server::Server(){
  /* Initialize socket structure */
}

void Server::server_run(){
  socklen_t clilen;
  struct sockaddr_in serv_addr, cli_addr;
  clilen = sizeof(cli_addr);
   // TODO : create socket and get file descriptor
  sockfd = socket(AF_INET, SOCK_STREAM, 0);

  if(sockfd < 0){
    perror("ERROR opening socket");
    exit(1);
  }
  int tr = 1;
  if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &tr, sizeof(int)) == -1){
    perror("setsockopt");
    exit(1);
  }
  bzero((char *) &serv_addr, sizeof(serv_addr));
  portno = 5001;

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port = htons(portno);

  // TODO : Now bind the host address using bind() call.

  if(bind(sockfd, (sockaddr *)&serv_addr, sizeof(serv_addr)) < 0){
    perror("ERROR on binding");
    exit(1);
  }
  // TODO : start listening for the clients,
  // here process will go in sleep mode and will wait for the incoming connection1
  listen(sockfd, 5);


  // TODO: Accept actual connection from the client
  cout << "successful binding" << endl;
  while (1) {
    cout << "gonna accept" << endl;
    newsockfd = accept(sockfd, (sockaddr*)&cli_addr, (socklen_t*)&clilen);
    cout << "accepted" << endl;
    threads.push_back((thread)thread(&Server::create_connection, this, newsockfd));
    // TODO escape this loop, if the client sends message "quit"
  }
   // TODO : inside this while loop, implement communicating with read/write or send/recv function
   return;
}

Server::~Server(){
}

void Server::create_connection(int sock){
  struct timeval tv;
  tv.tv_sec = 3;
  tv.tv_usec = 0;
  setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);
  printf("starting new connection");
  int n;
  char buffer[256];
  bzero(buffer, 256);
  n = recv(sock, buffer, 256, 0);
  cout << buffer << endl;
  if(n < 0){
    printf("recv() error\n");
    return;
  }
  else if(n == 0){
    printf("Client disconnected unexpectedly\n");
    return;
  }
  else{
    handling_connection(buffer, n, sock);
  }
  door_mutex.lock();
  string ans = "100\n";
  ans += (to_string(client_finder[sock]->room_id));
  ans += "\n";
  ans += (string)client_finder[sock]->name;
  ans += "\n";
  buffer_cpy(ans, buffer);
  send(sock, buffer, 256, 0);
  for(auto clint : room_finder[client_finder[sock]->room_id].clients){
    if(clint.second.socket_id != sock){
      ans = "101\n" + client_finder[sock]->name + "\n" + to_string(client_finder[sock]->room_id) + "\n";
      buffer_cpy(ans, buffer);
      send(clint.second.socket_id, buffer, 256, 0);
    }
  }
  door_mutex.unlock();
  int cnt = 0;
  while(true){
    cnt++;
    bzero(buffer, 256);
    n = recv(sock, buffer, 256, 0);
    if(n <= 0){
      door_mutex.lock();
      cout << "close connection wits descriptor " << sock << endl;
      for(auto i : room_finder[client_finder[sock]->room_id].clients){
        if(i.second.socket_id != sock){
          string ans = "402\n";
          ans += client_finder[sock]->name + '\n';
          ans += to_string(client_finder[sock]->room_id) + '\n';
          buffer_cpy(ans, buffer);
          send(i.second.socket_id, buffer, 256, 0);
        }

      }
      remove_client(sock);
      door_mutex.unlock();
      return;
    }
    else
      handling_message(buffer, n, sock);
  }

}

void Server::handling_connection(char* buffer, int n, int sock){
  door_mutex.lock();
  int pos = 0;
  while(buffer[pos] != '\n'){
    pos++;
  }
  pos++;
  string room_number = readword(pos, buffer);
  int roomid = stoi(room_number);
  string user_name = readword(pos, buffer);
  if(!room_finder.count(roomid)){
    room_finder[roomid] = Room(roomid);
  }
  room_finder[roomid].add_user(sock, user_name);
  room_finder[roomid].sock_finder[user_name] = sock;
  client_finder[sock] = &(room_finder[roomid].clients[sock]);
  door_mutex.unlock();
}

void Server::send_message_timer(int sock, string ans, int tisp){
  //cout << ans << endl;
  sleep(tisp);
  char buffer[256];
  buffer_cpy(ans, buffer);
  send(sock, buffer, 256, 0);
}

void Server::handling_message(char* buffer, int n, int sock){
  door_mutex.lock();
  int pos = 0;
  int protocol_number = stoi(readword(pos, buffer));
  vector<string> receiver_names;
  if(protocol_number == 200){
    int num_ofrcv = stoi(readword(pos, buffer));
    for(int i = 0; i < num_ofrcv; i++)
      receiver_names.push_back(readword(pos, buffer));
    string _message;
    while(buffer[pos] != '\n')
      _message += buffer[pos++];
    _message += '\n';
    cout << _message << endl;
    vector<string> nnames;
    for(int i = 0; i < num_ofrcv; i++){
      string ans = "200\n";
      int count1 = 0;
      int strl = receiver_names[i].length();
      string current_name = "";
      bool status = 0;
      string coun = "";
      for(int j = 0; j < strl; j++){
        if(receiver_names[i][j] == '#')
          status = 1;
        else if(status)
          coun += receiver_names[i][j];
        else
          current_name += receiver_names[i][j];
      }
      if(coun != "")
        count1 = stoi(coun);
      ans += current_name + "\n";
      ans += client_finder[sock]->name + "\n";
      ans += _message;
      buffer_cpy(ans, buffer);
      if(room_finder[client_finder[sock]->room_id].sock_finder.count(current_name)){
        if(!count1)
          send(room_finder[client_finder[sock]->room_id].sock_finder[current_name], buffer, 256, 0);
        else{
          threads.push_back((thread)thread(&Server::send_message_timer, this, room_finder[client_finder[sock]->room_id].sock_finder[current_name], ans, count1));
        }
      }
      else
        nnames.push_back(current_name);
    }
    if(nnames.size()){
      string ans = "400\n";
      ans += to_string(nnames.size());
      for(int i = 0; i < nnames.size(); i++){
        ans += " " + nnames[i];
      }
      ans += '\n';
      buffer_cpy(ans, buffer);
      send(sock, buffer, 256, 0);
    }
  }
  if(protocol_number == 201){
    string _message = "";
    while(buffer[pos] != '\n')
      _message += buffer[pos++];
    _message += '\n';
    for(auto i : room_finder[client_finder[sock]->room_id].clients){
      if(i.second.socket_id != sock){
        string ans = "200\n";
        ans += i.second.name + '\n';
        ans += client_finder[sock]->name + '\n';
        ans += _message;
        buffer_cpy(ans, buffer);
        send(i.second.socket_id, buffer, 256, 0);
      }
    }
  }
  if(protocol_number == 103){
    int roomid = stoi(readword(pos, buffer));
    if(!room_finder.count(roomid))
      room_finder[roomid] = Room(roomid);
    room_finder[roomid].add_user(sock, client_finder[sock]->name);
    int temp_roomid = client_finder[sock]->room_id;
    room_finder[temp_roomid].clients_size--;
    room_finder[temp_roomid].sock_finder.erase(client_finder[sock]->name);
    room_finder[temp_roomid].names.erase(client_finder[sock]->name);
    room_finder[temp_roomid].clients.erase(sock);
    client_finder[sock] = &(room_finder[roomid].clients[sock]);
    string ans = "100\n";
    ans += (to_string(client_finder[sock]->room_id));
    ans += "\n";
    ans += (string)client_finder[sock]->name;
    ans += "\n";
    buffer_cpy(ans, buffer);
    send(sock, buffer, 256, 0);
    for(auto clint : room_finder[client_finder[sock]->room_id].clients){
      if(clint.second.socket_id != sock){
        ans = "101\n" + client_finder[sock]->name + "\n" + to_string(client_finder[sock]->room_id) + "\n";
        buffer_cpy(ans, buffer);
        send(clint.second.socket_id, buffer, 256, 0);
      }
    }
  }
  if(protocol_number == 404){
    string ans = "404\n" + client_finder[sock]->name + "\n";
    buffer_cpy(ans, buffer);
    send(sock, buffer, 256, 0);
  }
  if(protocol_number == 104){
    string ans = "104\n";
    int rooom = client_finder[sock]->room_id;
    ans += to_string(rooom) + "\n";
    ans += to_string(room_finder[rooom].clients_size);
    for(auto i : room_finder[rooom].clients){
      ans += " " + i.second.name;
    }
    ans += "\n";
    buffer_cpy(ans, buffer);
    send(sock, buffer, 256, 0);
  }
  door_mutex.unlock();
}

void Server::Room::add_user(int sock, string& name){
  int cnt = 0;
  while(names.count(name)){
    if(cnt == 0)
      names[name] = 1;
    else if(cnt == 1)
      name += "_2";
    else if(cnt > 1){
      name.pop_back();
      name += to_string(cnt + 1);
    }
    cnt++;
  }
  names[name] = 1;
  clients[sock].set_client(sock, id, name);
  clients_size++;
  sock_finder[name] = sock;
}

void Server::remove_client(int sock){
  int temp_roomid = client_finder[sock]->room_id;
  room_finder[temp_roomid].clients_size--;
  room_finder[temp_roomid].sock_finder.erase(client_finder[sock]->name);
  room_finder[temp_roomid].names.erase(client_finder[sock]->name);
  room_finder[temp_roomid].clients.erase(sock);
  client_finder[sock] = NULL;
}

int main(){
  Server ss;
  ss.server_run();
}
