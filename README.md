# Network_Final_Project 
INFO-6016 Network Programming - Final Project

You are tasked with designing and developing a game lobby browser. This game lobby
browser will be used by players to inform them what game lobbies are open, and which game
lobbies they can join. You are responsible for developing the game client, game server,
authentication server, and database(s) for the game.


Developed by Euclides Araujo
Based on previous work done by himself, Benjamin Taylor and Jorge Amengol


**How to run the programs**

* Build a `Debug` or `Release` `x86` version of the solution.
* Copy the file `libmysql.dll` from the main folder to the respective `Debug` or `Release` folder.
* Start the `GameServer.exe`.
* Start the Authentication_Server.exe and type the IP address of the server. Use `localhost` if you are in the same network. Type in the Port Number, the Server currently uses port 5000.
* Start the GameClient.exe.
* Connect to the Server by typing its IP. Use `localhost` if you are in the same network. Type in the Port Number, the Server currently uses port 5000.
* Type in your User name
* If the Server is running you should be some debug messages there indicating what is happening.
* Type `-help` to see a list of commands. Any command should be inserted as `-command` follwed imediately by `return` or it will not work.
* Type `-auth` Starts the authentication process for a user.
* Type `-list` List the lobbies availables on the Server.
* Type `-create` Creates a lobby in the Game Server.
* Type `-join` Joins a lobby in the Game Server.
* Type `-leave` Leaves a specific lobby from the Game Server.
* Type `-exit` Exit the client application.