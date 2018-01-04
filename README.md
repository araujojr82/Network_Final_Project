# Info-6016_ServerClient
INFO-6016 Network Programming - Assignment #2 

Create an authentication service that provides a way for your chat server to authenticate
users. This authentication service should be reusable. You must use C++

From Assigment #1:
Create a chat server and client that can handle multiple connections simultaneously. The server must
be done in C++ and must use BSD sockets. The client can be a simple text based interface, or you
can use a GUI. The client must also use BSD sockets. 

This assignment will be done by a group of 2 students:
* Benjamin Taylor
* Euclides de Araujo Jr.
* Jorge Amengol

Read the [17F-Assignment1.pdf](https://github.com/amengol/Info-6016_ServerClient/blob/master/17F-Assignment1.pdf) for the detailded assigment.  
Read the [17F-Assignment2.pdf](https://github.com/amengol/Info-6016_ServerClient/blob/master/17F-Assignment2.pdf) for the detailded assigment.  
Read the [Basic_Protocol_Structure.MD](https://github.com/amengol/Info-6016_ServerClient/blob/master/Basic_Protocol_Structure.MD) for the detailded description of the underlying protocol structure.  
Read the [DB_INFO.txt](https://github.com/amengol/Info-6016_ServerClient/blob/master/DB_INFO.txt) for breaf information about the Database connection.  


**How to run the programs**

* Build a `Debug` or `Release` `x86` version of the solution.
* Copy the file `libmysql.dll` from the main folder to the respective `Debug` or `Release` folder.
* Start the `Server.exe`.
* Start the Authentication_Server.exe and type the IP address of the server. If you are connected to the same network it should work fine too. If you are running in the same computer, just type `localhost` or the loopback IP `127.0.0.1`. You should see the message `Chat Server->Authentication Server validated!` at the Authentication Server console
* Start the Client.exe.
* Connect to the Server by typing its IP. If you are connected to the same network it should work fine too. If you are running in the same computer, just type `localhost` or the loopback IP `127.0.0.1`.
* Type in your first name. Now you should see some welcome message.
* If the Server is running you should be some debug messages there indicating what is happening.
* Press any key to continue.
* Type `-help` to see a list of commands. Any command should be inserted as `-command` follwed imediately by `return` or it will not work.
* Type `-new` to create a new user and follow on screen instructions.
* Type `-auth` to authenticate your user and follow on screen instructions.
* Type `-join` to be able to enter a room name. Don't type it right in the same line, just wait for it to ask for the room name.
* Type `-leave` to leave a room or `-exit` to close the chat.
* Start another Client program and repeat the process to be able to chat between two instances of it.
