mkdir bin
mkdir log
mkdir lib
mkdir stdData
mkdir userSubmition
mkdir SPJ

make
make clean

cp src/backMsg/backMsg.py bin
cp src/update/update.py bin

chmod +x launch.sh
