#include "Channel.hpp"
#include "Event.hpp"

Channel::Channel()
{
	this->channel_flag_ = 0b1000;
	this->key_ = "";
	this->topic_ = "";
	this->member_limit_ = 0;
}

void Channel::change_nick(User &usr, std::string new_nick)
{
	std::vector<User>::iterator it;
	for (it = channel_members_.begin(); it != channel_members_.end(); it++)
	{
		if (it->nickname_ == usr.nickname_)
		{
			it->nickname_ = new_nick;
			break;
		}
	}
}

std::vector<User> Channel::sort_users(void)
{
	std::vector<User> ret;

	ret.push_back(channel_members_.at(0));
	if (channel_members_.size() == 1)
		return ret;
	for (std::vector<User>::iterator it = channel_members_.begin(); it != channel_members_.end(); ++it)
	{
		std::vector<User>::iterator ret_it = ret.begin();
		for (; ret_it != ret.end(); ++ret_it)
		{
			if (*it > *ret_it)
			{
				continue;
			}
			else if (*it < *ret_it || *it == *ret_it)
			{
				break;
			}
		}
		if (ret_it == ret.end() || *it != *ret_it)
			ret.insert(ret_it, *it);
	}
	return ret;
}

std::string Channel::get_user_list_str(void)
{
	std::string ret;
	std::vector<User> sort = sort_users();
	for (std::vector<User>::iterator it = sort.begin(); it != sort.end(); ++it)
	{
		ret += (is_host(*it) ? "@" + it->nickname_ : it->nickname_);
		ret += " ";
	}
	return ret;
}

event_map Channel::send_all(User &sender, User &target, std::string msg, int remocon)
{
	event_map ret;
	std::vector<User>::iterator it;

	for (it = channel_members_.begin(); it != channel_members_.end(); it++)
	{
		event_pair packet;

		switch (remocon)
		{
		case JOIN:
			packet = Sender::join_message(sender, *it, this->get_channel_name());
			break;
		case PART:
			packet = Sender::part_message(sender, *it, this->get_channel_name(), msg);
			break;
		case PRIV:
			if (sender == *it)
			{
				packet.first = sender.client_sock_;
				packet.second = "";
			}
			else
			{
				packet = Sender::privmsg_channel_message(sender, *it, msg, this->get_channel_name());
			}
			break;
		case KICK:
			packet = Sender::kick_message(sender, *it, target.nickname_, this->get_channel_name(), msg);
			break;
		case QUIT:
			packet = (sender == *it) ? Sender::quit_leaver_message(sender, msg)
															 : Sender::quit_member_message(sender, *it, msg);
			break;
		case NOTICE:
			if (sender == *it)
			{
				packet.first = sender.client_sock_;
				packet.second = "";
			}
			else
			{
				packet = Sender::notice_channel_message(sender, *it, msg, this->get_channel_name());
			}
			break;
		case TOPIC:
			packet = Sender::topic_message(sender, *it, this->get_channel_name(), msg);
			break;
		case NICK:
			packet = Sender::nick_well_message(sender, *it, msg);
			break;
		case MODE:
			packet = Sender::mode_message(sender, *it, this->get_channel_name(), msg);
			break;
		case INVITE:
			if (sender == *it) // invitor 에게 보내는 메세지
			{
				packet = Sender::invitor_message(sender, target, this->get_channel_name());
			}
			else // 방에 있는 사람들에게 보내는 메세지
			{
				packet = Sender::invite_message(sender, *it, this->get_channel_name());
			}
			break;
		}

		ret.insert(packet);
	}
	if (remocon == INVITE)
	{
		event_pair packet = Sender::invitee_message(sender, target, this->get_channel_name());
		ret.insert(packet);
	}
	return ret;
}

std::string &Channel::get_channel_name(void)
{
	return this->name_;
}

std::string &Channel::get_topic(void)
{
	return this->topic_;
}

bool Channel::is_channel_members(User &usr)
{
	std::vector<User>::iterator it;

	for (it = channel_members_.begin(); it != channel_members_.end(); it++)
	{
		if (it->client_sock_ == usr.client_sock_)
			return 1;
	}
	return 0;
}

void Channel::add_to_channel_member(User &joiner)
{
	channel_members_.push_back(joiner);
}

void Channel::set_channel_name(std::string &chan_name)
{
	this->name_ = chan_name;
}

void Channel::set_topic(std::string &topic)
{
	this->topic_ = topic;
}

std::vector<User> &Channel::get_users()
{
	return this->channel_members_;
}
void Channel::delete_from_channel_member(User &usr)
{
	if (is_host(usr))
		unset_host(usr);

	std::vector<User>::iterator it = std::find(this->channel_members_.begin(),
																						 this->channel_members_.end(), usr);
	std::size_t size = std::distance(this->channel_members_.begin(), it);
	this->channel_members_.erase(this->channel_members_.begin() + size);
}
bool Channel::operator==(const Channel &t) const
{
	return (this->name_ == t.name_);
}

std::vector<uintptr_t> &Channel::get_hosts(void)
{
	return this->hosts_;
}

void Channel::set_host(uintptr_t host_sock)
{
	this->hosts_.push_back(host_sock);
}

void Channel::set_host(User &user)
{
	uintptr_t host_sock = user.client_sock_;
	this->hosts_.push_back(host_sock);
}

void Channel::unset_host(uintptr_t host_sock)
{
	std::vector<uintptr_t>::iterator it = std::find(this->hosts_.begin(), this->hosts_.end(), host_sock);
	std::size_t size = std::distance(this->hosts_.begin(), it);
	this->hosts_.erase(this->hosts_.begin() + size);
}

void Channel::unset_host(User &host_user)
{
	uintptr_t host_sock = host_user.client_sock_;
	std::vector<uintptr_t>::iterator it = std::find(this->hosts_.begin(), this->hosts_.end(), host_sock);
	std::size_t size = std::distance(this->hosts_.begin(), it);
	this->hosts_.erase(this->hosts_.begin() + size);
}

bool Channel::is_host(uintptr_t client_sock)
{
	std::vector<uintptr_t>::iterator it;

	for (it = hosts_.begin(); it != hosts_.end(); it++)
	{
		if (*it == client_sock)
			return true;
	}
	return false;
}

bool Channel::is_host(User &usr)
{
	std::vector<uintptr_t>::iterator it;
	uintptr_t client_sock = usr.client_sock_;

	for (it = hosts_.begin(); it != hosts_.end(); it++)
	{
		if (*it == client_sock)
			return true;
	}
	return false;
}

int Channel::get_member_limit(void)
{
	return this->member_limit_;
}

void Channel::set_member_limit(int &member_limit)
{
	this->member_limit_ = member_limit;
}

void Channel::set_flag(Channel &channel, t_mode_input &mode)
{
	if (mode.mode_type == PLUS_I && !(channel.channel_flag_ & F_INVITE_ONLY))
	{
		channel.channel_flag_ += F_INVITE_ONLY;
	}
	else if (mode.mode_type == MINUS_I && (channel.channel_flag_ & F_INVITE_ONLY))
	{
		channel.channel_flag_ -= F_INVITE_ONLY;
	}
	else if (mode.mode_type == PLUS_K && !(channel.channel_flag_ & F_KEY_NEEDED))
	{
		channel.channel_flag_ += F_KEY_NEEDED;
	}
	else if (mode.mode_type == MINUS_K && (channel.channel_flag_ & F_KEY_NEEDED))
	{
		channel.channel_flag_ -= F_KEY_NEEDED;
	}
	else if (mode.mode_type == PLUS_T && !(channel.channel_flag_ & F_TOPIC_OWNERSHIP))
	{
		channel.channel_flag_ += F_TOPIC_OWNERSHIP;
	}
	else if (mode.mode_type == MINUS_T && (channel.channel_flag_ & F_TOPIC_OWNERSHIP))
	{
		channel.channel_flag_ -= F_TOPIC_OWNERSHIP;
	}
	else if (mode.mode_type == PLUS_L && !(channel.channel_flag_ & F_LIMITED_MEMBERSHIP))
	{
		channel.channel_flag_ += F_LIMITED_MEMBERSHIP;
	}
	else if (mode.mode_type == MINUS_L && (channel.channel_flag_ & F_LIMITED_MEMBERSHIP))
	{
		channel.channel_flag_ -= F_LIMITED_MEMBERSHIP;
	}
	else
		return;
}

bool Channel::has_invitation(const uintptr_t &usr)
{
	std::vector<uintptr_t>::iterator it;

	for (it = invitations_.begin(); it != invitations_.end(); it++)
	{
		if (*it == usr)
			return true;
	}
	return false;
}
void Channel::invite_user(uintptr_t user)
{
	invitations_.push_back(user);
}

void Channel::set_password(Channel &tmp_channel, t_mode_input mode)
{
	tmp_channel.key_ = mode.param;
}

bool Channel::check_password(Channel &tmp_channel, const std::string &tmp_password)
{
	if (tmp_channel.key_ != tmp_password)
	{
		return false;
	}
	return true;
}