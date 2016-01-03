#!/bin/bash

for ((i=0; i<10000; i++)) 
do
   cat 5cols.csv  >> 5cols_populate.csv;
done;

