# Basic Protocol Structure

### Chat Client / Chat Server

#### Header:

| Packet Length | Message ID |
|:-------------:|:----------:|
|    4 bytes    |   1 byte   |

#### Messages:

| Message ID |       Description      |  Header | Size of next field |     Field description    | Size of next field |     Field description    |
|:----------:|:----------------------:|:-------:|:------------------:|:------------------------:|:------------------:|:------------------------:|
|      1     |        Join Room       | 5 bytes |       1 byte       | Room Name (<= 256 bytes) |       1 byte       | User Name (<= 256 bytes) |
|      2     |       Leave Room       | 5 bytes |       1 byte       | Room Name (<= 256 bytes) |                    |                          |
|      3     |        Send Text       | 5 bytes |       2 bytes      |  Text (<= 65,536 bytes)  |                    |                          |
|      4     |     Create Account     | 5 bytes |       1 bytes      |   Email (<= 256 bytes)   |       1 byte       | Password (<= 256 bytes)  |
|      5     |      Authenticate      | 5 bytes |       1 bytes      |   Email (<= 256 bytes)   |       1 byte       | Password (<= 256 bytes)  |

### Chat Server / Authentication Server

#### Header:

| Packet Length | Message ID |
|:-------------:|:----------:|
|    4 bytes    |   1 byte   |

#### Messages:

| Message ID |        Description      |  Header | Size of next field |       Field description      | Size of next field |     Field description    |
|:----------:|:-----------------------:|:-------:|:------------------:|:----------------------------:|:------------------:|:------------------------:|
|      6     |      Validate Server    | 5 bytes |       1 byte       |  Server Name (<= 256 bytes)  |       1 byte       |    Hash (<= 256 bytes)   |
|      7     |     CreateAccountWeb    | 5 bytes |       2 byte       | Protocbuff (<= 65,536 bytes) |                    |                          |
|      8     | CreateAccountWebSuccess | 5 bytes |       2 byte       | Protocbuff (<= 65,536 bytes) |                    |                          |
|      9     | CreateAccountWebFailure | 5 bytes |       2 byte       | Protocbuff (<= 65,536 bytes) |                    |                          |
|     10     |     AuthenticateWeb     | 5 bytes |       2 byte       | Protocbuff (<= 65,536 bytes) |                    |                          |
|     11     |  AuthenticateWebSuccess | 5 bytes |       2 byte       | Protocbuff (<= 65,536 bytes) |                    |                          |
|     12     |  AuthenticateWebFailure | 5 bytes |       2 byte       | Protocbuff (<= 65,536 bytes) |                    |                          |