mkdir -p bin
g++ -std=c++17 server.cpp settings.cpp -o bin/server
g++ -std=c++17 client.cpp -o bin/client
