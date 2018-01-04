# How we intend to marked in this assignment:

**Authentication Protocol ( 7 marks ):**  
*The authentication protocol was created accordingly the assignment, usging Google Protocol Buffers. It's structure can be checked at:*
[authentication.proto](https://github.com/amengol/Info-6016_ServerClient/blob/master/protobuf/bin/authentication.proto)

**Authentication Service Database (3 marks):**  
*The Database follows the suggested structure. The schema is avaible at:*
[DB.sql](https://github.com/amengol/Info-6016_ServerClient/blob/master/DB.sql)

**Authentication Service (17 marks):**
1. Must use TCP (1 mark)  
*It uses TCP for communication*

2. Must use lengthprefix header for serialization (3 marks)  
*It uses lengthprefix header via the cConnection class*

3. Must be able to create a new account (8 marks total)  
*It creates new accounts as can be seen via the Database*

   3.1. Must use SHA256 as the hash algorithm (2 marks)  
   *It uses SHA256 via the sha256.h file*

   3.2. Must use a randomized salt for EACH password (2 marks)  
   *The salt is randomized as can be seen in the Salt.cpp file*

   3.3. Must add this account to the MySQL Database (2 marks)  
   *It creates new accounts as can be seen via the Database*

   3.4. Must respond with a failure reason on failure. (1 mark)  
   *It respond with `Account Already Exists` as configured in the Google Protocol Buffers*

   3.5. Must respond with “success” on success (1 mark)  
   *It respond with `Success` on success as configured in the Google Protocol Buffers*

4. Must be able to authenticate an account (5 marks total)  
*It authenticate the account*

   4.1. Must hash the plaintext with SHA256 ( 1 mark)  
   *It uses SHA256 via the sha256.h file to hash the plaintext*

   4.2. Must compare this hash to the database hash properly (1 mark)  
   *It compares properly as the user is only allowed to chat after it*

   4.3. Must respond with a failure reason on failure (1 mark)  
   *It respond with `Invalid Credentials` as configured in the Google Protocol Buffers*

   4.4. Must respond with success on success (1 mark)  
   *It respond with `Success` on success as configured in the Google Protocol Buffers*

   4.5. Must update the `last_login` column in the `user` table in MySQL (1 marks)  
   *It is updating the `last_login`. It can be verified by looking at the tables after loging in*

**Authentication Client (7 marks):**  
*The authentication client is the chat server*
1. Must use TCP ( 1 mark)
2. Must use lengthprefix header for serialization (3 marks)
3. Must connect to the authentication service (1 mark)
4. Should be able to create a new account (1 mark)
5. Should be able to authenticate a user (1 mark)  
*It fullfils all itens*

**Chat Client (6 marks)**  
1. Should be able send a command: REGISTER email password (1 mark)  
*This is achieved via the -new command at the client*

2. If registration was successful, it should tell that client: “Registration successful” (1 mark)  
*It receives this message from the server*

3. If registration failed, it should tell that client the reason for the failure (1 mark)  
*It receives this message from the server*

4. Should be able to send a command: AUTHENTICATE email password (1 mark)  
*This is achieved via the -auth command at the client*

5. If authentication was successful, it should say “Authentication successful, account
created on [DATE IN DATABASE]” (1 mark)  
*This can be checked at the Client console by authenticating a user*

6. If authentication failed, it should tell that client the reason for the failure (1 mark)  
*It receives authentication faillure messages from the server*



*Our project is published here https://github.com/amengol/Info-6016_ServerClient and has more than 40 commits, started since the beginning  of October.*
