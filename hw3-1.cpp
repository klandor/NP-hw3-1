#include <iostream>
#include <cstdio>
using namespace std;
int main (int argc, char * const argv[]) {
	cout << "Content-type: text/html\n\n";
	string s1,s2;
	while( getline(cin, s1, '=') ){
		getline(cin, s2, '&');
		cout << s1 << '=' << s2 << endl;
	}
	return 0;
}
