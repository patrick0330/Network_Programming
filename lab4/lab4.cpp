#include <iostream>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <time.h>
#include <sys/wait.h>

using namespace std;

static void sig_handler(int sig){
    int retval;
    
    if ( sig == SIGCHLD ){ 
        wait(&retval);

    }
}

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
    sockaddr_in client;
    socklen_t client_size = sizeof(client);
    char host[NI_MAXHOST];
    char svc[NI_MAXSERV];
    while(true){

        int client_socket = accept(listening, (sockaddr *)&client, &client_size);
        if(client_socket == -1){
            cout << "Client connecting problem";
            return -4;
        }   
        // memset(host, 0, NI_MAXHOST);
        // memset(svc, 0, NI_MAXSERV);  
        int result = getnameinfo((sockaddr *)&client, sizeof(client), host, NI_MAXHOST, svc, NI_MAXSERV, 0);

        if(result){
            cout << host << " Connect on the " << svc << endl;
        }
        else{
            inet_ntop(AF_INET, &client.sin_addr, host, NI_MAXHOST);
            cout << host << " Connect on " << ntohs(client.sin_port) << endl;
        }

        // int exit_status;
        char* total_command[argc - 1];
                for(int i = 2; i < argc;i++){
                    string s(argv[i]);
                    // cout << s << endl;
                    total_command[i - 2] = strdup(s.c_str());
                }

                // cout << total_command[0] << endl;
                total_command[argc - 2] = NULL;
        int save = dup(STDOUT_FILENO);
        signal(SIGCHLD,sig_handler);
        pid_t pid = fork();

        switch(pid){
            case -1:
                perror("fork()");
                exit(-1);
            case 0: //child
                    
                // cout << save << endl;
                
                dup2(client_socket, STDIN_FILENO);
                dup2(client_socket, STDOUT_FILENO);
                dup2(client_socket, STDERR_FILENO);
                if(execvp(total_command[0], total_command) == -1){
                    dup2(save, STDOUT_FILENO);            
                    cout << "Error Ocurred!" << endl;
                    close(save);
                    // dup2(client_socket, STDOUT_FILENO);
                }
                
                close(listening); 
            default:// parent
                close(client_socket);
                // wait(&exit_status);
                continue;
        }
    }
    return 0;
}