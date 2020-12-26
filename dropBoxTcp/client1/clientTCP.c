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
#include <fcntl.h> 

#define SEM_WRITER_NAME "/swriter"
#define SEM_READER_NAME "/sreader"

#define MAX 1500
#define SERVPORT 7200
#define TRUE 1
#define ARRSIZE 10

char *arrFileName[ARRSIZE];
int numberOfFiles;
char globalName[20];

sem_t * sem_writer;
sem_t * sem_reader;

typedef struct{
    int id;
    int msgLen;
    char fileName[15];
    char msg[MAX];
}Package; 

void * server(void *args){

    int serverPort,sockfd,idChannel,len,flag=1;
    struct sockaddr_in server,client;
    
    serverPort = *((int*)args);   
    
    sockfd = socket(AF_INET, SOCK_STREAM, 0); 

    if(sockfd == -1){ 
        printf("server socket creation failed...\n"); 
        exit(0); 
    }

    if(setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&flag,sizeof(flag))==-1){
        printf("server setsockopt failed...\n"); 
        exit(0); 
    }

    bzero(&server, sizeof(server)); 

    server.sin_family = AF_INET; 
    server.sin_addr.s_addr = htonl(INADDR_ANY); 
    server.sin_port = htons(serverPort); 

    if ((bind(sockfd, (struct sockaddr*)&server, sizeof(server))) != 0) { 
        printf("server socket bind failed...\n"); 
        exit(0); 
    }

    if((listen(sockfd, 5)) != 0) { 
        printf("server listen failed...\n"); 
        exit(0); 
    }

    len = sizeof(client);
    
    while(1){

        idChannel= accept(sockfd,(struct sockaddr*)&client, &len);

        if(idChannel<0){ 
            printf("server acccept failed...\n"); 
            exit(0); 
        }else{
            printf("server acccept the client...\n"); 
        }



    }

    close(sockfd);
}

//Funcion que revisa si el archivo ya esta registrado en la 
//lista de archivos. 
//Si es nuevo archivo lo agrega a la lista y retorna 0
//Si ya estaba registrado en la lista retorna 1
int findFile(char *fileName){

    for(int i=0;i<numberOfFiles;i++){
        if(strcmp(arrFileName[i],fileName)==0){
            return 1;
        }
    }
    arrFileName[numberOfFiles]=fileName;
    numberOfFiles++;
    return 0;
}

//Hilo que revisa el directorio de archivos continuamente
void * scanDir(void *args){
    DIR *ptrDir;
    struct dirent *ptrDirent;

    while(TRUE){
        
        ptrDir = opendir("./syncFolder");
        while ((ptrDirent = readdir(ptrDir)) != NULL) {
            if(ptrDirent->d_type == DT_REG){
                char* newFileName;
                newFileName=(char*)malloc(sizeof(char)*20);
                strcpy(newFileName,ptrDirent->d_name);
                if(findFile(newFileName)==0){
                    sem_wait(sem_writer);
                        strcpy(globalName,newFileName);
                    sem_post(sem_reader);
                }
            }
        }
        closedir(ptrDir);
        usleep(500000);
    }
}

//Funcion que envia archivo al servidor
int sendFile(char *fileName){
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
    server.sin_port = htons(SERVPORT);     
    
    if (connect(sockfd, (struct sockaddr*)&server, sizeof(server)) != 0) { 
        printf("connection with the server failed 127.0.0.1:%d \n",SERVPORT); 
        return 0; 
    }else{
        printf("connected to the server 127.0.0.1:%d \n",SERVPORT); 
    }

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

    strcpy(pkg.fileName,fileName);

    int nread;

    while((nread=fread(pkg.msg,sizeof(char),MAX,fp))>0){

        pkg.id=idMsg++;
        pkg.msgLen =nread; 

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
    close(sockfd);

    return 1;
}

int main(int argc,char *argv[]){

    int serverPort;
    serverPort=atoi(argv[1]);    

    sem_writer = sem_open(SEM_WRITER_NAME, O_CREAT, S_IRUSR | S_IWUSR,1);
    sem_reader = sem_open(SEM_READER_NAME, O_CREAT, S_IRUSR | S_IWUSR,0);

    pthread_t serverTh,scanDirTh;
    pthread_create(&scanDirTh,NULL,(void*)scanDir,NULL);
    pthread_create(&serverTh,NULL,(void*)server,(void*)&serverPort);
    
    char *finalName;
    finalName = (char*)malloc(sizeof(char)*20);

    while(TRUE){
        
        sem_wait(sem_reader);
            strcpy(finalName,globalName);
        sem_post(sem_writer);

        //Envio de archivo
        if(sendFile(finalName)){
            printf("%s uploaded\n",finalName);
        }else{
            printf("error sending file\n");
        } 
    }

    return 0;
} 