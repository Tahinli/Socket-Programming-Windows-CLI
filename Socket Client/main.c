#ifndef WIN32_LEAG_AND_MEAN
#define WIN32_LEAG_AND_MEAN
#endif

#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <process.h>
#include <stdatomic.h>
//#include <unistd.h>
#include <conio.h>

#pragma comment(lib, "Ws2_32.lib")

#define BUFLEN 512
#define PORT YOUR-PORT(INT)
#define ADDRESS "YOUR-SERVER-IP"(STR)
int first= !0;
int pass= 0;
int key=3;
char si = 126;

//global Running variable
_Atomic char running = 0;

DWORD WINAPI sendThreadFunc(LPVOID lpParam);


int main(void) {
	printf("Welcome!\n");
	int res;
	char dec[BUFLEN];
	int tilda =0;

	//Init
	WSADATA wsaData;
	res = WSAStartup(MAKEWORD(2,2), &wsaData);
	if(res)
		{
			printf("Startup Failed: %d\n", res);
			return -1;
		}

	//SETUP Client

	//Construct socket
	SOCKET client;
	client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(client == INVALID_SOCKET)
		{
			printf("Error with construction: %d\n", WSAGetLastError());
			WSACleanup();
			return -1;
		}
	//connect to adress
	struct sockaddr_in address;
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = inet_addr(ADDRESS);
	address.sin_port = htons(PORT);
	res = connect(client, (struct sockaddr *)&address, sizeof(address));
	if(res == SOCKET_ERROR)
		{
			printf("Connection Failed : %d\n",WSAGetLastError());
			closesocket(client);
			WSACleanup();
			return -1;
		}
	else if (client == INVALID_SOCKET)
		{
			printf("Connection Failed : %d\n",WSAGetLastError());
			WSACleanup();
			return -1;
		}

	printf("Connection Parameters =  %s:%d\n", ADDRESS, PORT);
	running = !0;

	//Main Loop

	// Send Thread
	DWORD thrdId;
	HANDLE sendThread = CreateThread(NULL, 0, sendThreadFunc, &client, 0, &thrdId);
	if(sendThread)
		{
			printf("Send Thread Started with Thread ID: %d\n", thrdId);
		}
	else
		{
			printf("Send thread failed: %d\n", GetLastError());
		}
	
	//receive loop
	char recvbuf[BUFLEN];
	do
	{
		res = recv(client, recvbuf, BUFLEN, 0);
		recvbuf[res] = '\0';
		strcpy(dec,recvbuf);
		tilda = 0;
		if(res >0)
			{
				for(int i=0;i<strlen(dec);i++)
					{
						if(dec[i]==126 && tilda==0)
							{
								dec[i]=26;
								for(int j=i+1;j<strlen(dec);j++)
									{
										dec[j]=dec[j]-key;
									}
								tilda=1;
							}
					}
				printf("%s\n",dec);
			}
		else if(!res)
			{
				printf("Connection closed.\n");
				running = 0;
			}
		else
			{
				printf("Receive Failed: %d\n",WSAGetLastError());
				running = 0;
			}
	}
	while(res >0);

	running = 0;

	//connection finished, terminator for thread
	if(CloseHandle(sendThread))
		{
			printf("Send thread closed successfully.\n");
		}

	//CleanUP

	res = shutdown(client, SD_BOTH);
	if(res==SOCKET_ERROR)
		{
			printf("Shutdown failed : %d\n",WSAGetLastError());
			closesocket(client);
			WSACleanup();
			return 1;
		}
	closesocket(client);
	WSACleanup();
	

























	return 0;
}

DWORD WINAPI sendThreadFunc(LPVOID lpParam)
	{
		SOCKET client = *(SOCKET*)lpParam;
		char sendbuf[BUFLEN];
		int sendbuflen, res;
		char name[20];
		char passw[20];
		char enc[BUFLEN];

		while(running)
			{
				strcpy(enc,"");
				strcpy(sendbuf,"");
				if(first!=0)
					{
						//Name
						strcpy(name,"");
						printf("\t\t\tName = ");
						scanf("%s",name);
						printf("\n");
						
						res=send(client, name,strlen(name),0);
						if(res!=strlen(name))
							{
								printf("\n---Name couldn't send, your first successful message will be your name\n");
							}
						first=0;
					}
				if(pass==0)
							{
								//PassW
								strcpy(passw,"");
								printf("\t\t\tPassword = ");
								scanf("%s",passw);
								printf("\n");
								res=send(client, passw,strlen(passw),0);
								if(res!=strlen(passw))
									{
										printf("\n---Password couldn't send, your first successful message will be your password\n");
									}
								pass=1;
								system("cls");
							}
				else
					{
						fgets(sendbuf, BUFLEN-1, stdin);
						sendbuf[strlen(sendbuf)-1]='\0';
						sendbuflen = strlen(sendbuf);
						if(sendbuf[0]!='\n' && !(sendbuf[0]=='\t'&&sendbuflen==2))
							{
								if(sendbuf[0]!='/')
									{
										strcat(enc,&si);
										strcat(enc,sendbuf);
										for(int i =1;i<strlen(enc);i++)
											{
												enc[i]=enc[i]+key;
											}
										
									}
								else
									{
										strcat(enc,sendbuf);
									}
								enc[strlen(enc)]='\0';
								if(strlen(enc)>1)
									{
										res=send(client, enc,strlen(enc),0);
										printf("\n");
										if(res !=strlen(enc))
											{
												printf("---SYSTEM--- = Send Failed.\n");
											}
									}
							}
					}
			}
				
		return 0;
	}
