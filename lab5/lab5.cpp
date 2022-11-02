#include <iostream>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <time.h>
#include <sys/wait.h>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <random>
#include <vector>


using namespace std;


int from = 1000;
int to = 9999;
random_device rand_dev;
mt19937 generator(rand_dev());
uniform_int_distribution<int> distr(from, to);
vector<string> user_name(FD_SETSIZE, "");
vector<string> user_ip(FD_SETSIZE, "");

int online = 0;

int main(int argc, char* argv[]){
    int listening = socket(AF_INET, SOCK_STREAM, 0);
    if(listening == -1){
        cout << "Can't create server!" << endl;
        return -1;
    }
    sockaddr_in hint;
    hint.sin_family = AF_INET;
    hint.sin_port = htons(atoi(argv[1]));
    inet_pton(AF_INET, "127.0.0.1", &hint.sin_addr);

    if(bind(listening, (sockaddr *)&hint, sizeof(hint)) == -1){
        cout << "Can't bind to IP/port";
        return -2;
    }

    if(listen(listening, SOMAXCONN) == -1){
        cout << "Can't listen";
        return -3;
    }

    fd_set master;
    FD_ZERO(&master);

    FD_SET(listening, &master);
    sockaddr_in client_info;
    socklen_t client_size = sizeof(client_info);
    char host[NI_MAXHOST];
    char svc[NI_MAXSERV];

    while(true){
        fd_set copy = master;
        cout << "Current online: " << online << endl;
        select(FD_SETSIZE, &copy, nullptr, nullptr, nullptr);
        if(FD_ISSET(listening, &copy)){
            // cout << listening << endl;
            int client = accept(listening, (sockaddr *)&client_info, &client_size);
            cout << "client is " << client << endl;
            if(client >= FD_SETSIZE){
                client -= FD_SETSIZE;
                while(1){
                    if(FD_ISSET(client, &copy)){
                        client ++;
                    }
                    else{
                        break;
                    }
                    if(client >= FD_SETSIZE){
                        client -= FD_SETSIZE;
                    }
                }
            }
            FD_SET(client, &master);
            int result = getnameinfo((sockaddr *)&client_info, sizeof(client_info), host, NI_MAXHOST, svc, NI_MAXSERV, 0);
            inet_ntop(AF_INET, &client_info.sin_addr, host, NI_MAXHOST);
            string ip = string(host) + ":" + string(svc);
            
            user_ip[client] = ip;
            cout << "* client connected from " << ip << endl;
            
            online ++;
            int user_id = distr(generator);
            user_name[client] = to_string(user_id);

            stringstream cur_time;
            time_t t = time(nullptr);
            cur_time << put_time(localtime(&t), "%Y-%m-%d %H:%M:%S");
            string str_time = cur_time.str();
            string welcome_msg = " *** Welcome to the simple CHAT server\n";
            string online_msg = " *** Total " + to_string(online) + " users online now. Your name is <" + user_name[client] + ">\n";
            string msg = str_time + welcome_msg + str_time + online_msg;
            send(client, msg.c_str(), msg.size() + 1, MSG_NOSIGNAL);            
            
            for(int i = 0;i < FD_SETSIZE;i++){
                if(i != client && i != listening){
                    // cout << "i is " << i << endl;                    
                    string new_guest = str_time + " *** User <" + user_name[client] + "> has just landed on the server\n";
                    send(i, new_guest.c_str(), new_guest.size() + 1, MSG_NOSIGNAL);
                    // cout << "finished sending " << endl;
                }
                // cout << i << endl;
            }
        }
        for(int i = 0;i < FD_SETSIZE;i++){        
            char buf[256];
            memset(buf, 0, 256);
            if((i != listening) && FD_ISSET(i, &copy)){
                // cout << "fd : " << i << endl;
                int nBytes = recv(i, buf, 128, 0);
                // cout << buf[0] << endl;
                if(nBytes <= 0){
                    close(i);
                    // cout << "close " << i << endl;
                    FD_CLR(i, &master);
                    for(int j = 0;j < FD_SETSIZE;j++){
                        if(j != i && j != listening){
                            stringstream cur_time;
                            time_t t = time(nullptr);
                            cur_time << put_time(localtime(&t), "%Y-%m-%d %H:%M:%S");
                            string str_time = cur_time.str();
                            string leave_msg = str_time + " *** User <" +  user_name[i] + "> has left the server\n";
                            send(j, leave_msg.c_str(), leave_msg.size() + 1, MSG_NOSIGNAL);
                        }
                    }
                    cout << "* client " << user_ip[i] << " disconnected" << endl;
                    online --;
                    user_name[i] = "";
                    user_ip[i] = "";
                }
                else{                    
                    if(buf[0] == '/'){                        
                        string cmd = string(buf,nBytes - 1);
                        // cout << cmd << endl;
                        if(cmd.substr(0, 6) == "/name "){
                            // cout << "Set names" << endl;
                            string old_name = user_name[i];
                            string new_name = cmd.substr(6);
                            user_name[i] = new_name;

                            stringstream cur_time;
                            time_t t = time(nullptr);
                            cur_time << put_time(localtime(&t), "%Y-%m-%d %H:%M:%S");
                            string str_time = cur_time.str();
                            string name_msg = str_time + " *** Nickname changed to <" + new_name + ">\n";
                            send(i, name_msg.c_str(), name_msg.size() + 1, MSG_NOSIGNAL);

                            for(int j = 0;j < FD_SETSIZE;j++){
                                if(j != i && j != listening){
                                    // warning 
                                    stringstream cur_time;
                                    time_t t = time(nullptr);
                                    cur_time << put_time(localtime(&t), "%Y-%m-%d %H:%M:%S");
                                    string str_time = cur_time.str();
                                    string name_broadcast = str_time + " *** User <" + old_name + "> renamed to <" + new_name + ">\n";
                                    send(j, name_broadcast.c_str(), name_broadcast.size() + 1, MSG_NOSIGNAL);
                                }
                            }

                        }
                        else if(cmd.substr(0, 4) == "/who"){
                            string title = "--------------------------------------------------\n";                            
                            string users = "";
                            for(int k = 0;k < FD_SETSIZE;k++){
                                if(!user_name[k].empty()){
                                    if(i == k){
                                        users += "* " + user_name[k] + "     " + user_ip[k] + "\n"; 
                                    }
                                    else{
                                        users += "  " + user_name[k] + "     " + user_ip[k] + "\n"; 
                                    }                                            
                                }
                            }
                            string list_msg = title + users + title;
                            send(i, list_msg.c_str(), list_msg.size() + 1, MSG_NOSIGNAL);                                    
                                
                            
                        }
                        else{
                            stringstream cur_time;
                            time_t t = time(nullptr);
                            cur_time << put_time(localtime(&t), "%Y-%m-%d %H:%M:%S");
                            string str_time = cur_time.str();
                            string unknown_msg = str_time + " *** Unknown or incomplete command <" + cmd + ">\n";    
                            send(i, unknown_msg.c_str(), unknown_msg.size() + 1, MSG_NOSIGNAL);
                        }
                        continue;
                    }
                    for(int j = 0;j < FD_SETSIZE;j++){
                        // cout << j << endl;
                        if(j != listening && j != i){
                            stringstream cur_time;
                            time_t t = time(nullptr);
                            cur_time << put_time(localtime(&t), "%Y-%m-%d %H:%M:%S");
                            string str_time = cur_time.str();
                            string chat_msg = str_time + " <" + user_name[i] + "> " + buf;
                            send(j, chat_msg.c_str(), chat_msg.size() + 1, MSG_NOSIGNAL);                            
                        }
                        
                    }
                    // cout << "OUT!!" << endl;
                }
            }
            
            
            
        }
        
    }
    cout << "Out of while loop!!" << endl;
    return 0;
}