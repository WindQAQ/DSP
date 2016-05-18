#!/usr/bin/python3

import sys, re

dictionary = dict()
# read Big5-ZhuYin.map into dictionary
for line in open(sys.argv[1], "r", encoding='big5hkscs'):
	wordlist = re.split('[/\s]', line)
	word = wordlist[0]

	for x in wordlist:
		if not x:
			continue
		if x[0] not in dictionary:
			dictionary[x[0]] = set()
		dictionary[x[0]].add(word)

with open(sys.argv[2], "w", encoding='big5hkscs') as outfile:
	for key, value in dictionary.items():
		print(key, '\t', ' '.join(str(x) for x in value), file=outfile)
