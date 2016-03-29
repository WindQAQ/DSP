#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hmm.h"

double viterbi(const HMM* hmm, const char* seq)
{
	double de[MAX_SEQ][MAX_STATE] = {{0.0}};	// delta for viterbi
	
	// initialize delta
	for(int i = 0; i < hmm->state_num; i++)
		de[0][i] = hmm->initial[i] * hmm->observation[seq[0]-'A'][i];

	int length = strlen(seq);
	for(int t = 1; t < length; t++){
		for(int j = 0; j < hmm->state_num; j++){
			double max = -1.0;
			for(int i = 0; i < hmm->state_num; i++){
				double cur = de[t-1][i] * hmm->transition[i][j];
				if(max < cur)
					max = cur;
			}
			de[t][j] = max * hmm->observation[seq[t]-'A'][j];
		}
	}
	
	double maxP = -1.0;
	for(int i = 0; i < hmm->state_num; i++)
		if(maxP < de[length-1][i])
			maxP = de[length-1][i];
	
	return maxP;
}

int main(int argc, char* argv[])
{
	HMM models[MAX_LINE];

	int model_cnt = load_models(argv[1], models, MAX_LINE);
	
	char seq[MAX_SEQ] = "";
	FILE* data = open_or_die(argv[2], "r");
	FILE* result = open_or_die(argv[3], "w");
	
	while(fscanf(data, "%s", seq) != EOF){
		int bestM = -1;
		double maxP = -1.0;
		for(int i = 0; i < model_cnt; i++){
			double curP = viterbi(&models[i], seq);
			if(maxP < curP){
				maxP = curP;
				bestM = i;
			}
		}	
		fprintf(result, "%s %e\n", models[bestM].model_name, maxP);
	}

	fclose(data);
	fclose(result);

	return 0;
}
