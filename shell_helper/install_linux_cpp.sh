# see more from https://solarianprogrammer.com/2017/12/13/linux-wsl-install-clang-libcpp-compile-cpp-17-programs/
cat /etc/os-release


sudo apt update -y
sudo apt upgrade -y
sudo apt install build-essential xz-utils curl -y

curl -SL http://releases.llvm.org/9.0.0/clang+llvm-9.0.0-x86_64-linux-gnu-ubuntu-18.04.tar.xz | tar -xJC .
mv clang+llvm-9.0.0-x86_64-linux-gnu-ubuntu-18.04 clang_9.0.0
sudo mv clang_9.0.0 /usr/local
export PATH=/usr/local/clang_9.0.0/bin:$PATH
export LD_LIBRARY_PATH=/usr/local/clang_9.0.0/lib:$LD_LIBRARY_PATH


