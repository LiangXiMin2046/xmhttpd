#ifndef SIMPLEHTTP_SOCKETSOPS_H
#define SIMPLEHTTP_SOCKETSOPS_H

#include <sys/socket.h>
#include <stdint.h>

namespace Sockets
{

int createSocketOrDie();

void bindOrDie(int fd,struct sockaddr* addr);

void listenOrDie(int fd);

int accept(int fd);

int createListenSocketOrDie(uint16_t port);

} // namespace Sockets

#endif //SIMPLEHTTP_SOCKETSOPS_H
