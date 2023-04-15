In this application, we implemented three C programs,
- Peer_Client
- Relay_Server
- Peer_Nodes

These programs communicate with each other based on TCP sockets. The aim is to implement a simple Relay based Peer-to-Peer System.

## About the Project

![image](https://user-images.githubusercontent.com/88557062/232182601-92118620-9e5e-4763-92e2-979cd46e85b2.png)

At the beginning, the Peer_Nodes (peer 1/2/3 as depicted in Figure 1) establish a connection with the Relay_Server using the TCP port they are already familiar with. Once the connection is established, each Peer_Node shares its IP address and PORT with the Relay_Server before closing the connection (refer to Figure 1). The Relay_Server actively maintains all the received information. Now the Peer_Nodes act as servers and wait for Peer_Clients to connect (phase three).

In the second phase, the Peer_Client connects to the Relay_Server using the server's TCP port. Upon successful connection, it requests the active Peer_Nodes information from the Relay_Server (refer to Figure 2). The Relay_Server responds to the Peer_Client with the active Peer_Nodes information it currently has. Once the Peer_Client receives the response message, it gracefully closes the connection.

In the third phase, a set of files (such as *.txt) is distributed evenly among the three Peer_Nodes. The Peer_Client takes "file_Name" as input from the user and then connects to the Peer_Nodes one at a time using the response information. Once connected, the Peer_Client attempts to fetch the file from the Peer_Node. If the file is present, the Peer_Node provides the file content to the Peer_Client, which prints the content in its terminal. If the file is not present, the Peer_Client connects to the next Peer_Node and repeats the process. This continues until the Peer_Client gets the file content or all entries in the Relay_Server Response are exhausted (assuming there are only three or four Peer_Nodes in the system).

## Implementation Details

We implemented the functionalities using appropriate REQUEST and RESPONSE Message formats. After each negotiation phase, the TCP connection on both sides are closed gracefully releasing the socket resource. The code accepts the IP Address and Port number from the command line.

Prototypes for Client and Server
- Client: <executable code><Server IP Address><Server Port number>
- Server: <executable code><Server Port number>

## How to Run

- compile the three codes
  - `c++ -o server server.c`
  - `c++ -o client client.c`
  - `c++ -o node node.c`

  (the nodes can be in other directories as well, it doesn't matter.)
- run 
  - `/server`
  - `/node`
  - `/node`
  - `/node`
  - `/client`

  server address is localhost here

The requested file should be with a node. The client will ask for a file name and return the file if it is found with any node.
