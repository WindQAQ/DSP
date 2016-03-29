#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hmm.h"

typedef struct{
	int sample_num;
	double init[MAX_STATE];
	double tn[MAX_STATE][MAX_STATE];
	double td[MAX_STATE][MAX_STATE];
	double on[MAX_OBSERV][MAX_STATE];
	double od[MAX_OBSERV][MAX_STATE];
}P;

void initialP(P* p)
{
	p->sample_num = 0;
	memset(p->init, 0.0, sizeof(p->init));
	memset(p->tn, 0.0, sizeof(p->tn));
	memset(p->td, 0.0, sizeof(p->td));
	memset(p->on, 0.0, sizeof(p->on));
	memset(p->od, 0.0, sizeof(p->od));
}

void forward(const HMM *hmm, const char* seq, double al[MAX_SEQ][MAX_STATE])
{
	int length = strlen(seq);
	
	// initialize alpha
	for(int i = 0; i < hmm->state_num; i++)
		al[0][i] = hmm->initial[i] * hmm->observation[seq[0]-'A'][i];

	for(int t = 0; t < length - 1; t++)
		for(int j = 0; j < hmm->state_num; j++)
			for(int i = 0; i < hmm->state_num; i++)
				al[t+1][j] += al[t][i] * hmm->transition[i][j] * hmm->observation[seq[t+1]-'A'][j];

	return;
}

void backward(const HMM *hmm, const char* seq, double be[MAX_SEQ][MAX_STATE])
{
	int length = strlen(seq);
		
	//initialize beta
	for(int i = 0; i < hmm->state_num; i++)
		be[length-1][i] = 1.0;

	for(int t = length - 2; t >= 0; t--)
		for(int i = 0 ; i < hmm->state_num; i++)
			for(int j = 0 ; j < hmm->state_num; j++)
				be[t][i] += hmm->transition[i][j] * hmm->observation[seq[t+1]-'A'][j] * be[t+1][j];
	
	return;
}

void forward_backward(const HMM* hmm, const char* seq, 
					double al[MAX_SEQ][MAX_STATE], double be[MAX_SEQ][MAX_STATE], P* p)
{
	double ga[MAX_SEQ][MAX_STATE] = {{0.0}}; // gamma
	double ep[MAX_SEQ][MAX_STATE][MAX_STATE] = {{{0.0}}}; // epsilon

	int length = strlen(seq);
	
	// calculate gamma
	for(int t = 0 ; t < length; t++){
		double gd = 0.0;
		for(int i = 0; i < hmm->state_num; i++){
			double gn = al[t][i] * be[t][i];
			ga[t][i] = gn;
			gd += gn;
		}

		for(int i = 0; i < hmm->state_num; i++)
			ga[t][i] /= gd;
	}

	// calculate epsilon
	for(int t = 0; t < length - 1; t++){
		double ed = 0.0;
		for(int i = 0; i < hmm->state_num; i++)
			for(int j = 0; j < hmm->state_num; j++){
				double en = al[t][i] * hmm->transition[i][j] * hmm->observation[seq[t+1]-'A'][j] * be[t+1][j];
				ep[t][i][j] = en;
				ed += en;
			}
		
		for(int i = 0; i < hmm->state_num; i++)
			for(int j = 0; j < hmm->state_num; j++)
				ep[t][i][j] /= ed;
	}
	
	p->sample_num++;
	// acculmulate
	for(int i = 0; i < hmm->state_num; i++)
		p->init[i] += ga[0][i];

	for(int i = 0; i < hmm->state_num; i++)
		for(int j = 0; j < hmm->state_num; j++)
			for(int t = 0; t < length - 1; t++){
				p->tn[i][j] += ep[t][i][j];
				p->td[i][j] += ga[t][i];
			}

	for(int k = 0; k < hmm->observ_num; k++)
		for(int i = 0; i < hmm->state_num; i++)
			for(int t = 0; t < length; t++){
				if(seq[t] - 'A' == k)
					p->on[k][i] += ga[t][i];
				p->od[k][i] += ga[t][i];
			}
	
	return;
}

void updateHMM(HMM* hmm, const P* p)
{
	for(int i = 0; i < hmm->state_num; i++)
		hmm->initial[i] = p->init[i] / p->sample_num;

	for(int i = 0; i < hmm->state_num; i++)
		for(int j = 0; j < hmm->state_num; j++)
			hmm->transition[i][j] = p->tn[i][j] / p->td[i][j];
	
	for(int i = 0; i < hmm->observ_num; i++)
		for(int j = 0; j < hmm->state_num; j++)
			hmm->observation[i][j] = p->on[i][j] / p->od[i][j];

	/* double s = 0.0;
	for(int i = 0; i <hmm->state_num; i++)
		s += hmm->initial[i];
	printf("initail sum : %f\n", s);

	for(int i = 0; i < hmm->state_num; i++){
		double sum = 0.0;
		for(int j = 0; j < hmm->state_num; j++)
			sum += hmm->transition[i][j];
		printf("rowsum[%d]: %f\n", i, sum);
	}	

	for(int i = 0; i < hmm->state_num; i++){
		double sum = 0.0;
		for(int j = 0; j < hmm->observ_num; j++)
			sum += hmm->observation[j][i];
		printf("colsum[%d]: %f\n", i, sum);		
	} */
		
	return;
}

void train_model(HMM* hmm, const char* seq_model_name)
{
	FILE* seq_model = open_or_die(seq_model_name, "r");
	char seq[MAX_SEQ] = "";
	P p;
	
	initialP(&p);

	while(fscanf(seq_model, "%s", seq) != EOF){
		double al[MAX_SEQ][MAX_STATE] = {{0.0}}; // alpha for forward algorithm
		double be[MAX_SEQ][MAX_STATE] = {{0.0}}; // beta for backward algorithm

		forward(hmm, seq, al);
		backward(hmm, seq, be);
		forward_backward(hmm, seq, al, be, &p);
	}

	updateHMM(hmm, &p);
	
	fclose(seq_model);

	return;
}

int main(int argc, char* argv[])
{
	int iteration = atoi(argv[1]);

	HMM hmm;
	loadHMM(&hmm, argv[2]);

	for(int i = 0; i < iteration; i++)
		train_model(&hmm, argv[3]);

	FILE* model = open_or_die(argv[4], "w");
	dumpHMM(model, &hmm);
	fclose(model);

	return 0;
}
