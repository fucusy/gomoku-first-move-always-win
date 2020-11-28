# substring /h8_h7_g8_i8_g6_g7_f7_e8_e6/
filename=$1
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


if [[ "$OSTYPE" == "linux-gnu"* ]]; then
  echo "I am linuX"
  clang++ -std=c++17 -stdlib=libc++ -Wall -pedantic *.cpp -o noninteractive -DCLI -DTEST -DNOINTERACTIVE -DLOADCACHE
elif [[ "$OSTYPE" == "darwin"* ]]; then
  echo "I am mac"
  g++  *.cpp -o interactive -std=c++17  -DCLI -DTEST `pkg-config --cflags --libs protobuf`
else
  echo "this platform is not supported"
fi

cd script
python3 generate_tmp_file.py $filename
../interactive ./tmp.txt

