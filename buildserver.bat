cd .\Server\src\
gcc -o ..\bin\wservo wservo.c -lws2_32 -Wall
gcc -o ..\bin\wclient wclient.c -lws2_32 -Wall