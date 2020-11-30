#include<stdio.h>
int main(){
    FILE *fp;
    for (int i=0;i<10;i++){
        fp = fopen("./files/value1.txt", "a");
        fputs("\n1111",fp);
        fclose(fp);
    }
   
}