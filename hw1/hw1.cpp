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
#include <algorithm>



using namespace std;

vector<string> nick_name(FD_SETSIZE, "");
vector<string> user_ip(FD_SETSIZE, "");
vector<string> real_name(FD_SETSIZE, "");

vector<string> channel_list(FD_SETSIZE, "");
vector<vector<string>> channel_user(FD_SETSIZE);
vector<string> channel_topic(FD_SETSIZE, "");
int channel_counter[FD_SETSIZE] = {0};
bool nick_check[FD_SETSIZE] = {0};
bool user_check[FD_SETSIZE] = {0};
bool registered[FD_SETSIZE] = {0};
string server_name = "mircd";
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
        // cout << "Current online: " << online << endl;
        select(FD_SETSIZE, &copy, nullptr, nullptr, nullptr);
        if(FD_ISSET(listening, &copy)){
            // cout << listening << endl;
            int client = accept(listening, (sockaddr *)&client_info, &client_size);
            // cout << "client is " << client << endl;
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
            
        }
        for(int i = 0;i < FD_SETSIZE;i++){        
            char buf[256];
            memset(buf, 0, 256);            
            if((i != listening) && FD_ISSET(i, &copy)){
                // cout << "fd : " << i << endl;
                int nBytes = recv(i, buf, 128, 0);
                cout << buf << endl;
                if(nBytes <= 0){
                    close(i);
                    // cout << "close " << i << endl;
                    FD_CLR(i, &master);
                    // for(int j = 0;j < FD_SETSIZE;j++){
                    //     if(j != i && j != listening){
                    //         stringstream cur_time;
                    //         time_t t = time(nullptr);
                    //         cur_time << put_time(localtime(&t), "%Y-%m-%d %H:%M:%S");
                    //         string str_time = cur_time.str();
                    //         string leave_msg = str_time + " *** User <" +  nick_name[i] + "> has left the server\n";
                    //         send(j, leave_msg.c_str(), leave_msg.size() + 1, MSG_NOSIGNAL);
                    //     }
                    // }
                    // cout << "* client " << user_ip[i] << " disconnected" << endl;
                    online --;
                    nick_name[i] = "";
                    user_ip[i] = "";
                    real_name[i] = "";
                }
                else{                                          
                    string cmds = string(buf,nBytes - 1);
                    string delimiter = "\r\n";
                    int pos = 0;   

                    vector<string> cmd_set;
                    string token;
                    while((pos = cmds.find(delimiter)) != string::npos){
                        token = cmds.substr(0, pos);
                        // cout << "token is " << token << endl;
                        cmd_set.push_back(token);
                        cmds.erase(0, pos + delimiter.length());
                    }
                    // cout << "cmds is "<< cmds << endl;
                    // cout << "len is " << cmds.length() << endl;

                    // check \r for last character
                    if(((int)cmds[cmds.length() - 1]) == 13){
                        cmds = cmds.substr(0, cmds.length() - 1);
                    }                    
                    cmd_set.push_back(cmds);

                    delimiter = " ";
                    for(int j = 0;j < cmd_set.size();j++){
                        vector<string> cmd;
                        pos = 0;
                        while((pos = cmd_set[j].find(delimiter)) != string::npos){
                            token = cmd_set[j].substr(0, pos);
                            cmd.push_back(token);
                            cmd_set[j].erase(0, pos + delimiter.length());
                        }
                        cmd.push_back(cmd_set[j]);
                        // cout << "cmd_set is "<< cmd_set[j] << endl;
                        // cout << "len is " << cmd_set[j].length() << endl;
                        if(cmd[0] == "NICK"){
                            string err_msg;
                            // 431 error
                            if(cmd.size() == 1){
                                err_msg = ":" + server_name + " 431 " + ":No nickname given\r\n";
                                send(i, err_msg.c_str(), err_msg.size(), MSG_NOSIGNAL);
                                continue;
                            }

                            string old_name = nick_name[i];
                            string new_name = cmd[1];
                            cout << "NICK NAME LIST" << endl;
                            for(int k = 0;k < FD_SETSIZE;k++){
                                if(!nick_name[k].empty()){
                                    cout << nick_name[k] << endl;
                                }
                            }
                            // 436 error
                            if(find(nick_name.begin(), nick_name.end(), new_name) != nick_name.end()){
                                err_msg = ":" + server_name + " 436 " + nick_name[i] +  " :Nickname collision KILL\r\n";
                                send(i, err_msg.c_str(), err_msg.size(), MSG_NOSIGNAL);
                                continue;
                            }

                            // change nick name
                            if(!old_name.empty()){                                
                                string namechange_msg = ":" + old_name + " NICK " + new_name + "\r\n";
                                for(int k = 0;k < FD_SETSIZE;k++){
                                    send(k, namechange_msg.c_str(), namechange_msg.size(), MSG_NOSIGNAL);
                                }                                
                                nick_name[i] = new_name;
                                continue;;
                            }                         
                            nick_name[i] = new_name;
                            stringstream cur_time;
                            time_t t = time(nullptr);
                            cur_time << put_time(localtime(&t), "%Y-%m-%d %H:%M:%S");
                            string str_time = cur_time.str();
                            string name_msg = str_time + " *** Nickname changed to <" + new_name + ">\n";
                            cout << name_msg;
                            nick_check[i] = true;
                            // send(i, name_msg.c_str(), name_msg.size() + 1, MSG_NOSIGNAL);
                            if(nick_check[i] && user_check[i]){
                                string welcome_msg = ":" + server_name + " 001 " + nick_name[i] + " :Welcome to the minized IRC deamon!\r\n";
                                welcome_msg += ":" + server_name + " 251 " + nick_name[i] + " :There are " + to_string(online) + " users and 0 invisible on 1 server\r\n";
                                welcome_msg += ":" + server_name + " 375 " + nick_name[i] + " :- " + server_name +  " Message of the day -\r\n";
                                welcome_msg += ":" + server_name + " 372 " + nick_name[i] + " :- " + "Hello, World!\r\n";
                                welcome_msg += ":" + server_name + " 372 " + nick_name[i] + " :- " + "              @                    _ \r\n";
                                welcome_msg += ":" + server_name + " 372 " + nick_name[i] + " :- " + "  ____  ___   _   _ _   ____.     | |\r\n";
                                welcome_msg += ":" + server_name + " 372 " + nick_name[i] + " :- " + " /  _ `'_  \\ | | | '_/ /  __|  ___| |\r\n";
                                welcome_msg += ":" + server_name + " 372 " + nick_name[i] + " :- " + " | | | | | | | | | |   | |    /  _  |\r\n";
                                welcome_msg += ":" + server_name + " 372 " + nick_name[i] + " :- " + " | | | | | | | | | |   | |__  | |_| |\r\n";
                                welcome_msg += ":" + server_name + " 372 " + nick_name[i] + " :- " + " |_| |_| |_| |_| |_|   \\____| \\___,_|\r\n";
                                welcome_msg += ":" + server_name + " 372 " + nick_name[i] + " :- " + " minimized internet relay chat daemon\r\n";
                                welcome_msg += ":" + server_name + " 372 " + nick_name[i] + " :-\r\n";
                                welcome_msg += ":" + server_name + " 376 " + nick_name[i] + " :End of message of the day\r\n";
                                send(i, welcome_msg.c_str(), welcome_msg.size(), MSG_NOSIGNAL);
                                nick_check[i] = false;
                                user_check[i] = false;
                                registered[i] = true;
                            }

                        }
                        else if(cmd[0] == "USER"){
                            string err_msg;
                            // 461 error
                            if(cmd.size() == 1){
                                err_msg = ":" + server_name + " 461 " + nick_name[i] + " USER :Not enought parameters\r\n";
                                send(i, err_msg.c_str(), err_msg.size(), MSG_NOSIGNAL);
                                continue;
                            }
                            string name = "";
                            for(int k = 4;k < cmd.size();k++){
                                name += cmd[k] + " ";
                            }
                            if(name[name.length() - 1] == ' '){
                                name = name.substr(0, name.length() - 1);
                            }
                            real_name[i] = name;
                            user_check[i] = true;
                            if(nick_check[i] && user_check[i]){
                                string welcome_msg = ":" + server_name + " 001 " + nick_name[i] + " :Welcome to the minized IRC deamon!\r\n";
                                welcome_msg += ":" + server_name + " 251 " + nick_name[i] + " :There are " + to_string(online) + " users and 0 invisible on 1 server\r\n";
                                welcome_msg += ":" + server_name + " 375 " + nick_name[i] + " :- " + server_name +  " Message of the day -\r\n";
                                welcome_msg += ":" + server_name + " 372 " + nick_name[i] + " :- " + "Hello, World!\r\n";
                                welcome_msg += ":" + server_name + " 372 " + nick_name[i] + " :- " + "              @                    _ \r\n";
                                welcome_msg += ":" + server_name + " 372 " + nick_name[i] + " :- " + "  ____  ___   _   _ _   ____.     | |\r\n";
                                welcome_msg += ":" + server_name + " 372 " + nick_name[i] + " :- " + " /  _ `'_  \\ | | | '_/ /  __|  ___| |\r\n";
                                welcome_msg += ":" + server_name + " 372 " + nick_name[i] + " :- " + " | | | | | | | | | |   | |    /  _  |\r\n";
                                welcome_msg += ":" + server_name + " 372 " + nick_name[i] + " :- " + " | | | | | | | | | |   | |__  | |_| |\r\n";
                                welcome_msg += ":" + server_name + " 372 " + nick_name[i] + " :- " + " |_| |_| |_| |_| |_|   \\____| \\___,_|\r\n";
                                welcome_msg += ":" + server_name + " 372 " + nick_name[i] + " :- " + " minimized internet relay chat daemon\r\n";
                                welcome_msg += ":" + server_name + " 372 " + nick_name[i] + " :-\r\n";
                                welcome_msg += ":" + server_name + " 376 " + nick_name[i] + " :End of message of the day\r\n";
                                send(i, welcome_msg.c_str(), welcome_msg.size(), MSG_NOSIGNAL);
                                nick_check[i] = false;
                                user_check[i] = false;
                                registered[i] = true;
                            }
                        }
                        else if(cmd[0] == "USERS"){
                            string user_msg = ":" + server_name + " 392 " + nick_name[i] + " :UserID                           Terminal  Host\r\n";
                            for(int k = 0;k < FD_SETSIZE;k++){
                                if(!nick_name[k].empty()){
                                    user_msg += ":" + server_name + " 393 " + nick_name[i] + " :" + nick_name[k] + "                           -         " + user_ip[k] + "\r\n";
                                }
                            }
                            send(i, user_msg.c_str(), user_msg.size(), MSG_NOSIGNAL);                           
                        }
                        else if(cmd[0] == "JOIN"){
                            string err_msg;
                            // 461 error
                            if(cmd.size() == 1){
                                err_msg = ":" + server_name + " 461 " + nick_name[i] + " JOIN :Not enought parameters\r\n";
                                send(i, err_msg.c_str(), err_msg.size(), MSG_NOSIGNAL);
                                continue;
                            }
                            // channel list
                            // channel user
                            // channel counter
                            // channel topic
                            string channel_name = "";
                            if(cmd[1][0] == '#'){
                                channel_name = cmd[1];
                            }
                            else{
                                channel_name = "#" + cmd[1];
                            }
                            string join_msg;
                            vector<string>::iterator it;
                            it = find(channel_list.begin(), channel_list.end(), channel_name);
                            // if channel exist 
                            if(it != channel_list.end()){
                                vector<string>::iterator user_checker;
                                int channel_pos = it - channel_list.begin();                                
                                user_checker = find(channel_user[channel_pos].begin(), channel_user[channel_pos].end(), nick_name[i]);
                                // if user is in channel
                                if(user_checker != channel_user[channel_pos].end()){
                                    cout << nick_name[i] << " found in " << channel_name << "'s user list" << endl;
                                }
                                else{
                                    channel_user[channel_pos].push_back(nick_name[i]);
                                    channel_counter[channel_pos] ++;
                                }
                                for(int k = 0;k < FD_SETSIZE;k++){                                    
                                    if(find(channel_user[channel_pos].begin(), channel_user[channel_pos].end(), nick_name[k]) != channel_user[channel_pos].end()){
                                        join_msg = ":" + nick_name[i] + " JOIN " + channel_name + "\r\n";
                                        if(k == i){
                                            // get channel users
                                            string user = channel_user[channel_pos].front();
                                            for(int m = 1;m < channel_user[channel_pos].size();m++){
                                                user += " " + channel_user[channel_pos][m];
                                            }
                                            // if channel topic is empty
                                            if(channel_topic[channel_pos].empty()){                                    
                                                join_msg += ":" + server_name + " 331 " + nick_name[i] + " " + channel_name + " :No topic is set\r\n";
                                                join_msg += ":" + server_name + " 353 " + nick_name[i] + " " + channel_name + " :" + user + "\r\n";
                                                join_msg += ":" + server_name + " 366 " + nick_name[i] + " " + channel_name + " :End of Names List\r\n";
                                            }
                                            else{
                                                join_msg += ":" + server_name + " 331 " + nick_name[i] + " " + channel_name + " :" + channel_topic[channel_pos] + "\r\n";
                                                join_msg += ":" + server_name + " 353 " + nick_name[i] + " " + channel_name + " :" + user + "\r\n";
                                                join_msg += ":" + server_name + " 366 " + nick_name[i] + " " + channel_name + " :End of Names List\r\n";
                                            }
                                        }
                                        send(k, join_msg.c_str(), join_msg.size(), MSG_NOSIGNAL);
                                    }                                    
                                }                            
                            }
                            else{
                                for(int k = 0;k < FD_SETSIZE;k++){
                                    if(channel_list[k].empty()){
                                        channel_list[k] = channel_name;
                                        channel_user[k].push_back(nick_name[i]);
                                        channel_counter[k] ++;
                                        join_msg = ":" + nick_name[i] + " JOIN " + channel_name + "\r\n";
                                        join_msg += ":" + server_name + " 331 " + nick_name[i] + " " + channel_name + " :No topic is set\r\n";
                                        join_msg += ":" + server_name + " 353 " + nick_name[i] + " " + channel_name + " :" + nick_name[i] + "\r\n";
                                        join_msg += ":" + server_name + " 366 " + nick_name[i] + " " + channel_name + " :End of Names List\r\n";                                        
                                        break;
                                    }
                                }
                                send(i, join_msg.c_str(), join_msg.size(), MSG_NOSIGNAL);
                            }
                            
                        }
                        else if(cmd[0] == "NAMES"){
                            string channel_name = cmd[1];
                            vector<string>::iterator it;
                            it = find(channel_list.begin(), channel_list.end(), channel_name);
                            string name_msg;
                            if(it != channel_list.end()){
                                int channel_pos = it - channel_list.begin();
                                string user = channel_user[channel_pos].front();
                                for(int k = 1;k < channel_user[channel_pos].size();k++){
                                    user += " " + channel_user[channel_pos][k];
                                }
                                name_msg = ":" + server_name + " 353 " + nick_name[i] + " " + channel_name + " :" + user + "\r\n";
                                name_msg += ":" + server_name + " 366 " + nick_name[i] + " " + channel_name + " :End of Names List\r\n";
                            }
                            else{
                                cout << "No Such Channel" << endl;
                            }                   
                            send(i, name_msg.c_str(), name_msg.size(), MSG_NOSIGNAL);
                            
                        }
                        else if(cmd[0] == "PART"){
                            string err_msg;
                            // 461 error
                            if(cmd.size() == 1){
                                err_msg = ":" + server_name + " 461 " + nick_name[i] + " PART :Not enought parameters\r\n";
                                send(i, err_msg.c_str(), err_msg.size(), MSG_NOSIGNAL);
                                continue;
                            }
                            string channel_name = cmd[1];
                            vector<string>::iterator it;
                            it = find(channel_list.begin(), channel_list.end(), channel_name);
                            string part_msg;
                            if(it != channel_list.end()){
                                int channel_pos = it - channel_list.begin();                                
                                vector<string>::iterator user_checker;
                                user_checker = find(channel_user[channel_pos].begin(), channel_user[channel_pos].end(), nick_name[i]);
                                if(user_checker != channel_user[channel_pos].end()){
                                    part_msg = ":" + nick_name[i] + " PART " + ":" + channel_name + "\r\n";
                                    for(int k = 0;k < FD_SETSIZE;k++){
                                        if(find(channel_user[channel_pos].begin(), channel_user[channel_pos].end(), nick_name[k]) != channel_user[channel_pos].end()){
                                            send(k, part_msg.c_str(), part_msg.size(), MSG_NOSIGNAL);
                                        }                                    
                                    }
                                    channel_user[channel_pos].erase(user_checker);
                                    channel_counter[channel_pos] --;            
                                    
                                }
                                else{
                                    // 442 error
                                    err_msg = ":" + server_name + " 442 " + nick_name[i] + " " + channel_name + " :You are not on that channel\r\n";
                                    send(i, err_msg.c_str(), err_msg.size(), MSG_NOSIGNAL);
                                    continue;                               
                                }                                
                            }
                            else{
                                // 403 error
                                err_msg = ":" + server_name + " 403 " + nick_name[i] + " " + channel_name + " :No such channel\r\n";
                                send(i, err_msg.c_str(), err_msg.size(), MSG_NOSIGNAL);
                                continue;
                            }
                            // send(i, part_msg.c_str(), part_msg.size(), MSG_NOSIGNAL);
                        }
                        else if(cmd[0] == "PRIVMSG"){
                            string err_msg;
                            // 411 error
                            if(cmd.size() == 1){
                                err_msg = ":" + server_name + " 411 " + nick_name[i] + " :No recipient given (PRIVMSG)\r\n";
                                send(i, err_msg.c_str(), err_msg.size(), MSG_NOSIGNAL);
                                continue;
                            }
                            else if(cmd.size() == 2){ // 412 error
                                err_msg = ":" + server_name + " 412 " + nick_name[i] + " :No text to send\r\n";
                                send(i, err_msg.c_str(), err_msg.size(), MSG_NOSIGNAL);
                                continue;
                            }
                            string channel_name = cmd[1];
                            string msg = "";
                            for(int k = 2;k < cmd.size();k++){
                                msg += cmd[k] + " ";
                            }
                            if(msg[msg.length() - 1] == ' '){
                                msg = msg.substr(0, msg.length() - 1);
                            }
                            vector<string>::iterator it;
                            it = find(channel_list.begin(), channel_list.end(), channel_name);
                            string priv_msg;
                            if(it != channel_list.end()){
                                int channel_pos = it - channel_list.begin();                                
                                priv_msg = ":" + nick_name[i] + " PRIVMSG " + channel_name + " " + msg + "\r\n";
                                for(int k = 0;k < FD_SETSIZE;k++){
                                    if(k == i){
                                        continue;;
                                    }
                                    if(find(channel_user[channel_pos].begin(), channel_user[channel_pos].end(), nick_name[k]) != channel_user[channel_pos].end()){
                                        send(k, priv_msg.c_str(), priv_msg.size(), MSG_NOSIGNAL);
                                    }                                    
                                }
                                                              
                            }
                            else{
                                // 401 error
                                err_msg = ":" + server_name + " 401 " + nick_name[i] + " " + channel_name + " :No such nick/channel\r\n";
                                send(i, err_msg.c_str(), err_msg.size(), MSG_NOSIGNAL);
                                continue;
                            }
                        }
                        else if(cmd[0] == "LIST"){
                            string list_msg = ":" + server_name + " 321 " + nick_name[i] + " Channel :Users Name\r\n";
                            for(int k = 0;k < FD_SETSIZE;k++){
                                if(channel_list[k].empty()){
                                    break;
                                }
                                list_msg += ":" + server_name + " 322 " + nick_name[i] + " " + channel_list[k] + " " + to_string(channel_counter[k]) + " " + channel_topic[k] + "\r\n";
                            }
                            list_msg += ":" + server_name + " 323 " + nick_name[i] + " :End of List\r\n";
                            send(i, list_msg.c_str(), list_msg.size(), MSG_NOSIGNAL);
                        }
                        else if(cmd[0] == "TOPIC"){
                            string err_msg;
                            // 461 error
                            if(cmd.size() == 1){
                                err_msg = ":" + server_name + " 461 " + nick_name[i] + " TOPIC :Not enought parameters\r\n";
                                send(i, err_msg.c_str(), err_msg.size(), MSG_NOSIGNAL);
                                continue;
                            }

                            string channel_name = cmd[1];
                            vector<string>::iterator it;
                            it = find(channel_list.begin(), channel_list.end(), channel_name);
                            if(it != channel_list.end()){
                                int channel_pos = it - channel_list.begin();

                                // 442 error
                                if(find(channel_user[channel_pos].begin(), channel_user[channel_pos].end(), nick_name[i]) == channel_user[channel_pos].end()){
                                    err_msg = ":" + server_name + " 442 " + nick_name[i] + " " + channel_name + " :You are not on that channel\r\n";
                                    send(i, err_msg.c_str(), err_msg.size(), MSG_NOSIGNAL);
                                    continue;
                                }

                                string topic_msg;
                                // change to new topic
                                if(cmd.size() > 2){
                                    string new_topic = "";                                
                                    for(int k = 2;k < cmd.size();k++){
                                        new_topic += cmd[k] + " ";
                                    }
                                    if(new_topic[new_topic.length() - 1] == ' '){
                                        new_topic = new_topic.substr(0, new_topic.length() - 1);
                                    }
                                    topic_msg = ":" + server_name + " 332 " + nick_name[i] + " " +  channel_name + " " + new_topic + "\r\n";  
                                    send(i, topic_msg.c_str(), topic_msg.size(), MSG_NOSIGNAL);
                                    channel_topic[channel_pos] = new_topic;               
                                }
                                else{ // show topic
                                    if(channel_topic[channel_pos].empty()){
                                        topic_msg = ":" + server_name + " 331 " + nick_name[i] + " " +  channel_name + " :No topic is set\r\n";
                                    }
                                    else{
                                        topic_msg = ":" + server_name + " 332 " + nick_name[i] + " " +  channel_name + " " + channel_topic[channel_pos] + "\r\n"; 
                                    }
                                    send(i, topic_msg.c_str(), topic_msg.size(), MSG_NOSIGNAL);
                                }
                            }
                            else{
                                cout << "Channel not found" << endl;
                            }
                            
                        }
                        else if(cmd[0] == "PING"){
                            string ping_msg = "PONG " + cmd[1] + "\r\n"; 
                            send(i, ping_msg.c_str(), ping_msg.size(), MSG_NOSIGNAL);
                        }
                        else if(cmd[0] == "QUIT"){
                            close(i);
                            FD_CLR(i, &master);
                            online --;
                            nick_name[i] = "";
                            user_ip[i] = "";
                            real_name[i] = "";
                        }
                        else{
                            string unknown_msg = ":" + server_name + " 421 " + nick_name[i] + " " + cmd[0] + " :Unknown command\r\n";    
                            send(i, unknown_msg.c_str(), unknown_msg.size(), MSG_NOSIGNAL);
                        }

                    }                

                    
                    
                }
            }
            
            
            
        }
        
    }
    cout << "Out of while loop!!" << endl;
    return 0;
}