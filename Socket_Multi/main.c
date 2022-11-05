#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>

#pragma comment(lib, "Ws_32.lib")

#define BUFLEN 512
#define PORT 27015
#define ADDRESS "93.190.8.248"
#define MAX_CLIENTS 5


//FUNCS

int dc(int i);
int mte(char*);

//variables
	fd_set socketSet;
	SOCKET clients[MAX_CLIENTS];
	SOCKET sd, max_sd;

	int curNoClients = 0;
	struct sockaddr_in clientAddr;
	int clientAddrlen;
	char running = !0;
	char recvbuf[BUFLEN];
	char sysM[BUFLEN];
	char message[BUFLEN+50];
	char lastIP[32];
	char lastPort[16];
	char arIP[MAX_CLIENTS][32];
	char arPort[MAX_CLIENTS][16];
	char dIP[MAX_CLIENTS*5][32];
	char dPort[MAX_CLIENTS*5][16];
	int dcl=0;
	int first[MAX_CLIENTS][1];
	char lastName[20];
	char arName[MAX_CLIENTS][20];
	char dName[MAX_CLIENTS*5][20];
	char pass[]="mandalina";
	char gpass[20];
	int arPas[MAX_CLIENTS];



	
	
int main(void) {
	printf("Hi!\n");
	
	int res, sendRes;
	
	//Init
	WSADATA wsaData;
	res = WSAStartup(MAKEWORD(2,2), &wsaData);
	if(res)
		{
			printf("Startup failed : %d\n",res);
			return 1;
		}
	
	
	//Setup Server
	
	//Construct
	SOCKET listener;
	listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (listener == INVALID_SOCKET)
		{
			printf("Error with construction: %d\n",WSAGetLastError());
			WSACleanup();
			return 1;
		}
		
	//Setup for multi
	char multiple = !0;
	res = setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &multiple, sizeof(multiple));
	if (res < 0)
		{
			printf("Multiple Client Setup Failed : %d\n", WSAGetLastError());
			closesocket(listener);
			WSACleanup();
			return 1;
		}
	
	//Bind Address
	struct sockaddr_in address;
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = inet_addr(ADDRESS);
	address.sin_port = htons(PORT);
	bind(listener, (struct sockaddr*)&address,sizeof(address));
	if ( res == SOCKET_ERROR)
		{
			printf("Bind failed &d\n", WSAGetLastError());
			closesocket(listener);
			WSACleanup();
			return 1;
		}
		
	//set listener
	res = listen(listener, SOMAXCONN);
	if ( res == SOCKET_ERROR)
		{
			printf("Listen failed &d\n", WSAGetLastError());
			closesocket(listener);
			WSACleanup();
			return 1;
		}
		
	printf("Accepting on %s:%d\n", ADDRESS, PORT);
	
	//Main Loop
		
	//clear client array
	memset(clients, 0, MAX_CLIENTS * sizeof(SOCKET));
	for(int i=0;i<MAX_CLIENTS;i++)
		{
			strcpy(&arIP[i][0],"");
			strcpy(&arPort[i][0],"");
			strcpy(&arName[i][0],"");
			first[i][0]=-1;
			arPas[i]=-1;
		}
	for(int i=0;i<MAX_CLIENTS*5;i++)
		{
			strcpy(&dIP[i][0],"");
			strcpy(&dPort[i][0],"");
			strcpy(&dName[i][0],"");
		}
	strcpy(sysM,"");
	strcpy(lastIP,"");	
	strcpy(lastPort,"");
	strcpy(message,"");
	strcpy(lastName,"");
	
	
	//Loop
	while(running)
		{
			strcpy(sysM,"");
			//clear the set
			FD_ZERO(&socketSet);
			
			//add listener
			FD_SET(listener, &socketSet);
			for(int i = 0;i<MAX_CLIENTS;i++)
				{
					sd = clients[i];
					if(sd>0)
						{
							//add an active client to the set
							FD_SET(sd, &socketSet);
						}
					if(sd>max_sd)
						{
							max_sd = sd;
						}
				}
			int activity = select(max_sd+1, &socketSet, NULL, NULL, NULL);
			if (activity < 0)
				{
					continue;
				}
			//determine if listener socket has activity
			if(FD_ISSET(listener, &socketSet))
				{
					//accept connection
					sd = accept(listener, NULL, NULL);
					if(sd == INVALID_SOCKET)
						{
							printf("Error Accepting : %d \n", WSAGetLastError());
						}
					//get client info
					clientAddrlen = sizeof(address);
					getpeername(sd, (struct sockaddr*)&clientAddr, &clientAddrlen);
					strcat(lastIP,inet_ntoa(clientAddr.sin_addr));
					itoa(ntohs(clientAddr.sin_port),lastPort,10);
					printf("Client connected at %s:%s\n", lastIP,lastPort);
					
					
					//add to array
					if(curNoClients >= MAX_CLIENTS )
						{
							printf("FULL\n");
							send(sd,"\n ---SERVER--- = Server is FULL, Going to be Disconnected\n",strlen("\n ---SYSTEM--- = Server is FULL, Going to be Disconnected\n"),0);
							closesocket(sd);
							shutdown(sd, SD_BOTH);
						}
					else
						{
							//scan through list
							for(int i = 0; i< MAX_CLIENTS; i++)
								{
									if(!clients[i])
										{
											clients[i]= sd;
											printf("Added to the List at index: %d\n", i);
											strcpy(&arIP[i][0],lastIP);
											strcpy(&arPort[i][0],lastPort);
											first[i][0]=1;
											arPas[i]=0;
											curNoClients++;
											strcat(sysM,"\n---SERVER--- = ");
											strncat(sysM,lastIP,strlen(sysM));
											strncat(sysM,":",strlen(sysM));
											strncat(sysM,lastPort,strlen(sysM));
											strncat(sysM," Has Connected\n",strlen(sysM));
											
											for(int j = 0; j<MAX_CLIENTS; j++)
												{
													if(sd==clients[j] || arPas[j]==0 || first[j][0]==1)
														{
															continue;
														}
													else
														{
															send(clients[j],sysM, strlen(sysM), 0);
														}
												}
											break;
										}
								}
						}
					strcpy(sysM,"");
					strcpy(lastIP,"");
					strcpy(lastPort,"");
					strcpy(lastName,"");
				}
			// iterate through clients
			for(int i=0;i<MAX_CLIENTS;i++)
				{
					strcpy(message,"");
					if(!clients[i])
						{
							continue;
						}
					sd = clients[i];
					
					//determine if clients have activity
					if(FD_ISSET(sd, &socketSet))
						{
							//get message
							res = recv(sd, recvbuf, BUFLEN, 0);
							if(res > 0)
								{
											//print message
											recvbuf[res] = '\0';
											printf("%d.Client -Received (%d): %s \n",i,res,recvbuf);
										

									//test if leave
									if(!memcmp(recvbuf, "/leave", 6))
										{
											dc(i);
										}
									//test if quit
									else if(!memcmp(recvbuf, "/quit", 5*sizeof(char)))
										{
											running = 0;
											break;
										}
										
									else
										{
											//Name
											if(first[i][0]==1)
												{
													strcpy(lastName,recvbuf);
													if(lastName[0]==13)
														{
															strcpy(lastName,"Unknown");
														}
													else
														{
															strcpy(&arName[i][0],lastName);
															strcpy(lastName,"");
															first[i][0]=0;
														}
												}
											//Password
											else if(arPas[i]==0)
												{
													strcpy(gpass,recvbuf);
													if(strcmp(pass,gpass)==0)
														{
															arPas[i]=1;
															send(sd,"---SERVER--- = Connection Successful\n",strlen("---SERVER--- = Connection Successful\n"),0);
															strcat(sysM,"---SERVER--- = ");
															strcat(sysM,&arIP[i][0]);
															strcat(sysM,":");
															strcat(sysM,&arPort[i][0]);
															strcat(sysM," is Authorized as ");
															strcat(sysM,&arName[i][0]);
															mte(sysM);
															
														}
													else
														{
															send(sd,"Password is wrong\n",strlen("Password is wrong\n"),0);
															dc(i);
														}
												}
											//send message to everyone
											else
												{
													for(int j = 0; j<MAX_CLIENTS; j++)
														{
															if(sd==clients[j] || arPas[j]==0 || first[j][0]==1)
																{
																	continue;
																}
															else
																{
														
														//NAME is ADDRESS
																/*strcat(message,&arIP[i][0]);
																strcat(message,":");
																strcat(message,&arPort[i][0]);
																strcat(message," ");
																strcat(message,"=");
																strcat(message," ");
																strcat(message,recvbuf);
																send(clients[j],message,strlen(message),0);*/
																
														//NAME is NAME
																strcat(message,&arName[i][0]);
																strcat(message," ");
																strcat(message,"=");
																strcat(message," ");
																strcat(message,recvbuf);
																send(clients[j],message,strlen(message),0);
																}
														}
													res = 0;	
												}
										}
								}
							else
								{
									dc(i);
								}
						}
				}
		}
	
	
	
	
	
	//disconnect everyone
	for(int i = 0; i<MAX_CLIENTS;i++)
		{
			if(clients[i]>0)
				{
					dc(i);
				}
		}
	
	
	
	//shutdown client
	//res = shutdown(sd, SD_BOTH);
	if(res == SOCKET_ERROR)
		{
			printf("Client shutdown failed : %d \n",WSAGetLastError());
			
		}
	closesocket(sd);
	
	
	
	//ShutDown
	closesocket(listener);
	//Cleanup
	res = WSACleanup();
	if(res)
		{
			printf("Cleanup failed %d\n",res);
			return 1;
		}
	printf("DONE\n");
	return 0;
}




int dc(int i)
	{
		
		//close message
									getpeername(sd, (struct sockaddr*)&clientAddr, &clientAddrlen);
									strcat(lastIP,inet_ntoa(clientAddr.sin_addr));
									itoa(ntohs(clientAddr.sin_port),lastPort,10);
									printf("Client disconnected at %s:%s\n", lastIP,lastPort);
									closesocket(sd);
									shutdown(sd, SD_BOTH);
									clients[i]=0;
									strcpy(&arIP[i][0],"");
									strcpy(&arPort[i][0],"");
									
									first[i][0]=-1;
									arPas[i]=-1;
									curNoClients--;
									if(dcl<MAX_CLIENTS*5)
										{
											strcpy(&dIP[dcl][0],lastIP);
											strcpy(&dPort[dcl][0],lastPort);
											strcpy(&dName[dcl][0],&arName[i][0]);
											dcl++;
										}											
									else
										{
											printf("\nDCList is Full !\n");
										}
									strcat(sysM,"\n---SERVER--- = ");
									strcat(sysM,lastIP);
									strcat(sysM,":");
									strcat(sysM,lastPort);
									strcat(sysM,"-->");
									strcat(sysM,&arName[i][0]);
									strncat(sysM," Has Disconnected\n",strlen(sysM));
									strcpy(&arName[i][0],"");
											
									for(int j = 0; j<MAX_CLIENTS; j++)
										{
											if(sd==clients[j] || arPas[j]==0 || first[j][0]==1)
												{
													continue;
												}													
											else
												{
													send(clients[j],sysM, strlen(sysM), 0);
												}
										}
									strcpy(sysM,"");
									strcpy(lastIP,"");
									strcpy(lastPort,"");
									return 0;
	}
int mte(char* message)
	{
		for(int j = 0; j<MAX_CLIENTS; j++)
														{
															if(arPas[j]==0 || first[j][0]==1)
																{
																	continue;
																}
															else
																{
																	send(clients[j],message,strlen(message),0);
																}
														}
		return 0;
	}