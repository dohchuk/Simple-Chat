#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <netdb.h>
#include <netinet/in.h>

#include <string>
#include <cstring>
#include <iostream>
#include <thread>

using namespace std;

string readword(int& pos, char* buffer){
  string ans;
  while(buffer[pos] != '\n' && buffer[pos] !=  ' '){
    ans += buffer[pos];
    pos++;
  }
  pos++;
  return ans;
}

void buffer_cpy(string s, char *buf){
  bzero(buf, 256);
  int len = s.length();
  for(int i = 0; i < len; i++)
    buf[i] = s[i];
}

void sending_handling_message(int sockfd, string s){
    int pos = 0;
    char buf[256];
    usleep(1000);
    buffer_cpy(s, buf);
    if(s[0] != '/'){
      string names = "";
      int num_ofus = 1;
      while(buf[pos] != ':'){
        if(buf[pos] != ',')
          names += buf[pos];
        else
          num_ofus++;
        pos++;
      }
      names.pop_back();
      string ans = "200\n";
      ans += to_string(num_ofus);
      ans += " " + names + '\n';
      pos += 2;
      if(names == "All"){
        ans = "201\n";
      }
      while(buf[pos] != '\n')
        ans += buf[pos++];
      ans += '\n';
      buffer_cpy(ans, buf);
      send(sockfd, buf, 256, 0);
    }
    else{
      string command = readword(pos, buf);
      if(s == "/quit\n"){
        string ans = "404\n";
        buffer_cpy(ans, buf);
        send(sockfd, buf, 256, 0);
      }
      else if(command == "/join"){
        string room = readword(pos, buf);
        string ans = "103\n";
        ans += room + "\n";
        buffer_cpy(ans, buf);
        send(sockfd, buf, 256, 0);
      }
      else if(command == "/list"){
        string ans = "104\n";
        buffer_cpy(ans, buf);
        send(sockfd, buf, 256, 0);
      }

    }


}

void receive_handling_message(char* buf){
  int pos = 0;
  string protocol = readword(pos, buf);
  if(protocol == "100"){
    string room_id = readword(pos, buf);
    string joiner_name = readword(pos,buf);
    cout << "Hello " + joiner_name + "! This is room #" + room_id << endl;
  }
  else if(protocol == "101"){
    string joiner_name = readword(pos, buf);
    string room_id = readword(pos, buf);
    cout << joiner_name + " joined room " + room_id << endl;
  }
  else if(protocol == "102"){
    string leaver_name = readword(pos, buf);
    string room_id = readword(pos, buf);
    cout << leaver_name + " is disconnected from room " + room_id << endl;
  }
  else if(protocol == "200"){
    string receiver_name = readword(pos, buf);
    string sender_name = readword(pos, buf);
    string message = "";
    while(buf[pos] != '\n')
      message += buf[pos++];
    cout << sender_name + " : " + message << endl;
  }
  else if(protocol == "104"){
    string room_id = readword(pos, buf);
    int num_users = stoi(readword(pos, buf));
    string temp;
    cout << "> This is list of users in room #" + room_id << endl;
    for(int i = 0; i < num_users; i++){
      temp = readword(pos, buf);
      cout << i + 1 << ": " + temp << endl;
    }
  }
  else if(protocol == "400"){
    int num_abs_users = stoi(readword(pos, buf));
    cout << "There is no such users :";
    string temp;
    for(int i = 0; i < num_abs_users; i++){
      temp = readword(pos, buf);
      cout << " " + temp;
      if(i != num_abs_users - 1)
        cout << ",";
    }
    cout << endl;
  }
  else if(protocol == "404"){
    string name = readword(pos, buf);
    cout << "Good bye " + name << endl;
    exit(0);
  }
  else if(protocol == "402"){
    string leaver_name = readword(pos, buf);
    string room_id = readword(pos, buf);
    cout << leaver_name + " is disconnected from room #" + room_id << endl;
  }
}

void receiver(int sockfd){
  char buffer[256];
  while(true){
    bzero(buffer, 256);
    int checker = recv(sockfd, buffer, 256, 0);
    if(checker == 0)
      exit(0);
    receive_handling_message(buffer);
    //int lel = recv(sockfd, bufer, 256, 0);
    //cout << lel << bufer << endl;
  }
}

void autosend(int sockfd){
  char buffer[256];
  sleep(1);
  while(true){
    bzero(buffer, 256);
    string k = "500\n";
    buffer_cpy(k, buffer);
    send(sockfd, buffer, 256, 0);
    sleep(3);
  }
}


int main(int argc, char *argv[]) {
 int sockfd, portno, n;
 struct sockaddr_in serv_addr;
 struct hostent *server;

 char buffer[256];
 string srvip = "";
 int pos1 = 0;
 while(argv[1][pos1] != ':')
  srvip += argv[1][pos1++];
 pos1++;
 string portstr = "";
 while(argv[1][pos1] != '\0')
  portstr += argv[1][pos1++];
 portno = stoi(portstr);
 server = gethostbyname((const char *)srvip.c_str());
 if (server == NULL) {
    fprintf(stderr,"ERROR, no such host\n");
    exit(0);
 }
 socklen_t clilen;
 bzero((char *) &serv_addr, sizeof(serv_addr));
 serv_addr.sin_family = AF_INET;
 bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
 serv_addr.sin_port = htons(portno);

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

 if(connect(sockfd,(struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0){
   printf("error on connection");
   return 0;
 }
 // TODO : create socket and get file descriptor


  // TODO : connect to server with server address which is set above (serv_addr)

  // TODO : inside this while loop, implement communicating with read/write or send/recv function
  string name_input = argv[3];
  string room_input = argv[2];
  string connection_string = "";

  bzero(buffer,256);
  string temp_buf = "100\n";
  temp_buf += room_input + "\n";
  temp_buf += name_input + "\n";
  //temp_buf += buffer;
  temp_buf.push_back('\n');
  buffer_cpy(temp_buf, buffer);
  send(sockfd, buffer, 256, 0);
     // ==> here
  thread rcv_thread(receiver, sockfd);
  thread health_thread(autosend, sockfd);
  string input_line;
  while(true){
    getline(cin, input_line);
    input_line += "\n";
    sending_handling_message(sockfd, input_line);
  }
     // TODO escape this loop, if the server sends message "quit"
  rcv_thread.join();
   return 0;
}
