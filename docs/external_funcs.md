### Описание разрешенных функций

### socket()
socket - create an endpoint for communication
Synopsis
```
#include <sys/socket.h>

int socket(int domain, int type, int protocol);
```
Description
The socket() function shall create an unbound socket in a communications domain, and return a file descriptor that can be used in later function calls that operate on sockets.
The socket() function takes the following arguments:

- domain
Specifies the communications domain in which a socket is to be created.
- type
Specifies the type of socket to be created.
- protocol
Specifies a particular protocol to be used with the socket. Specifying a protocol of 0 causes socket() to use an unspecified default protocol appropriate for the requested socket type.
The domain argument specifies the address family used in the communications domain. The address families supported by the system are implementation-defined.

Symbolic constants that can be used for the domain argument are defined in the <sys/socket.h> header.

The type argument specifies the socket type, which determines the semantics of communication over the socket. The following socket types are defined; implementations may specify additional socket types:

Return Value
Upon successful completion, socket() shall return a non-negative integer, the socket file descriptor. Otherwise, a value of -1 shall be returned and errno set to indicate the error.