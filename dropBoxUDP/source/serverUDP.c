#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<netinet/ip.h>
#include<arpa/inet.h>
#include<semaphore.h>
#include<pthread.h>
#include<fcntl.h> 

#define SERVPORT 7200
#define MAX 1500
#define IPLEN 16
#define TRUE 1

typedef struct{
    int id;
    int msgLen;
    char fileName[40];
    char msg[MAX];
}Package;

typedef struct{
    char ip[IPLEN];
    int serverPort;
    int idServerPort;
}configPackage;

typedef struct{
    int serverPort;
    char ip[IPLEN];
}serverData;

typedef struct{
    int serverPort;
    int clientServPort;
}ports;

serverData serverList[40];
int idServerList;
int idServerPort=7500;

void * auxServer(void * args){

    ports ps;
    memcpy((void*)&ps,args,sizeof(ports));

    int port = ps.serverPort;

    printf("puerto %d\n",port);

    int localSockfd,clilen,flag=1; 
    struct sockaddr_in server,client; 
    
    localSockfd = socket(AF_INET, SOCK_DGRAM, 0); 

    if(localSockfd == -1){ 
        printf("socket creation failed...\n"); 
        exit(0); 
    }

    if(setsockopt(localSockfd,SOL_SOCKET,SO_REUSEADDR,&flag,sizeof(flag))==-1){
        printf("setsockopt failed...\n"); 
        exit(0); 
    }

    bzero(&server, sizeof(server)); 

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl(INADDR_ANY); 
    server.sin_port = htons(port); 

    if ((bind(localSockfd, (struct sockaddr*)&server, sizeof(server))) != 0) { 
        printf("socket bind failed...\n"); 
        exit(0); 
    }

    clilen = sizeof(client);

    int banFinal=1;
    while(banFinal){

        char path[30]="./syncFolder/";
        FILE *fp;
        Package pkg;
        while(TRUE){
            bzero(pkg.msg, MAX); 

            int nrecv = recvfrom(localSockfd,(void*)&pkg,sizeof(Package),0,(struct sockaddr*)&client,&clilen);

            if(pkg.id==0){
                strcat(path,pkg.fileName);
                fp = fopen(path, "a+");
                if(fp==NULL){
                    printf("error opening the file");
                    exit(0);
                }
            }else if(pkg.id==-1){
                printf("%s received\n",pkg.fileName);
                fclose(fp);

                int nServers = idServerList; 
                for(int i=0;i<(nServers);i++){
                    if(ps.clientServPort!=serverList[i].serverPort){
                        printf("%d\n",serverList[i].serverPort);
                    }
                        
                }  

                break;
            }else if(pkg.id==-2){
                close(localSockfd);
                printf("closed conection");
                banFinal=0;
                break;
            }

            fwrite(pkg.msg,sizeof(char),pkg.msgLen,fp);

            bzero(pkg.msg, MAX);
            pkg.id++;

            int nsend = sendto(localSockfd,(void*)&pkg,sizeof(Package),0,(struct sockaddr*)&client,clilen);

        }   
    }
}

void * scanTerminal(void * args){
    int sockfd = *((int*)args);
    char str[40];
    scanf("%s",str);
    if(strncmp(str,"exit",4)==0){
        
        close(sockfd);
        printf("closed conection server");
        exit(0);
    }
}

int main(int argv,char *args[]){

    pthread_t scanTerminalTh;

    int sockfd, clilen,flag=1; 
    struct sockaddr_in server,client; 
    
    sockfd = socket(AF_INET, SOCK_DGRAM, 0); 

    if(sockfd == -1){ 
        printf("socket creation failed...\n"); 
        exit(0); 
    }

    pthread_create(&scanTerminalTh,NULL,(void*)scanTerminal,(void*)&sockfd);


    if(setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&flag,sizeof(flag))==-1){
        printf("setsockopt failed...\n"); 
        exit(0); 
    }

    bzero(&server, sizeof(server)); 

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl(INADDR_ANY); 
    server.sin_port = htons(SERVPORT); 

    if ((bind(sockfd, (struct sockaddr*)&server, sizeof(server))) != 0) { 
        printf("socket bind failed...\n"); 
        exit(0); 
    }

    clilen = sizeof(client);

    configPackage confP;
    while(TRUE){

        bzero((void*)&confP, sizeof(configPackage));

        int nrecv = recvfrom(sockfd,(void*)&confP,sizeof(configPackage),0,(struct sockaddr*)&client,&clilen); 

        serverList[idServerList].serverPort = confP.serverPort;
        strcpy(serverList[idServerList].ip,confP.ip);
        idServerList++;
   
        ports * ps;
        ps = (ports*)malloc(sizeof(ports));

        ps->serverPort=idServerPort++;
        ps->clientServPort=confP.serverPort;

        pthread_t *auxServerTh = (pthread_t*)malloc(sizeof(pthread_t));
        pthread_create(auxServerTh, NULL,(void*)auxServer,(void*)ps);
            
        bzero((void*)&confP, sizeof(Package)); 
        confP.serverPort=-1;
        confP.idServerPort=ps->serverPort;
        int nsend = sendto(sockfd,(void*)&confP,sizeof(configPackage),0,(struct sockaddr*)&client,clilen);
 
    }
    return 0;
}