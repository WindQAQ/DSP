#include <stdio.h>
#include <stdlib.h>
#include "Ngram.h"
#include "Vocab.h"
#include "VocabMap.h"
#include "Trellis.cc"
#include "LHash.cc"

#ifndef LogP_Pseudo_Zero
#define LogP_Pseudo_Zero -100
#endif

typedef const VocabIndex* VocabContext;

const VocabIndex emptyContext[] = {Vocab_None};

void disambigfile(File&);
void disambigline(const VocabIndex*);

Ngram *lm;
VocabMap *map;
Vocab lmVoc, ZhuYin, Big5;

static int order = 2;

VocabIndex getIndex(const VocabIndex& widbig5)
{
	VocabIndex i = lmVoc.getIndex(map->vocab2.getWord(widbig5));
	return (i == Vocab_None)? lmVoc.getIndex(Vocab_Unknown): i;
}

void disambigfile(File& testdata)
{
	char *line = NULL;
	while(line = testdata.getline()){
		VocabString words[maxWordsPerLine];
		
		unsigned howmany = Vocab::parseWords(line, words, maxWordsPerLine);
		VocabIndex wids[maxWordsPerLine+2];

		map->vocab1.getIndices(words, &wids[1], maxWordsPerLine, map->vocab1.unkIndex());
		
		wids[0] = map->vocab1.ssIndex();	
		wids[howmany+1] = map->vocab1.seIndex();
		wids[howmany+2] = Vocab_None;
		
		disambigline(wids);
	}

	return;
}

void disambigline(const VocabIndex* wids)
{
	unsigned len = Vocab::length(wids);

	Trellis <VocabContext> trellis(len);

	/* first pos*/
	{
		VocabMapIter iter(*map, wids[0]);
		VocabIndex context[2];
		Prob prob;
		context[1] = Vocab_None;
		while(iter.next(context[0], prob))
			trellis.setProb(context, ProbToLogP(prob));
	}

	/* iterate until Vocab_None */
	unsigned pos = 1;
	while(wids[pos] != Vocab_None){
		trellis.step();

		/* iterate all Big5 to specific ZhuYin */
		VocabMapIter curIter(*map, wids[pos]);
		VocabIndex curwid;
		Prob curProb;
		// cout << ZhuYin.getWord(wids[pos]) << ": ";
		while(curIter.next(curwid, curProb)){
			// cout << Big5.getWord(curwid) << " ";
			LogP unigramProb = lm->wordProb(getIndex(curwid), emptyContext);

			VocabIndex newContext[maxWordsPerLine+2];
			newContext[0] = curwid;

			TrellisIter <VocabContext> preIter(trellis, pos - 1);
			VocabContext preContext;
			LogP preProb;
			while(preIter.next(preContext, preProb)){
				VocabIndex pre[] = {getIndex(preContext[0]), Vocab_None};
				LogP transProb = lm->wordProb(getIndex(curwid), pre);
				if(transProb == LogP_Zero && unigramProb == LogP_Zero)
					transProb = LogP_Pseudo_Zero;
				//cout << map->vocab2.getWord(preContext[0]) << ", " << map->vocab2.getWord(curwid) << ", ";
				//cout << lmVoc.getWord(curwid) << ", " << lmVoc.getIndex(Big5.getWord(curwid)) << ", ";
				//cout << ZhuYin.getWord(wids[pos]) << ", " << Big5.getWord(curwid) << ", ";
				//cout << "From: "  << (map->vocab2.use(), preContext) << ", "
				//	 << "To: " << (map->vocab2.use(), newContext) <<  ", "
				//	 << "Trans: " << transProb << endl;

				int i = 0;
				for(i = 0; i < maxWordsPerLine && preContext[i] != Vocab_None; i++)
					newContext[i+1] = preContext[i];

				newContext[i+1] = Vocab_None;

				unsigned used;
				lm->contextID(newContext, used);
				newContext[(used > 0)? used: 1] = Vocab_None;

				trellis.update(preContext, newContext, transProb);
			}
		}
		// cout << endl;
		pos++;
	}

	VocabContext* hiddenContext = new VocabContext[len+1];
	trellis.viterbi(hiddenContext, len);

	for(int i = 0; i < len; i++)
		printf("%s%s", map->vocab2.getWord(hiddenContext[i][0]), (i == len-1)? "\n": " ");
	
	delete [] hiddenContext;

	return;
}

int main(int argc, char* argv[])
{
	order = atoi(argv[8]);

{
	File lmfile(argv[6], "r");
	lm = new Ngram(lmVoc, order);
	lm->read(lmfile);
	lmfile.close();

	File mapfile(argv[4], "r");
	map = new VocabMap(ZhuYin, Big5);
	map->read(mapfile);
	mapfile.close();
}
	//VocabIndex t = lmVoc.getIndex("是");
	//VocabIndex p[] = {lmVoc.getIndex("可"), Vocab_None};

	//cout << "P(是 | 可) = " << lm->wordProb(t, p) << endl;

	File testdata(argv[2], "r");
	disambigfile(testdata);	
	testdata.close();

	delete lm;
	delete map;
	
	return 0;	
}
