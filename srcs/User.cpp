#include "User.hpp"

User::User(void)
{
	bzero(this, sizeof(*this));
}

void User::input_user(const std::string &username, const std::string &mode, const std::string &unused, const std::string &realname)
{
	username_ = username;
	mode_ = mode;
	unused_ = unused;
	realname_ = realname;
}
