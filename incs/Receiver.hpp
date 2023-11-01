#pragma once

#include <sys/socket.h>
#include <sys/event.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "KeventHandler.hpp"
#include "Parser.hpp"
#include "Event.hpp"

class Parser;

class Receiver
{
private:
	static KeventHandler kq_;
	Parser parser_;
	event_map &udata_;
	event_map carriage_backup_;
	sockaddr_in server_addr_;
	uintptr_t server_sock_;

	int client_read_event_handler_(struct kevent &cur_event);
	int client_write_event_handler_(struct kevent &cur_event);
	void init_socket_(const uintptr_t &port);
	void bind_socket_();
	void carriage_remover_(std::string &to_remove);

public:
	Receiver(event_map &serv_udata, const uintptr_t &port, const std::string &password);

	void stop_receiver();
	void start();
	static KeventHandler &get_Kevent_Handler();
};
