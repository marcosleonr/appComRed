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

#define SEM_PRODUCER_NAMEXT "/producer_ext"
#define SEM_CONSUMER_NAMEXT "/consumer_ext"

sem_t *sem_prodext;
sem_t *sem_consext;



FILE *fp;
sem_t *sem_arr[NSEMS];
int totalproduts;

/*
    sem_arr[0]  sem_arr[1]   productor y consumidor region critica1
    sem_arr[2]  sem_arr[3]   productor y consumidor region critica1
    sem_arr[4]  sem_arr[5]   productor y consumidor region critica2
    sem_arr[6]  sem_arr[7]   productor y consumidor region critica3
    sem_arr[8]  sem_arr[9]   productor y consumidor region critica4
    sem_arr[10] sem_arr[11]  productor y consumidor region critica5
*/

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

    while(nproducs<=10){

        for(int j=0,k=0;j<NSEMS;j+=2,k++){   

            int val1,val2;
            sem_getvalue(&sem_arr[j],&val1);
            sem_getvalue(&sem_arr[j+1],&val2);

            printf("p%d c%d\n",val1,val2);


            if(sem_trywait(sem_arr[j])!=EAGAIN){//sem productor
                
                strcpy(criticReg[k],value);
                nproducs++;
                //printf("P %s en region %d\n",criticReg[k],k);
                sem_post(sem_arr[j+1]);//sem consumidor
                
                //Con este break se sale del ciclo formar el productor
                break;
            }
        }
        break;
    }
}

void *consumer(void *args){
    int id = *((int*)args);
    while(1){

        if(totalproduts==10) pthread_exit(NULL);
        for(int j=0,k=0;j<NSEMS;j+=2,k++){   
            if(sem_trywait(sem_arr[j+1])!=EAGAIN){//sem consumidor
                
               // printf("C %d consumiendo %s en region %d\n",id,criticReg[k],k);
                totalproduts++;
                //Funcion para escribir en archivo dependiendo del
                // valor que se leyó
                write2file(criticReg[k][0]);

                sem_post(sem_arr[j]); //sem productor
                break;
            }
        }
        break;
    }
}

int main(int argc,char *argv[]){

    sem_prodext = sem_open(SEM_PRODUCER_NAMEXT, O_CREAT, S_IRUSR | S_IWUSR,2);
    sem_consext = sem_open(SEM_CONSUMER_NAMEXT, O_CREAT, S_IRUSR | S_IWUSR,0);

    char sem_name[10];
    int ini_val;

    //Creacion e inicializacion de semaforos para 6 regiones criticas
    for (int i=0;i<NSEMS;i++){  
        
        //Generacion automatica de nombres
        sprintf(sem_name,"/sem%d",i);

        //si el valor es impar entonces es un semaforo
        // productor y se inicializa con 1
        ini_val = (i%2==0)? 1:0;

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
    for(int i=0;i<1;i++){
        pthread_create(&prodth[i], NULL,(void*)producer,(void*)&args[i]);
    }

    //Identificador consumidor
    int icons[3];

    //Creacion hilos consumidor
    for(int i=0;i<1;i++){
        icons[i]=i;
        pthread_create(&consth[i],NULL,(void*)consumer,(void*)&icons[i]);
    }


    //Esperar finalizacion hilos productor
    for(int i=0;i<1;i++){
        pthread_join(prodth[i],NULL);   
    } 

    //Esperar finalizacion hilos consumidor
    for(int i=0;i<1;i++){
        pthread_join(consth[i],NULL);
    }

    //Libera recursos utilizados por el semaforo
    //for(int i=0;i<NSEMS;i++){
    //    sem_close(sem_arr[i]);
    //}
    

   

    return 0;
}