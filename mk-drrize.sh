#!/bin/sh
#http://openresty.org/download/drizzle7-2011.07.21.tar.gz
#sudo apt-get install gperf
#sudo apt-get install intltool
#sudo apt-get install libprotoc-dev
#sudo apt-get install protobuf-compiler
#sudo apt-get install uuid-dev
#sudo apt-get install lib64readline6-dev
sudo apt-get install libboost-all-dev
#sudo apt-get install libboost-program-options-dev
./configure --without-server
make libdrizzle-1.0
sudo make install-libdrizzle-1.0
sudo cp /usr/local/lib/libdrizzle.so.1 /usr/lib
