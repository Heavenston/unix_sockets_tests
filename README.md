
# Intro
This is project i use for testing unix sockets.
This can also be used as examples. Although they lack comments.

# The different binaries
## Datagram
There is two binaries that are using datagram sockets.

### DgramPaired
This binary uses the socketpair function to create two connected sockets.
Then forks the program and sends data between the two forks.
It will first ask the user for input to send from one fork to the other.
Than sends numbers from 0 to 9 before closing.

### DgramConnected
This binary won't do anything interactive. It's the same system as DgramPaired. But
uses named named sockets to send data.

## Stream
Only one binary is using stream sockets.

### StreamConnected
To run this executable you have to give as it's first argument either 'client' or 'server'.
You should only run one process with 'server' but can run any number of 'client'.
The clients will connect to the server, than ask the user to type. Each message will be
sent to the server, which has given unique ids to each clients to print the received messages.

# How to compile
This project uses cmake to generate the build files:
```sh
mkdir build  # Make a build directory
cd build
cmake ..     # Generake build files
make         # Build binaries
```
After that the different binaries should be located in the build folder.
