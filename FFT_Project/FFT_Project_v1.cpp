#include <stdio.h>
#include <sndfile.h>
#include <iostream>
#include <fftw3.h>
#include <math.h>

using namespace std;

#define BUFFER_LEN		1024
#define BUFFER_START	524288 - BUFFER_LEN
#define BUFFER_END		524288
#define MAX_CHANNELS	2
#define MAX_LOW_FREQ	85
#define MAX_HIGH_FREQ	255
#define PI				3.14159265358979323846


int maxRollingAvg[BUFFER_LEN];

int ReadSong_Sample(SNDFILE*, double*);
int* AnalyzeSample(fftw_complex*, int, int, int, bool);
float ConvertTodB(fftw_complex*, int);
int CalcRollingAverage(float, int, int);
void SudoAverage(int, int);
void GetAverage(int, int, fftw_plan, double*, SNDFILE*, fftw_complex*);



int maxValue = 0;
int minValue = 10000;

int main(void){
	//FILE *fp;
	SNDFILE *infile;
	SF_INFO sfinfo;

	fftw_plan plan;
	//fftw_complex* out;
	fftw_complex out[BUFFER_LEN];

	
	int *rollingAvgPtr;

	int songTime;
	int num, numRead, SamplesRead, binSize;
	int numItems, frames, sampleRate, channels;
	double* buf;
	float* beatArray;

	//Initialize rolling average
	for(int i=0; i < BUFFER_LEN; i++){
		maxRollingAvg[i] = 0;
		minRollingAvg[i] = 100;
	}

	//Try to open file
	if(!(infile = sf_open("C:\\Users\\Hegman\\Desktop\\bass.aiff", SFM_READ, &sfinfo))){
		cout<<"Not able to open input file song.wav"<<endl;
		sf_perror(NULL);
		system("PAUSE");
		return(1);
	}
	else{
		cout<<"File, song.wav, opened successfully!"<<endl;
	}

	//If there are too many channels, let the user know!
	if(sfinfo.channels > MAX_CHANNELS){
		cout<<"Not able to process more than 2 channels"<<endl;
	}

	frames = sfinfo.frames;
	sampleRate = sfinfo.samplerate;
	channels = 1;
	numItems = frames*channels;
	songTime = frames/sampleRate;

	cout<<"Frames: "<<frames<<endl;
	cout<<"Sample rate: "<<sampleRate<<endl;
	cout<<"Channels: "<<channels<<endl;
	cout<<"Num items: "<<numItems<<endl;
	cout<<"Song time (seconds): "<<songTime<<endl;

	buf = (double *)malloc(BUFFER_LEN*sizeof(double));
	if(buf==NULL) exit(1);

	plan = fftw_plan_dft_r2c_1d(BUFFER_LEN, buf, out, FFTW_ESTIMATE);

	GetAverage(sampleRate, songTime, plan, buf, infile, out); 

	//cout<<"sampleRate*songtime: "<<(sampleRate*songTime)+8<<endl;
	//cout<<"SamplesRead: "<<SamplesRead<<endl;
	
	system("PAUSE");
	for(int i=0; i<BUFFER_LEN; i++){
		cout<<"Freq: "<<(sampleRate/BUFFER_LEN)*i<<"    "<<maxRollingAvg[i]<<endl;
	}
	
	free(buf);
	sf_close(infile);
	fftw_destroy_plan(plan);
	system("PAUSE");
	return(0);
}


int ReadSong_Sample(SNDFILE* infile, double* buf){
	return(sf_read_double(infile, buf, BUFFER_LEN));
}

/*
int* AnalyzeSample(fftw_complex* out, int SamplesRead, int N, int sampleRate, bool check){
	float magdB = 0;

	for(int n=0;n<=300;n++){
		magdB = ConvertTodB(out, n);
		if(magdB < -1000 && (check)){
			cout<<"Mag: "<<magdB<<"     "<<n<<endl;
		}
		rollingAvg[n] = CalcRollingAverage(rollingAvg[n], n+1, magdB);
		if(rollingAvg[n] < -1000 && (check)){
			cout<<"Avg: "<<out[n][0]<<"     "<<n<<endl;
			system("PAUSE");
		}
	}
	
	return(rollingAvg);
}
*/
int CalcRollingAverage(float oldAvg, int n, int newValue){
	return(oldAvg * (float)(n-1)/n + (float)newValue/n);
}

void SudoAverage(int input, int indx){
	if(input > maxRollingAvg[indx]){
		maxRollingAvg[indx] = input;
	}
}

float ConvertTodB(fftw_complex* out, int indx){
	//Mag = sqrt(pow(out[0][n], 2)+pow(out[1][n], 2));
	//magdB = 20*log10f(Mag);
	return(20*log10f(sqrt(pow(out[0][indx],2)+pow(out[1][indx],2))));
}

void GetAverage(int sampleRate, int songTime, fftw_plan plan, double* buf, SNDFILE* infile, fftw_complex* out){
	int SamplesRead = 0;
	while (SamplesRead<sampleRate*songTime){
		ReadSong_Sample(infile, buf);
		sf_read_double(infile, buf, BUFFER_LEN);
		fftw_execute(plan);
		for(int i=0; i<BUFFER_LEN; i++){
			SudoAverage(out[i][0],i);
		}
		SamplesRead += BUFFER_LEN;
	}
}

