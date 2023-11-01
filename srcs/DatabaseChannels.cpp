#include "Channel.hpp"
#include "Database.hpp"
#include "Event.hpp"
#include "User.hpp"
#include "Color.hpp"
#include <sys/_types/_size_t.h>
#include <iostream>
#include <sstream>

std::vector<Channel> &Database::get_channels()
{
	return channel_list_;
}

bool Database::is_channel(std::string &chan_name)
{
	std::vector<Channel>::iterator it;

	for (it = channel_list_.begin(); it != channel_list_.end(); ++it)
	{
		if (it->get_channel_name() == chan_name)
		{
			return true;
		}
	}
	return false;
}

bool Database::is_user_in_channel(User &leaver)
{
	std::vector<Channel>::iterator it = channel_list_.begin();

	for (; it != channel_list_.end(); ++it)
	{
		if (it->is_channel_members(leaver))
		{
			return true;
		}
	}
	return false;
}

Channel &Database::create_channel(User &joiner, std::string &chan_name)
{
	Channel tmp;

	tmp.set_channel_name(chan_name);
	tmp.add_to_channel_member(joiner);
	tmp.set_host(joiner);
	channel_list_.push_back(tmp);
	return channel_list_.back();
}

void Database::delete_channel(std::string &chan_name)
{
	Channel tmp;

	tmp.set_channel_name(chan_name);

	std::vector<Channel>::iterator it = std::find(channel_list_.begin(),
																								channel_list_.end(), tmp);
	std::size_t size = std::distance(this->channel_list_.begin(), it);
	this->channel_list_.erase(this->channel_list_.begin() + size);
}

Channel &Database::select_channel(std::string &chan_name)
{
	std::vector<Channel>::iterator it = channel_list_.begin();

	for (; it != channel_list_.end(); ++it)
	{
		if (it->get_channel_name() == chan_name)
		{
			return *it;
		}
	}
	return *it;
}

Channel &Database::select_channel(const std::string &chan_name)
{
	std::vector<Channel>::iterator it = channel_list_.begin();

	for (; it != channel_list_.end(); ++it)
	{
		if (it->get_channel_name() == chan_name)
		{
			return *it;
		}
	}
	return *it;
}

Channel &Database::select_channel(User &connector)
{
	std::vector<Channel>::iterator it = channel_list_.begin();

	for (; it != channel_list_.end(); ++it)
	{
		if (it->is_channel_members(connector))
		{
			return *it;
		}
	}
	return *it;
}

event_map Database::join_channel(User &joiner, const std::string &tmp_chan_name, const std::string &tmp_password)
{
	event_map ret;
	event_pair tmp;
	std::string chan_name(tmp_chan_name);
	Channel &cur_channel = select_channel(tmp_chan_name);

	if (is_user_in_channel(joiner))
	{
		event_pair tmp2;
		tmp = Sender::join_message(joiner, joiner, chan_name);
		tmp2 = Sender::part_message(joiner, joiner, chan_name, "invalid : No Double join");
		tmp.second += tmp2.second;
		ret.insert(tmp);
		return ret;
	}
	if (is_channel(chan_name) == false)
	{
		Channel &channel = create_channel(joiner, chan_name);
		tmp = Sender::join_message(joiner, joiner, chan_name);
		ret.insert(tmp);
		event_map_iter it = ret.find(joiner.client_sock_);
		it->second += Sender::join_353_message(joiner, channel.get_channel_name(), "@" + joiner.nickname_);
		it->second += Sender::join_366_message(joiner, channel.get_channel_name());
	}
	else if ((cur_channel.channel_flag_ & F_INVITE_ONLY) && !cur_channel.has_invitation(joiner.client_sock_))
	{
		tmp = Sender::cannot_join_message(joiner, chan_name); // cannot_join_messeage_invite
		ret.insert(tmp);
	}
	else if (!(cur_channel.channel_flag_ & F_INVITE_ONLY) && (cur_channel.channel_flag_ & F_KEY_NEEDED) && !cur_channel.check_password(cur_channel, tmp_password))
	{
		tmp = Sender::cannot_join_message_key(joiner, chan_name); // connot_join_message_key
		ret.insert(tmp);
	}
	else if (!(cur_channel.channel_flag_ & F_INVITE_ONLY) && (cur_channel.channel_flag_ & F_LIMITED_MEMBERSHIP) && static_cast<int>(cur_channel.get_users().size()) >= cur_channel.get_member_limit())
	{
		tmp = Sender::cannot_join_message_limit(joiner, chan_name);
		ret.insert(tmp);
	}
	else
	{
		cur_channel.add_to_channel_member(joiner);
		const std::string &chan_user_list(cur_channel.get_user_list_str());
		ret = cur_channel.send_all(joiner, joiner, "Join \"" + chan_name + "\" channel, " + joiner.nickname_, JOIN);
		event_map_iter it = ret.find(joiner.client_sock_);
		it->second += Sender::join_353_message(joiner, cur_channel.get_channel_name(), chan_user_list);
		it->second += Sender::join_366_message(joiner, cur_channel.get_channel_name());
	}
	return ret;
}

event_map Database::quit_channel(User &leaver, std::string &chan_name, const std::string &msg_)
{
	event_pair tmp;
	event_map ret;
	std::string msg(msg_);

	if (is_channel(chan_name) == false)
	{
		tmp = Sender::no_channel_message(leaver, chan_name);
		ret.insert(tmp);
		return ret;
	}
	Channel &channel = select_channel(chan_name);
	if (channel.is_channel_members(leaver) == 0)
	{
		tmp = Sender::no_user_message(leaver, leaver.nickname_);
		return ret;
	}
	std::vector<User> &users = channel.get_users();
	const int user_size = users.size();
	ret = channel.send_all(leaver, leaver, msg, QUIT);
	channel.delete_from_channel_member(leaver);

	if (user_size == 1)
		delete_channel(chan_name);
	return ret;
}

event_map Database::part_channel(User &leaver, std::string &chan_name, const std::string &msg_)
{
	event_pair tmp;
	event_map ret;
	std::string msg(msg_);

	if (is_channel(chan_name) == false)
	{
		tmp = Sender::no_channel_message(leaver, chan_name);
		ret.insert(tmp);
		return ret;
	}
	Channel &channel = select_channel(chan_name);
	if (channel.is_channel_members(leaver) == 0)
	{
		tmp = Sender::no_user_message(leaver, leaver.nickname_);
		return ret;
	}
	std::vector<User> users = channel.get_users();
	int user_size = users.size();
	ret = channel.send_all(leaver, leaver, msg, PART);
	channel.delete_from_channel_member(leaver);
	if (user_size == 1)
		delete_channel(chan_name);
	return ret;
}

event_map Database::channel_msg(User &sender, std::string chan_name, const std::string &msg)
{
	event_map ret;
	event_pair tmp;

	if (is_channel(chan_name) == false)
	{
		tmp = Sender::no_channel_message(sender, chan_name);
		ret.insert(tmp);
		return ret;
	}
	Channel &channel = select_channel(chan_name);
	if (channel.is_channel_members(sender) == false)
	{
		tmp = Sender::no_user_message(sender, sender.nickname_);
		ret.insert(tmp);
		return ret;
	}
	ret = channel.send_all(sender, sender, msg, PRIV);
	return ret;
}

event_map Database::notice_channel(User &sender, std::string chan_name, const std::string &msg)
{
	event_map ret;
	event_pair tmp;

	if (is_channel(chan_name) == false)
	{
		tmp = Sender::no_channel_message(sender, chan_name);
		ret.insert(tmp);
		return ret;
	}
	Channel &channel = select_channel(chan_name);
	if (channel.is_channel_members(sender) == false)
	{
		tmp = Sender::no_user_message(sender, sender.nickname_);
		ret.insert(tmp);
		return ret;
	}
	ret = channel.send_all(sender, sender, msg, NOTICE);
	return ret;
}

event_map Database::kick_channel(User &host, User &target, std::string &chan_name, std::string &msg)
{
	event_map ret;
	event_pair tmp;

	if (is_channel(chan_name) == false)
	{
		tmp = Sender::no_channel_message(host, chan_name);
		ret.insert(tmp);
		return ret;
	}
	Channel &channel = select_channel(chan_name);
	if (channel.is_host(host))
	{
		if (channel.is_channel_members(target) == true)
		{
			ret = channel.send_all(host, target, msg, KICK);
			channel.delete_from_channel_member(target);
			if (channel.get_users().size() == 0)
				delete_channel(chan_name);
		}
		else
		{
			tmp = Sender::kick_error_no_user_message(host, host.nickname_, target.nickname_, chan_name);
			ret.insert(tmp);
		}
	}
	else
	{
		tmp = Sender::kick_error_not_op_message(host, host.nickname_, chan_name);
		ret.insert(tmp);
	}
	return ret;
}

/**		nick_channel   **/
/**		@brief NICK 명령어를 채널 안에서 한 경우   **/
/**		@brief 변경했다고 채널 모두에게 send_all, 채널 객체 nick 변경  **/

event_map Database::nick_channel(User &nicker, std::string &send_msg)
{
	event_map ret;
	event_pair tmp;
	User dummy_user;

	Channel &channel = select_channel(nicker);
	ret = channel.send_all(nicker, dummy_user, send_msg, NICK);
	channel.change_nick(nicker, send_msg);
	return ret;
}

event_map Database::set_topic(const uintptr_t &ident, std::string &chan_name, std::string &topic)
{
	event_map ret;
	event_pair tmp = valid_user_checker_(ident, "TOPIC");

	if (tmp.second.size())
	{
		ret.insert(tmp);
		return ret;
	}
	User &cur_user = select_user(ident);
	if (is_channel(chan_name) == false)
	{
		tmp = Sender::no_channel_message(cur_user, chan_name);
		ret.insert(tmp);
		return ret;
	}
	Channel &channel = select_channel(chan_name);
	if (topic.empty())
	{
		if (!channel.get_topic().empty())
		{
			tmp = Sender::show_channel_topic(cur_user, channel.get_channel_name(), channel.get_topic());
			ret.insert(tmp);
		}
		return ret;
	}
	if ((channel.channel_flag_ & F_TOPIC_OWNERSHIP) && !(channel.is_host(cur_user)))
	{
		tmp = Sender::topic_access_error(cur_user, chan_name);
		ret.insert(tmp);
	}
	else if (is_user(cur_user.nickname_) == false)
	{
		tmp = Sender::not_on_the_channel_message(cur_user, chan_name);
		ret.insert(tmp);
	}
	else
	{
		std::string topic_msg = "Topic was changed to " + topic;
		channel.set_topic(topic);
		ret = channel.send_all(cur_user, cur_user, topic_msg, TOPIC);
	}
	return ret;
}

event_map Database::command_mode_i_on(const uintptr_t &ident, t_mode_input &mode)
{
	event_map ret;
	event_pair tmp;

	Channel &tmp_channel = select_channel(mode.target);
	User &host = select_user(ident);
	if (tmp_channel.is_host(host))
	{
		tmp_channel.set_flag(tmp_channel, mode);
		ret = tmp_channel.send_all(host, host, "+i", MODE);
	}
	else
	{
		tmp = Sender::mode_error_not_op_message(host, mode.target);
		ret.insert(tmp);
	}
	return ret;
}

event_map Database::command_mode_i_off(const uintptr_t &ident, t_mode_input &mode)
{
	event_map ret;
	event_pair tmp;

	Channel &tmp_channel = select_channel(mode.target);
	User &host = select_user(ident);
	if (tmp_channel.is_host(host))
	{
		tmp_channel.set_flag(tmp_channel, mode);
		ret = tmp_channel.send_all(host, host, "-i", MODE);
	}
	else
	{
		tmp = Sender::mode_error_not_op_message(host, mode.target);
		ret.insert(tmp);
	}
	return ret;
}

event_map Database::command_mode_k_on(const uintptr_t &ident, t_mode_input &mode)
{
	event_map ret;
	event_pair tmp;

	if (mode.param.length() == 0)
	{
		tmp = Sender::mode_syntax_error(select_user(ident), mode.target, mode.option, "key", "key");
		ret.insert(tmp);
		return ret;
	}
	Channel &tmp_channel = select_channel(mode.target);
	User &host = select_user(ident);
	if (tmp_channel.is_host(host))
	{
		tmp_channel.set_password(tmp_channel, mode);
		tmp_channel.set_flag(tmp_channel, mode);
		ret = tmp_channel.send_all(host, host, "+k", MODE);
	}
	else
	{
		tmp = Sender::mode_error_not_op_message(host, mode.target);
		ret.insert(tmp);
	}
	return ret;
}

event_map Database::command_mode_k_off(const uintptr_t &ident, t_mode_input &mode)
{
	event_map ret;
	event_pair tmp;

	Channel &tmp_channel = select_channel(mode.target);
	User &host = select_user(ident);
	if (tmp_channel.is_host(host))
	{
		tmp_channel.set_flag(tmp_channel, mode);
		ret = tmp_channel.send_all(host, host, "-k", MODE);
	}
	else
	{
		tmp = Sender::mode_error_not_op_message(host, mode.target);
		ret.insert(tmp);
	}
	return ret;
}

event_map Database::command_mode_o_on(const uintptr_t &ident, t_mode_input &mode)
{
	event_map ret;
	event_pair tmp;

	if (mode.param.length() == 0)
	{
		tmp = Sender::mode_syntax_error(select_user(ident), mode.target, mode.option, "op", "nick");
		ret.insert(tmp);
		return ret;
	}
	else if (!is_user(mode.param))
	{
		tmp = Sender::mode_no_user_message(select_user(ident), mode.param);
		ret.insert(tmp);
		return ret;
	}

	Channel &target_channel = select_channel(mode.target);
	User &host_user = select_user(ident);
	if (target_channel.is_host(host_user))
	{
		if (!target_channel.is_host(select_user(mode.param)))
		{
			target_channel.set_host(select_user(mode.param));
		}
		ret = target_channel.send_all(host_user, host_user, "+o", MODE);
	}
	else
	{
		tmp = Sender::mode_error_not_op_message(host_user, mode.target);
		ret.insert(tmp);
	}
	return ret;
}

event_map Database::command_mode_o_off(const uintptr_t &ident, t_mode_input &mode)
{
	event_map ret;
	event_pair tmp;

	if (mode.param.length() == 0)
	{
		tmp = Sender::mode_syntax_error(select_user(ident), mode.target, mode.option, "op", "nick");
		ret.insert(tmp);
		return ret;
	}
	else if (!is_user(mode.param))
	{
		tmp = Sender::mode_no_user_message(select_user(ident), mode.param);
		ret.insert(tmp);
		return ret;
	}

	Channel &target_channel = select_channel(mode.target);
	User &host_user = select_user(ident);
	if (target_channel.is_host(host_user))
	{
		if (target_channel.is_host(select_user(mode.param)))
		{
			target_channel.unset_host(select_user(mode.param));
		}
		ret = target_channel.send_all(host_user, host_user, "-o", MODE);
	}
	else
	{
		tmp = Sender::mode_error_not_op_message(host_user, mode.target);
		ret.insert(tmp);
	}
	return ret;
}

event_map Database::command_mode_t_on(const uintptr_t &ident, t_mode_input &mode)
{
	event_map ret;
	event_pair tmp;

	Channel &target_channel = select_channel(mode.target);
	User &host_user = select_user(ident);
	if (target_channel.is_host(host_user))
	{
		target_channel.set_flag(target_channel, mode);
		ret = target_channel.send_all(host_user, host_user, "+t", MODE);
	}
	else
	{
		tmp = Sender::mode_error_not_op_message(host_user, mode.target);
		ret.insert(tmp);
	}
	return ret;
}

event_map Database::command_mode_t_off(const uintptr_t &ident, t_mode_input &mode)
{
	event_map ret;
	event_pair tmp;

	Channel &target_channel = select_channel(mode.target);
	User &host_user = select_user(ident);
	if (target_channel.is_host(host_user))
	{
		target_channel.set_flag(target_channel, mode);
		ret = target_channel.send_all(host_user, host_user, "-t", MODE);
	}
	else
	{
		tmp = Sender::mode_error_not_op_message(host_user, mode.target);
		ret.insert(tmp);
	}
	return ret;
}

event_map Database::command_mode_l_on(const uintptr_t &ident, t_mode_input &mode)
{
	event_map ret;
	event_pair tmp;
	int limit_num = 0;
	std::stringstream get_limit_num_stream(mode.param);
	get_limit_num_stream >> limit_num;

	if (mode.param.length() == 0)
	{
		tmp = Sender::mode_syntax_error(select_user(ident), mode.target, mode.option, "limit", "limit");
		ret.insert(tmp);
		return ret;
	}
	else if (limit_num < 0)
	{
		tmp = Sender::mode_syntax_error_l_negative_num(select_user(ident), mode.target, mode.option, mode.param);
		ret.insert(tmp);
		return ret;
	}

	Channel &target_channel = select_channel(mode.target);
	User &host_user = select_user(ident);
	if (target_channel.is_host(host_user))
	{
		target_channel.set_flag(target_channel, mode);
		target_channel.set_member_limit(limit_num);
		get_limit_num_stream.str("");
		get_limit_num_stream.clear();
		get_limit_num_stream << "\b+l :" << limit_num;
		ret = target_channel.send_all(host_user, host_user, get_limit_num_stream.str(), MODE);
	}
	else
	{
		tmp = Sender::mode_error_not_op_message(host_user, mode.target);
		ret.insert(tmp);
	}
	return ret;
}

event_map Database::command_mode_l_off(const uintptr_t &ident, t_mode_input &mode)
{
	event_map ret;
	event_pair tmp;

	if (mode.param.length() != 0)
	{
		tmp = Sender::mode_wrong_message(ident, mode.param.at(mode.param.length() - 1));
		ret.insert(tmp);
		return ret;
	}

	Channel &target_channel = select_channel(mode.target);
	User &host_user = select_user(ident);
	if (target_channel.is_host(host_user))
	{
		target_channel.set_flag(target_channel, mode);
		ret = target_channel.send_all(host_user, host_user, "-l", MODE);
	}
	else
	{
		tmp = Sender::mode_error_not_op_message(host_user, mode.target);
		ret.insert(tmp);
	}
	return ret;
}
