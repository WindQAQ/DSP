CC=gcc
CFLAGS=-std=c99 -O2
TARGETS=train test
MODEL=model_0?.txt
RESULT=result?.txt
RM=rm -f

all: $(TARGETS)

train: train.c
	$(CC) $(CFLAGS) $< -o $@

test: test.c
	$(CC) $(CFLAGS) $< -o $@

build_model:
	$(CC) $(CFLAGS) train.c hmm.h -o train
	./train 1 model_init.txt seq_model_01.txt model_01.txt	
	./train 1 model_init.txt seq_model_02.txt model_02.txt	
	./train 1 model_init.txt seq_model_03.txt model_03.txt	
	./train 1 model_init.txt seq_model_04.txt model_04.txt	
	./train 1 model_init.txt seq_model_05.txt model_05.txt	

run_test:
	$(CC) $(CFLAGS) test.c hmm.h -o test
	./test modellist.txt testing_data1.txt result1.txt

accuracy:
	paste result1.txt testing_answer.txt | awk '$$1 == $$3 {cnt++} END {print cnt/2500}'

clean: 
	$(RM) $(TARGETS) $(RESULT) $(MODEL)
