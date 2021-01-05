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

#define SEM_WRITER_NAME "/swriter"
#define SEM_READER_NAME "/sreader"

#define MAX 1500
#define SERVPORT 7200
#define TRUE 1
#define ARRSIZE 30
#define IPLEN 16

char *arrFileName[ARRSIZE];
char *recFileName[ARRSIZE];
int numberOfFiles;
int numberOfRecFiles;
int sockfd;
int localServerPort;//Puerto del servidor 
char globalName[40];
int scanTerminalFlag=1;

sem_t * sem_writer;
sem_t * sem_reader;

typedef struct{
    int id;
    int msgLen;
    char fileName[40];
    char msg[MAX];    
}Package; 

typedef struct{
    int serverPort;
    char ip[IPLEN];
}serverData;


void * server(void *args){

    int sockfd,idChannel,len,flag=1;
    struct sockaddr_in server,client;
        
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
    server.sin_port = htons(localServerPort); 

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

        char path[30]="./syncFolder/";
        FILE *fp;
        Package pkg;
        int banFile=1;
    
        while(TRUE){

            bzero(pkg.msg, MAX); 

            int nbytesr=recvfrom(idChannel,&pkg,sizeof(Package),0,NULL,NULL);

            if(pkg.id==-1){
                printf("%s received\n",pkg.fileName);
                break;
            }

            if(banFile){
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
                banFile=0; 
            }    

            fwrite(pkg.msg,sizeof(char),pkg.msgLen,fp);

            bzero(pkg.msg, MAX);
            pkg.id++;

            int nbytess=sendto(idChannel,&pkg,sizeof(Package),0,NULL,0);
        }
        fclose(fp);
        close(idChannel);
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

    for(int i=0;i<numberOfRecFiles;i++){
        if(strcmp(recFileName[i],fileName)==0){
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
                newFileName=(char*)malloc(sizeof(char)*40);
                strcpy(newFileName,ptrDirent->d_name);
                if(findFile(newFileName)==0){
                    strcpy(globalName,newFileName);
                    scanTerminalFlag=0;
                    //printf("se esta enviando el archivo %s\n",globalName);
                    while(scanTerminalFlag==0);
                }
            }
        }
        closedir(ptrDir);
        usleep(500000);
    }
}

//Funcion que envia archivo al servidor
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

    Package pkg;

    strcpy(pkg.fileName,fileName);

    int nread;

    while((nread=fread(pkg.msg,sizeof(char),MAX,fp))>0){

        pkg.id=idMsg++;
        pkg.msgLen =nread; 

        int nbytess=sendto(sockfd,&pkg,sizeof(Package),0,NULL,0);
            
        bzero(pkg.msg,MAX); 

        int nbytesr=recvfrom(sockfd,&pkg,sizeof(Package),0,NULL,NULL);

        //printf("idclient %d  idserv %d\n",idMsg,pkg.id);
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

void * scanTerminal(void * args){
    char str[40];
    //while(TRUE){
        bzero(str,40);
        scanf("%s",str);
        if(strncmp(str,"exit",4)==0){
            Package final;
            final.id=-2;
            int nbytess=sendto(sockfd,&final,sizeof(Package),0,NULL,0);
            close(sockfd);
            printf("closed conection");
            exit(0);
        }

}

int main(int argc,char *argv[]){

    localServerPort=atoi(argv[1]);    

    sem_writer = sem_open(SEM_WRITER_NAME, O_CREAT, S_IRUSR | S_IWUSR,1);
    sem_reader = sem_open(SEM_READER_NAME, O_CREAT, S_IRUSR | S_IWUSR,0);

    pthread_t serverTh,scanDirTh,scanTerminalTh;
    pthread_create(&scanDirTh,NULL,(void*)scanDir,NULL);
    pthread_create(&serverTh,NULL,(void*)server,NULL);
    pthread_create(&scanTerminalTh,NULL,(void*)scanTerminal,NULL);
    
    int flag=1;
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

    struct sockaddr_in local_address;
    int addr_size = sizeof(local_address);
    getsockname(sockfd, (struct sockaddr *)&local_address, &addr_size);
    int localport;
    localport=htons(local_address.sin_port);
    //printf("local port %d\n",localport);

    //Envio de primer paquete con datos del servidor que esta alojado en el cliente    
    serverData sD;
    sD.serverPort = localServerPort;
    strcpy(sD.ip,"192.168.1.93");
    int nbytess=sendto(sockfd,&sD,sizeof(serverData),0,NULL,0);
    //Recepcion confirmacion del servidor
    int nbytesr=recvfrom(sockfd,&sD,sizeof(serverData),0,NULL,NULL);
    
    if(sD.serverPort!=-1){
        printf("error localserver info");
        exit(0);
    }else{
        //printf("localserver info\n");
    }

    char *finalName;
    finalName = (char*)malloc(sizeof(char)*40);

    while(TRUE){
        
        //sem_wait(sem_reader);
        while(scanTerminalFlag);
        strcpy(finalName,globalName);
        //sem_post(sem_writer);

        //Envio de archivo
        if(sendFile(finalName)){
            printf("%s send\n",finalName);
        }else{
            printf("error sending file\n");
        } 
        scanTerminalFlag=1;
    }

    return 0;
} 