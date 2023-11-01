#pragma once

#include "Color.hpp"
#include <iostream>
#include <unistd.h>
#include <vector>
#include <sys/event.h>
#include <sys/time.h>

class KeventHandler
{
private:
	int kqueue_fd_;

	std::vector<struct kevent> event_list_;
	void initialize_kevent(uintptr_t ident, int16_t filter, uint16_t flags, uint32_t fflags, int64_t data, void *udata);
public:
	KeventHandler();
	~KeventHandler();

	std::vector<struct kevent> set_monitor(const bool &end_signal);
	void add_read_event(uintptr_t ident);
	void add_server_event(uintptr_t ident);
	void add_write_event(uintptr_t ident);
	void add_exit_event(uintptr_t ident);
	void delete_event(const struct kevent &event);
	void delete_server(uintptr_t serv_sock);
};
