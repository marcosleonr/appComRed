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

#define SEM1_NAME "/s1"
#define SEM2_NAME "/s2"


typedef struct{
    int id;
    int msglen;
    char filename[40];
    char msg[MAX];
}Package; 

typedef struct{
    int serverPort;
    char ip[IPLEN];
}serverData;

typedef struct{
    int port;
    char fileName[40];
}packageClient;

serverData serverList[40];
int idServerList;
char strDelete[40];
int banDelete;

sem_t *sem1;
sem_t *sem2;

int sendFile(int sockfd,char *fileName){
    
    int idMsg = 0,filesize,n=0;
    char path[30]="./syncFolder/";
    strcat(path,fileName);

    FILE *fp;
    fp = fopen(path,"rb");

    if(fp==NULL){
        printf("error opening the file");
        return 0;
    } 

    fseek(fp, 0L, SEEK_END);
    filesize = ftell(fp);
    rewind(fp);

    Package pkg;

    strcpy(pkg.filename,fileName);

    int nread;

    while((nread=fread(pkg.msg,sizeof(char),MAX,fp))>0){

        pkg.id=idMsg++;
        pkg.msglen =nread; 

        int nbytess=sendto(sockfd,&pkg,sizeof(Package),0,NULL,0);
            
        bzero(pkg.msg,MAX); 

        int nbytesr=recvfrom(sockfd,&pkg,sizeof(Package),0,NULL,NULL);

        if(pkg.id!=idMsg){
            printf("error sending file\n");
            return 0;
        }
        bzero(pkg.msg,MAX);
    }

    pkg.id=-1;
    int nbytess=sendto(sockfd,&pkg,sizeof(Package),0,NULL,0);

    fclose(fp);
   
    return 1;
}

void * client(void *args){
    packageClient pC = *((packageClient*)args);
    //printf("puerto %d",pC.port);
    //printf("nombre archivo %s",pC.fileName);

    int sockfd,flag=1;
    struct sockaddr_in server, client;
      
    sockfd = socket(AF_INET, SOCK_STREAM, 0); 
    if (sockfd == -1){  
        printf("socket creation failed...\n"); 
        return 0;
    }

    if(setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&flag,sizeof(flag))==-1){
        printf("setsockopt failed...\n"); 
        return 0; 
    }

    bzero(&server, sizeof(server)); 
  
    server.sin_family = AF_INET; 
    server.sin_addr.s_addr = inet_addr("127.0.0.1"); 
    server.sin_port = htons(pC.port);     
    
    if (connect(sockfd, (struct sockaddr*)&server, sizeof(server)) != 0) { 
        printf("connection with the server failed 127.0.0.1:%d \n",pC.port); 
        return 0; 
    }else{
        printf("connected to the server 127.0.0.1:%d \n",pC.port); 
    }

    if(banDelete==1){
        
        Package delete;

        delete.id = -4;
        strcpy(delete.filename, strDelete);

        int nbytess=sendto(sockfd,&delete,sizeof(Package),0,NULL,0);
        printf("Send element to delete:  %s \n", delete.filename);       
        banDelete = 0;
        
    }else{
        if(sendFile(sockfd,pC.fileName)){
            printf("%s send\n",pC.fileName);
        }else{  
            printf("error sending file\n");
        } 
    }

    close(sockfd);
}

void *handler(void *args){

    int *idCh = (int*)args, serverPC;
    serverData sD;

    //struct sockaddr_in local_address;
    //int addr_size = sizeof(local_address);
    //getpeername(*idCh, (struct sockaddr *)&local_address, &addr_size);
    //int localport;
    //localport=htons(local_address.sin_port);
    //printf("puerto cliente %d\n",localport);
    
    //Recibir datos del servidor localizado en el cliente
    int nbytesr=recvfrom(*idCh,&sD,sizeof(serverData),0,NULL,NULL);
    
    serverList[idServerList].serverPort = sD.serverPort;
    strcpy(serverList[idServerList].ip,sD.ip);
    idServerList++;

    serverPC=sD.serverPort;

    //printf("%d\n",serverList[idServerList-1].serverPort);
    
    //Envio confirmacion
    sD.serverPort=-1;
    int nbytess=sendto(*idCh,&sD,sizeof(serverData),0,NULL,0);

    int banFinal=1;
    while(banFinal){

        char path[30]="./syncFolder/";
        int banFile=1;
        FILE *fp;
        Package pkg;
    
        while(TRUE){

            bzero(pkg.msg, MAX); 

            int nbytesr=recvfrom(*idCh,&pkg,sizeof(Package),0,NULL,NULL);

            if(pkg.id==-1){
                printf("%s received\n",pkg.filename);
                
                int nServers = idServerList; 
                for(int i=0;i<(nServers);i++){

                    if(serverPC!=serverList[i].serverPort){
                        //Creacion de clientes que enviaran el archivo recibido
                        packageClient *pC;
                        pC = (packageClient*)malloc(sizeof(packageClient));
                        pC->port=serverList[i].serverPort;
                        strncpy(pC->fileName,pkg.filename,40);

                        pthread_t *clientTh = (pthread_t*)malloc(sizeof(pthread_t));
                        pthread_create(clientTh, NULL,(void*)client,(void*)pC);
                        free(clientTh);   
                    }                   
                
                }               
                break;
            }else if(pkg.id==-2){
                close(*idCh);
                free(idCh);
                printf("closed conection");
                banFinal=0;
                break;
            }
            else if(pkg.id == -3){
                bzero(strDelete,40);
                strcat(path,pkg.filename);
                strcpy(strDelete, pkg.filename);
                remove(path);
                printf("%s deleted \n", path);
                

                int nServers = idServerList; 
                for(int i=0;i<(nServers);i++){

                    if(serverPC!=serverList[i].serverPort){
                        //Creacion de clientes que enviaran el archivo recibido
                        packageClient *pC;
                        pC = (packageClient*)malloc(sizeof(packageClient));
                        pC->port=serverList[i].serverPort;
                        strncpy(pC->fileName,pkg.filename,40);
                        while(banDelete == 1);
                        banDelete = 1;
                        pthread_t *clientTh = (pthread_t*)malloc(sizeof(pthread_t));
                        pthread_create(clientTh, NULL,(void*)client,(void*)pC);
                        free(clientTh);   
                    }                   
                
                } break;

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

            int nbytess=sendto(*idCh,&pkg,sizeof(Package),0,NULL,0);

        }     
        if(banFinal==0)break; 
        if(pkg.id != -3){
            fclose(fp);
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
    int sockfd,connfd,len,flag=1; 
    struct sockaddr_in server,client; 
    
    sockfd = socket(AF_INET, SOCK_STREAM, 0); 

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

    return 0;
}