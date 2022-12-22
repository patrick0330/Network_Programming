#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <iostream>
using namespace std;

#define err_quit(m) { perror(m); exit(-1); }

int main(int argc, char *argv[]) {
    if(argc < 4) {
		return -fprintf(stderr, "usage: %s <path-to-store-files> <total-number-of-files> <port>\n", argv[0]);
	}
	int s;
	struct sockaddr_in sin;
    int file_num = stoi(argv[argc-2]);
    char file_path[128];
    strcpy(file_path, argv[argc-3]);

    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_port = htons(atoi(argv[argc-1]));

    if((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
		err_quit("socket");

    if(bind(s, (struct sockaddr*) &sin, sizeof(sin)) < 0)
		err_quit("bind");

    FILE *fp;
    for(int i = 0;i < file_num;i++){
		struct sockaddr_in csin;
		socklen_t csinlen = sizeof(csin);
		char buf[32768];
        char file[256];
        sprintf(file,"%s/%06d", file_path, i);
        int n = 220 * 1024;
        setsockopt(s, SOL_SOCKET, SO_RCVBUF, &n, sizeof(n));
		int rlen;
		rlen = recvfrom(s, buf, sizeof(buf), 0, (struct sockaddr*) &csin, &csinlen);		
        // cout << "receive " << rlen << " bytes" << endl;
        // cout << "data is " << buf << endl;
        fp = fopen(file, "w");
        fwrite(buf, sizeof(char), rlen, fp);
		fclose(fp);
        string msg = "got packet from " + to_string(i) + " \n";
        sendto(s, msg.c_str(), sizeof(msg), 0, (struct sockaddr*) &csin, csinlen);
	}
    close(s);

}