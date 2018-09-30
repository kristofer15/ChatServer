# ChatServer
A Chat Server that implements port knocking and small API.  
Client which knocks also follows.   
This is an assigment for the course Computer Networks (TSAM) in Reykjavik University.

## Info

Useful info below

### Development

All development was done on **Manjaro Linux** OS.

### Compiling

Compiling can be done using the *build.sh* script

```
// make sure you are in root of repository
cd ChatServer/

// make sure build.sh is executable if it's not already
chmod +x build.sh

// run build script
./build.sh
```

### Running the server

After compiling you can run the server using the following command

```
// again make sure you are in root of repository
cd ChatServer/

// run the server
./bin/server
```
### Running the client

After compiling you can run the client using the following command

```
// again make sure you are in root of repository
cd ChatServer/

// run client with 4 arguments 
./bin/client <host IP/domain name> <port> <port> <port>
```

