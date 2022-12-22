#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>
using namespace std;

#define err_quit(m) { perror(m); exit(-1); }

int main(int argc, char *argv[]) {
    if(argc < 5) {
		return -fprintf(stderr, "usage: %s <path-to-read-files> <total-number-of-files> <port> <server-ip-address>\n", argv[0]);
	}
    int file_num = stoi(argv[argc-3]);
    char file_path[128];
    strcpy(file_path, argv[argc-4]);
    // cout << file_size << endl;
    // cout << file_path << endl;
	int s;
	struct sockaddr_in sin;

    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_port = htons(atoi(argv[argc-2]));
    inet_pton(AF_INET, argv[argc-1], &sin.sin_addr);

    if((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
		err_quit("socket");
    
    int file_count = 0;
    FILE *fp;
    for(int i = 0;i < file_num;i++){        
        char file[256];
        char buf[32768]; 
        sprintf(file,"%s/%06d", file_path, i);

        // cout << file << endl;
        fp = fopen(file, "r");
        int bytes = fread(buf, sizeof(char), 32768, fp);
        // cout << buf << endl;
        fclose(fp);
        // cout << "Read " << bytes << " bytes" << endl;
        // cout << buf << endl;
        // struct timeval tv;
        // tv.tv_sec = 1;
        // tv.tv_usec = 0;
        // setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
		sendto(s, buf, bytes, 0, (sockaddr* )&sin, sizeof(sin));
        char msg[1024];
        recvfrom(s, msg, sizeof(buf), 0, NULL, NULL);
        cout << msg;
        // sleep(0.5);
    }	
	
    close(s);

}