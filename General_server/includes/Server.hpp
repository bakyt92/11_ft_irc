/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ufitzhug <ufitzhug@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/16 19:07:53 by ufitzhug          #+#    #+#             */
/*   Updated: 2024/04/16 21:12:29 by ufitzhug         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
#define SERVER_HPP

#include <iostream> // general
#include <poll.h> // for poll
#include <sys/socket.h> // for socket, getsockopt, setsockopt + getaddrinfo, FREEADDRINFO
// bind, connect, listen, accept, inet_addr, inet_ntoa, send, recv, 
#include <unistd.h> // for close, lseek, fstat, fcntl
#include <netdb.h> // GETPROTOBYNAME, gethostbyname, + getaddrinfo, FREEADDRINFO
#include <sys/types.h> // getaddrinfo, lseek, fstat
#include <arpa/inet.h> // htons, htonl, ntohl, ntohs +inet_addr, inet_ntoa
#include <netinet/in.h> // inet_addr, inet_ntoa
#include <signal.h> // signal, sigaction
#include <sys/stat.h> // fstat
#include <fcntl.h> // fcntl
#include <poll.h> // poll

#endif