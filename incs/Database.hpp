#pragma once

#include "Channel.hpp"
#include "Sender.hpp"
#include "Event.hpp"

class Database
{
private:
	std::vector<Channel> channel_list_;
	std::vector<User> user_list_;

public:
	std::vector<User> &get_user_list(void);
	event_pair valid_user_checker_(const uintptr_t &ident, const std::string &command_type);

	event_map channel_msg(User &sender, std::string chan_name, const std::string &msg);
	event_map notice_channel(User &sender, std::string chan_name, const std::string &msg);
	bool is_channel(std::string &chan_name);
	bool is_user_in_channel(User &leaver);
	Channel &create_channel(User &joiner, std::string &chan_name);
	void delete_channel(std::string &chan_name);
	Channel &select_channel(std::string &chan_name);
	Channel &select_channel(const std::string &chan_name);
	Channel &select_channel(User &connector);

	event_map set_topic(const uintptr_t &ident, std::string &chan_name, std::string &topic);
	event_map kick_channel(User &host, User &target, std::string &chan_name, std::string &msg);
	event_map join_channel(User &joiner, const std::string &chan_name_, const std::string &tmp_password);
	event_map part_channel(User &leaver, std::string &chan_name, const std::string &msg_);
	event_map quit_channel(User &leaver, std::string &chan_name, const std::string &msg_);
	event_map nick_channel(User &nicker, std::string &send_msg);
	std::vector<Channel> &get_channels();

	void delete_error_user(const uintptr_t &ident);
	event_pair command_pass(const uintptr_t &ident);
	event_map command_nick(const uintptr_t &ident, std::string &nick_name);
	event_pair command_user(const uintptr_t &ident, const std::string &username, const std::string &mode,
													const std::string &unused, const std::string &realname);
	event_pair command_pong(const uintptr_t &ident, std::string &msg);
	event_map command_quit(const uintptr_t &ident, const std::string &chan_name);
	event_map command_privmsg(const uintptr_t &ident, const std::string &target_name, const std::string &msg);
	event_map command_notice(const uintptr_t &ident, const std::string &target_name, const std::string &msg);
	event_pair command_privmsg(std::string &target_name, std::string &line, const uintptr_t &ident);
	event_map command_join(const uintptr_t &ident, const std::string &chan_name, const std::string &password);
	event_map command_part(const uintptr_t &ident, std::string &chan_name, const std::string &msg);
	event_map command_kick(const uintptr_t &ident, const std::string &target_name, std::string &chan_name, std::string &msg);
	event_map command_mode(const uintptr_t &ident, t_mode_input &mode);
	event_map command_invite(const uintptr_t &ident, std::string &user, std::string &chan_name);
	event_map command_mode_i_on(const uintptr_t &ident, t_mode_input &mode);
	event_map command_mode_i_off(const uintptr_t &ident, t_mode_input &mode);
	event_map command_mode_k_on(const uintptr_t &ident, t_mode_input &mode);
	event_map command_mode_k_off(const uintptr_t &ident, t_mode_input &mode);
	event_map command_mode_o_on(const uintptr_t &ident, t_mode_input &mode);
	event_map command_mode_o_off(const uintptr_t &ident, t_mode_input &mode);
	event_map command_mode_t_on(const uintptr_t &ident, t_mode_input &mode);
	event_map command_mode_t_off(const uintptr_t &ident, t_mode_input &mode);
	event_map command_mode_l_on(const uintptr_t &ident, t_mode_input &mode);
	event_map command_mode_l_off(const uintptr_t &ident, t_mode_input &mode);
	static event_map (Database::*mode_handlers[N_MODE_TYPE])(const uintptr_t &ident, t_mode_input &mode);

	bool is_user(const uintptr_t &ident);
	bool is_user(const std::string &nickname);
	User &select_user(const uintptr_t &ident);
	User &select_user(const std::string &nickname);
	void delete_user(User &leaver);
	bool is_valid_nick(std::string &new_nick);

	event_pair bot_privmsg(User &cur_usr, const std::string &msg);
	void bot_maker(const std::string &name);
};
