#include <bits/stdc++.h>

using namespace std;
char* gen_secret(){
    static int len;
	static char buf[64];
	
	int num1 = rand();
	cout << "num1 is " << num1 << endl;
	len  = snprintf(buf,     10, "%x", num1);
	num1 = rand();
	cout << "num1 is " << num1 << endl;
	len += snprintf(buf+len, 10, "%x", num1);
	num1 = rand();
	cout << "num1 is " << num1 << endl;
	len += snprintf(buf+len, 10, "%x", num1);
	num1 = rand();
	cout << "num1 is " << num1 << endl;
	len += snprintf(buf+len, 10, "%x", num1);
	buf[len] = '\0';
	return buf;

}
int main(){
    int seed;
    cin >> hex >> seed;    
    int check;
    int key;
    char *secret;
    key = rand();
    srand(seed);    
    secret = gen_secret();
    cout << "secret is " << secret << endl; 
    key = rand();
    secret = gen_secret();
    cout << "secret is " << secret << endl; 
    
 
    
}