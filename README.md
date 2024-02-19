# Design Overview
The application consists of an index server and several
peers, these peers are the content servers, and clients to each other and host the files for
download. The index server maintains a list of registered content between all peers and serves
to provide connection information to these peers through UDP communication. The file transfer
and communication between individual peers is all done through TCP communication. Figure 1
illustrates the architecture: 

![image](https://github.com/Pharazz/P2PFileSharingNetwork/assets/32317419/ff4eb729-1a99-472d-a06f-8f4cdd5f6548)

## Index Server
The index server works as a UDP-based download traffic coordinator for the TCP peer-based
system. The server maintains a list of registered peers, and the content that they can provide in
addition to their IP address and the port that their TCP socket can be connected to. The index
server converses with peers through UDP request PDUs, listed below in the Network Architecture section. The server uses a structure called Index that it instantiates within an array
to organize the peer data, that stores information as follows:

● Char contentName[100]

● Peer sources[PEERSIZE]

This struct maintains a string of chars that describe the content available (effectively the
filename that can be downloaded) and an array of Peer structs. Figure 3 displays the
architecture of the index Server. 

![image](https://github.com/Pharazz/P2PFileSharingNetwork/assets/32317419/117f72a5-4ee5-4efa-b05f-54cabfaeb707)

The Peer struct stored within the Index struct above is a further sub-organization of the specific
peer that exists at that element of the array of Index structs, and has the following information: 

● Char peername[100]

● Char port[100]

● Char address [100]

● Int lastUsed 

## Peer
The peer works as a UDP and TCP content server and client. The peer maintains a single UDP
socket and a list of its registered content, and TCP socket information for each one (one content
per socket). The peer converses with the index server through UDP request PDUs, of behaviour
of which is described in the Network Architecture section. Figure 4 displays the architecture of
the Peer. 

![image](https://github.com/Pharazz/P2PFileSharingNetwork/assets/32317419/ee9d403c-19fd-413c-b512-01ecf13ea0e6)

To communicate with other peers, a peer would connect to their TCP sockets directly once the
index server provides them with the socket information. The peer uses a structure called
Threads that it instantiates as an array to organize the registered content, and socket
information as follows:

● Int port

● Int pid

● Char content [100]

This struct maintains the port the socket is open on, the pid of the child process which handles
that socket, and the name of its registered content.

# Network Behavior

The behavior of this network can be described on a case-by-case basis. The data is sent through packets which have been described as a structure which denotes the type and thereafter has a message with a fixed maximum size. See the Types of PDU's below:

![image](https://github.com/Pharazz/P2PFileSharingNetwork/assets/32317419/5d07d6d1-e61f-4063-8eb4-5a4169efa7a8)

## Content Registration
![image](https://github.com/Pharazz/P2PFileSharingNetwork/assets/32317419/5def13b9-0906-482d-ad08-d0a060d331bc)
When a peer wants to register a piece of content it creates a TCP socket for this content. It then
sends this information as an R type PDU to the index server via its existing UDP socket, and the
index server processes the request.

The index server makes the following checks:

1. Is this content name within the size limits?
2. Is this new content being registered?
3. Has a Peer with this name already registered this content?

If the first check fails then the content is denied registration, the second check is simply to see
whether or not the Index server needs to make a brand-new entry, or add another peer to an
existing entry, the final check is to stop duplicate entries and on a pass it sends an error to the
peer and prompts it to try again with a different peername. 

On a successful registration the Index server responds with an acknowledgement that the
content has been successfully registered as displayed in Figure 5 and updates its Index as
described in Figure 6. 
![image](https://github.com/Pharazz/P2PFileSharingNetwork/assets/32317419/6a033914-817c-43c0-8411-95f13043eec6)

## Content Deregistration
![image](https://github.com/Pharazz/P2PFileSharingNetwork/assets/32317419/c49342f3-9df9-4b10-8a4d-4eaf493f0b8e)

When a peer wants to Deregister a piece of content it sends a T type PDU to the index server
via its existing UDP socket, and the index server processes the request.
The index server makes the following checks:

1. Is this content registered?
2. Is this peer a server of this content?
3. Is this peer the last server for this content?
   
If the first two checks return errors (E type PDUs) on failures as if the content is unregistered or
the peer is not a content server of that content, then deregistration is impossible. The final check
is simply to see whether or not the Index server needs to clear this entry as with no more
content servers the entry space should be freed so that different content can be registered in its
place. In either case the index server removes the peer from the index and returns an
acknowledgement that the peer has been deregistered.

On reception of this acknowledgement the Peer closes its TCP socket which hosts said content.
This behavior is described in Figure 7, and the update to the index in Figure 8. 

![image](https://github.com/Pharazz/P2PFileSharingNetwork/assets/32317419/3737e3c6-89ad-4520-a7b6-dcb874bfbd68)

## Content Listing
![image](https://github.com/Pharazz/P2PFileSharingNetwork/assets/32317419/97790774-f430-47b2-8a38-c642f4bd4920)

Figure 9 describes a content listing interaction between a content server. The Peer sends an O
type PDU and on reception of any O type PDU the index responds with the list of registered
content. On reception the Peer should print out this list in the command line

## Content Download
![image](https://github.com/Pharazz/P2PFileSharingNetwork/assets/32317419/d995ff9f-86e8-4528-bd2f-7c74bb0de7df)

When a peer wants to download a piece of content it sends a S type PDU with the content
name and its own peername to the index server via its existing UDP socket, and the index
server processes the request.

The index server checks if the content has been registered. If not, it returns an error (E type
PDUs) indicating that this content is not available. Otherwise, the index server checks through
its list and finds a content server to download from. If there are multiple content servers to
choose from, the index server will choose one that has not been used last and then set that
peer as the most recently used. The index server returns the socket information and peer name
of the content server in an S Type PDU as displayed in Figure 10. 

On reception of this S Type PDU, the peer will open a new TCP socket to download the content
and connect to the socket whose information was just received with a D Type PDU. When the
content server receives this D Type PDU on its TCP socket it begins transmitting the content
data through C Type PDU(s). This behavior is described by Figure 11.

![image](https://github.com/Pharazz/P2PFileSharingNetwork/assets/32317419/ac8aac85-87a9-49d4-8580-738a2798ed37)

Once the download has been completed the Peer then registers as a content server of its newly
acquired content on the socket it has created for it. This procedure is exactly the same as when
registering new content as referenced by Figure 5. The updates to the index server on a
successful search are described by Figure 12. 

![image](https://github.com/Pharazz/P2PFileSharingNetwork/assets/32317419/d0afa2d2-7f39-4b06-8d57-47fa6a6230ba)

## Quitting

When a peer chooses to quit as a content server it parses through its list of registered content
and associated sockets and sends a content deregistration request for each of them. This
behavior is described above and in Figure 7. At this point all content has been deregistered and
the sockets and child processes hosting them should all be closed. 



