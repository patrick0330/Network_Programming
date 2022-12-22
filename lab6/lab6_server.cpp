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

    int sink_fd = socket(AF_INET, SOCK_STREAM, 0);
    // cout << "sink_fd is " << sink_fd << endl;
    if(sink_fd == -1){
        cout << "Can't create sink_fd!" << endl;
        return -1;
    }

    // set up server info
    sockaddr_in command_server;
    command_server.sin_family = AF_INET;
    command_server.sin_port = htons(atoi(argv[1])); // 9998 
    inet_pton(AF_INET, "127.0.0.1", &command_server.sin_addr);

    sockaddr_in sink_server;
    sink_server.sin_family = AF_INET;
    sink_server.sin_port = htons(atoi(argv[1]) + 1); // 9999
    inet_pton(AF_INET, "127.0.0.1", &sink_server.sin_addr);

    // bind the server to the fd
    if(bind(command_fd, (sockaddr *)&command_server, sizeof(command_server)) == -1){
        cout << "Can't bind to command_fd";
        return -2;
    }

    if(bind(sink_fd, (sockaddr *)&sink_server, sizeof(sink_server)) == -1){
        cout << "Can't bind to sink_fd";
        return -2;
    }

    //listen to port
    if(listen(command_fd, SOMAXCONN) == -1){
        cout << "Can't listen";
        return -3;
    }

    if(listen(sink_fd, SOMAXCONN) == -1){
        cout << "Can't listen";
        return -3;
    }
    int command_pos = -1;
    fd_set master;
    FD_ZERO(&master);
    FD_SET(command_fd, &master);
    FD_SET(sink_fd, &master);
    int counter = 0;
    struct timeval last_reset;
    bool last_set = false;
    int sink_connection = 0;
    while(true){
        fd_set copy = master;
        select(FD_SETSIZE, &copy, nullptr, nullptr, nullptr);
        if(FD_ISSET(command_fd, &copy)){
            int client = accept(command_fd, nullptr, nullptr);
            cout << "A client connected on command server" << endl;
            FD_SET(client, &master);            
            // string msg = " *** Welcome to the command server\n";            
            // send(client, msg.c_str(), msg.size() + 1, MSG_NOSIGNAL);
            command_pos = client;            
            
        }
        else if(FD_ISSET(sink_fd, &copy)){
            int client = accept(sink_fd, nullptr, nullptr);
            sink_connection ++;
            // cout << "Client connected on sink server, client is " << client << endl;
            FD_SET(client, &master);                 
            // string msg = " *** Welcome to the sink server\n";        
            // send(client, msg.c_str(), msg.size() + 1, MSG_NOSIGNAL);  
        }
        for(int i = 0;i < FD_SETSIZE;i++){        
            char buf[1024];
            memset(buf, 0, 1024);
            if((i != command_fd) && (i != sink_fd) && FD_ISSET(i, &copy)){
                int nBytes = recv(i, buf, 1024, 0);
                cout << buf;
                // cout << "Bytes received is " << nBytes << endl;
                if(nBytes <= 0){
                    close(i);                       
                    FD_CLR(i, &master);
                    cout << "A client disconnected from server" << endl;
                    if(i != command_pos){
                        sink_connection --;
                    }
                    // cout << "online is " << sink_connection << endl;
                }
                else if(i == command_pos){                                                
                    string cmd = string(buf,nBytes - 1);
                    if(cmd.substr(0, 6) == "/reset"){
                        struct timeval tv;
                        gettimeofday(&tv, NULL);
                        string reset_msg = to_string(tv.tv_sec) + "." + to_string(tv.tv_usec) + " RESET " + to_string(counter) + "\n";
                        send(i, reset_msg.c_str(), reset_msg.size() + 1, MSG_NOSIGNAL);
                        counter = 0;
                        last_reset.tv_sec = tv.tv_sec;
                        last_reset.tv_usec = tv.tv_usec;
                        last_set = true;
                    }
                    else if(cmd.substr(0, 5) == "/ping"){
                        struct timeval tv;
                        gettimeofday(&tv, NULL);
                        string ping_msg = to_string(tv.tv_sec) + "." + to_string(tv.tv_usec) + " PONG\n";
                        send(i, ping_msg.c_str(), ping_msg.size() + 1, MSG_NOSIGNAL);                            
                    }
                    else if(cmd.substr(0, 7) == "/report"){
                        if(!last_set){
                            cout << "last_time not set" << endl;
                            continue;
                        }
                        struct timeval tv;
                        gettimeofday(&tv, NULL);
                        double elapse_time;
                        double mbps;

                        elapse_time = tv.tv_sec - last_reset.tv_sec;
                        elapse_time += (tv.tv_usec - last_reset.tv_usec) / 1000000.0;
                        mbps = 8.0 * counter / 1000000.0 / elapse_time;
                        string report_msg = to_string(tv.tv_sec) + "." + to_string(tv.tv_usec) + " REPORT " + to_string(counter) + " " + to_string(elapse_time) + "s " + to_string(mbps) + "Mbps\n";
                        send(i, report_msg.c_str(), report_msg.size() + 1, MSG_NOSIGNAL);                            
                    }
                    else if(cmd.substr(0, 8) == "/clients"){
                        struct timeval tv;
                        gettimeofday(&tv, NULL);
                        string client_msg = to_string(tv.tv_sec) + "." + to_string(tv.tv_usec) + " CLIENTS " + to_string(sink_connection) + "\n";
                        send(i, client_msg.c_str(), client_msg.size() + 1, MSG_NOSIGNAL);                            
                    }
                    else{
                        string err_msg;
                        err_msg = "No such command\n";
                        send(i, err_msg.c_str(), err_msg.size() + 1, MSG_NOSIGNAL);  
                    }
                    
                }
                else{            
                    // cout << "Bytes received is " << nBytes << endl;
                    counter += nBytes;
                }
            }
        }          

        
    }
    return 0;
}