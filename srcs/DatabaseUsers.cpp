#include "Database.hpp"
#include "Event.hpp"
#include "User.hpp"
#include <string>
#include <sys/_types/_ct_rune_t.h>

#include "ServerStatus.hpp"

event_map (Database::*Database::mode_handlers[N_MODE_TYPE])(const uintptr_t &ident, t_mode_input &mode) =
		{
				&Database::command_mode_i_on,
				&Database::command_mode_i_off,
				&Database::command_mode_t_on,
				&Database::command_mode_t_off,
				&Database::command_mode_l_on,
				&Database::command_mode_l_off,
				&Database::command_mode_o_on,
				&Database::command_mode_o_off,
				&Database::command_mode_k_on,
				&Database::command_mode_k_off,
};

std::vector<User> &Database::get_user_list(void)
{
	return user_list_;
}

void Database::delete_error_user(const uintptr_t &ident)
{
	if (!is_user(ident))
	{
		return;
	}
	User &cur_usr = select_user(ident);
	if (is_user_in_channel(cur_usr))
	{
		Channel &cur_channel = select_channel(cur_usr);

		std::vector<User> &users = cur_channel.get_users();
		const int user_size = users.size();
		cur_channel.delete_from_channel_member(cur_usr);
		if (user_size == 1)
			delete_channel(cur_channel.get_channel_name());
	}
	user_list_.erase(remove(user_list_.begin(), user_list_.end(), cur_usr), user_list_.end());
}

event_pair Database::valid_user_checker_(const uintptr_t &ident, const std::string &command_type)
{
	event_pair ret;

	ret.first = ident;
	if (!is_user(ident))
	{
		return Sender::password_incorrect_464(ident);
	}
	User &cur_user = select_user(ident);
	if (!(cur_user.flag_ & F_PASS))
	{
		return Sender::password_incorrect_464(ident);
	}
	if (command_type == "NICK" || command_type == "USER")
	{
		return ret;
	}
	if (!(cur_user.flag_ & F_NICK))
	{
		return Sender::command_not_registered_451(ident, command_type);
	}
	if (!(cur_user.flag_ & F_USER))
	{
		return Sender::command_not_registered_451(cur_user, command_type);
	}
	return ret;
}

User &Database::select_user(const uintptr_t &ident)
{
	std::vector<User>::iterator it;

	for (it = user_list_.begin(); it != user_list_.end(); it++)
	{
		if (it->client_sock_ == ident)
		{
			return (*it);
		}
	}
	return (*it);
}

User &Database::select_user(const std::string &nickname)
{
	std::vector<User>::iterator it;

	for (it = user_list_.begin(); it != user_list_.end(); it++)
	{
		if (it->nickname_ == nickname)
		{
			return (*it);
		}
	}
	return (*it);
}

bool Database::is_user(const uintptr_t &ident)
{
	for (std::vector<User>::iterator it = user_list_.begin(); it != user_list_.end(); it++)
	{
		if (it->client_sock_ == ident)
		{
			return (true);
		}
	}
	return (false);
}
bool Database::is_user(const std::string &nickname)
{
	for (std::vector<User>::iterator it = user_list_.begin(); it != user_list_.end(); it++)
	{
		if (it->nickname_ == nickname)
		{
			return (true);
		}
	}
	return (false);
}

void Database::delete_user(User &leaver)
{
	int idx = 0;
	std::vector<User>::iterator it;

	for (it = user_list_.begin(); it != user_list_.end(); it++)
	{
		if (*it == leaver)
		{
			user_list_.erase(user_list_.begin() + idx);
			break;
		}
		idx++;
	}
}

/**		is_valid_nick   **/
/**		@brief NICK이 유효한지 검사  **/
/**		@brief NICK이 첫글자가 알파벳, _ 가 아니면 에러  **/
/**		@brief NICK이 첫글자 제외 알파벳, 숫자, _ 아니면 에러  **/

bool Database::is_valid_nick(std::string &new_nick)
{
	if (!isalpha(static_cast<int>(new_nick[0])) && static_cast<int>(new_nick[0]) != '_')
		return false;
	for (std::size_t i(1); i < new_nick.size(); i++)
	{
		if (!isalnum(static_cast<int>(new_nick[i])) && static_cast<int>(new_nick[i]) != '_')
		{
			return false;
		}
	}
	return true;
}

event_map Database::command_mode(const uintptr_t &ident, t_mode_input &mode)
{
	event_map ret;
	event_pair tmp = valid_user_checker_(ident, "MODE");

	if (tmp.second.size())
	{
		ret.insert(tmp);
	}
	else if (is_channel(mode.target) == false)
	{
		tmp = Sender::no_channel_message(select_user(ident), mode.target);
		ret.insert(tmp);
		return ret;
	}
	else
		ret = (this->*mode_handlers[mode.mode_type])(ident, mode);
	return ret;
}

event_map Database::command_invite(const uintptr_t &ident, std::string &user, std::string &chan_name)
{
	event_map ret;
	event_pair tmp = valid_user_checker_(ident, "INVITE");

	if (tmp.second.size())
	{
		ret.insert(tmp);
	}
	else if (is_channel(chan_name) == false)
	{
		event_pair tmp = Sender::no_channel_message(select_user(ident), chan_name);
		ret.insert(tmp);
	}
	else if (is_user(user) == false)
	{
		event_pair tmp = Sender::invite_no_user_message(select_user(ident), user);
		ret.insert(tmp);
	}
	else if (is_user_in_channel(select_user(user)) == true) // 이미 이 방에 있는 유저라면
	{
		event_pair tmp = Sender::already_in_channel_message(select_user(ident), chan_name);
		ret.insert(tmp);
	}
	else
	{
		User &invitor = select_user(ident);
		User &invited_user = select_user(user);
		Channel &cur_channel = select_channel(chan_name);
		cur_channel.invite_user(invited_user.client_sock_);
		ret = cur_channel.send_all(invitor, invited_user, chan_name, INVITE);
		tmp.first = invitor.client_sock_;
		tmp.second.clear();
		ret.insert(tmp);
	}
	return ret;
}

event_pair Database::command_pass(const uintptr_t &ident)
{
	event_pair tmp;

	tmp.first = ident;
	if (!is_user(ident))
	{
		User tmp_user;
		tmp_user.client_sock_ = ident;
		tmp_user.flag_ |= F_PASS;
		user_list_.push_back(tmp_user);
	}
	ServerStatus::print_users(user_list_);
	return tmp;
}

event_map Database::command_nick(const uintptr_t &ident, std::string &new_nick)
{
	event_map ret;
	event_pair tmp = valid_user_checker_(ident, "NICK");

	if (tmp.second.size())
	{
		ret.insert(tmp);
		return ret;
	}
	User &cur_usr = select_user(ident);
	/* 유효한 NICK 인지 확인 */
	if (!is_valid_nick(new_nick))
	{
		/* NICK이 비어있는 경우*/
		if (cur_usr.nickname_.empty())
			tmp = Sender::nick_wrong_message(ident, new_nick);
		/* NICK이 비어있지 않지만, 잘못된 경우 */
		else
		{
			User &existed_usr = select_user(cur_usr.nickname_);
			tmp = Sender::nick_wrong_message(existed_usr, new_nick);
		}
		ret.insert(tmp);
		return ret;
	}
	/* 새로운 NICK이 이미 존재하는 NICK인 경우*/
	if (is_user(new_nick))
	{
		User &existed_usr = select_user(new_nick);
		/* 자기 자신인 경우 문제 없음 */
		if (ident == existed_usr.client_sock_)
			return ret;
		/* 이미 다른 유저가 NICK과 username을 선점한 경우 */
		if (existed_usr.username_.size())
		{
			/* 아직 자신의 NICK이 없는 경우 */
			if (cur_usr.nickname_.empty())
				tmp = Sender::nick_error_message(ident, new_nick);
			/* 자신의 NICK이 있는 경우 */
			else
				tmp = Sender::nick_error_message(cur_usr, new_nick);
			ret.insert(tmp);
			return ret;
		}
		/* NICK을 빼앗는 경우 */
		tmp = Sender::nick_error_message2(existed_usr, new_nick);
		ret.insert(tmp);
		existed_usr.nickname_.clear();
		existed_usr.flag_ &= ~F_NICK;
	}
	/* NICK 처음 설정하는 경우 */
	if (!(cur_usr.flag_ & F_NICK))
	{
		cur_usr.nickname_ = new_nick;
		cur_usr.flag_ |= F_NICK;
		/* USER가 먼저 설정 돼있다면, welcome_message 출력 */
		if (cur_usr.flag_ & F_USER)
		{
			tmp = Sender::welcome_message_connect(cur_usr);
			ret.insert(tmp);
		}
	}
	else
	{
		/* 만약 NICK 명령어를 쓴 유저가 채널 안에 있다면 바꾸고, send_all */
		if (is_user_in_channel(cur_usr))
		{
			ret = nick_channel(cur_usr, new_nick);
		}
		tmp = Sender::nick_well_message(cur_usr, cur_usr, new_nick);
		cur_usr.nickname_ = new_nick;
		ret.insert(tmp);
	}
	ServerStatus::print_users(user_list_);
	return ret;
}

event_pair Database::command_user(const uintptr_t &ident, const std::string &username, const std::string &mode, const std::string &unused, const std::string &realname)
{
	event_pair ret = valid_user_checker_(ident, "USER");

	if (ret.second.size())
	{
		return ret;
	}
	User &cur_usr = select_user(ident);
	cur_usr.input_user(username, mode, unused, realname);
	if (!(cur_usr.flag_ & F_USER))
	{
		cur_usr.flag_ |= F_USER;
		if (cur_usr.flag_ & F_NICK)
			ret = Sender::welcome_message_connect(cur_usr);
		else
			ret = Sender::user_302_message(cur_usr);
	}
	else
		ret = Sender::user_302_message(cur_usr);
	ServerStatus::print_users(user_list_);
	return ret;
}

event_pair Database::command_pong(const uintptr_t &ident, std::string &msg)
{
	event_pair ret = valid_user_checker_(ident, "PING");

	if (ret.second.size())
	{
		return ret;
	}
	User &cur_user = select_user(ident);
	if (msg.at(0) == ':' && msg.length() == 1)
	{
		ret = Sender::command_no_origin_specified_409(cur_user, "PING");
		return ret;
	}
	if (msg.at(0) == ':')
		msg = msg.substr(1, msg.length() - 1);
	ret = Sender::pong(ident, msg);
	return ret;
}

event_map Database::command_join(const uintptr_t &ident, const std::string &chan_name, const std::string &tmp_password)
{
	event_map ret;
	event_pair tmp = valid_user_checker_(ident, "JOIN");

	if (tmp.second.size())
	{
		ret.insert(tmp);
		return ret;
	}
	else if (is_user(ident))
	{
		User &cur_usr = select_user(ident);
		if (chan_name.empty())
		{
			if (!is_user(ident))
			{
				tmp = Sender::command_empty_argument_461(ident, "JOIN");
				ret.insert(tmp);
			}
			else
			{
				tmp = Sender::command_empty_argument_461(cur_usr, "JOIN");
				ret.insert(tmp);
			}
		}
		else if (chan_name.at(0) != '#')
		{
			tmp = Sender::join_invalid_channel_name_message(cur_usr, chan_name);
			ret.insert(tmp);
		}
		else if (chan_name.find(',') != std::string::npos)
		{
			tmp = Sender::join_invalid_channel_name_message(cur_usr, chan_name);
			ret.insert(tmp);
		}
		else
		{
			ret = join_channel(cur_usr, chan_name, tmp_password);
		}
	}
	return ret;
}

event_map Database::command_part(const uintptr_t &ident, std::string &chan_name, const std::string &msg)
{
	event_map ret;
	event_pair tmp = valid_user_checker_(ident, "PART");

	if (tmp.second.size())
	{
		ret.insert(tmp);
		return ret;
	}
	User &cur_usr = select_user(ident);
	ret = part_channel(cur_usr, chan_name, msg);
	return ret;
}

event_map Database::command_quit(const uintptr_t &ident, const std::string &msg)
{
	event_map ret;
	event_pair tmp = valid_user_checker_(ident, "QUIT");

	if (tmp.second.size())
	{
		ret.insert(tmp);
		return ret;
	}
	else if (is_user(ident))
	{
		User &cur_usr = select_user(ident);

		if (is_user_in_channel(cur_usr))
		{
			Channel &cur_channel = select_channel(cur_usr);
			ret = quit_channel(cur_usr, cur_channel.get_channel_name(), msg);
		}
		else
		{
			tmp = Sender::quit_leaver_message(cur_usr, msg);
			ret.insert(tmp);
		}
		user_list_.erase(remove(user_list_.begin(), user_list_.end(), cur_usr), user_list_.end());
	}
	return ret;
}

event_map Database::command_privmsg(const uintptr_t &ident, const std::string &target_name, const std::string &msg)
{
	event_map ret;
	event_pair tmp = valid_user_checker_(ident, "PRIVMSG");

	if (tmp.second.size())
	{
		ret.insert(tmp);
		return ret;
	}
	if (is_user(ident))
	{
		User &cur_usr = select_user(ident);

		tmp.first = ident;
		if (target_name.at(0) == '#')
		{
			ret = channel_msg(cur_usr, target_name, msg);
		}
		else
		{
			if (target_name == "BOT")
			{
				tmp = bot_privmsg(cur_usr, msg);
				ret.insert(tmp);
			}
			else if (is_user(target_name))
			{
				User &tar_usr = select_user(target_name);
				tmp = Sender::privmsg_p2p_message(cur_usr, tar_usr, msg);
				ret.insert(tmp);
				tmp.first = cur_usr.client_sock_;
				tmp.second.clear();
				ret.insert(tmp);
			}
			else
			{
				tmp = Sender::privmsg_no_user_error_message(cur_usr, target_name);
				ret.insert(tmp);
			}
		}
	}
	return ret;
}

event_map Database::command_notice(const uintptr_t &ident, const std::string &target_name, const std::string &msg)
{
	event_map ret;
	event_pair tmp = valid_user_checker_(ident, "NOTICE");

	if (tmp.second.size())
	{
		ret.insert(tmp);
		return ret;
	}
	if (target_name == "BOT")
	{
		ret.insert(tmp);
		return ret;
	}
	User &cur_usr = select_user(ident);
	if (target_name.at(0) == '#')
	{
		ret = notice_channel(cur_usr, target_name, msg);
	}
	else
	{
		if (is_user(target_name))
		{
			User &tar_usr = select_user(target_name);
			tmp = Sender::notice_p2p_message(cur_usr, tar_usr, msg);
			ret.insert(tmp);
			tmp.first = cur_usr.client_sock_;
			tmp.second.clear();
			ret.insert(tmp);
		}
		else
		{
			tmp = Sender::notice_no_nick_message(cur_usr, cur_usr);
			ret.insert(tmp);
		}
	}
	return ret;
}

event_map Database::command_kick(const uintptr_t &ident, const std::string &target_name, std::string &chan_name, std::string &msg)
{
	event_pair tmp = valid_user_checker_(ident, "KICK");
	event_map ret;

	if (tmp.second.size())
	{
		ret.insert(tmp);
		return ret;
	}
	if (is_user(ident))
	{
		User &kicker = select_user(ident);
		if (is_user(target_name))
		{
			User &target = select_user(target_name);
			ret = kick_channel(kicker, target, chan_name, msg);
		}
		else
		{
			tmp = Sender::no_user_message(kicker, target_name);
			ret.insert(tmp);
		}
	}
	return ret;
}

void Database::bot_maker(const std::string &name)
{

	User tmp_usr;

	tmp_usr.nickname_ = name;
	tmp_usr.input_user("Dummy", "Dummy", "localhost", "Dummy");
	user_list_.push_back(tmp_usr);
}
event_pair Database::bot_privmsg(User &cur_usr, const std::string &msg)
{
	event_pair tmp;
	std::string bot_msg;

	if (msg == "!command")
	{
		bot_msg = "NICK, USER, PING, JOIN, QUIT, PRIVMSG, KICK, PART, TOPIC, NOTICE";
	}
	else if (msg == "!channel")
	{
		if (channel_list_.empty())
		{
			bot_msg = "NO CHANNEL IN THIS SERVER!";
		}
		else
		{
			bot_msg = "● [CHANNEL LIST] : ";
			for (std::size_t i(0); i < channel_list_.size(); ++i)
			{
				bot_msg += std::to_string(i) + ". " + channel_list_[i].get_channel_name() + " : " + channel_list_[i].get_topic() + ((i == (channel_list_.size() - 1)) ? "" : ", ");
			}
		}
	}
	else if (msg == "!user")
	{
		bot_msg = "● [USER LIST] : ";
		std::size_t n(1);
		for (std::size_t i(1); i < user_list_.size(); ++i)
		{
			if ((user_list_[i].flag_ & F_PASS) && (user_list_[i].flag_ & F_NICK))
				bot_msg += std::to_string(n++) + ". " + user_list_[i].nickname_ + ((i == (user_list_.size() - 1)) ? "" : ", ");
		}
	}
	else
	{
		bot_msg = "THAT IS NOT MY COMMAND. YOU CAN USE : '!command' & '!channel' & !user.";
	}
	tmp = Sender::privmsg_bot_message(cur_usr, bot_msg);
	return tmp;
}
