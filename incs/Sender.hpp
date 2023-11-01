#pragma once

#include "Event.hpp"
#include "User.hpp"
#include "Color.hpp"

class Sender
{
public:
	static event_pair mode_wrong_message(const uintptr_t &ident, const char &mode_option);
	static event_pair pong(const uintptr_t &socket, const std::string &msg); //@Todo: pong_message
	static event_pair command_no_origin_specified_409(const User &sender, const std::string &command);
	static event_pair command_not_registered_451(const User &sender, const std::string &command);
	static event_pair command_not_registered_451(const uintptr_t &sock, const std::string &command);
	static event_pair command_empty_argument_461(const User &sender, const std::string &command);
	static event_pair command_empty_argument_461(const uintptr_t &sock, const std::string &command);
	static event_pair welcome_message_connect(const User &sender);
	static event_pair user_302_message(const User &sender);
	static event_pair nick_well_message(const User &sender, const User &receiver, const std::string &new_nick);
	static event_pair nick_error_message(const User &sender, const std::string &new_nick);
	static event_pair nick_error_message2(const User &sender, const std::string &new_nick); //@Todo: nick_error_message_steal
	static event_pair nick_error_message(const uintptr_t &sock, const std::string &new_nick);
	static event_pair nick_wrong_message(const User &sender, const std::string &new_nick);
	static event_pair nick_wrong_message(const uintptr_t &sock, const std::string &new_nick);
	static event_pair password_incorrect_464(const uintptr_t &sock);
	static event_pair quit_leaver_message(const User &sender, std::string leave_message);
	static event_pair quit_member_message(const User &sender, const User &receiver, std::string leave_message);
	static event_pair privmsg_bot_message(const User &sender, const std::string &msg);
	static event_pair privmsg_p2p_message(const User &sender, const User &target, const std::string &msg);
	static event_pair privmsg_channel_message(const User &sender, const User &receiver, const std::string &channel, const std::string &msg);
	static event_pair privmsg_no_user_error_message(const User &sender, const std::string &target);

	static event_pair join_message(const User &sender, const User &receiver, const std::string &channel);
	static event_pair join_invalid_channel_name_message(const User &sender, const std::string invaild_channel);
	static event_pair part_message(const User &sender, const User &receiver, const std::string &channel, const std::string &msg);
	static event_pair kick_message(const User &sender, const User &receiver, const std::string &subject, const std::string &channel, const std::string &msg);
	static event_pair kick_error_not_op_message(const User &sender, const std::string &host, const std::string &channel);
	static event_pair kick_error_no_user_message(const User &sender, const std::string &host, const std::string &target, const std::string &channel);
	static event_pair topic_message(const User &sender, const User &receiver, const std::string &channel, const std::string &topic);
	static event_pair topic_access_error(const User &sender, const std::string &channel);
	static event_pair show_channel_topic(const User &sender, const std::string &channel_name, const std::string &channel_topic);


	static event_pair notice_p2p_message(const User &sender, const User &target, const std::string &msg);
	static event_pair notice_channel_message(const User &sender, const User &receiver, const std::string &channel, const std::string &msg);
	static event_pair notice_no_nick_message(const User &sender, const User &receiver);
	static event_pair no_channel_message(const User &sender, const std::string &channel);
	static event_pair not_on_the_channel_message(const User &sender, const std::string &channel);

	static event_pair no_user_message(const User &sender, const std::string &target);
	static event_pair mode_error_not_op_message(const User &sender, const std::string &channel);
	static event_pair command_too_many_argument_461(const uintptr_t &sock, const std::string &command);
	static event_pair mode_no_user_message(const User &sender, const std::string &target);
	static event_pair mode_syntax_error(const User &sender, const std::string &target, const std::string &mode_option, const std::string &description, const std::string &err_syntax);
	static event_pair mode_syntax_error_l_negative_num(const User &sender, const std::string &target, const std::string &mode_option, const std::string &mode_param);

	static std::string join_353_message(const User &sender, const std::string &chan_name, const std::string &chan_user_list);
	static std::string join_366_message(const User &sender, const std::string &chan_name);
	static event_pair mode_message(const User &sender, const User &receiver, const std::string &channel, const std::string &mode_type);

	static event_pair already_in_channel_message(const User &target_user, const std::string &channel);
	static event_pair invite_message(const User &sender, const User &receiver, const std::string &channel);
	static event_pair invitee_message(const User &sender, const User &receiver, const std::string &channel);
	static event_pair invite_no_user_message(const User &sender, const std::string &target_user);
	static event_pair invitor_message(const User &invitor, const User &target_user, const std::string &channel);

	static event_pair cannot_join_message(const User &receiver, const std::string &channel);
	static event_pair cannot_join_message_key(const User &receiver, const std::string &channel);
	static event_pair cannot_join_message_limit(const User &receiver, const std::string &channel);

private:
	static const std::string server_name_;
};
