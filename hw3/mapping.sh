#!/bin/bash

function parse()
{
	LANG=zh_TW.big5
	LC_ALL=zh_TW.big5

	gawk -F '[ /]' '{
						for(i=1; i<=NF; i++){
							if(!x[$1,substr($i,1,1)]++){
								len[substr($i,1,1)]++; 
								y[substr($i,1,1)][len[substr($i,1,1)]]=$1 
							}
						}
					} 
					END{
						for(i in y){ 
							printf "%s\t",i; 
							for(j in y[i])
								printf "%s ", y[i][j]; 
							print ""
						}
					}' $1
}

parse $1 >$2
