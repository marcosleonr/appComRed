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

	strcat(fname,num);
	strcat(fname,ext);

	strcat(pathmain,fname);

	Mat img=imread(pathmain);

	Mat img1 = img.clone();
	Mat img2 = img.clone();
	Mat img3 = img.clone();

	for (int row = 0; row < img1.rows; row++){
  		for (int col = 0; col < img1.cols; col++){
        	img1.at<Vec3b>(row, col)[0] = 255;
			img2.at<Vec3b>(row, col)[1] = 255;
			img3.at<Vec3b>(row, col)[2] = 255;
   		}		
	}

	strcat(pathb,fname);
	strcat(pathg,fname);
	strcat(pathr,fname);

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
}
/*

compilar
g++ `pkg-config --cflags opencv` main.cpp `pkg-config --libs opencv` -lpthread
ejecutar
./a.out


imshow("window",img);
	waitKey(0);
	//imshow("window",img);
	//waitKey(0);
	//imwrite("img1.jpg",img);
header
https://stackoverflow.com/questions/46426249/opencv-modify-image-pixels/46426440
https://riptutorial.com/opencv/example/28276/pixel-by-pixel-modification-of-images
https://stackoverflow.com/questions/12461848/accessing-pixel-value-using-cvmatat-operator
https://www.youtube.com/watch?v=Kur8JL2GqWQ&list=PLZBN9cDu0MSlHMjJughb11ydICWbA8OBe&index=3

	Mat img=imread("img.jpg");
	Mat img1 = img.clone();
	Mat img2 = img.clone();
	Mat img3 = img.clone();

	for (int row = 0; row < img1.rows; row++){
  		for (int col = 0; col < img1.cols; col++){
        	img1.at<Vec3b>(row, col)[0] = 255;
			img2.at<Vec3b>(row, col)[1] = 255;
			img3.at<Vec3b>(row, col)[2] = 255;
   		}		
	}


	imwrite("img1.jpg",img1);
	imwrite("img2.jpg",img2);
	imwrite("img3.jpg",img3);
*/
