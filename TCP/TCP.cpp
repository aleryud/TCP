#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<winsock2.h>
#include<ws2tcpip.h>

#pragma comment(lib,"ws2_32.lib")

#define PORT 8080
#define BUFFER_SIZE 1024

#define SERVER_IP "192.168.149.171"

void Server();
void Client();
void Error(const char print[]);

int main() {
	int con = -1;
	printf("Server[0]\nClient[1]\n>>>");
	scanf_s("%d",&con);
	if (!con) {
		Server();
	}
	else {
		Client();
	}
}

void Error(const char print[]) {
	printf("%s failed : %d\n",print,WSAGetLastError());
	WSACleanup();
}

void Server() {
	WSADATA wsa;
	SOCKET server_fd, client_fd;
	struct sockaddr_in server_addr, client_addr;
	int addr_len = sizeof(client_addr);
	char buffer[BUFFER_SIZE] = { 0 };

	//1
	if (WSAStartup(MAKEWORD(2,2) , &wsa)) {
		Error("WSAStartup");
		return;
	}

	//2
	server_fd = socket(AF_INET , SOCK_STREAM , 0);
	if (server_fd == INVALID_SOCKET) {
		Error("socket");
		return;
	}

	//3
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(PORT);

	//4
	if (bind(server_fd,(struct sockaddr*)&server_addr,sizeof(server_addr)) == SOCKET_ERROR) {
		Error("bind");
		return;
	}

	//5
	if (listen(server_fd, 3) == SOCKET_ERROR) {
		Error("listen");
		closesocket(server_fd);
		return;

	}

	printf("Sever listening on port %d...\n",PORT);

	//6
	client_fd = accept(server_fd,(struct sockaddr*)&client_addr, &addr_len);
	if (client_fd == INVALID_SOCKET) {
		Error("accept");
		closesocket(server_fd);
		return;
	}

	//7
	char client_ip[BUFFER_SIZE] = { 0 };
	inet_ntop(AF_INET,&client_addr.sin_addr,client_ip,INET_ADDRSTRLEN);
	printf("Client connected from %s:%d\n",client_ip,ntohs(client_addr.sin_port));

	//8
	int bytes_received = recv(client_fd,buffer,BUFFER_SIZE,0);
	if (bytes_received == SOCKET_ERROR)
		printf("recv failed:%d\n",WSAGetLastError());
	else {
		buffer[bytes_received] = '\0';
		printf("Received: %s\n", buffer);
	}

	//9
	send(client_fd,"Hello",strlen("Hello"), 0);
	printf("Response sent.\n");

	//10
	closesocket(client_fd);
	closesocket(server_fd);
	Error("Exit");
	return;
}

void Client() {
	WSADATA wsa;
	SOCKET sockfd;
	struct sockaddr_in server_addr, client_addr;
	int addr_len = sizeof(client_addr);
	char buffer[BUFFER_SIZE] = { 0 };

	//1：初始化winsock
	if (WSAStartup(MAKEWORD(2,2),&wsa)) {
		Error("WSAStartup");
		return;
	}

	//2：创建TCP套接字
	sockfd = socket(AF_INET,SOCK_STREAM,0);
	if (sockfd == INVALID_SOCKET) {
		Error("socket");
		return;
	}

	//3:地址设置
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(PORT);
	if (inet_pton(AF_INET,SERVER_IP,&server_addr.sin_addr) <= 0) {
		printf("invalid");
		closesocket(sockfd);
		return;
	}

	//4:链接服务器
	if (connect(sockfd,(struct sockaddr*)&server_addr,sizeof(server_addr)) == SOCKET_ERROR) {
		Error("connection");
		closesocket(sockfd);
		return;
	}

	//5:发送数据
	char message[] = "Hello";
	send(sockfd,message,strlen(message),0);
	printf("Sent: %s\n",message);

	//6:接收响应
	int bytes_received = recv(sockfd, buffer, BUFFER_SIZE, 0);
	if (bytes_received == SOCKET_ERROR)
		printf("recv failed:%d\n", WSAGetLastError());
	else {
		buffer[bytes_received] = '\0';
		printf("Received: %s\n", buffer);
	}

	//7关闭
	closesocket(sockfd);
	Error("Exit");
	return;
}