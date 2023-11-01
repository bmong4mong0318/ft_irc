#pragma once

#include "User.hpp"

class ServerStatus
{
public:
	static void print_users(const std::vector<User> &vect_user);
	static void print_input(const std::string &input);
	static void print_recived(const uintptr_t &socket, const std::string &command);
	static void print_send(const uintptr_t &socket, const std::string &msg);
};
