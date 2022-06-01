# substring h8_h7_g8_i8 g6
filename=$1
next_step=$2
if [[ "${filename:(-4):1}" == "_" ]]; then
    sub_count=4
else
    sub_count=3
fi

full_filename=./gomoku/divided/${filename:0:${#filename}-$sub_count}/$filename.txt
mkdir ./gomoku/divided/${filename:0:${#filename}-$sub_count}/
touch $full_filename
echo $filename | tr '_' '\n' > $full_filename
#make a correct filename location for given board
./problem_divide.sh $full_filename ${next_step}

#solve all the white move location
./problem_solve.sh /${filename}_${next_step}/ $3 $4

