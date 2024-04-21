### Описание разрешенных функций

### memset()
*Name*
memset - fill memory with a constant byte
*Synopsis*
```
#include <string.h>
void *memset(void *s, int c, size_t n);
```
*Description*
The memset() function fills the first n bytes of the memory area pointed to by s with the constant byte c.
*Return Value*
The memset() function returns a pointer to the memory area s.

### socket()
https://linux.die.net/man/3/socket
socket - create an endpoint for communication
*Synopsis*
```
#include <sys/socket.h>

int socket(int domain, int type, int protocol);
```
*Description*
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

*Return Value*
Upon successful completion, socket() shall return a non-negative integer, the socket file descriptor. Otherwise, a value of -1 shall be returned and errno set to indicate the error.

### close 
close - close a file descriptor
*Synopsis*
```
#include <unistd.h>

int close(int fildes);
```
*Description*
The close() function shall deallocate the file descriptor indicated by fildes. To deallocate means to make the file descriptor available for return by subsequent calls to open() or other functions that allocate file descriptors. All outstanding record locks owned by the process on the file associated with the file descriptor shall be removed (that is, unlocked).
If close() is interrupted by a signal that is to be caught, it shall return -1 with errno set to [EINTR] and the state of fildes is unspecified. If an I/O error occurred while reading from or writing to the file system during close(), it may return -1 with errno set to [EIO]; if this error is returned, the state of fildes is unspecified.
When all file descriptors associated with a pipe or FIFO special file are closed, any data remaining in the pipe or FIFO shall be discarded.
When all file descriptors associated with an open file description have been closed, the open file description shall be freed.
If the link count of the file is 0, when all file descriptors associated with the file are closed, the space occupied by the file shall be freed and the file shall no longer be accessible.

