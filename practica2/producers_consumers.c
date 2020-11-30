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

#define NSEMS 12
#define NCRITICR 6

sem_t *sem_arr[NSEMS];

/*
    sem_arr[0]  sem_arr[1]   productor y consumidor region critica1
    sem_arr[2]  sem_arr[3]   productor y consumidor region critica1
    sem_arr[4]  sem_arr[5]   productor y consumidor region critica2
    sem_arr[6]  sem_arr[7]   productor y consumidor region critica3
    sem_arr[8]  sem_arr[9]   productor y consumidor region critica4
    sem_arr[10] sem_arr[11]  productor y consumidor region critica5
*/

/*
    sem_arr[2]  sem_arr[3]   productor y consumidor region critica1
    sem_arr[4]  sem_arr[5]   productor y consumidor region critica2
    sem_arr[6]  sem_arr[7]   productor y consumidor region critica3
    sem_arr[8]  sem_arr[9]   productor y consumidor region critica4
    sem_arr[10] sem_arr[11]  productor y consumidor region critica5
    sem_arr[12] sem_arr[13]  productor y consumidor region critica6
*/

char criticReg[6][5];

void *producer(void *args){
    
    //Valor a producir (1111,2222,3333 o 4444)
    char value[5];
    strcpy(value,(char*)args);
    int nproducs=0;

    while(nproducs<=10000){
        for(int j=0,k=0;j<NSEMS;j+=2,k++){   
            if(sem_trywait(sem_arr[j])!=EAGAIN){
                
                strcpy(criticReg[k],value);
                nproducs++;
                printf("Productor %s en region %d",criticReg[k],k);
                sem_post(sem_arr[j+1]);
                
                //Con este break se sale del ciclo formar el productor
                break;
            }
        }
    }
}

void *consumer(void *args){

}

int main(int argc,char *argv[]){

    char sem_name[10];

    int ini_val=0;

    //Creacion e inicializacion de semaforos para 6 regiones criticas
    for (int i=0;i<NSEMS;i++){  
        sprintf(sem_name,"/sem%d",i);
        sem_arr[i] = sem_open((const char*)sem_name, O_CREAT, S_IRUSR | S_IWUSR,ini_val);

        if(sem_arr[i]==SEM_FAILED){
            printf("Error creacion sem%d",i);
            exit(-1);
        }
    }

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

    //Esperar finalizacion hilos productor
    for(int i=0;i<nprod;i++){
        pthread_join(prodth[i],NULL);   
    } 

    return 0;
}