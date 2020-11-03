#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>


int **m1,**m2,**m3;

int m1r,m1c,m2r,m2c,m3r,m3c;

typedef struct {
    int l;
    int h;
}pair;

void * function(void * parametro){

    pair *pr = (pair*)parametro;

    int l=pr->l,h=pr->h,c2=0;

    //printf("%d %d\n",l,h);

    for(int i=l;i<=h;i++){
        for(int j=0;j<m2c;j++){
            for(int k=0;k<m1c;k++){
               m3[i][j]+=m1[i][k]*m2[k][j];
            }
            //printf(" %d %d\n",i,j);
        }
    }

}

int main(int argc, char *argv[]){
  
    //time_t t;
    //srand((unsigned) time(&t));

    int nt,rxt,r,l=0,h=0;

    if(argc!=6){
        printf("parametros incompletos\n");
        exit(0);
    }

    m1r=atoi(argv[1]);
    m1c=atoi(argv[2]);
    m2r=atoi(argv[3]);
    m2c=atoi(argv[4]);
    m3r=m1r;
    m3c=m2c;

    nt=atoi(argv[5]);

    if(nt<1 || nt>m1r){
        printf("error en el # de hilos");
        exit(0);
    }else if(m1c!=m2r){
        printf("error en la longitud de las tablas");
        exit(0);        
    }

    m1 = (int **)malloc(sizeof(int*)*m1r);
    m2 = (int **)malloc(sizeof(int*)*m2r);
    m3 = (int **)malloc(sizeof(int*)*m3r);

    for(int i=0;i<m1r;i++){
        m1[i]=(int*)malloc(sizeof(int)*m1c);
        for(int j=0;j<m1c;j++){
            int rr=rand()%6;
            m1[i][j]=rr;
            printf("%d ",m1[i][j]);
        }
        printf("\n");
    }

    printf("\n");

    for(int i=0;i<m2r;i++){
        m2[i]=(int*)malloc(sizeof(int)*m2c);
        for(int j=0;j<m2c;j++){
            int rr=rand()%6;
            m2[i][j]=rr;
            printf("%d ",m2[i][j]);
        }
        printf("\n");
    }

    for(int i=0;i<m3r;i++){
        m3[i]=(int*)malloc(sizeof(int)*m3c);
        for(int j=0;j<m3c;j++){
            m3[i][j]=0;
        }
    }  

    rxt=m1r/nt;
    r=m1r%nt;

    pthread_t *threads;
    threads = (pthread_t *)malloc(sizeof(pthread_t)*nt);

    pair * pairs;
    pairs = (pair*)malloc(sizeof(pair)*nt);


    printf("\n");

    for(int i=0;i<nt;i++){

        if(i==0){
            l=0;
            h+=(rxt+r-1);
        }else{
            l=h+1;
            h+=rxt;
        }
        pairs[i].l=l;
        pairs[i].h=h;
        pthread_create(&threads[i], NULL, function,(void*)&pairs[i]);
    }

    for(int i=0; i<nt; i++){
        pthread_join(threads[i], NULL);
    }

    printf("\n");

    for(int i=0;i<m3r;i++){
        for(int j=0;j<m3c;j++){
            printf("%d ",m3[i][j]);
        }
        printf("\n");
    }  

    free(threads);

    exit(0);
}
