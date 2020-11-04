#include<opencv2/core.hpp>
#include<opencv2/imgcodecs.hpp>
#include<opencv2/highgui.hpp>
#include<opencv2/imgproc.hpp>
#include<iostream>

#include<pthread.h>

using namespace cv;
using std::cout;
using std::endl;


void * func(void * arg){

	int n=*((int*)arg);

	char pathmain[30]="./main/";
	char pathb[30]="./blue/";
	char pathg[30]="./green/";
	char pathr[30]="./red/";

	char fname[20]="img", ext[5]=".jpg";
	
	char num[5];
    sprintf(num,"%d",n);

    /*path recurso origen*/
	strcat(fname,num);
	strcat(fname,ext);
	strcat(pathmain,fname);

	/*path recurso destino*/
	strcat(pathb,fname);
	strcat(pathg,fname);
	strcat(pathr,fname);

	/*lectura de la imagen en formato BGR*/
	Mat img=imread(pathmain,IMREAD_COLOR);

	if(img.empty()){
		cout<<"could not read the image: "<<fname<<endl;
		exit(1);
	}

	/*Se realian tres copias de la imagen*/
	Mat img1 = img.clone();
	Mat img2 = img.clone();
	Mat img3 = img.clone();


	for (int row = 0; row < img1.rows; row++){
  		for (int col = 0; col < img1.cols; col++){
        	/*dependiento de la imagen se satura el canal Blue, Green o Red*/ 
        	img1.at<Vec3b>(row, col)[0] = 255;
			img2.at<Vec3b>(row, col)[1] = 255;
			img3.at<Vec3b>(row, col)[2] = 255;
   		}		
	}

	/*se almacena la nueva imagen en su respectiva carpeta, conservando el
	formato de origen*/

	imwrite(pathb,img1);
	imwrite(pathg,img2);
	imwrite(pathr,img3);
	
}

int main(){
	int nimg=15;
	
	pthread_t *threads;
    threads = (pthread_t *)malloc(sizeof(pthread_t)*nimg);

	int nums[nimg];
	for(int i=0;i<nimg;i++)nums[i]=i;

	for(int i=0;i<nimg;i++){
		pthread_create(&threads[i], NULL, func,(void*)&nums[i]);
	}

	for(int i=0; i<nimg; i++){
    	pthread_join(threads[i], NULL);
    }

    free(threads);
}