#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include<semaphore.h>
#include<unistd.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<errno.h>
#include<sys/shm.h>
#include<sys/ipc.h>
#include<string.h>

#define SEM_PRODUCER_NAME1 "/producer1"
#define SEM_CONSUMER_NAME1 "/consumer1"
#define SEM_PRODUCER_NAME2 "/producer2"
#define SEM_CONSUMER_NAME2 "/consumer2"
#define SEM_PRODUCER_NAME3 "/producer3"
#define SEM_CONSUMER_NAME3 "/consumer3"
#define SEM_PRODUCER_NAME4 "/producer4"
#define SEM_CONSUMER_NAME4 "/consumer4"
#define SEM_PRODUCER_NAME5 "/producer5"
#define SEM_CONSUMER_NAME5 "/consumer5"
#define SEM_PRODUCER_NAME6 "/producer6"
#define SEM_CONSUMER_NAME6 "/consumer6"

sem_t *sem_prod1;
sem_t *sem_cons1;
sem_t *sem_prod2;
sem_t *sem_cons2;
sem_t *sem_prod3;
sem_t *sem_cons3;
sem_t *sem_prod4;
sem_t *sem_cons4;
sem_t *sem_prod5;
sem_t *sem_cons5;
sem_t *sem_prod6;
sem_t *sem_cons6;
FILE *fp;
int totalproduts=0;
char criticReg[6][5];

void write2file(char i){
    if(i=='1'){
        fp = fopen("./files/value1.txt", "a");
        fputs("1111\n",fp);
        fclose(fp);
    }else if(i=='2'){
        fp = fopen("./files/value2.txt", "a");
        fputs("2222\n",fp);
        fclose(fp);
    }else if(i=='3'){
        fp = fopen("./files/value3.txt", "a");
        fputs("3333\n",fp);
        fclose(fp);                
    }else if(i=='4'){
        fp = fopen("./files/value4.txt", "a");
        fputs("4444\n",fp);
        fclose(fp);
    }
}

void *producer(void *args){
    
    //Valor a producir (1111,2222,3333 o 4444)
    char value[5];
    strcpy(value,(char*)args);

    int nproducs=0;

    while(nproducs<=10000){

        if(sem_trywait(sem_prod1)==0){
            strcpy(criticReg[0],value);
            nproducs++;
            //printf("P %s en region 1\n",criticReg[0]);
            sem_post(sem_cons1);     
        }else if(sem_trywait(sem_prod2)==0){
            strcpy(criticReg[1],value);
            nproducs++;
            //printf("P %s en region 2\n",criticReg[1]);
            sem_post(sem_cons2);  
        }else if(sem_trywait(sem_prod3)==0){
            strcpy(criticReg[2],value);
            nproducs++;
            //printf("P %s en region 3\n",criticReg[2]);
            sem_post(sem_cons3);  
        }else if(sem_trywait(sem_prod4)==0){
            strcpy(criticReg[3],value);
            nproducs++;
            //printf("P %s en region 4\n",criticReg[3]);
            sem_post(sem_cons4);
        }else if(sem_trywait(sem_prod5)==0){
            strcpy(criticReg[4],value);
            nproducs++;
            //printf("P %s en region 5\n",criticReg[4]);
            sem_post(sem_cons5);  
        }else if(sem_trywait(sem_prod6)==0){
            strcpy(criticReg[5],value);
            nproducs++;
            //printf("P %s en region 6\n",criticReg[5]);
            sem_post(sem_cons6);
        }

    }
}

void *consumer(void *args){
    int id = *((int*)args);
    while(1){

        if(totalproduts==40000)
            exit(0);

        if(sem_trywait(sem_cons1)==0){       
            //printf("C %d consumiendo %s en region 1\n",id,criticReg[0]);
            totalproduts++;
            write2file(criticReg[0][0]);
            sem_post(sem_prod1);
        }else if(sem_trywait(sem_cons2)==0){
            //printf("C %d consumiendo %s en region 2\n",id,criticReg[1]);
            totalproduts++;
            write2file(criticReg[1][0]);
            sem_post(sem_prod2);
        }else if(sem_trywait(sem_cons3)==0){
            //printf("C %d consumiendo %s en region 3\n",id,criticReg[2]);
            totalproduts++;
            write2file(criticReg[2][0]);
            sem_post(sem_prod3);
        }else if(sem_trywait(sem_cons4)==0){
            //printf("C %d consumiendo %s en region 4\n",id,criticReg[3]);
            totalproduts++;
            write2file(criticReg[3][0]);
            sem_post(sem_prod4);
        }else if(sem_trywait(sem_cons5)==0){
            //printf("C %d consumiendo %s en region 5\n",id,criticReg[4]);
            totalproduts++;
            write2file(criticReg[4][0]);
            sem_post(sem_prod5);
        }else if(sem_trywait(sem_cons6)==0){
            //printf("C %d consumiendo %s en region 6\n",id,criticReg[5]);
            totalproduts++;
            write2file(criticReg[5][0]);
            sem_post(sem_prod6);
        }
        
        
    }
}

int main(int argc,char *argv[]){

    sem_prod1 = sem_open(SEM_PRODUCER_NAME1, O_CREAT, S_IRUSR | S_IWUSR,1);
    sem_cons1 = sem_open(SEM_CONSUMER_NAME1, O_CREAT, S_IRUSR | S_IWUSR,0);
    sem_prod2 = sem_open(SEM_PRODUCER_NAME2, O_CREAT, S_IRUSR | S_IWUSR,1);
    sem_cons2 = sem_open(SEM_CONSUMER_NAME2, O_CREAT, S_IRUSR | S_IWUSR,0);
    sem_prod3 = sem_open(SEM_PRODUCER_NAME3, O_CREAT, S_IRUSR | S_IWUSR,1);
    sem_cons3 = sem_open(SEM_CONSUMER_NAME3, O_CREAT, S_IRUSR | S_IWUSR,0);
    sem_prod4 = sem_open(SEM_PRODUCER_NAME4, O_CREAT, S_IRUSR | S_IWUSR,1);
    sem_cons4 = sem_open(SEM_CONSUMER_NAME4, O_CREAT, S_IRUSR | S_IWUSR,0);
    sem_prod5 = sem_open(SEM_PRODUCER_NAME5, O_CREAT, S_IRUSR | S_IWUSR,1);
    sem_cons5 = sem_open(SEM_CONSUMER_NAME5, O_CREAT, S_IRUSR | S_IWUSR,0);
    sem_prod6 = sem_open(SEM_PRODUCER_NAME6, O_CREAT, S_IRUSR | S_IWUSR,1);
    sem_cons6 = sem_open(SEM_CONSUMER_NAME6, O_CREAT, S_IRUSR | S_IWUSR,0);
    
    int nprod=4,ncons=3;
    pthread_t *prodth,*consth;
    prodth = (pthread_t*)malloc(sizeof(pthread_t)*nprod);
    consth = (pthread_t*)malloc(sizeof(pthread_t)*ncons);

    //Valores a producir
    char args[4][5];
    strcpy(args[0],"1111");
    strcpy(args[1],"2222");
    strcpy(args[2],"3333");
    strcpy(args[3],"4444");    

    //Creacion hilos productor
    for(int i=0;i<nprod;i++){
        pthread_create(&prodth[i], NULL,(void*)producer,(void*)&args[i]);
    }

    //Identificador consumidor
    int icons[3];

    //Creacion hilos consumidor
    for(int i=0;i<ncons;i++){
        icons[i]=i;
        pthread_create(&consth[i],NULL,(void*)consumer,(void*)&icons[i]);
    }


    //Esperar finalizacion hilos productor
    for(int i=0;i<nprod;i++){
        pthread_join(prodth[i],NULL);   
    } 

    //Esperar finalizacion hilos consumidor
    for(int i=0;i<ncons;i++){
        pthread_join(consth[i],NULL);
    }

    //Libera recursos utilizados por el semaforo
    sem_close(sem_prod1);
    sem_close(sem_cons1);
    sem_close(sem_prod2);
    sem_close(sem_cons2);
    sem_close(sem_prod3);
    sem_close(sem_cons3);
    sem_close(sem_prod4);
    sem_close(sem_cons4);
    sem_close(sem_prod5);
    sem_close(sem_cons5);
    sem_close(sem_prod6);
    sem_close(sem_cons6);

    
    //Cerrar hilos productor
    for(int i=0;i<nprod;i++){
        free(&prodth[i]);   
    } 

    //Cerrar hilos consumidor
    for(int i=0;i<ncons;i++){
        free(&consth[i]);
    }

    return 0;
}