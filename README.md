# Simple-Chat

Multi-room user chat on terminal.

This is assignment of Computer Networks course

server's default port is 5000

List of commands:
* /client xxx.xxx.xxx.xxx:5000 "room number" "Your Name" - command to join to the room
* All : text - to send message to all users in current room
* Bob, Dan, Stan : text - to send message to users Bob, Dan and Stan
* Bob#4, Dan#10, Stan#3 : text - to send message to users Bob, Dan and Stan after 4, 10 and 3 seconds respectively 
* /quit - to leave chat
* /room "roomnumer" - to change room
* /list - receives list of users in the current room

List of function:
* After joining room, joiner receives welcome message and other users of current room receives message about new user in room
* Server checks connection every 3 seconds. If there is no connection with client, server disconnects from user
* After disconnecting from client or leaving of this client, users of this room receives message about disconnecting client


