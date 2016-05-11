# The following two variable will be commandline determined by TA
# For testing, you could uncomment them.
# SRIPATH ?= /data/DSP_HW3/103_2/srilm-1.5.10
# MACHINE_TYPE ?= i686-m64
SRIPATH = $(shell pwd)/srilm-1.5.10
MACHINE_TYPE ?= i686-m64
LM ?= bigram.lm

CXX = g++
CXXFLAGS = -O3 -I$(SRIPATH)/include -w
vpath lib%.a $(SRIPATH)/lib/$(MACHINE_TYPE)

TARGET = mydisambig
SRC = mydisambig.cpp
OBJ = $(SRC:.cpp=.o)
TO = ZhuYin-Big5.map
FROM = Big5-ZhuYin.map
RESULT1=result1
RESULT2=result2
.PHONY: all clean map run

all: $(TARGET)

$(TARGET): $(OBJ) -loolm -ldstruct -lmisc
	$(CXX) $(LDFLAGS) -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $<

$(RESULT2): $(TARGET)
	@echo "Check directory $@";
	@if [ ! -d $@ ]; then \
		mkdir $@; \
	fi;
	@for i in $(shell seq 1 10) ; do \
		echo "Running $$i.txt"; \
		./mydisambig -text $$i.txt -map $(TO) -lm $(LM) -order 2 > $@/$$i.txt; \
	done;

$(RESULT1): disambig
	@echo "Check directory $@";
	@if [ ! -d $@ ]; then \
		mkdir $@; \
	fi;
	@for i in $(shell seq 1 10) ; do \
		echo "Running $$i.txt"; \
		./disambig -text $$i.txt -map $(TO) -lm $(LM) -order 2 > $@/$$i.txt; \
	done;

prepare: parse_testdata map ngram

ngram: map parse_testdata

map: mapping.sh
	./$< $(FROM) $(TO)

parse_testdata:
	@for i in $(shell seq 1 10); do \
		echo "Separate $$i.txt"; \
		./separator_big5.pl testdata/$$i.txt >$$i.txt; \
	done;

clean:
	$(RM) $(OBJ) $(TARGET)
	$(RM) -r $(RESULT1) $(RESULT2)
