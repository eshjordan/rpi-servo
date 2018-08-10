cd ./Server/src/
sudo gcc -o ../bin/rservo rservo.c -lwiringPi -lpthread -Wall
sudo gcc -o ../bin/rclient rclient.c -lwiringPi -lpthread -Wall