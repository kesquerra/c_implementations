#!/bin/bash

declare -A MATRIXL
declare -A MATRIXR

function readLeft()
{
   local ROW=0
   local COL=0
   local LINE
   local VAL

   while read LINE
   do
      COL=0
      for VAL in ${LINE}
      do
	 MATRIXL[${ROW},${COL}]=${VAL}
	 ((COL++))
      done
      ((ROW++))
   done < $1

   MATRIXL[ROWS]=${ROW}
   MATRIXL[COLS]=${COL}
}

function readRight()
{
   local ROW=0
   local COL=0
   local LINE
   local VAL

   while read LINE
   do
      COL=0
      for VAL in ${LINE}
      do
	 MATRIXR[${ROW},${COL}]=${VAL}
	 ((COL++))
      done
      ((ROW++))
   done < $1

   MATRIXR[ROWS]=${ROW}
   MATRIXR[COLS]=${COL}
}

if [ $# -lt 1 ]
then
   echo "not enough arguments"
   exit 1
fi

if [ $1 = "dims" ]
then
   readLeft $2
   echo "${MATRIXL[ROWS]} ${MATRIXL[COLS]}"

   exit 0
fi

if [ $1 = "mean" ]
then
   declare -a MEAN

   readLeft $2

   for (( ROW=0 ; ROW<${MATRIXL[ROWS]} ; ROW++ ))
   do
      for (( COL=0 ; COL<${MATRIXL[COLS]} ; COL++ ))
      do
	 MEAN[${COL}]=$(( MEAN[${COL}]+MATRIXL[${ROW},${COL}] ))
      done
   done
   for (( COL=0 ; COL<${MATRIXL[COLS]} ; COL++ ))
   do
      MEAN[${COL}]=$(( MEAN[${COL}]/${MATRIXL[ROWS]} ))
      if [ ${COL} -ne 0 ]
      then
	 printf " "
      fi
      printf "%d" ${MEAN[${COL}]}
   done
   printf "\n"
fi

if [ $1 = "transp" ]
then
   declare -A TRANSPOSE

   readLeft $2

   for (( ROW=0 ; ROW<${MATRIXL[ROWS]} ; ROW++))
   do
      for (( COL=0 ; COL<${MATRIXL[COLS]} ; COL++))
      do
	 TRANSPOSE[${COL},${ROW}]=${MATRIXL[${ROW},${COL}]}
      done
   done
   TRANSPOSE[ROWS]=${MATRIXL[COLS]}
   TRANSPOSE[COLS]=${MATRIXL[ROWS]}

   for (( ROW=0 ; ROW<${TRANSPOSE[ROWS]} ; ROW++ ))
   do
      for (( COL=0 ; COL<${TRANSPOSE[COLS]} ; COL++ ))
      do
	 if [ ${COL} -ne 0 ]
	 then
	    printf " "
	 fi
	 printf "%d" ${TRANSPOSE[${ROW},${COL}]}
      done
      printf "\n"
   done
   exit 0
fi

if [ $1 = "add" ]
then
   declare -A SUM

   if [ $# -lt 3 ]
   then
      echo "not enough files given"
      exit 1
   fi

   readLeft $2
   readRight $3

   if [ ${MATRIXL[ROWS]} -ne ${MATRIXR[ROWS]} ]
   then
      echo "both matrices should have the same dimensions"
      exit 1
   fi
   if [ ${MATRIXL[COLS]} -ne ${MATRIXR[COLS]} ]
   then
      echo "both matrices should have the same dimensions"
      exit 1
   fi

   for (( ROW=0 ; ROW<${MATRIXL[ROWS]} ; ROW++ ))
   do
      for (( COL=0 ; COL<${MATRIXL[COLS]} ; COL++ ))
      do
	 SUM[${ROW},${COL}]=$(( MATRIXL[${ROW},${COL}]+MATRIXR[${ROW},${COL}] ))
	 VAL=${SUM[${ROW},${COL}]}
	 if [ ${COL} -ne 0 ]
	 then
	    printf " "
	 fi
	 printf "%d" ${VAL}
      done
      printf "\n"
   done
   exit 0
fi

if [ $1 = "mult" ]
then
   declare -A MULT

   if [ $# -lt 3 ]
   then
      echo "not enough files given"
      exit 1
   fi

   readLeft $2
   readRight $3

   if [ ${MATRIXL[ROWS]} -ne ${MATRIXR[COLS]} ]
   then
      echo "cols in left matrix must equal rows in right matrix"
      exit 1
   fi

   MULT[ROWS]=${MATRIXL[ROWS]}
   MULT[COLS]=${MATRIXR[COLS]}

   for (( I=0 ; I<${MATRIXL[ROWS]} ; I++ ))
   do
      for (( J=0 ; J<${MATRIXR[COLS]} ; J++ ))
      do
	 for (( K=0 ; K<${MATRIXL[COLS]} ; K++ ))
	 do
	    TMP=$(( MATRIXL[${I},${K}]*MATRIXR[${K},${J}] ))
	    MULT[${I},${J}]=$(( MULT[${I},${J}]+${TMP} ))
	 done
      done
   done
   for (( ROW=0 ; ROW<${MULT[ROWS]} ; ROW++ ))
   do
      for (( COL=0 ; COL<${MULT[COLS]} ; COL++ ))
      do
	 if [ ${COL} -ne 0 ]
	 then
	    printf " "
	 fi
	 echo -n ${MULT[${ROW},${COL}]}
      done
      printf "\n"
   done
   exit 0
fi
