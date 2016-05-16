# The following two variable will be commandline determined by TA
# For testing, you could uncomment them.
# SRIPATH ?= /data/DSP_HW3/103_2/srilm-1.5.10
# MACHINE_TYPE ?= i686-m64
SRIPATH ?= $(shell pwd)/srilm-1.5.10
MACHINE_TYPE ?= i686-m64
LM ?= bigram.lm

CXX = g++
CXXFLAGS = -O3 -I$(SRIPATH)/include -w
vpath lib%.a $(SRIPATH)/lib/$(MACHINE_TYPE)

TARGET = mydisambig
TARGETV2 = mydisambig_v2
SRC = mydisambig.cpp
OBJ = $(SRC:.cpp=.o)
OBJV2 = mydisambig_v2.o
MAP = mapping.sh
TO = ZhuYin-Big5.map
FROM = Big5-ZhuYin.map
RESULT1 = result1
RESULT2 = result2
RESULT3 = result3
.PHONY: all clean map run

all: $(TARGET)

$(TARGET): $(OBJ) -loolm -ldstruct -lmisc
	$(CXX) $(LDFLAGS) -o $@ $^

$(TARGETV2): $(OBJV2) -loolm -ldstruct -lmisc
	$(CXX) $(LDFLAGS) -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $<

run: $(RESULT2)

$(RESULT1): disambig
	@echo "Check directory $@";
	@if [ ! -d $@ ]; then \
		mkdir $@; \
	fi;
	@for i in $(shell seq 1 10) ; do \
		echo "Running $$i.txt"; \
		./$< -text $$i.txt -map $(TO) -lm $(LM) -order 2 > $@/$$i.txt; \
	done;

$(RESULT2): $(TARGET)
	@echo "Check directory $@";
	@if [ ! -d $@ ]; then \
		mkdir $@; \
	fi;
	@for i in $(shell seq 1 10) ; do \
		echo "Running $$i.txt"; \
		./$< -text $$i.txt -map $(TO) -lm $(LM) -order 2 > $@/$$i.txt; \
	done;

$(RESULT3): $(TARGETV2)
	@echo "Check directory $@";
	@if [ ! -d $@ ]; then \
		mkdir $@; \
	fi;
	@for i in $(shell seq 1 10) ; do \
		echo "Running $$i.txt"; \
		./$< -text $$i.txt -map $(TO) -lm $(LM) -order 2 > $@/$$i.txt; \
	done;

map: $(MAP)
	./$< $(FROM) $(TO)

prepare: parse_testdata map ngram

ngram: $(MAP) parse_testdata
	./ngram-count -text corpus_seg.txt -write lm.cnt -order 2
	./ngram-count -read lm.cnt -lm bigram.lm -unk -order 2

parse_testdata:
	@for i in $(shell seq 1 10); do \
		echo "Separate $$i.txt"; \
		./separator_big5.pl testdata/$$i.txt >$$i.txt; \
	done;

clean:
	$(RM) $(OBJ) $(TARGET)

cleaner: 
	$(RM) $(OBJ) $(TARGET) $(OBJV2) $(TARGETV2)

cleanest:
	$(RM) $(OBJ) $(TARGET) $(OBJV2) $(TARGETV2)
	$(RM) -r $(RESULT2) $(RESULT3)
	$(RM) lm.cnt bigram.lm
	$(RM) $(TO)
