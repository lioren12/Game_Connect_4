# Game_Connect_4
The famous game over Sockets with Server side &amp; 2 Clients - Multi-threading project.

Option to input from file (InputFile1.txt)

six threads over implementation:

SendThread
ReceiveThread
UserInterfaceThread
GameThread * 2 (for each player)
ConnectToClientsThread

Work flow:
-> Server is up and wait for first client to connect
-> First client connect and server approves
-> Second client connect and server approves
* Any other connections will be failed with error message.
