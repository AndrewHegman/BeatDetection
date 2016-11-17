#include <stdio.h>
#include <stdlib.h>
#include <fftw3.h>
#include <math.h>
#include <sndfile.h>
#include <iostream>

using namespace std;

#define BUFFER_LEN 		1024
#define MAX_CHANNELS	1
#define SONG_SOURCE		"C:\\Users\\Hegman\\Desktop\\bass.aiff"
#define CONSTANT		1.4

void PopulateWindowBuffer(int*, SNDFILE*, fftw_complex*, fftw_plan, fftw_complex*);
int CalculateAverageEnergy(int*);
void BeatDetection(int[], int, int, int*);

int main(void){
	fftw_plan plan;
	fftw_complex out[BUFFER_LEN];
	SNDFILE *infile;
	SF_INFO sfinfo;

	int averageEnergy = 0;
	int window[43];
	
	if(!(infile = sf_open(SONG_SOURCE, SFM_READ, &sfinfo))){
		printf("Not able to open input file %s.\n",SONG_SOURCE);
		sf_perror(NULL);
		system("PAUSE");
		return(1);
	}else{
		printf("File, %s, opened successfully!\n", SONG_SOURCE);
	}
	if(sfinfo.channels > MAX_CHANNELS){
		printf("\nNot able to process more than %d channels!\n", MAX_CHANNELS);
		printf("Press enter to sample song using %d channel(s)\n", MAX_CHANNELS);
		printf("else, please exit (ctrl+c)\n\n");
		system("PAUSE");
	}

	int frames = sfinfo.frames;
	int sampleRate = sfinfo.samplerate;
	int channels = 1;
	int numItems = frames*channels;
	int songTime = frames/sampleRate;
	double** instEnergyWindow = new double*[songTime];
	for(int i=0;i<songTime;i++){
		instEnergyWindow[i] = new double[11];
	}


	cout<<"Frames: "<<frames<<endl;
	cout<<"Sample rate: "<<sampleRate<<endl;
	cout<<"Channels: "<<channels<<endl;
	cout<<"Num items: "<<numItems<<endl;
	cout<<"Song time (seconds): "<<songTime<<endl;

	fftw_complex* buf = (fftw_complex*)fftw_malloc(numItems*sizeof(fftw_complex));
	if(buf==NULL){
		cout<<"Failed to allocate memory!"<<endl;
		system("PAUSE");
		exit(1);
	}
	int *beatArray = new int[songTime*43];	//Each 1/5th of a second is checked for a beat, so the array size should be songtime (in seconds) * 5
	plan = fftw_plan_dft_1d(BUFFER_LEN, buf, out, FFTW_FORWARD, FFTW_ESTIMATE);
	//plan = fftw_plan_dft_1d(BUFFER_LEN, buf, out, FFTW_FORWARD, FFTW_ESTIMATE);
	//All initialization is done

	//cout<<"Items read: "<<sf_read_double(infile, buf, numItems)<<endl;
	//for(int i=0;i<numItems;i++){
	//	buf[i] *= 1000000;
	//}
	PopulateWindowBuffer(window, infile, buf, plan, out);
	/*
	double tempSum;
	double persistentSum;
	for(int i=0;i<songTime;i++){
		tempSum = 0;
		persistentSum = 0;
		for(int j=i*44100;j<(i*44100)+44100;j++){
			tempSum += buf[j];
			persistentSum += buf[j];
			if(j%4410 == 0){		// 1/10th of a second has been processed
				instEnergyWindow[i][(j/44100)] = tempSum/4410;
				tempSum = 0;
			}
		}
		instEnergyWindow[i][0] = persistentSum/44100;
	}
	&instEnergyWindow;
	for(int i=0;i<songTime;i++){
		for(int j=i*44100;j<(i*44100)+44100;j++){
			cout<<i<<": "<<endl;
			cout<<j<<": "<<instEnergyWindow[i][j]<<endl;
		}
	}
	*/

	/*
	for(int i=0; i<songTime*43; i++){
		PopulateWindowBuffer(window, infile, buf, plan, out);
		//averageEnergy = CalculateAverageEnergy(window);
		//CalculateConstant();
		BeatDetection(window, CalculateAverageEnergy(window), CONSTANT, beatArray);
	}
	for(int i=0; i<songTime*43; i++){
		if(beatArray[i] == 1){
			cout<<i%43<<" ("<<i<<")"<<": BEAT"<<endl;
		}
	}
	*/
	fftw_free(buf);
	sf_close(infile);
	fftw_destroy_plan(plan);
	system("PAUSE");
	return(0);
	
}

void PopulateWindowBuffer(int *window, SNDFILE *infile, fftw_complex *buf, fftw_plan plan, fftw_complex *out){
	//Stores 1024 values from audio file into buf
	double* sndfile_buf = (double*)fftw_malloc(9133056*sizeof(double));
	sf_read_double(infile, sndfile_buf, 9133056);
	cout<<"Here"<<endl;
	system("PAUSE");
	for(int i=396900; i<44100*10;i++){
		*buf[i-396900] = sndfile_buf[i];
	}
	
	//Executes FFT on buf
	fftw_execute(plan);
	for(int i=0; i<512; i++){
		float mag = sqrt( (out[i][0]*out[i][0]) + (out[i][1]*out[i][1]) );
		//cout<<((44100l/512l)*i)<<"Hz: "<<sqrt(((out[i][0]*out[i][0])+(out[i][1]*out[i][1])))<<endl;
		cout<<((44100l/512l)*i)<<"Hz: "<<out[i][0]<<", "<<out[i][1]<<", "<<20*log(mag)<<endl;
		//Stores 1024 values from audio file into buf
		//sf_read_double(infile, sndfile_buf, BUFFER_LEN);

		//Executes FFT on buf
		//fftw_execute(plan);
	}
}

int CalculateAverageEnergy(int window[]){
	int sum = 0;
	for(int i=0;i<43;i++){
		sum += window[i];
	}
	return(sum/43);
}

void BeatDetection(int window[], int averageEnergy, int offset, int *beatArray){
	for(int i=0;i<43;i++){
		if(window[i]*offset > averageEnergy){
			beatArray[i] = 1;
		}else{
			beatArray[i] = 0;
		}
	}
}
