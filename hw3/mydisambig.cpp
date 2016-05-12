#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <vector>
#include "Ngram.h"
#include "VocabMap.h"

#ifndef MAXWORDS
#define MAXWORDS 256
#endif

#ifndef FAKE_ZERO
#define FAKE_ZERO -10000
#endif

const VocabIndex emptyContext[] = {Vocab_None};

void recognize_file(Ngram& lm, VocabMap& map, File& testdata);
LogP Viterbi(Ngram& lm, VocabMap& map, VocabString* words, unsigned count);
VocabIndex getIndex(VocabString w);

static int ngram_order;
static Vocab voc; // Vocabulary in Language Model
static Vocab vocZ, vocB; // Vocabulary ZhuYin and Big5 in ZhuYin-to-Big5 map

VocabIndex getIndex(VocabString w)
{
	VocabIndex wid = voc.getIndex(w);
	return (wid == Vocab_None)? voc.getIndex(Vocab_Unknown): wid; 
}

LogP Viterbi(Ngram& lm, VocabMap& map, VocabString* words, unsigned count)
{
	LogP Pro[MAXWORDS][1024] = {0.0};
	VocabIndex BackTrack[MAXWORDS][1024];
	VocabIndex IntToIndex[MAXWORDS][1024];
	int size[MAXWORDS] = {0};

	/* Viterbi Algorithm initial */
{
	Prob p;
	VocabIndex w;
	VocabMapIter iter(map, vocZ.getIndex(words[0]));
	for(int i = 0; iter.next(w, p); size[0]++, i++){
		VocabIndex index = getIndex(vocB.getWord(w));
		LogP curP = lm.wordProb(index, emptyContext);
		if(curP == LogP_Zero) curP = FAKE_ZERO;
		Pro[0][i] = curP;
		IntToIndex[0][i] = w;
		BackTrack[0][i] = 0;
		// printf("start: %s, p: %e\n", vocB.getWord(w), curP);
	}
}

	/* Viterbi Algorithm iteratively solve  */
	for(int t = 1; t < count; t++){
		Prob p;
		VocabIndex w;
		VocabMapIter iter(map, vocZ.getIndex(words[t]));
		for(int j = 0; iter.next(w, p); size[t]++, j++){
			VocabIndex index = getIndex(vocB.getWord(w));
			LogP maxP = -1.0/0.0, pre;
			for(int i = 0; i < size[t-1]; i++){
				VocabIndex context[] = {getIndex(vocB.getWord(IntToIndex[t-1][i])), Vocab_None};
				LogP curP = lm.wordProb(index, context);
				LogP unigramP = lm.wordProb(index, emptyContext);
				
				if(curP == LogP_Zero && unigramP == LogP_Zero)
					curP = FAKE_ZERO;

				//if(t==1)
				//	printf("word: %s, pre: %s, cur: %.6f, pre: %.6f --> %.6f\n", vocB.getWord(w), vocB.getWord(IntToIndex[t-1][i]), curP, Pro[t-1][i], curP+Pro[t-1][i]);

				curP += Pro[t-1][i];		
		
				if(curP > maxP){
					maxP = curP;
					BackTrack[t][j] = i;
				//if(t==1)
				//	printf("in, word: %s, pre: %s, cur: %.6f, pre: %.6f, max: %.6f\n", vocB.getWord(w), vocB.getWord(IntToIndex[t-1][i]), curP, Pro[t-1][i], curP);
				}
			}
			Pro[t][j] = maxP;
			IntToIndex[t][j] = w;
		}
	}

	/* maximize probability */
	LogP maxP = -1.0/0.0;
	int end = -1;
	for(int i = 0; i < size[count-1]; i++){
		LogP curP = Pro[count-1][i];
		if(curP > maxP){
			maxP = curP;
			end = i;
		}
	}

	/* BackTrack from end */
	int path[MAXWORDS];
	path[count-1] = end;
	for(int t = count-2; t >= 0; t--)
		path[t] = BackTrack[t+1][path[t+1]];

	for(int t = 0; t < count; t++)
		printf("%s ", vocB.getWord(IntToIndex[t][path[t]]));

	return maxP;
}

void recognize_file(Ngram& lm, VocabMap& map, File& testdata)
{
	char* line = NULL;
	while(line = testdata.getline()){
		VocabString WordsInLine[MAXWORDS];
		unsigned count = Vocab::parseWords(line, WordsInLine, maxWordsPerLine); 

		printf("%s ", Vocab_SentStart); // <s> at the beginning
		LogP MaxP = Viterbi(lm, map, WordsInLine, count);
		printf("%s\n", Vocab_SentEnd); // </s> at the end
	}
}

int main(int argc, char *argv[])
{
    ngram_order = atoi(argv[8]);

	Ngram lm(voc, ngram_order);	
	VocabMap map(vocZ, vocB);
	
	/* read Language Model and ZhuYin-to-Big5 map */
    {
        File lmFile(argv[6], "r" );
        lm.read(lmFile);
        lmFile.close();

		File mapfile(argv[4], "r");
		map.read(mapfile);
		mapfile.close();
    }
	
	/* recognize testdata (argv[2])*/
	File testdata(argv[2], "r");

	recognize_file(lm, map, testdata);
	
	testdata.close();
 
	return 0;
}

