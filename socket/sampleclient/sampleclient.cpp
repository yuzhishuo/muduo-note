#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <string>
#include <netinet/in.h>
#include<unistd.h>
#include<iostream>
#define PORT 8000
int main(int argc, char const *argv[])
{
	
	int sfd;
	sfd = socket(AF_INET, SOCK_STREAM, 0);
	
	sockaddr_in sa;
	bzero(static_cast<void*>(&sa), sizeof sa);
	sa.sin_family = AF_INET;
	sa.sin_port = htons(PORT);
	sa.sin_addr.s_addr = htonl(INADDR_ANY);

	bind(sfd, (sockaddr*)(&sa), sizeof sa);

	listen(sfd, 5);

	sockaddr_in sa_r;
	int sfd_r;
	unsigned int si_r = sizeof sa_r;
	char buf_r[100];
	
	while(1)
	{
		sfd_r = accept(sfd,( sockaddr*)(&sa_r),  &si_r);
		recv(sfd_r,static_cast<void*>(&buf_r), sizeof buf_r, 0);
		std::cout<< buf_r << std::endl;
		close(sfd_r);			
	}

	return 0;
}
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <string>
#include <netinet/in.h>
#include <unistd.h>
#include <iostream>
#define PORT 8000
int main(int argc, char const *argv[])
{
	
	int sfd;
	sfd = socket(AF_INET, SOCK_STREAM, 0);
	
	sockaddr_in sa;
	bzero(static_cast<void*>(&sa), sizeof sa);
	sa.sin_family = AF_INET;
	sa.sin_port = htons(PORT);
	sa.sin_addr.s_addr = htonl(INADDR_ANY);

	bind(sfd, (sockaddr*)(&sa), sizeof sa);

	listen(sfd, 5);

	sockaddr_in sa_r;
	int sfd_r;
	unsigned int si_r = sizeof sa_r;
	char buf_r[100];
	
	while(1)
	{
		sfd_r = accept(sfd,( sockaddr*)(&sa_r),  &si_r);
		recv(sfd_r,static_cast<void*>(&buf_r), sizeof buf_r, 0);
		std::cout<< buf_r << std::endl;
		close(sfd_r);			
	}

	return 0;
}
