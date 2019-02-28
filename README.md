# Game_Connect_4
The famous game over Sockets with Server side &amp; 2 Clients - Multi-threading project.

Option to input from file (InputFile1.txt)

six threads over implementation:

SendThread
ReceiveThread
UserInterfaceThread
GameThread * 2 (for each player)
ConnectToClientsThread

# Work flow:

-> Server is up and wait for first client to connect.

-> First client connects and server approves.

-> Second client connects and server approves.

* Any other connections will be failed with error message.

-> Game started.

-> When game ends, both players are disconnected and the server waits for new players to connect.
