#include <bits/stdc++.h>
using namespace std;

void sighandler(int);

int main()
{
   signal(SIGINT, sighandler);

   while(1) 
   {
      printf("Going to sleep for a second...\n");
      sleep(1);
   }

   return(0);
}

void sighandler(int signum)
{
   printf("Caught signal %d, coming out...\n", signum);
   exit(1);
}