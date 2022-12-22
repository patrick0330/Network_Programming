#include <iostream>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <ctime>
#include <iomanip>
#include <vector>

using namespace std;

int main(int argc, char* argv[]){
// create sockets
    int command_fd = socket(AF_INET, SOCK_STREAM, 0);
    // cout << "command_fd is " << command_fd << endl;
    if(command_fd == -1){
        cout << "Can't create command_fd!" << endl;
        return -1;
    }
    sockaddr_in command_client;
    command_client.sin_family = AF_INET;
    command_client.sin_port = htons(atoi(argv[2])); // 9998 
    inet_pton(AF_INET, argv[1], &command_client.sin_addr);
    int err = connect(command_fd,(struct sockaddr *)&command_client,sizeof(command_client));
    if(err == -1){
        cout << "Connection failed" << endl;
    }
    while(true){
        string msg;
        cin >> msg;
        char receiveMessage[1024];
        msg += "\n";
        send(command_fd, msg.c_str(), msg.size() + 1, 0);
        recv(command_fd, receiveMessage, 1024, 0);
        cout << receiveMessage << endl;
    }
    
    return 0;
}