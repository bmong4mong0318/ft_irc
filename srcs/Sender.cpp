#include "Sender.hpp"

const std::string Sender::server_name_ = "irc.local";

/****************************       <PING && PONG && USE && etc>       ****************************/

/** @brief PING명령에 대한 응답으로 PONG 전송 **/
event_pair Sender::pong(const uintptr_t &socket, const std::string &msg)
{
	event_pair ret;

	const std::string &pong_reply = ":" + Sender::server_name_ + " PONG " + Sender::server_name_ + " :" + msg;
	ret = std::make_pair(socket, pong_reply + "\r\n");
	return ret;
}

/** @brief 461 - 충분한 수의 parameter가 오지 않았을 때 보내는 패킷 매세지 **/
event_pair Sender::command_empty_argument_461(const User &sender, const std::string &command)
{
	event_pair ret;

	const std::string &error_message = ":" + Sender::server_name_ + " 461 " + sender.nickname_ + " " + command + " :Not enough parameters";
	ret = std::make_pair(sender.client_sock_, error_message + "\r\n");
	return ret;
}

/** @brief 461 - 충분한 수의 parameter가 오지 않았을 때 보내는 패킷 메세지 **/
event_pair Sender::command_empty_argument_461(const uintptr_t &sock, const std::string &command)
{
	event_pair ret;

	const std::string &error_message = ":" + Sender::server_name_ + " 461 * " + command + " :Not enough parameters";
	ret = std::make_pair(sock, error_message + "\r\n");
	return ret;
}

/** @brief 451 - 클라이언트가 등록되지 않았을 때 보내는 오류 패킷 메세지 (USER가 등록이 안된 경우) **/
event_pair Sender::command_not_registered_451(const User &sender, const std::string &command)
{
	event_pair ret;

	const std::string &error_message = ":" + Sender::server_name_ + " 451 " + sender.nickname_ + " " + command + " :You have not registered";
	ret = std::make_pair(sender.client_sock_, error_message + "\r\n");
	return ret;
}

/** @brief 451 - 클라이언트가 등록되지 않았을 때 보내는 오류 패킷 매세지 (NICK이 등록이 안된 경우) **/
event_pair Sender::command_not_registered_451(const uintptr_t &sock, const std::string &command)
{
	event_pair ret;

	const std::string &error_message = ":" + Sender::server_name_ + " 451 * " + " " + command + " :You have not registered";
	ret = std::make_pair(sock, error_message + "\r\n");
	return ret;
}

/** @brief 409 - PING or PONG message missing the originator parameter **/
event_pair Sender::command_no_origin_specified_409(const User &sender, const std::string &command)
{
	event_pair ret;

	const std::string &error_message = ":" + Sender::server_name_ + " 409 " + sender.nickname_ + " " + command + " :No origin specified";
	ret = std::make_pair(sender.client_sock_, error_message + "\r\n");
	return ret;
}

// /****************************       <NICK>       ****************************/

/** @brief nickname이 정상적으로 바뀌었을 때 보내는 패킷 메시지 **/
event_pair Sender::nick_well_message(const User &sender, const User &receiver, const std::string &new_nick) // 1st done
{
	event_pair ret;

	const std::string &nick_msg = ":" + sender.nickname_ + "!" + sender.username_ + "@" + sender.unused_ + " NICK :" + new_nick;
	ret = std::make_pair(receiver.client_sock_, nick_msg + "\r\n");
	return ret;
}

/** @brief 904 - 잘못된 패드워드를 입력하였을 때 보내는 오류 패킷 메세지 **/
event_pair Sender::password_incorrect_464(const uintptr_t &sock)
{
	event_pair ret;

	const std::string &nick_msg = ":" + Sender::server_name_ + " 464 " + "User :Password Incorrect, Server disconnected";
	ret = std::make_pair(sock, nick_msg + "\r\n");
	return ret;
}

/** @brief 433 - 이미 있는 nickname을 사용 혹은 그것으로 교체하려 할 때 보내는 오류 패킷 메세지 **/
event_pair Sender::nick_error_message(const User &sender, const std::string &new_nick) // 1st done
{
	event_pair ret;

	const std::string &nick_msg = ":" + Sender::server_name_ + " 432 " + sender.nickname_ + " " + new_nick + " :Nickname is already in use.";
	ret = std::make_pair(sender.client_sock_, nick_msg + "\r\n");
	return ret;
}

/** @brief 433 - nickname을 만드는 규칙에 위배될 때 보내는 오류 패킷 메세지 **/
event_pair Sender::nick_error_message2(const User &sender, const std::string &new_nick) // 1st done
{
	event_pair ret;

	const std::string &nick_msg = ":" + Sender::server_name_ + " 433 " + sender.nickname_ + " " + new_nick + " :Nickname overruled.";
	ret = std::make_pair(sender.client_sock_, nick_msg + "\r\n");
	return ret;
}

/** @brief 433 - 이미 있는 nickname을 사용 혹은 그것으로 교체하려 할 때 보내는 오류 패킷 메세지 **/
event_pair Sender::nick_error_message(const uintptr_t &sock, const std::string &new_nick)
{
	event_pair ret;

	const std::string &nick_msg = ":" + Sender::server_name_ + " 433 * " + new_nick + " :Nickname is already in use.";
	ret = std::make_pair(sock, nick_msg + "\r\n");
	return ret;
}

/** @brief 432 - nickname을 만드는 규칙에 위배될 때 보내는 오류 패킷 메세지 **/
event_pair Sender::nick_wrong_message(const User &sender, const std::string &new_nick)
{
	event_pair ret;

	const std::string &nick_msg = ":" + Sender::server_name_ + " 432 " + sender.nickname_ + " " + new_nick + " Erroneous Nickname.";
	ret = std::make_pair(sender.client_sock_, nick_msg + "\r\n");
	return ret;
}

/** @brief 432 - nickname을 만드는 규칙에 위배될 때 보내는 오류 패킷 메세지 **/
event_pair Sender::nick_wrong_message(const uintptr_t &sock, const std::string &new_nick)
{
	event_pair ret;

	const std::string &nick_msg = ":" + Sender::server_name_ + " 432 " + " * " + " " + new_nick + " Erroneous Nickname.";
	ret = std::make_pair(sock, nick_msg + "\r\n");
	return ret;
}

// /****************************       <Connect server || channel>       ****************************/

/** @brief connect 시 보내는 환영 패킷 메세지 **/
event_pair Sender::welcome_message_connect(const User &sender)
{
	event_pair ret;

	const std::string &msg001 = ":" + Sender::server_name_ + " 001 " + sender.nickname_ + " :Welcome to the 42's irc network " + sender.nickname_ + "!" + sender.mode_;
	ret = std::make_pair(sender.client_sock_, msg001 + "\r\n");
	return ret;
}

/** @brief user시 보내는 302 패킷 메세지 **/
event_pair Sender::user_302_message(const User &sender)
{
	event_pair ret;
	std::string msg;

	if (sender.flag_ & F_NICK)
		msg = ":" + Sender::server_name_ + " 302 " + sender.nickname_ + " :" + sender.username_ + "=+root@127.0.0.1 " + sender.mode_ + "=+root@127.0.0.1 " + sender.unused_ + "=+root@127.0.0.1 " + sender.realname_ + "=+root@127.0.0.1";
	else
		msg = ":" + Sender::server_name_ + " 302  :";

	ret = std::make_pair(sender.client_sock_, msg + "\r\n");
	return ret;
}

// /****************************       <QUIT>       ****************************/

/** @brief quit을 한 클라이언트에 보내는 패킷 메세지 **/
event_pair Sender::quit_leaver_message(const User &sender, std::string leave_message)
{
	event_pair ret;

	if (leave_message.empty())
	{
		leave_message = "leaving";
	}

	const std::string &quit_channel_message = "ERROR :Closing link: (" + sender.nickname_ + ") [Quit: " + leave_message + "]";
	ret = std::make_pair(sender.client_sock_, quit_channel_message + "\r\n");
	return ret;
}

/** @brief quit을 한 클라이언트이외 같은 채널 내 다른 클라이언트들 보내는 패킷 메세지 **/
event_pair Sender::quit_member_message(const User &sender, const User &receiver, std::string leave_message)
{
	event_pair ret;

	if (leave_message.empty())
	{
		leave_message = "leaving";
	}
	const std::string &quit_message = ":" + sender.nickname_ + "!" + sender.username_ + "@" + sender.unused_ + " QUIT :Quit: " + leave_message;
	ret = std::make_pair(receiver.client_sock_, quit_message + "\r\n");
	return ret;
}

// /****************************       <JOIN>       ****************************/

/**  @brief join 성공 시 보내는 패킷 메세지 **/
event_pair Sender::join_message(const User &sender, const User &receiver, const std::string &channel)
{
	event_pair ret;

	const std::string &join_message = ":" + sender.nickname_ + "!" + sender.username_ + "@127.0.0.1 JOIN :" + channel;
	ret = make_pair(receiver.client_sock_, join_message + "\r\n");
	return ret;
}

/** @brief 476 - join 할 때 invaild한 채널 이름을 만들 경우 보내는 오류 패킷 메세지 ex) "#" 과 같은 이름 **/
event_pair Sender::join_invalid_channel_name_message(const User &sender, const std::string invaild_channel)
{
	event_pair ret;

	const std::string &error_message = ":" + Sender::server_name_ + " 476 " + sender.nickname_ + " " + invaild_channel + " :Invaild channel name";
	ret = std::make_pair(sender.client_sock_, error_message + "\r\n");
	return ret;
}

/** @brief 353 - join 시 같이 보내는 패킷 메세지 **/
std::string Sender::join_353_message(const User &sender, const std::string &chan_name, const std::string &chan_user_list)
{
	const std::string &ret = ":" + Sender::server_name_ + " 353 " + sender.nickname_ + " " + "=" + " " + chan_name + " :" + chan_user_list;
	return ret + "\r\n";
}

/** @brief 366 - join 시 같이 보내는 패킷 메세지 **/
std::string Sender::join_366_message(const User &sender, const std::string &chan_name)
{
	const std::string &ret = ":" + Sender::server_name_ + " 366 " + sender.nickname_ + " " + chan_name + " :End of /NAMES list.";
	return ret + "\r\n";
}

/****************************       <PART>       ****************************/

/** @brief part 할 때 leaver 외에 다른 클라이언트들에게도 보내는 패킷 메세지 **/
event_pair Sender::part_message(const User &sender, const User &receiver, const std::string &channel, const std::string &msg)
{
	event_pair ret;

	const std::string &part_message = ":" + sender.nickname_ + "!" + sender.username_ + "@" + receiver.mode_ + " PART " + channel + " " + msg + "";
	ret = std::make_pair(receiver.client_sock_, part_message + "\r\n");
	return ret;
}

/****************************       <KICK>       ****************************/

/** @brief kick 할 때 target외에 다른 클라이언트들에게 보내지는 패킷 메세지 **/
event_pair Sender::kick_message(const User &sender, const User &receiver, const std::string &target, const std::string &channel, const std::string &msg)
{
	event_pair ret;

	const std::string &kick_message = ":" + sender.nickname_ + "!" +
																		sender.username_ + '@' + sender.unused_ + " KICK " + channel + " " + target + " :" + msg + "";
	ret = std::make_pair(receiver.client_sock_, kick_message + "\r\n");
	return ret;
}

/** @brief 482 - operator 권한이 없는 클라이언트가 kick 명령어를 사용하려 할 때 보내는 오류 메세지 **/
event_pair Sender::kick_error_not_op_message(const User &sender, const std::string &host, const std::string &channel)
{
	event_pair ret;

	const std::string &kick_message = ":" + Sender::server_name_ +
																		" 482 " + host + " " + channel + " :You must be a channel operator";
	ret = std::make_pair(sender.client_sock_, kick_message + "\r\n");
	return ret;
}

/** @brief 441 - kick을 당할 target 클라이언트가 존재하지 않을 경우 보내는 오류 패킷 메세지 **/
event_pair Sender::kick_error_no_user_message(const User &sender, const std::string &host, const std::string &target, const std::string &channel)
{
	event_pair ret;

	const std::string &kick_message = ":" + Sender::server_name_ +
																		" 441 " + host + " " + target + " " + channel + " :They are not on that channel";
	ret = std::make_pair(sender.client_sock_, kick_message + "\r\n");
	return ret;
}

/****************************       <Privmsg>       ****************************/

/** @brief BOT을 call 했을 때 bot에게 보내는 패킷 메세지 **/
event_pair Sender::privmsg_bot_message(const User &sender, const std::string &msg)
{
	event_pair ret;

	const std::string &privmsg = ":BOT!dummy@" + sender.unused_ + " PRIVMSG " +
															 sender.nickname_ + " :" + msg;
	ret = std::make_pair(sender.client_sock_, privmsg + "\r\n");
	return (ret);
}

/** @brief target에게 private message를 보낼 때 target 클라이언트에게 보내는 패킷 메세지 **/
event_pair Sender::privmsg_p2p_message(const User &sender, const User &target, const std::string &msg)
{
	event_pair ret;

	const std::string &privmsg = ":" + sender.nickname_ + "!" + sender.username_ + "@" + sender.unused_ + " PRIVMSG " +
															 target.nickname_ + " :" + msg;
	ret = std::make_pair(target.client_sock_, privmsg + "\r\n");
	return (ret);
}

/** @brief target channel에 있는 receiver 클라이언트에게 private message를 보낼 때 클라이언트들에게 가는 패킷 메세지 **/
event_pair Sender::privmsg_channel_message(const User &sender, const User &receiver, const std::string &msg, const std::string &channel)
{
	event_pair ret;

	const std::string &privmsg = ":" + sender.nickname_ + "!" + sender.username_ + "@" + sender.unused_ + " PRIVMSG " +
															 channel + " :" + msg;
	ret = std::make_pair(receiver.client_sock_, privmsg + "\r\n");
	return (ret);
}

/** @brief 401 - privmsg를 존재하지 않는 target에게 보낼 때 클라이언트에게 보내지는 오류 패킷 메세지 **/
event_pair Sender::privmsg_no_user_error_message(const User &sender, const std::string &target)
{
	event_pair ret;

	const std::string &privmsg = ":" + sender.nickname_ + " 401 " + sender.nickname_ + " " + target + " :No such nick";
	ret = std::make_pair(sender.client_sock_, privmsg + "\r\n");
	return (ret);
}

/****************************       <NOTICE>       ****************************/

/** @brief target 클라이언트에게 notice 명령어를 사용할 때 target 클라이언트에게 보내지는 패킷 메세지 **/
event_pair Sender::notice_p2p_message(const User &sender, const User &target, const std::string &msg)
{
	event_pair ret;

	const std::string &privmsg = ":" + sender.nickname_ + "@" + sender.unused_ + " NOTICE " +
															 target.nickname_ + " :" + msg;
	ret = std::make_pair(target.client_sock_, privmsg + "\r\n");
	return (ret);
}

/** @brief target channel에게 notice 를 할 때 receiver 클라이언트에게 보내지는 패킷 메세지 **/
event_pair Sender::notice_channel_message(const User &sender, const User &receiver, const std::string &msg, const std::string &channel)
{
	event_pair ret;

	const std::string &privmsg = ":" + sender.nickname_ + "@" + sender.unused_ + " NOTICE " +
															 channel + " :" + msg;
	ret = std::make_pair(receiver.client_sock_, privmsg + "\r\n");
	return (ret);
}

/** @brief  notice의 대상이 되는 receiver 클라이언트가 존재하지 않을 때 sender에게 보내는 패킷 메시지 **/
event_pair Sender::notice_no_nick_message(const User &sender, const User &receiver)
{
	event_pair ret;

	const std::string &privmsg = ":" + Sender::server_name_ + " 401 " + sender.nickname_ + " " + receiver.username_ +
															 " No such nick";
	ret = std::make_pair(sender.client_sock_, privmsg + "\r\n");
	return (ret);
}

/****************************       <TOPIC>       ****************************/

/** @brief topic이 변경됬음을 알리는 패킷 메시지 **/
event_pair Sender::topic_message(const User &sender, const User &receiver, const std::string &channel, const std::string &topic)
{
	event_pair ret;

	const std::string &topic_msg = ":" + sender.unused_ + "!" + sender.username_ + " TOPIC " +
																 channel + " " + topic;
	ret = std::make_pair(receiver.client_sock_, topic_msg + "\r\n");
	return (ret);
}

/**  @brief 482 mode +t일 때 권한 문제로 실패 시 보내는 패킷 메세지 **/
event_pair Sender::topic_access_error(const User &sender, const std::string &channel)
{
	event_pair ret;

	const std::string &topic_msg = ":" + Sender::server_name_ + " 482 " + sender.nickname_ + " " + channel + " :You do not have access to change the topic on this channel";
	ret = std::make_pair(sender.client_sock_, topic_msg + "\r\n");
	return ret;
}

/**  @brief 332 return the topic of channel **/
event_pair Sender::show_channel_topic(const User &sender, const std::string &channel_name, const std::string &channel_topic)
{
	event_pair ret;

	const std::string &topic_msg = ":" + Sender::server_name_ + " 332 " + sender.nickname_ + " " + channel_name + " :" + channel_topic;
	ret = std::make_pair(sender.client_sock_, topic_msg + "\r\n");
	return ret;
}

/****************************       <NO ** message>       ****************************/
event_pair Sender::not_on_the_channel_message(const User &sender, const std::string &channel)
{
	event_pair ret;

	const std::string &no_msg = ":" + Sender::server_name_ + " 442 " + sender.nickname_ + " " + channel +
															 " :You’re not on that channel!";
	ret = std::make_pair(sender.client_sock_, no_msg + "\r\n");
	return (ret);
}

/** @brief 403 - 존재하지 않는 channel을 parameter로 했을 때 발생하는 패킷 메세지 **/
event_pair Sender::no_channel_message(const User &sender, const std::string &channel)
{
	event_pair ret;
	std::string no_msg;

	if (channel == "#")
	{
		no_msg = ":" + Sender::server_name_ + " 403 " + sender.nickname_ + " " +
						 channel + " " + ":No such const user in every channel";
	}
	else
	{
		no_msg = ":" + Sender::server_name_ + " 403 " + sender.nickname_ + " " +
						 channel + " " + ":No such channel";
	}
	ret = std::make_pair(sender.client_sock_, no_msg + "\r\n");
	return (ret);
}

/** @brief 401 - 존재하지 않는 target 클라이언트를 parameter로 사용하려 할 때 보내는 패킷 메시지 **/
event_pair Sender::no_user_message(const User &sender, const std::string &target)
{
	event_pair ret;

	const std::string &no_msg = ":" + Sender::server_name_ + " 401 " + sender.nickname_ + " " +
															target + " " + ":No such const user";
	ret = std::make_pair(sender.client_sock_, no_msg + "\r\n");
	return (ret);
}

// /****************************       <MODE>       ****************************/

/**  @brief mode 성공 시 보내는 패킷 메세지 **/
event_pair Sender::mode_message(const User &sender, const User &receiver, const std::string &channel, const std::string &mode_type)
{
	event_pair ret;

	const std::string &join_message = ":" + sender.nickname_ + "!" + sender.username_ + "@127.0.0.1 MODE " + channel + " :" + mode_type;
	ret = make_pair(receiver.client_sock_, join_message + "\r\n");
	return ret;
}

/**  @brief 482 operator가 아니고 MODE 실패 시 보내는 패킷 메세지 **/
event_pair Sender::mode_error_not_op_message(const User &sender, const std::string &channel)
{
	event_pair ret;

	const std::string &join_message = ":" + Sender::server_name_ + " 482 " + sender.nickname_ + " " + channel + " " + "You're not channel operator";
	ret = make_pair(sender.client_sock_, join_message + "\r\n");
	return ret;
}

/**  @brief mode 불필요한 파라미터가 들어왔을 시 보내는 패킷 메세지 **/
event_pair Sender::command_too_many_argument_461(const uintptr_t &sock, const std::string &command)
{
	event_pair ret;

	const std::string &error_message = ":" + Sender::server_name_ + " 461 * " + command + " : too many parameters";
	ret = std::make_pair(sock, error_message + "\r\n");
	return ret;
}

/** @brief 472 MODE 옵션이 잘못됐을때 보내는 패킷 메시지 **/

event_pair Sender::mode_wrong_message(const uintptr_t &socket, const char &mode_option)
{
	event_pair ret;

	const std::string &error_message = ":" + Sender::server_name_ + " 472 " + mode_option + " :is not a recognized channel mode.";
	ret = std::make_pair(socket, error_message + "\r\n");
	return ret;
}

/** @brief 401 - 존재하지 않는 target 클라이언트를 parameter로 사용하려 할 때 보내는 패킷 메시지 **/
event_pair Sender::mode_no_user_message(const User &sender, const std::string &target)
{
	event_pair ret;

	const std::string &no_msg = ":" + Sender::server_name_ + " 401 " + sender.nickname_ + " " +
															target + " " + ":No such nick";
	ret = std::make_pair(sender.client_sock_, no_msg + "\r\n");
	return (ret);
}

/** @brief 696 - param이 입력되어야 하는 mode 명령어에서 param이 존재하지 않을 때 **/
event_pair Sender::mode_syntax_error(const User &sender, const std::string &target, const std::string &mode_option, const std::string &description, const std::string &err_syntax)
{
	event_pair ret;

	const std::string &msg = ":" + Sender::server_name_ + " 696 " + sender.nickname_ + " " +
													 target + " " + mode_option + " * :You must specify a parameter for the " + description + " mode. Syntax: <" + err_syntax + ">.";
	ret = std::make_pair(sender.client_sock_, msg + "\r\n");
	return (ret);
}

/** @brief 696 - param이 입력되어야 하는 mode 명령어에서 param이 존재하지 않을 때 **/
event_pair Sender::mode_syntax_error_l_negative_num(const User &sender, const std::string &target, const std::string &mode_option, const std::string &mode_param)
{
	event_pair ret;

	const std::string &msg = ":" + Sender::server_name_ + " 696 " + sender.nickname_ + " " +
													 target + " " + mode_option + " " + mode_param + " :Invalid limit mode parameter. Syntax: <limit>.";
	ret = std::make_pair(sender.client_sock_, msg + "\r\n");
	return (ret);
}

/**  @brief invite  **/
/** 127.000.000.001.06667-127.000.000.001.59616: :irc.local 443 A B #6 :is already on channel **/
event_pair Sender::already_in_channel_message(const User &target_user, const std::string &channel)
{
	event_pair ret;

	const std::string &error_message = ":" + Sender::server_name_ + " 443 " + target_user.nickname_ + " " + channel + " :is already on channel";
	ret = std::make_pair(target_user.client_sock_, error_message + "\r\n");
	return ret;
}

/**  @brief invite_message  **/
/*
127.000.000.001.06667-127.000.000.001.43726: :A!root@127.0.0.1 INVITE C :#6
*/
event_pair Sender::invite_message(const User &invitor, const User &invited_user, const std::string &channel)
{
	event_pair ret;

	const std::string &invite_message = ":" + invitor.nickname_ + "!" + invitor.username_ + "@127.0.0.1 INVITE " + invited_user.nickname_ + " :" + channel;
	ret = std::make_pair(invited_user.client_sock_, invite_message + "\r\n");
	return ret;
}

event_pair Sender::invite_no_user_message(const User &invitor, const std::string &invited_user)
{
	event_pair ret;

	const std::string &error_message = ":" + invited_user + " No such nick ";
	ret = std::make_pair(invitor.client_sock_, error_message + "\r\n");
	return ret;
}

event_pair Sender::invitor_message(const User &invitor, const User &invited_user, const std::string &channel)
{
	event_pair ret;

	const std::string &inviting_message = ":" + server_name_ + " 341 " + invitor.nickname_ + " " + invited_user.nickname_ + " " + ":" + channel;
	ret = std::make_pair(invitor.client_sock_, inviting_message + "\r\n");
	return ret;
}

event_pair Sender::invitee_message(const User &sender, const User &receiver, const std::string &channel)
{
	event_pair ret;

	const std::string &inviting_message = ":" + server_name_ + " NOTICE " + channel + " :*** " + sender.nickname_ + " invited " + receiver.nickname_ + " into the channel ";
	ret = std::make_pair(receiver.client_sock_, inviting_message + "\r\n");
	return ret;
}

event_pair Sender::cannot_join_message(const User &receiver, const std::string &channel)
{
	event_pair ret;

	const std::string &inviting_message = ":" + server_name_ + " 473 " + receiver.nickname_ + " " + channel + " :Cannot join channel (invite only)";
	ret = std::make_pair(receiver.client_sock_, inviting_message + "\r\n");
	return ret;
}

/* 127.000.000.001.06667-127.000.000.001.42570: :irc.local 475 B #4 :Cannot join channel (incorrect channel key) */
event_pair Sender::cannot_join_message_key(const User &receiver, const std::string &channel)
{
	event_pair ret;

	const std::string &key_error_message = ":" + server_name_ + " 475 " + receiver.nickname_ + " " + channel + " :Cannot join channel (incorrect channel key)";
	ret = std::make_pair(receiver.client_sock_, key_error_message + "\r\n");
	return ret;
}

/* 127.000.000.001.06667-127.000.000.001.49612: :irc.local 471 b #4 :Cannot join channel (channel is full) */
event_pair Sender::cannot_join_message_limit(const User &receiver, const std::string &channel)
{
	event_pair ret;

	const std::string &key_error_message = ":" + server_name_ + " 471  " + receiver.nickname_ + " " + channel + " :Cannot join channel (channel is full)";
	ret = std::make_pair(receiver.client_sock_, key_error_message + "\r\n");
	return ret;
}
