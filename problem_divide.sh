#for example ./divided/h8_h7_g8_i8_g6_g7_f7_e8_e6/h8_h7_g8_i8_g6_g7_f7_e8_e6_d5.txt
filename=$1
next_step=$2
echo "input board to resolve $filename"
if [[ "$OSTYPE" == "linux-gnu"* ]]; then
  echo "I am linuX"
  export PATH=/usr/local/clang_9.0.0/bin:$PATH
  export LD_LIBRARY_PATH=/usr/local/clang_9.0.0/lib:$LD_LIBRARY_PATH
elif [[ "$OSTYPE" == "darwin"* ]]; then
  echo "I am mac"
else
  echo "this platform is not supported"
fi


cd gomoku
rm divide

echo $next_step >> ./black_steps.txt
python3 ./script/divide.py ../$filename ./black_steps.txt

