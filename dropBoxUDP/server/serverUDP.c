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

#define PORT 7200
#define MAX 1500
#define IPLEN 16
#define TRUE 1

typedef struct{
    int id;
    int msglen;
    char filename[40];
    char msg[MAX];
}Package; 

int main(int argv,char *args[]){

    int sockfd,clilen,flag=1; 
    struct sockaddr_in server,client; 
    
    sockfd = socket(AF_INET, SOCK_DGRAM, 0); 

    if(sockfd == -1){ 
        printf("socket creation failed...\n"); 
        exit(0); 
    }

    if(setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&flag,sizeof(flag))==-1){
        printf("setsockopt failed...\n"); 
        exit(0); 
    }

    bzero(&server, sizeof(server)); 

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl(INADDR_ANY); 
    server.sin_port = htons(PORT); 

    if ((bind(sockfd, (struct sockaddr*)&server, sizeof(server))) != 0) { 
        printf("socket bind failed...\n"); 
        exit(0); 
    }

    clilen = sizeof(client);

    char path[30]="./syncFolder/";
    int banFile=1;
    FILE *fp;
    Package pkg;
    int banFinal=1;

    while(1){
        bzero(pkg.msg, MAX); 

        int nrecv = recvfrom(sockfd,(void*)&pkg,sizeof(Package),0,(struct sockaddr*)&client,&clilen);

        if(pkg.id==-1){
            printf("%s received\n",pkg.filename);
            break;
        }

        if(banFile){
            strcat(path,pkg.filename);
            fp = fopen(path, "a+");
            if(fp==NULL){
                printf("error opening the file");
                 exit(0);
            }
            banFile=0; 
        }    

        fwrite(pkg.msg,sizeof(char),pkg.msglen,fp);

        bzero(pkg.msg, MAX);
        pkg.id++;

        int nsend = sendto(sockfd,(void*)&pkg,sizeof(Package),0,(struct sockaddr*)&client,clilen);
    
    }
    fclose(fp);

    close(sockfd);

    return 0;
}