#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<netinet/ip.h>
#include<string.h>
#include<arpa/inet.h>

#define PORT 7200
#define MAX 1500

typedef struct{
    int id;
    int msglen;
    char filename[15];
    char msg[MAX];
}Package; 

void sendMsg(int sockfd,char *filename){

    int idMsg = 0,filesize,n=0;
    char path[30]="./source/", en ='\0';
    strcat(path,filename);
    strncat(path,&en,1);

    FILE *fp;
    fp = fopen(path,"rb");

    if(fp==NULL){
        printf("error opening the file");
        exit(0);
    } 

    fseek(fp, 0L, SEEK_END);
    filesize = ftell(fp);
    rewind(fp);

    Package pkg;

    strcpy(pkg.filename,filename);

    int nread;

    while((nread=fread(pkg.msg,sizeof(char),MAX,fp))>0){

        pkg.id=idMsg++;
        pkg.msglen =nread; 

        int nbytess=sendto(sockfd,&pkg,sizeof(Package),0,NULL,0);
            
        bzero(pkg.msg,MAX); 

        int nbytesr=recvfrom(sockfd,&pkg,sizeof(Package),0,NULL,NULL);
        
        
        if(pkg.id!=idMsg){
            printf("error sending package\n");
            exit(0);
        }
        bzero(pkg.msg,MAX);
    }

    pkg.id=-1;
    int nbytess=sendto(sockfd,&pkg,sizeof(Package),0,NULL,0);

    printf("%s uploaded\n",filename);

    fclose(fp);
}

int main(int argc,char *argv[]){

    int sockfd, connfd,flag=1; 
    struct sockaddr_in server, client;
      
    sockfd = socket(AF_INET, SOCK_STREAM, 0); 
    if (sockfd == -1) { 
        printf("socket creation failed...\n"); 
        exit(0); 
    }

    if(setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&flag,sizeof(flag))==-1){
        printf("setsockopt failed...\n"); 
        exit(0); 
    }

    bzero(&server, sizeof(server)); 
  
    server.sin_family = AF_INET; 
    server.sin_addr.s_addr = inet_addr("127.0.0.1"); 
    server.sin_port = htons(PORT);     
    
    if (connect(sockfd, (struct sockaddr*)&server, sizeof(server)) != 0) { 
        printf("connection with the server failed...\n"); 
        exit(0); 
    }else{
        printf("connected to the server..\n"); 
    }

    sendMsg(sockfd,argv[1]);

    close(sockfd);
    
    return 0;
}