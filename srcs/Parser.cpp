#include "Parser.hpp"
#include "Receiver.hpp"
#include "Event.hpp"
#include "Color.hpp"
#include "Database.hpp"
#include "ServerStatus.hpp"

#include <sstream>
#include <string>
#include <sys/_types/_size_t.h>
#include <iostream>

const std::string Parser::commands[N_CMD] = {"PASS", "NICK", "USER", "PING", "JOIN", "QUIT", "PRIVMSG", "KICK", "PART", "TOPIC", "NOTICE", "MODE", "INVITE"};
void (Parser::*Parser::command_handlers[N_CMD])(const uintptr_t &, std::stringstream &, std::string &) =
		{&Parser::parse_pass_, &Parser::parse_nick_, &Parser::parse_user_, &Parser::parse_ping_, &Parser::parse_join_, &Parser::parse_quit_, &Parser::parse_privmsg_,
		 &Parser::parse_kick_, &Parser::parse_part_, &Parser::parse_topic_, &Parser::parse_notice_, &Parser::parse_mode_, &Parser::parse_invite_};

/**		command_toupper_   **/
/**		@brief NC로 소문자 명령을 보낼 경우 대문자로 변경하여 처리하기 위한 함수   **/
/**		@param command 대문자로 변경할 명령어   **/
const std::string Parser::command_toupper_(const char *command)
{
	std::string ret;

	for (std::size_t i(0); i < std::strlen(command); ++i)
	{
		ret += std::toupper(static_cast<int>(command[i]));
	}
	return ret;
}

Parser::Parser(event_map &serv_udata, const std::string &password)
		: parser_udata_(serv_udata), password_(password)
{
	database_.bot_maker("BOT");
}

/**		clear_all   **/
/**		@brief SIGINT가 들어올 경우 모든 클라이언트를 삭제하고 소켓까지 닫아주는 함수   **/
void Parser::clear_all()
{
	event_map to_delete;

	to_delete.clear();
	for (std::vector<User>::iterator it = database_.get_user_list().begin(); it != database_.get_user_list().end(); ++it)
	{
		if (it->client_sock_ == 0)
			continue;
		event_pair tmp = Sender::quit_leaver_message(*it, "");
		to_delete.insert(tmp);
		Receiver::get_Kevent_Handler().add_exit_event(it->client_sock_);
	}
	const bool flag = to_delete.size() ? false : true;
	std::vector<struct kevent> events = Receiver::get_Kevent_Handler().set_monitor(flag);
	for (std::size_t i(0); i < events.size(); ++i)
	{
		event_map_iter target = to_delete.find(events[i].ident);
		send(events[i].ident, target->second.c_str(), target->second.size(), 0);
		uintptr_t tmp_fd = events[i].ident;
		Receiver::get_Kevent_Handler().delete_event(events[i]);
		close(tmp_fd);
		to_delete.erase(target);
	}
}

void Parser::handle_error(const uintptr_t &ident)
{
	database_.delete_error_user(ident);
}

/**		message_resize_   **/
/**		@brief (:)이 있으면 to_send(이미 ':' 기준으로 잘린 문자열)을 사용하고, 510글자가 넘으면 510글자로 제한해주는 함수   **/
/**		@param tmp line_ss로 받은 문자열   **/
/**		@param to_send (:) 기준으로 재할당한 문자열   **/
std::string Parser::message_resize_(const std::string &tmp, const std::string &to_send)
{
	std::string ret;

	if (tmp.size() && tmp.at(0) == ':')
		ret = to_send;
	else
		ret = tmp;
	(ret.size() > 510) ? ret.resize(510) : ret.resize(ret.size());
	return ret;
}

/**		parse_command   **/
/**		@brief 클라이언트가 보낸 명령어를 개행을 기준으로 한 줄씩 파싱하여 처리하는 함수   **/
/**		@param ident 클라이언트의 소켓   **/
/**		@param command 들어온 명령어   **/
void Parser::parse_command(const uintptr_t &ident, std::string &command)
{
	std::stringstream ss(command);
	std::string line;

	while (std::getline(ss, line, '\n'))
	{
		std::stringstream line_ss(line);
		std::string command_type;
		std::size_t i(0);

		line_ss >> command_type;
		command_type = command_toupper_(command_type.c_str());
		ServerStatus::print_input(command_type);
		for (; i < N_CMD && (command_type != Parser::commands[i]); ++i)
		{
		}
		if (i < N_CMD)
		{
			std::size_t pos;
			if ((i == 3) && (line.find(':') != std::string::npos))
			{
				pos = line.find(':') - 1;
			}
			else
			{
				pos = line.find(':');
			}
			std::string to_send;
			if (pos == std::string::npos)
			{
				to_send.clear();
			}
			else
			{
				to_send = line.substr(pos + 1, line.length() - (pos + 1));
			}
			(this->*Parser::command_handlers[i])(ident, line_ss, to_send);
		}
		else
		{
			event_pair tmp;
			tmp.first = ident;
			push_write_event_(tmp);
		}
	}
}

/**		check_mode_err_type   **/
/**		@brief MODE 명령어를 파싱하는 함수   **/
/**		@brief mode로 들어오는 command line의 인자들을 각각 파싱한 후, 상황에 따른 에러메세지 반환   **/

int check_mode_err_type(t_mode_input &mode)
{
	if (mode.target.length() == 0 || mode.option.length() == 0)
		return CMD_EMPTY;
	else if (mode.option.length() == 2 && (mode.option.at(1) == 'i' || mode.option.at(1) == 't') && mode.param.length() > 0)
	{
		return CMD_TOO_MANY;
	}
	else if (mode.mode_type == MODE_TYPE_ERR)
	{
		return WRONG_MSG;
	}
	return -1;
}

void set_mode_input_type(t_mode_input &mode)
{
	mode.mode_type = MODE_TYPE_ERR;
	if (mode.option.length() != 2)
		return;

	char sign = mode.option.at(0);
	char option = mode.option.at(1);

	if (sign == '+')
	{
		if (option == 'i')
			mode.mode_type = PLUS_I;
		if (option == 't')
			mode.mode_type = PLUS_T;
		if (option == 'l')
			mode.mode_type = PLUS_L;
		if (option == 'o')
			mode.mode_type = PLUS_O;
		if (option == 'k')
			mode.mode_type = PLUS_K;
	}
	else if (sign == '-')
	{
		if (option == 'i')
			mode.mode_type = MINUS_I;
		if (option == 't')
			mode.mode_type = MINUS_T;
		if (option == 'l')
			mode.mode_type = MINUS_L;
		if (option == 'o')
			mode.mode_type = MINUS_O;
		if (option == 'k')
			mode.mode_type = MINUS_K;
	}
	return;
}

void set_mode_input(t_mode_input &mode, std::stringstream &line)
{
	line >> mode.target >> mode.option >> mode.param;
	set_mode_input_type(mode);
}

char get_wrong_mode_option(std::string wrong_mode)
{
	if (wrong_mode.length() < 2)
		return ' ';
	else
		return wrong_mode.at(1);
}

void Parser::parse_mode_(const uintptr_t &ident, std::stringstream &line_ss, std::string &to_send)
{
	static_cast<void>(to_send);
	t_mode_input mode;
	event_map ret;
	event_pair tmp;

	set_mode_input(mode, line_ss);
	switch (check_mode_err_type(mode))
	{
	case CMD_EMPTY:
		tmp = Sender::command_empty_argument_461(ident, "MODE");
		ret.insert(tmp);
		break;
	case CMD_TOO_MANY:
		tmp = Sender::command_too_many_argument_461(ident, "MODE");
		ret.insert(tmp);
		break;
	case WRONG_MSG:
		tmp = Sender::mode_wrong_message(ident, get_wrong_mode_option(mode.option));
		ret.insert(tmp);
		break;
	default:
		ret = database_.command_mode(ident, mode);
	}

	push_multiple_write_events_(ret, ident, 0);
}

/**		parse_invite_   **/
/**		@brief INVITE 명령어를 파싱하는 함수   **/
/**		@brief 매개변수가 없거나, 채널이 없거나, 유저가 없으면 에러메세지 반환   **/
void Parser::parse_invite_(const uintptr_t &ident, std::stringstream &line_ss, std::string &to_send)
{
	static_cast<void>(to_send);
	std::string channel;
	std::string user;
	event_map ret;

	line_ss >> user;
	line_ss >> channel;
	if (user.empty() || channel.empty())
	{
		event_pair tmp = Sender::command_empty_argument_461(ident, "INVITE");
		ret.insert(tmp);
	}
	else if (line_ss.rdbuf()->in_avail() != 0)
	{
		event_pair tmp = Sender::command_too_many_argument_461(ident, "INVITE");
		ret.insert(tmp);
	}
	else
	{
		ret = database_.command_invite(ident, user, channel);
	}
	push_multiple_write_events_(ret, ident, 0);
}

/**		parse_pass_   **/
/**		@brief PASS 명령어를 파싱하는 함수   **/
/**		@brief 매개변수가 없거나 패스워드가 틀리면 오류를 보내고, 정상적인 패스워드면 유저를 등록   **/
void Parser::parse_pass_(const uintptr_t &ident, std::stringstream &line_ss, std::string &to_send)
{
	static_cast<void>(to_send);
	std::string pw;
	event_pair ret;

	line_ss >> pw;
	ret.first = ident;
	if (pw.empty())
	{
		ret = Sender::command_empty_argument_461(ident, "PASS");
		push_write_event_(ret);
		return;
	}
	if (password_ != pw)
	{
		ret = Sender::password_incorrect_464(ident);
	}
	else
	{
		ret = database_.command_pass(ident);
	}
	push_write_event_(ret);
}

/**		parse_nick_   **/
/**		@brief NICK 명령어를 파싱하는 함수   **/
/**		@brief 매개변수가 없으면 에러를 보내고, 있으면 유저 메소드를 호출하여 닉네임을 등록   **/
/**		@brief valid NICK인지 확인, 이미 존재하는 NICK인지 확인, 점유 못한 NICK은 뺏음   **/
/**		@brief channel에서 NICK 변경시, send_all **/

void Parser::parse_nick_(const uintptr_t &ident, std::stringstream &line_ss, std::string &to_send)
{
	static_cast<void>(to_send);
	std::string nick;
	event_map ret;

	line_ss >> nick;
	if (nick.empty())
	{
		event_pair tmp = Sender::command_empty_argument_461(ident, "NICK");
		push_write_event_(tmp);
		return;
	}
	ret = database_.command_nick(ident, nick);
	push_multiple_write_events_(ret, ident, 0);
}

/**		parse_user_   **/
/**		@brief USER 명령어를 파싱하는 함수   **/
/**		@brief 매개변수가 없으면 에러를 보내고, 있으면 유저 메소드를 호출하여 유저를 등록   **/
void Parser::parse_user_(const uintptr_t &ident, std::stringstream &line_ss, std::string &real_name)
{
	event_map ret;
	event_pair tmp;
	std::string argument[3];
	std::string real;

	line_ss >> argument[0] >> argument[1] >> argument[2] >> real;
	for (std::size_t i(0); i < 3; ++i)
	{
		if (argument[i].empty() || (argument[i].size() && argument[i].at(0) == ':'))
		{
			tmp = Sender::command_empty_argument_461(ident, "USER");
			push_write_event_(tmp);
			return;
		}
	}
	real = message_resize_(real, real_name);
	if (real.empty())
	{
		tmp = Sender::command_empty_argument_461(ident, "USER");
		push_write_event_(tmp);
		return;
	}
	tmp = database_.command_user(ident, argument[0], argument[1], argument[2], real);
	ret.insert(tmp);
	push_multiple_write_events_(ret, ident, 0);
}

/**		parse_ping_   **/
/**		@brief PING 명령어를 파싱하는 함수   **/
/**		@brief 매개변수가 없으면 에러를 보내고, 있으면 유저 메소드를 호출하여 PONG 메세지 생성   **/
void Parser::parse_ping_(const uintptr_t &ident, std::stringstream &line_ss, std::string &to_send)
{
	event_pair ret;
	std::string msg;

	line_ss >> msg;
	msg = message_resize_(msg, to_send);
	if (msg.empty())
	{
		ret = Sender::command_empty_argument_461(ident, "PING");
		push_write_event_(ret);
		return;
	}
	ret = database_.command_pong(ident, msg);
	push_write_event_(ret);
}

/**		parse_quit_   **/
/**		@brief QUIT 명령어를 파싱하는 함수   **/
/**		@brief 유저 메소드를 불러 종료할 유저에 대한 메세지를 만들고, 채널에 있을 경우 다른 유저들에 대한 메세지도 만듬   **/
void Parser::parse_quit_(const uintptr_t &ident, std::stringstream &line_ss, std::string &to_send)
{
	event_map ret;
	std::string nick, tmp;

	while (line_ss >> tmp)
	{
		if (nick.size() && nick.at(0) != ':')
		{
			nick = tmp + " ";
		}
	}
	nick += to_send;
	ret = database_.command_quit(ident, to_send);
	push_multiple_write_events_(ret, ident, 1);
}

/**		parse_privmsg_   **/
/**		@brief PRIVMSG 명령어를 파싱하는 함수   **/
/**		@brief 매개변수가 없으면 에러를 보내고, 있으면 유저 메소드를 호출하여 상대에게 PRIVMSG 메세지 생성   **/
void Parser::parse_privmsg_(const uintptr_t &ident, std::stringstream &line_ss, std::string &to_send)
{
	std::string target, msg;
	event_map ret;

	line_ss >> target;
	line_ss >> std::ws;
	if (target.empty())
	{
		event_pair tmp = Sender::command_empty_argument_461(ident, "PRIVMSG");
		ret.insert(tmp);
	}
	else
	{
		std::getline(line_ss, msg);
		msg = message_resize_(msg, to_send);
		ret = database_.command_privmsg(ident, target, msg);
	}
	push_multiple_write_events_(ret, ident, 0);
}

/**		parse_notice_   **/
/**		@brief NOTICE 명령어를 파싱하는 함수   **/
/**		@brief 매개변수가 없으면 에러를 보내고, 있으면 유저 메소드를 호출하여 상대에게 NOTICE 메세지 생성   **/
void Parser::parse_notice_(const uintptr_t &ident, std::stringstream &line_ss, std::string &to_send)
{
	std::string target, msg;
	event_map ret;

	line_ss >> target;
	line_ss >> std::ws;
	if (target.empty())
	{
		event_pair tmp = Sender::command_empty_argument_461(ident, "NOTICE");
		ret.insert(tmp);
	}
	else
	{
		std::getline(line_ss, msg);
		msg = message_resize_(msg, to_send);
		ret = database_.command_notice(ident, target, msg);
	}
	push_multiple_write_events_(ret, ident, 0);
}

/**		parse_join_   **/
/**		@brief JOIN 명령어를 파싱하는 함수   **/
/**		@brief 매개변수가 없으면 에러를 보내고, 있으면 유저 메소드를 호출하여 채널에 join   **/
void Parser::parse_join_(const uintptr_t &ident, std::stringstream &line_ss, std::string &to_send)
{
	static_cast<void>(to_send);
	std::string chan_name;
	std::string tmp_password;
	event_map ret;

	line_ss >> chan_name >> tmp_password;
	ret = database_.command_join(ident, chan_name, tmp_password);
	push_multiple_write_events_(ret, ident, 2);
}

/**		parse_part_   **/
/**		@brief PART 명령어를 파싱하는 함수   **/
/**		@brief 매개변수가 없으면 에러를 보내고, 있으면 유저 메소드를 호출하여 채널에서 해당 유저를 나가게 함   **/
void Parser::parse_part_(const uintptr_t &ident, std::stringstream &line_ss, std::string &to_send)
{
	event_map ret;
	std::string chan_name, msg;

	line_ss >> chan_name;
	line_ss >> std::ws;
	std::getline(line_ss, msg);
	msg = message_resize_(msg, to_send);
	User parter = database_.select_user(ident);
	ret = database_.command_part(ident, chan_name, msg);

	push_multiple_write_events_(ret, ident, 2);
}

/**		parse_topic_   **/
/**		@brief TOPIC 명령어를 파싱하는 함수   **/
/**		@brief 매개변수가 없으면 에러를 보내고, 있으면 유저 메소드를 호출하여 채널의 토픽을 설정함   **/
void Parser::parse_topic_(const uintptr_t &ident, std::stringstream &line_ss, std::string &to_send)
{
	event_map ret;
	std::string chan_name, msg;

	line_ss >> chan_name;
	if (chan_name.empty())
	{
		event_pair tmp = Sender::command_empty_argument_461(ident, "TOPIC");
		ret.insert(tmp);
	}
	else
	{
		line_ss >> std::ws;
		std::getline(line_ss, msg);
		msg = message_resize_(msg, to_send);
		ret = database_.set_topic(ident, chan_name, msg);
	}
	push_multiple_write_events_(ret, ident, 2);
}

/**		parse_kick_   **/
/**		@brief KICK 명령어를 파싱하는 함수   **/
/**		@brief 매개변수가 없으면 에러를 보내고, 있으면 유저 메소드를 호출하여 채널에 있는 특정 대상을 kick함   **/
void Parser::parse_kick_(const uintptr_t &ident, std::stringstream &line_ss, std::string &to_send)
{
	event_map ret;
	event_pair tmp;
	std::string chan_name, target_name, msg;

	line_ss >> chan_name >> target_name >> msg;
	if (chan_name.empty() || target_name.empty())
	{
		event_pair tmp = Sender::command_empty_argument_461(ident, "KICK");
		ret.insert(tmp);
	}
	else
	{
		line_ss >> std::ws;
		msg = message_resize_(msg, to_send);
		ret = database_.command_kick(ident, target_name, chan_name, msg);
	}
	push_multiple_write_events_(ret, ident, 2);
}

/**		push_write_event_   **/
/**		@brief 발생한 한 개의 이벤트를 write 상태로 변경하는 함수   **/
void Parser::push_write_event_(event_pair &ret)
{
	if (ret.second.empty())
	{
		Receiver::get_Kevent_Handler().add_read_event(ret.first);
		return;
	}
	parser_udata_.insert(ret);
	(Receiver::get_Kevent_Handler()).add_write_event(ret.first);
}

/**		push_multiple_write_events_   **/
/**		@brief 발생한 여러 개의 이벤트들을 write 상태로 변경하는 함수   **/
void Parser::push_multiple_write_events_(event_map &ret, const uintptr_t &ident, const int flag)
{
	event_map_iter target = ret.find(ident);

	/**   등록할 이벤트가 없는 경우 해당 이벤트를 read 상태로 만들어 줌   **/
	if (ret.empty())
	{
		Receiver::get_Kevent_Handler().add_read_event(ident);
		return;
	}
	if (target != ret.end())
	{
		/**   여러 소켓으로 데이터를 보낼 때 나만 제외할 경우   **/
		if (target->second.empty() && flag == 0)
		{
			Receiver::get_Kevent_Handler().add_read_event(ident);
		}
		/**   해당 클라이언트를 종료할 경우   **/
		else if (flag == 1)
		{
			parser_udata_.insert(*target);
			Receiver::get_Kevent_Handler().add_exit_event(target->first);
		}
		/**   일반적으로 여러 소켓으로 데이터를 보낼 때 나를 제일 먼저 등록   **/
		else
		{
			parser_udata_.insert(*target);
			Receiver::get_Kevent_Handler().add_write_event(target->first);
		}
	}
	/**   내가 있으면 나를 제외한 클라이언트들의 이벤트를 등록함   **/
	for (event_map_iter iter = ret.begin(); iter != ret.end(); ++iter)
	{
		if (target != ret.end() && iter->first == ident)
		{
			continue;
		}
		parser_udata_.insert(*iter);
		(Receiver::get_Kevent_Handler()).add_write_event(iter->first);
	}
}
