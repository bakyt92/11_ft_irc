/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ufitzhug <ufitzhug@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/03 21:44:45 by ufitzhug          #+#    #+#             */
/*   Updated: 2024/05/03 21:44:48 by ufitzhug         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
class Server;

int main(int argc, char *argv[]) {
  // проверить с помощью strtol, не выходим ли за рамки int !
  if(argc != 3 || atoi(argv[1]) < 1024 || atoi(argv[1]) > 65535) {
    std::cout << "Invalid arguments.\nRun ./ircserv <port> <password>, port should be between 1024 and 65535\n";
    return 0;
  }
  Server s = Server(argv[1], argv[2]);
  try {
    s.init();
    s.run();
  } catch (std::exception &e) {
    std::cerr << e.what() << std::endl;
    return 0;
  }
  return 0;
}