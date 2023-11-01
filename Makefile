CXX		 = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98
DEPFLAGS = -MMD#-g -fsanitize=address
NAME	 = ircserv
DIR_SRC = srcs
DIR_INC = incs
DIR_BUILD = build

SRCS_ORG = main.cpp Channel.cpp DatabaseChannels.cpp DatabaseUsers.cpp KeventHandler.cpp \
			Parser.cpp Receiver.cpp Sender.cpp Server.cpp User.cpp Utils.cpp ServerStatus.cpp
SRCS	= $(addprefix $(DIR_SRC)/, $(SRCS_ORG))
OBJS	= $(patsubst %.cpp, $(DIR_BUILD)/%.o, $(SRCS_ORG))
DEPS	= $(patsubst %.cpp, $(DIR_BUILD)/%.d, $(SRCS_ORG))

all : $(NAME)

$(NAME) : $(OBJS)
	$(CXX) $(CXXFLAGS) $(DEPFLAGS) -I $(DIR_INC) $^ -o $@

$(DIR_BUILD)/%.o : $(DIR_SRC)/%.cpp
	@mkdir  -p $(DIR_BUILD)
	$(CXX) $(CXXFLAGS) $(DEPFLAGS) -I $(DIR_INC) -c $< -o $@

clean :
	$(RM) -rf $(DIR_BUILD)

fclean : clean
	$(RM) $(NAME)

re :
	make fclean
	make all

.PHONY : all clean fclean re

-include $(DEPS)