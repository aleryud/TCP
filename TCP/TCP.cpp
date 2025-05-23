﻿#define _CRT_SECURE_NO_WARNINGS
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<math.h>
#include<winsock2.h>
#include<ws2tcpip.h>
#include <iphlpapi.h>

#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib,"ws2_32.lib")

#define PORT 8080
#define BUFFER_SIZE 1024

#define SERVER_IP "192.168.149.171"

#define BASE64 "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"

void Server();
void Client();
void Error(const char print[]);
void Ascii_to_Base64(int model, const char IP[]);
int Base64_to_Ascii(int model, const char IP[]);
const char* get_network_ips();
unsigned long long IPChangeToLongMath(const char IP[]);
char* LongMathToIP(unsigned long long num);
char* encode_base62(unsigned long long num);
unsigned long long decode_base62(const char* base62_str);
void ServerExit(SOCKET server_fd, SOCKET* client_fd, int clientNum);



int main() {
	int con = -1;
	printf("Server[0]\nClient[1]\n>>>");
	scanf_s("%d", &con);
	if (!con) {
		Server();
	}
	else {
		Client();
	}
}

void Error(const char print[]) {
	printf("%s failed : %d\n", print, WSAGetLastError());
	WSACleanup();
}

void Server() {
	WSADATA wsa;
	SOCKET server_fd,* client_fd;
	struct sockaddr_in server_addr,* client_addr;
	int clientNum = 1;
	int addr_len = sizeof(client_addr[0]);
	char buffer[BUFFER_SIZE] = { 0 };

	client_fd = (SOCKET*)calloc(clientNum , sizeof(SOCKET));
	client_addr = (sockaddr_in*)calloc(clientNum , sizeof(sockaddr_in));

	//1:启用服务
	if (WSAStartup(MAKEWORD(2, 2), &wsa)) {
		Error("WSAStartup");
		return;
	}

	//2：创建TCP套接字
	server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_fd == INVALID_SOCKET) {
		Error("socket");
		return;
	}

	//3：设置地址
	server_addr.sin_family = AF_INET;//IPV4
	server_addr.sin_addr.s_addr = INADDR_ANY;//监听网卡
	server_addr.sin_port = htons(PORT);//端口

	//4：绑定套接字与IP及端口
	if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
		Error("bind");
		return;
	}

	//5：监听链接请求
	if (listen(server_fd, 5) == SOCKET_ERROR) {
		Error("listen");
		closesocket(server_fd);
		return;
	}

	printf("Sever listening on port %d...\nyour code:", PORT);

	printf(" %s\n", encode_base62(IPChangeToLongMath(get_network_ips())));

	while(1) {
		for (int f = 0; f < clientNum ;f++) {
			SOCKET* con_fd = NULL;
			struct sockaddr_in* con_addr = NULL;

			SOCKET con = NULL;
			char client_ip[BUFFER_SIZE] = { 0 };
			//6：接受请求
			if (client_fd[f] == (con = accept(server_fd, (struct sockaddr*)&client_addr[f], &addr_len)))
				goto massage;
			clientNum++;
			con_fd = (SOCKET*)realloc(client_fd, clientNum * sizeof(SOCKET));
			con_addr = (sockaddr_in*)realloc(client_addr,clientNum * sizeof(sockaddr_in));
			if (con_fd == NULL || con_addr == NULL) {
				free(con_fd);
				free(con_addr);
				break;
			}
			client_fd = con_fd;
			client_addr = con_addr;

			client_fd[f] = con;
			if (client_fd[f] == INVALID_SOCKET) {
				//Error("accept");
				//closesocket(server_fd);
				free(con_fd);
				free(con_addr);
				break;
			}

			//7：打印客户IP与端口
			inet_ntop(AF_INET, &client_addr[f].sin_addr, client_ip, INET_ADDRSTRLEN);
			printf("Client connected from %s:%d（%d）\n", client_ip, ntohs(client_addr[f].sin_port),f);

			//8：接收数据
	massage:int bytes_received = recv(client_fd[f], buffer, BUFFER_SIZE, 0);
			if (bytes_received == SOCKET_ERROR)
				printf("recv failed:%d\n", WSAGetLastError());
			else {
				buffer[bytes_received] = '\0';
				printf("Received: %s\n", buffer);
			}

			//9：发送响应数据
			char messageCode[50];
			int a = 0;
			while (getchar() == ' ');
			while ((messageCode[a++] = getchar()) != '\n');
			char* message = messageCode;
			send(client_fd[f], message, strlen(message), 0);
			printf("Response sent.\n");

			//释放
			free(con_fd);
			free(con_addr);
		}
	}

	//10结束
	for (int i = 0 ;i < clientNum;i++) {
		closesocket(client_fd[i]);
	}
	closesocket(server_fd);

	//自由
	free(client_addr);
	free(client_fd);

	Error("Exit");
	return;
}

void ServerExit(SOCKET server_fd, SOCKET* client_fd ,int clientNum) {
	for (int i = 0; i < clientNum; i++) {
		closesocket(client_fd[i]);
	}
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
	if (WSAStartup(MAKEWORD(2, 2), &wsa)) {
		Error("WSAStartup");
		return;
	}

	//2：创建TCP套接字
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == INVALID_SOCKET) {
		Error("socket");
		return;
	}

	//3:地址设置
	char inputCode[50];
	int i = 0;
	while (getchar() == ' ');
	while ((inputCode[i++] = getchar()) != '\n');//这个方法会读到回车
	//scanf_s(" %c", &inputCode[i++]);
	
	//printf("\ninput: %s\n", inputCode);
	
	//for (int i = 0;i<=strlen(inputCode);i++) {
	//	printf(" %d[%d]",inputCode[i],i);
	//}

	inputCode[strlen(inputCode) - 1] = '\0';//这里要去掉回车
	//printf("\nnumber: %d", decode_base62(inputCode));
	char* serverIP = LongMathToIP(decode_base62(inputCode));
	printf("\nIP: %s", serverIP);
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(PORT);
	if (inet_pton(AF_INET, serverIP, &server_addr.sin_addr) <= 0) {
		printf("invalid");
		closesocket(sockfd);
		return;
	}

	//4:链接服务器
	if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
		Error("connection");
		closesocket(sockfd);
		return;
	}

	//5:发送数据
	char messageCode[50];
	int a = 0;
	while (getchar() == ' ');
	while ((messageCode[a++] = getchar()) != '\n');
	char* message = messageCode;
	send(sockfd, message, strlen(message), 0);
	printf("Sent: %s\n", message);

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

const char* get_network_ips() {
	PIP_ADAPTER_ADDRESSES adapter_addrs = NULL;
	ULONG buf_len = 0;

	// 第一次调用获取缓冲区大小
	if (GetAdaptersAddresses(AF_INET, 0, NULL, NULL, &buf_len) == ERROR_BUFFER_OVERFLOW) {
		adapter_addrs = (PIP_ADAPTER_ADDRESSES)malloc(buf_len);
	}

	// 第二次调用获取实际数据
	if (GetAdaptersAddresses(AF_INET, 0, NULL, adapter_addrs, &buf_len) == ERROR_SUCCESS) {
		PIP_ADAPTER_ADDRESSES adapter = adapter_addrs;
		while (adapter) {
			// 排除未启用的适配器和环回接口
			if (adapter->OperStatus == IfOperStatusUp && !(adapter->Flags & IP_ADAPTER_RECEIVE_ONLY)) {
				PIP_ADAPTER_UNICAST_ADDRESS addr = adapter->FirstUnicastAddress;
				while (addr) {
					if (addr->Address.lpSockaddr->sa_family == AF_INET) {
						struct sockaddr_in* sockaddr = (struct sockaddr_in*)addr->Address.lpSockaddr;
						static char ip[INET_ADDRSTRLEN];
						inet_ntop(AF_INET, &sockaddr->sin_addr, ip, sizeof(ip));
						//printf("%s",ip);
						return ip;
					}
					addr = addr->Next;
				}
			}
			adapter = adapter->Next;
		}
	}

	free(adapter_addrs);
}

unsigned long long IPChangeToLongMath(const char IP[]) {
	int a = strlen(IP) - 1;
	long long result = 0;
	//printf("\nwhat15:%c %d\n", IP[a],a);
	for (int i = 0; i < 4; i++) {
		long long con = 0;
		//printf("\nwhat15:%d %d\n", IP[a], a);
		for (int zero = 0; a >= 0; a--, zero++) {
			//printf("\nwhat0:%d ,%d\n", IP[a],a);
			if (IP[a] != '.') {
				//printf("\nwhat1:%c\n", IP[a]);
				con += (IP[a] - '0') * pow(10, zero);

			}
			else {
				a--;
				con *= pow(256, i);
				break;
			}
			if (a == 0) {
				con *= pow(256, i);
			}
		}
		//		printf("\nwhat1:%lld,%d\n", con, i);
		result += con;
	}
	//printf("\nwhat?:%lld\n",result);
	return result;
}

char* LongMathToIP(unsigned long long num) {
	char* ip = (char*)malloc(16); // 足够存储IPv4地址（如"255.255.255.255"）
	int segments[4];

	// 从长整数中提取4个8位段
	segments[0] = (num >> 24) & 0xFF; // 第一段（最高8位）
	segments[1] = (num >> 16) & 0xFF; // 第二段
	segments[2] = (num >> 8) & 0xFF; // 第三段
	segments[3] = num & 0xFF;         // 第四段（最低8位）

	// 格式化为IPv4字符串
	sprintf(ip, "%d.%d.%d.%d", segments[0], segments[1], segments[2], segments[3]);
	return ip;
}

const char base62_chars[] = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
const int base62_size = sizeof(base62_chars) - 1;

// 将整数编码为 Base62 字符串，并返回该字符串
char* encode_base62(unsigned long long num) {
	if (num == 0) {
		char* result = (char*)malloc(2);
		if (result != NULL) {
			result[0] = '0';
			result[1] = '\0';
		}
		return result;
	}

	char buffer[65]; // 足够大以存储 64 位整数的 Base62 表示
	int index = 0;

	while (num > 0) {
		buffer[index++] = base62_chars[num % base62_size];
		num /= base62_size;
	}
	buffer[index] = '\0';

	// 反转字符串
	int len = index;
	char* result = (char*)malloc(len + 1);
	if (result != NULL) {
		for (int i = 0; i < len; i++) {
			result[i] = buffer[len - 1 - i];
		}
		result[len] = '\0';
	}
	return result;
}

// 将 Base62 字符串解码为整数
unsigned long long decode_base62(const char* input) {
	unsigned long long result = 0;
	int len = strlen(input);

	for (int i = 0; i < len; i++) {
		char c = input[i];
		int value;
		if (c >= '0' && c <= '9') {
			value = c - '0';
		}
		else if (c >= 'a' && c <= 'z') {
			value = c - 'a' + 10;
		}
		else if (c >= 'A' && c <= 'Z') {
			value = c - 'A' + 36;
		}
		else {
			fprintf(stderr, "Invalid character in Base62 string: %c\n", c);
			return 0;
		}

		result = result * base62_size + value;
	}

	return result;
}

void Ascii_to_Base64(int model, const char IP[]) {
	char base64[] = BASE64;
	char letter = 0;
	int length = 0, base = 0;
	int* con = 0, * con_2 = 0;
	con = (int*)malloc(sizeof(int));

	int IPi = 0;

	if (model) {
		length = 1;
		base = 1;
	}
	for (int num[8] = { 0 }; (IP[IPi] != '\0') && ((letter = IP[IPi++]) <= 127);) {
		con = (int*)realloc(con, ((length) += 8) * sizeof(int));
		for (int i = 1; letter > 0; letter /= 2, i++) {
			num[8 - i] = letter % 2;
		}
		for (int i = 0; i <= 7; i++) {
			con[(length)-8 + i] = num[i];
		}
		for (int i = 0; i <= 7; i++) {
			num[i] = 0;
		}
	}
	if ((length) % 6 != 0) {
		int a = 6 - ((length) % 6);
		con = (int*)realloc(con, ((length) += 6 - (length) % 6) * sizeof(int));
		for (; a >= 0; a--) {
			con[length - a] = 0;
		}
	}
	con_2 = (int*)malloc((length / 6) * sizeof(int));
	for (int i = 0; i < (length / 6); i++) {
		con_2[i] = 0;
	}
	for (int i = 0, i_2 = 0; i < (length); i_2++) {
		for (int a = 5; a >= 0; a--, i++) {
			con_2[i_2] += con[i] * pow(2, a);
		}
	}
	int i = 0;
	printf("\n");
	for (; i < length / 6; i++) {
		printf("%c", base64[con_2[i]]);
		if ((i + 1) % 6 == 0) {
		}
	}
	if (i % 4 != 0) {
		for (int a = 0; a < (4 - i % 4); a++) {
			printf("=");
		}
	}
	printf("\n");
}

int Base64_to_Ascii(int model, const char IP[]) {
	char base64[] = BASE64;
	char letter = 0;
	int num[6] = { 0 };
	int error = 0, length = 0;
	int* con = 0, * con_2 = 0;
	con = (int*)malloc(sizeof(int));

	int IPi = 0;

	for (; (IP[IPi] != '\0') && ((letter = IP[IPi++]) <= 127);) {
		for (int i = 0; 1; i++) {
			if (base64[i] == letter) {
				int save = i;
				con = (int*)realloc(con, ((length) += 6) * sizeof(int));
				for (int a = 1; save > 0; save /= 2, a++) {
					num[6 - a] = save % 2;
				}
				for (int a = 0; a <= 5; a++) {
					con[(length)-6 + a] = num[a];
				}
				for (int a = 0; a <= 5; a++) {
					num[a] = 0;
				}
				break;
			}
			if (i >= 65) {
				error++;
				break;
			}
		}
	}
	printf("\n");
	for (int a = 0, b = 0; b < (length / 8); b++) {
		int cha = 0;
		for (int i = 7; i >= 0; i--, a++) {
			cha += con[a] * pow(2, i);
		}
		printf("%c", (char)cha);
	}
	printf("\n");
	return 0;
}
