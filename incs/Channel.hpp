#pragma once

#include "Sender.hpp"
#include "Event.hpp"
#include "TypeMode.hpp"

#include <vector>
#include <algorithm>
#include <sstream>

enum e_send_switch
{
	JOIN,
	PART,
	PRIV,
	KICK,
	QUIT,
	NOTICE,
	TOPIC,
	NICK,
	MODE,
	INVITE
};

enum e_channel_flag
{
	F_INVITE_ONLY = 0b0001,
	F_KEY_NEEDED = 0b0010,
	F_LIMITED_MEMBERSHIP = 0b0100,
	F_TOPIC_OWNERSHIP = 0b1000
};

class Channel
{
private:
	std::vector<User> channel_members_;
	std::vector<uintptr_t> hosts_;
	std::vector<uintptr_t> invitations_;
	std::string name_;
	std::string topic_;
	std::string key_;
	int member_limit_;

public:
	char channel_flag_;

	Channel();
	std::vector<uintptr_t> &get_hosts(void);
	void set_host(uintptr_t host_sock);
	void set_host(User &user);
	void unset_host(uintptr_t host_sock);
	void unset_host(User &host_user);
	bool is_host(uintptr_t client_sock);
	bool is_host(User &user);
	std::string &get_channel_name(void);
	std::string &get_topic(void);
	int get_member_limit(void);
	void set_topic(std::string &topic);
	bool is_channel_members(User &usr);
	void change_nick(User &usr, std::string new_nick);
	void add_to_channel_member(User &joiner);
	void set_channel_name(std::string &chan_name);
	void delete_from_channel_member(User &usr);
	void invite_user(uintptr_t user_sock);
	void set_member_limit(int &member_limit);
	void set_password(Channel &tmp_channel, t_mode_input mode);

	std::string get_user_list_str(void);
	void set_flag(Channel &channel, t_mode_input &mode);
	bool has_invitation(const uintptr_t &usr);
	bool check_password(Channel &tmp_channel, const std::string &tmp_password);
	event_map send_all(User &sender, User &target, std::string msg, int remocon);

	std::vector<User> &get_users(void);
	std::vector<User> sort_users(void);
	bool operator==(const Channel &t) const;
};
