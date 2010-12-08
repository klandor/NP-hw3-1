#include <iostream>
#include <cstdio>
#include <sstream>
#include <map>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <stdlib.h>
#include <strings.h>
#define SERVER_NUM 5
using namespace std;

int readline(int fd,char *ptr,int maxlen)
{
	//dup2(1,2);
	int n,rc;
	char c;
	*ptr = 0;
	for(n=1;n<maxlen;n++)
	{
		if((rc=read(fd,&c,1)) == 1)
		{
			*ptr++ = c;
			if(c=='\n')  break;
		}
		else if(rc==0)
		{
			if(n==1)     
				return(0);
			else {
				n--;
				break;
			}
		}
		else if(n==1)
			return(-1);
		else {
			n--;
			break;
		}

	}
	
//	if ( n>0 && *(--ptr) == '\n') {
//		n--;
//		if (n>0 && *(--ptr) == '\r') {
//			n--;
//		}
//	}
	return(n);
}   

void print_column(int i, string s){
	cout << "<script>document.all['m"<< i+1 <<"'].innerHTML += \"" << s<< "<BR>\";</script>"<<endl;
}

int main (int argc, char * const argv[]) {
	
	signal(SIGPIPE, SIG_IGN);
	cout << "Content-type: text/html\n\n";
	cout << "<html><head>\n<meta http-equiv=\"Content-Type\" content=\"text/html; charset=big5\" />\n"
		<< "<title>Network Programming Homework 3</title>\n</head>\n<body bgcolor=#336699>\n"
		<< "<font face=\"Courier New\" size=2 color=#FFFF99>\n";
	// generate table
	cout << "<table width=\"800\" border=\"1\">\n<tr>\n";
	for (int i=0; i<SERVER_NUM; i++) {
		cout << "<td valign=\"top\" id=\"title"<<i+1<<"\">Server "<< i+1 <<": </td>";
	}
	cout << "\n<tr>\n";
	for (int i=0; i<SERVER_NUM; i++) {
		cout << "<td valign=\"top\" id=\"m"<<i+1<<"\"></td>";
	}
	cout << "\n</table></font>"<< endl;
	
	
	string s1,s2;
	
	char write_buffer[SERVER_NUM][50000];
	int begin[SERVER_NUM], end[SERVER_NUM];
	map<string,string> args;
	int status[SERVER_NUM], socket_fd[SERVER_NUM];
	
	// get query
	string query;
	if (string(getenv("REQUEST_METHOD")) == "GET") {
		query = getenv("QUERY_STRING");
	}
	else {
		getline(cin, query);
	}

	
	// parse query
	istringstream iss(query);
	for(int i=0; i<SERVER_NUM*3-1; i++){
		getline(iss, s1, '=');
		getline(iss, s2, '&');
		args[s1] = s2;
	}
	
	getline(iss, s1, '=');
	if(getline(iss, s2))
		args[s1] = s2;
	else
		args[s1] = "";
	
	

	fd_set connecting_fds; 
	int max_fd = 0;
	FD_ZERO(&connecting_fds);
	
	// preparation
	for (int i=0; i<SERVER_NUM; i++) {
		if ( args[string("IP")+(char)(i+'1')] == "" || args[string("PORT")+(char)(i+'1')] == "") // empty address
			status[i] = -1;
		else {  // create socket and connect
			
			
			cout << "<script>document.all['title"<< i+1 <<"'].innerHTML += \"" << args[string("IP")+(char)(i+'1')]
				<< ":" << args[string("PORT")+(char)(i+'1')]<< "\";</script>" << endl;
			
			// read the file into the buffer
			begin[i] = end[i] = 0;
			if(args[string("FILE")+(char)(i+'1')] != ""){
				int fd = open(args[string("FILE")+(char)(i+'1')].c_str(), O_RDONLY);
				if (fd <0) {
					print_column(i, "File \"" + args[string("FILE")+(char)(i+'1')] + "\" can not be opened.");
				}
				else {
					end[i] = read(fd, write_buffer[i], 50000);
					if (end[i] < 0) {
						print_column(i, "File \"" + args[string("FILE")+(char)(i+'1')] + "\" can not be read.");
						perror("read");
						end[i] = 0;
					}
					close(fd);
				}

				
			}
			

			
			struct hostent *he = 
				gethostbyname(args[string("IP")+(char)(i+'1')].c_str());
			int SERVER_PORT = atoi(args[string("PORT")+(char)(i+'1')].c_str());
			
			if (he == NULL || SERVER_PORT == 0) {
				status[i] = -1;
				print_column(i, "Error: Invalid Address");
				continue;
			}
			
			
			socket_fd[i] = socket(AF_INET,SOCK_STREAM,0);
			FD_SET(socket_fd[i], &connecting_fds);
			if (socket_fd[i]+1 > max_fd) {
				max_fd = socket_fd[i]+1;
			}
			
			//set non-blocking
			int flags = fcntl(socket_fd[i], F_GETFL, 0);
			fcntl(socket_fd[i], F_SETFL, flags|O_NONBLOCK);
			
			// connect
			
			struct sockaddr_in client_sin;
			bzero(&client_sin, sizeof(client_sin));
			client_sin.sin_family = AF_INET;
			client_sin.sin_addr = *((struct in_addr *)he->h_addr); 
			client_sin.sin_port = htons(SERVER_PORT);
			if(connect(socket_fd[i],(struct sockaddr *)&client_sin,
				sizeof(client_sin)) == -1)
			{
				//perror("connect");
				switch (errno) {
					case EALREADY:
						//cout << "Conecting..."<<endl;
						break;
					case ECONNREFUSED:
						perror("connect");
						status[i] = -1;
						break;
					case EINPROGRESS:
						//cout << "EINPROGRESS" << endl;
						break;

					default:
						perror("connect");
						break;
				}
				
				status[i]=0;
			}
			else {
				status[i] = 1;
			}
			
			
		}
		
		
		
			
	}
	
	
	bool conti = true;
		
	while (conti) {
		conti = false;
		fd_set wfds, rfds;
		FD_ZERO(&wfds);
		FD_ZERO(&rfds);
		
		// setting fd_set's
		for (int i=0; i<SERVER_NUM; i++) {
			switch (status[i]) {
				case 0:  // connecting
					FD_SET(socket_fd[i], &wfds);
					conti = true;
					break;

				case 1:
					FD_SET(socket_fd[i], &rfds);

					conti = true;
					break;
				case 2:
					FD_SET(socket_fd[i], &wfds);
					conti = true;
					break;


				default:
					break;
			}
		}
		if(conti)
			select(max_fd, &rfds, &wfds, NULL, NULL);
		else {
			break;
		}

		
		for (int i=0; i<SERVER_NUM; i++) {
			switch (status[i]) {
				case 0:  // connecting
				{
					if (FD_ISSET(socket_fd[i], &wfds)) {
						status[i]=1;
					}
					
				}
					
					break;
					
				case 1:
				{
					if (FD_ISSET(socket_fd[i], &rfds)){
						char buff[5000];
						int r = readline(socket_fd[i], buff, 4999);
						if (r<0) {
//							switch (r) {
//								case ETIMEDOUT:
//									print_column(i, "Error: Connection timed out");
//									break;
//								case ECONNREFUSED:
//									print_column(i, "Error: Connection refused");
//									break;
//
//								default:
//									perror("read");
//									cout << "errno="<<errno<<endl;
//									break;
//							}
							
							print_column(i, "Connection Error!!");
							close(socket_fd[i]);
							status[i] = -1;
							continue;
						}
						else if (r == 0){
							close(socket_fd[i]);
							status[i] = -1;
						}
						else {
							if(buff[r-1] == '\n')
							{
								r--;
								if(buff[r-1] == '\r')
									r--;

							}
							buff[r] = 0;
							
							
							if (string(buff).substr(0,2) == "% ") {
								string t = string(buff).substr(3);
								if ( t != "") {
									print_column(i,t);

								}
								
								status[i] = 2;
							}
							else{
								print_column(i,buff);
							}
						}
					}

//					if (FD_ISSET(socket_fd[i], &wfds)){
//						int w = write(socket_fd[i], write_buffer[i]+begin[i], end[i]-begin[i]);
//						if (w < 0) {
//							perror("write");
//							if (errno == EPIPE) {
//								close(socket_fd[i]);
//								status[i] = -1;
//							}
//						}
//						else {
//							begin[i] += w;
//						}
//
//					}
				}
					break;
					
				case 2: // write command
					if (FD_ISSET(socket_fd[i], &wfds)){
						int command_length=0;
						for (int j = begin[i]; j<end[i]; j++) {
							command_length++;
							if (write_buffer[i][j] == '\n') {
								break;
							}
						}
						
						int w = write(socket_fd[i], write_buffer[i]+begin[i], command_length);
						if (w < 0) {
							perror("write");
							if (errno == EPIPE) {
								close(socket_fd[i]);
								status[i] = -1;
							}
						}
						else {
							
							if (write_buffer[i][begin[i]+command_length-2] == '\r') {
								write_buffer[i][begin[i]+command_length-2] = 0;
							}
							else {
								write_buffer[i][begin[i]+command_length-1] = 0;
							}

							print_column(i,"% <b>" + string(write_buffer[i]+begin[i]) + "</b>");
							
							
							begin[i] += w;
						}
						status[i] = 1;
					}
					break;
					
				default:
					break;
			}
		}
	}
	
	cout << "</body></html>";
	return 0;
}
