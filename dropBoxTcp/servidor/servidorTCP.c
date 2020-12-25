#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<netinet/ip.h>
#include<pthread.h>

#define PORT 7200
#define MAX 1500

typedef struct{
    int id;
    int msglen;
    char filename[15];
    char msg[MAX];
}Package; 


void *handler(void *args){

    int *idCh = (int*)args, banFile=1;
    char path[30]="./dest/";
    FILE *fp;
    Package pkg;
    
    while(1){

        bzero(pkg.msg, MAX); 

        int nbytesr=recvfrom(*idCh,&pkg,sizeof(Package),0,NULL,NULL);

        if(banFile){
            strcat(path,pkg.filename);
            fp = fopen(path, "a+");
            if(fp==NULL){
                printf("error opening the file");
                exit(0);
            }
            banFile=0; 
        }

        if(pkg.id==-1){
            printf("%s completed\n",pkg.filename);
            break;
        }

        printf("%d\n",pkg.msglen);

        fwrite(pkg.msg,sizeof(char),pkg.msglen,fp);

        bzero(pkg.msg, MAX);
        pkg.id++;

        int nbytess=sendto(*idCh,&pkg,sizeof(Package),0,NULL,0);

    }

    fclose(fp);
    close(*idCh);
    free(idCh);
}

int main(int argv,char *args[]){

    int sockfd,connfd,len,flag=1; 
    struct sockaddr_in server,client; 
    
    sockfd = socket(AF_INET, SOCK_STREAM, 0); 

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

    if((listen(sockfd, 3)) != 0) { 
        printf("Listen failed...\n"); 
        exit(0); 
    }

    len = sizeof(client);
    
    while(1){

        int *idCh = (int*)malloc(sizeof(int));

        *idCh = accept(sockfd,(struct sockaddr*)&client, &len);

        if((*idCh)<0){ 
            printf("server acccept failed...\n"); 
            exit(0); 
        }else{
            printf("server acccept the client...\n"); 
        }

        pthread_t *thread = (pthread_t*)malloc(sizeof(pthread_t));

        pthread_create(thread, NULL,(void*)handler,(void*)idCh);

        free(thread);

    }

    close(sockfd);

    return 0;
}