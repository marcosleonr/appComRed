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
#include<pthread.h>
#include<dirent.h>
#include<sys/stat.h>
#include<semaphore.h>
#include<fcntl.h> 

#define MAX 1500
#define SERVPORT 7200
#define TRUE 1
#define ARRSIZE 30
#define IPLEN 16

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

char *arrFileName[ARRSIZE];
char *recFileName[ARRSIZE];
int numberOfFiles;
int numberOfRecFiles;

int sockfd;
int localServerPort; 
char globalName[40];
int scanTerminalFlag=1;
int newServPort;

void pop(char *str){
	int puntero = 0;
    for(int i=0;i < numberOfFiles;i++){
        if(strcmp(arrFileName[i],str)==0){
            for (int a = 0; a < numberOfFiles -1; ++a){
            	if (a == i || puntero == 1){
            		puntero = a + 1;
            		arrFileName[a] = arrFileName[puntero];
            		puntero = 1;
            	}
            	printf(" ***  %s\n",arrFileName[a] );
            }
            numberOfFiles--;
        }

    }

    for(int i=0;i < numberOfRecFiles;i++){
        if(strcmp(recFileName[i],str)==0){
            for (int a = 0; a < numberOfRecFiles -1; ++a){
            	if (a == i || puntero == 1){
            		puntero = a + 1;
            		recFileName[a] = recFileName[puntero];
            		puntero = 1;
            	}
            	printf(" ***  %s\n",recFileName[a] );
            }
            numberOfRecFiles--;
        }
    }
}

void * server(void * args){
    int serverSockfd, clilen,flag=1; 
    struct sockaddr_in server,client; 
    
    serverSockfd = socket(AF_INET, SOCK_DGRAM, 0); 

    if(serverSockfd == -1){ 
        printf("socket creation failed...\n"); 
        exit(0); 
    }

    if(setsockopt(serverSockfd,SOL_SOCKET,SO_REUSEADDR,&flag,sizeof(flag))==-1){
        printf("setsockopt failed...\n"); 
        exit(0); 
    }

    bzero(&server, sizeof(server)); 
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl(INADDR_ANY); 
    server.sin_port = htons(localServerPort); 

    if ((bind(serverSockfd, (struct sockaddr*)&server, sizeof(server))) != 0) { 
        printf("socket bind failed...\n"); 
        exit(0); 
    }

    clilen = sizeof(client);

    while(TRUE){
        char path[30]="./syncFolder/";
        FILE *fp;
        Package pkg;

        while(TRUE){
            
            bzero(pkg.msg, MAX); 
            int nrecv = recvfrom(serverSockfd,(void*)&pkg,sizeof(Package),0,(struct sockaddr*)&client,&clilen);

            if(pkg.id==0){
                strcat(path,pkg.fileName);

                char *str;
                str = (char*)malloc(sizeof(char)*40);
                strcpy(str,pkg.fileName);
                recFileName[numberOfRecFiles]=str;
                numberOfRecFiles++;


                fp = fopen(path, "a+");
                if(fp==NULL){
                    printf("error opening the file");
                    exit(0);
                }
            }else if(pkg.id==-1){
                printf("%s received\n",pkg.fileName);
                fclose(fp);
                break;
            }else if(pkg.id==-4){
                printf("se va a borrar\n");
            	strcat(path,pkg.fileName);

                remove(path);

                pop(pkg.fileName);
                break;
            }

            fwrite(pkg.msg,sizeof(char),pkg.msgLen,fp);

            bzero(pkg.msg, MAX);
            pkg.id++;

            int nsend = sendto(serverSockfd,(void*)&pkg,sizeof(Package),0,(struct sockaddr*)&client,clilen);

        } 
    }
    close(serverSockfd);
       
}

int findFile(char *fileName){

    for(int i=0;i<numberOfFiles;i++){
        if(strcmp(arrFileName[i],fileName)==0){
            return 1;
        }
    }

    for(int i=0;i<numberOfRecFiles;i++){
        if(strcmp(recFileName[i],fileName)==0){
            return 1;
        }
    }

    arrFileName[numberOfFiles]=fileName;
    numberOfFiles++;
    return 0;
}

void * scanDir(void *args){
    DIR *ptrDir;
    struct dirent *ptrDirent;

    while(TRUE){
        
        ptrDir = opendir("./syncFolder");
        while ((ptrDirent = readdir(ptrDir)) != NULL) {
            if(ptrDirent->d_type == DT_REG){
                char* newFileName;
                newFileName=(char*)malloc(sizeof(char)*40);
                strcpy(newFileName,ptrDirent->d_name);
                if(findFile(newFileName)==0){
                    strcpy(globalName,newFileName);
                    scanTerminalFlag=0;
                    while(scanTerminalFlag==0);
                }
            }
        }
        closedir(ptrDir);
        usleep(500000);
    }
}

int sendFile(char *fileName){

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

    struct sockaddr_in server;
    bzero(&server, sizeof(server)); 
    server.sin_family = AF_INET; 
    server.sin_addr.s_addr = inet_addr("127.0.0.1"); 
    server.sin_port = htons(newServPort);

    Package pkg;
    strcpy(pkg.fileName,fileName);

    int nread;
    while((nread=fread(pkg.msg,sizeof(char),MAX,fp))>0){

        pkg.id=idMsg++;
        pkg.msgLen =nread; 

        int nbytess=sendto(sockfd,(void*)&pkg,sizeof(Package),0,(struct sockaddr*)&server,sizeof(struct sockaddr_in));

        int nrecv=recvfrom(sockfd,(void*)&pkg,sizeof(Package),0,NULL,0);

        if(pkg.id!=idMsg){
            printf("error sending file\n");
            return 0;
        }
    }
    fclose(fp);

    pkg.id=-1;
    int nbytess=sendto(sockfd,(void*)&pkg,sizeof(Package),0,(struct sockaddr*)&server,sizeof(struct sockaddr_in));
   
    return 1;
}

void * scanTerminal(void * args){
    while(TRUE){

        char str[40];
        char path[40]="./syncFolder/";
        bzero(str,40);
        scanf("%s",str);

        struct sockaddr_in server;
        bzero(&server, sizeof(server)); 
        server.sin_family = AF_INET; 
        server.sin_addr.s_addr = inet_addr("127.0.0.1"); 
        server.sin_port = htons(newServPort);

        if(strncmp(str,"exit",4)==0){
            Package pkg;
            pkg.id=-2;
            int nbytess=sendto(sockfd,(void*)&pkg,sizeof(Package),0,(struct sockaddr*)&server,sizeof(struct sockaddr_in));
            close(sockfd);
            printf("closed conection\n");
            exit(0);
        }else{
            //Para eleminar el archivo local
            Package delete;
            strcat(path,str);
            delete.id = -3;
            strcpy(delete.fileName, str);
            int nbytess=sendto(sockfd,(void*)&delete,sizeof(Package),0,(struct sockaddr*)&server,sizeof(struct sockaddr_in));
            remove(path);
            printf(" deleted  %s \n", path);
            pop(str);
            
        }
    }

}

int main(int argc,char *argv[]){

    localServerPort=atoi(argv[1]);

    pthread_t scanDirTh,serverTh,scanTerminalTh;
    pthread_create(&scanDirTh,NULL,(void*)scanDir,NULL);
    pthread_create(&serverTh,NULL,(void*)server,NULL);
    pthread_create(&scanTerminalTh,NULL,(void*)scanTerminal,NULL);

    int connfd,flag=1; 
    struct sockaddr_in client,server; 
  
    sockfd = socket(AF_INET, SOCK_DGRAM, 0); 
    if (sockfd == -1) { 
        printf("socket creation failed...\n"); 
        exit(0); 
    }
    
    if(setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&flag,sizeof(flag))==-1){
        printf("setsockopt failed...\n"); 
        return 0; 
    }

    bzero(&client, sizeof(client)); 
    client.sin_family = AF_INET;
    client.sin_addr.s_addr = htonl(INADDR_ANY); 
    client.sin_port = htons(0); 

    if ((bind(sockfd, (struct sockaddr*)&client, sizeof(client))) != 0) { 
        printf("socket bind failed...\n"); 
        exit(0); 
    }
    
    bzero(&server, sizeof(server)); 
    server.sin_family = AF_INET; 
    server.sin_addr.s_addr = inet_addr("127.0.0.1"); 
    server.sin_port = htons(SERVPORT);

    configPackage confP;

    strcpy(confP.ip,"127.0.0.1");
    confP.serverPort=localServerPort;

    int nbytess=sendto(sockfd,(void*)&confP,sizeof(configPackage),0,(struct sockaddr*)&server,sizeof(server));
    bzero((void*)&confP,sizeof(configPackage));
    int nbytesr=recvfrom(sockfd,&confP,sizeof(configPackage),0,NULL,NULL);
    
    if(confP.serverPort!=-1){
        printf("error localserver info");
        exit(0);
    }else{
        newServPort=confP.idServerPort;
    }

    char *finalName;
    finalName = (char*)malloc(sizeof(char)*40);

    while(TRUE){
        
        while(scanTerminalFlag);
        strcpy(finalName,globalName);
        
        if(sendFile(finalName)){
            printf("%s send\n",finalName);
        }else{
            printf("Error sending file\n");
        } 
        
        scanTerminalFlag=1;
    }

    return 0;
} 