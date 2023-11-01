#include "ServerStatus.hpp"
#include "Color.hpp"
#include <vector>

void ServerStatus::print_users(const std::vector<User> &vect_user)
{
  std::cout << BOLDMAGENTA << "[ User list ]  \n"
            << RESET << std::endl;
  int i = 0;
  std::vector<User>::const_iterator it;
  for (it = vect_user.begin(); it != vect_user.end(); ++it, ++i)
  {
    const User &user = *it;
    std::cout << "  " << BLUE << i + 1 << ". " << RESET
              << BOLDWHITE << user.nickname_ << "(" << user.username_ << ", " << user.mode_ << ", " << user.unused_ << ", " << user.realname_ << ")" << RESET << std::endl;
  }
  std::cout << std::endl;
}

void ServerStatus::print_input(const std::string &input)
{
  std::cout << BOLDBLUE
            << ">> " << input << " <<\n"
            << RESET << std::endl;
}

void ServerStatus::print_recived(const uintptr_t &socket, const std::string &command)
{
  std::cout << BOLDGREEN << "\n < RECEIVE > \n"
            << RESET << std::endl
            << GRAY << "  [ Socket ] :  " << socket << std::endl
            << "  [ Input ] :  " << command << RESET << std::endl;
}
void ServerStatus::print_send(const uintptr_t &socket, const std::string &msg)

{
  std::cout << BOLDGREEN << "\n < SEND > \n"
            << RESET << std::endl
            << GRAY << "  [ Socket ] :  " << socket << std::endl
            << "  [ Message ]  : " << msg << RESET << std::endl;
}