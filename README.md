# C-Server-Client
Simple C tcp server/client that executes a remote ps command.


# Build
##### Build Server: `gcc server.c -o server`

##### Build Client: `gcc client.c -o client`

# Run
##### Run Server: `./server`

##### Run Client `./client`

# Client Usage

##### `SERVER_IP_ADDRESS [user | cpu | mem]`

Must include the server IP address to connect to. No specified option will result in user option by default.

### Options
User by default

##### User

`ps -u USERNAME -o pid,ppid,%cpu,%mem,args`

Username comes from the user running the client.

##### Cpu

`ps -NT -o pid,ppid,%cpu,%mem,args --sort -%cpu | head`

##### Mem

`ps -NT -o pid,ppid,%cpu,%mem,args --sort -%mem | head`
