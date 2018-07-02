#ifndef NETWORK_H
#define NETWORK_H

#include <boost/asio.hpp>
#include <boost/array.hpp>
#include "../Ruota.h"

std::vector<SP_MEMORY> __network_start(std::vector<SP_MEMORY> args);
std::vector<SP_MEMORY> __network_create_socket(std::vector<SP_MEMORY> args);
std::vector< std::string > getHostByName(std::string hostname);
std::vector<SP_MEMORY> __network_get_addresses(std::vector<SP_MEMORY> args);
std::vector<SP_MEMORY> __network_connect(std::vector<SP_MEMORY> args);
std::vector<SP_MEMORY> __network_send(std::vector<SP_MEMORY> args);
std::vector<SP_MEMORY> __network_listen(std::vector<SP_MEMORY> args);
std::vector<SP_MEMORY> __network_receive(std::vector<SP_MEMORY> args);
std::vector<SP_MEMORY> __network_shutdown(std::vector<SP_MEMORY> args);
std::vector<SP_MEMORY> __network_io_shutdown(std::vector<SP_MEMORY> args);

#endif