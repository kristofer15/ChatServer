mkdir -p bin
g++ -std=c++11 -pthread -fno-stack-protector server.cpp settings.cpp -o bin/server
g++ -std=c++11 client.cpp -o bin/client
