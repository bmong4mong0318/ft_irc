#pragma once

#include <cstring>
#include <unistd.h>

#include "Database.hpp"

class Receiver;

enum e_val
{
	N_CMD = 13
};

class Parser
{
private:
	event_map &parser_udata_;
	Database database_;
	const std::string &password_;

	static const std::string commands[N_CMD];
	static void (Parser::*command_handlers[N_CMD])(const uintptr_t &, std::stringstream &, std::string &);

	std::string set_message_(std::string &msg, const std::size_t &start);
	std::string message_resize_(const std::string &tmp, const std::string &to_send);
	void push_write_event_(event_pair &tmp);
	void push_multiple_write_events_(event_map &tmp, const uintptr_t &ident, const int flag);
	void parse_pass_(const uintptr_t &ident, std::stringstream &line_ss, std::string &to_send);
	void parse_nick_(const uintptr_t &ident, std::stringstream &line_ss, std::string &to_send);
	void parse_user_(const uintptr_t &ident, std::stringstream &line_ss, std::string &to_send);
	void parse_ping_(const uintptr_t &ident, std::stringstream &line_ss, std::string &to_send);
	void parse_quit_(const uintptr_t &ident, std::stringstream &line_ss, std::string &to_send);
	void parse_privmsg_(const uintptr_t &ident, std::stringstream &line_ss, std::string &to_send);
	void parse_notice_(const uintptr_t &ident, std::stringstream &line_ss, std::string &to_send);
	void parse_join_(const uintptr_t &ident, std::stringstream &line_ss, std::string &to_send);
	void parse_mode_(const uintptr_t &ident, std::stringstream &line_ss, std::string &to_send);
	void parse_invite_(const uintptr_t &ident, std::stringstream &line_ss, std::string &to_send);
	void parse_part_(const uintptr_t &ident, std::stringstream &line_ss, std::string &to_send);
	void parse_topic_(const uintptr_t &ident, std::stringstream &line_ss, std::string &to_send);
	void parse_kick_(const uintptr_t &ident, std::stringstream &line_ss, std::string &to_send);
	const std::string command_toupper_(const char *command);

public:
	Parser(event_map &serv_udata, const std::string &password);

	void clear_all();
	void parse_command(const uintptr_t &ident, std::string &command);
	void handle_error(const uintptr_t &ident);
};