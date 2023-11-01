#include "KeventHandler.hpp"

void exit_with_perror(const std::string &msg);

/**		keventHandler   **/
/**		@brief kqueue를 새로 등록   **/
KeventHandler::KeventHandler()
{
	kqueue_fd_ = kqueue();
	if (kqueue_fd_ < 0)
	{
		exit_with_perror("Failed to create kqueue");
	}
}

KeventHandler::~KeventHandler()
{
	if (kqueue_fd_ > 0)
	{
		close(kqueue_fd_);
	}
}

void KeventHandler::initialize_kevent(uintptr_t ident, int16_t filter, uint16_t flags, uint32_t fflags, int64_t data, void *udata)
{
	struct kevent event = {};

	EV_SET(&event, ident, filter, flags, fflags, data, udata);
	event_list_.push_back(event);
}

/**		set_monitor   **/
/**		@brief 새로 발생한 이벤트들을 kevent 벡터에 담아주는 함수   **/
/**		@param 이벤트 등록을 계속할 지 판별하는 flag   **/
std::vector<struct kevent> KeventHandler::set_monitor(const bool &is_end_signal)
{
	std::vector<struct kevent> res;

	if (is_end_signal)
		return res;
	struct kevent event_list[10];
	int event_num(-1);

	while ((event_num < 0))
	{
		event_num = kevent(kqueue_fd_, &(event_list_[0]), (int)event_list_.size(), event_list, 10, NULL);
	}
	for (int i(0); i < event_num; ++i)
	{
		res.push_back(event_list[i]);
	}
	event_list_.clear();
	return (res);
}

void KeventHandler::add_server_event(uintptr_t ident)
{
	initialize_kevent(ident, EVFILT_READ, EV_ADD, 0, 0, NULL);
}

void KeventHandler::add_read_event(uintptr_t ident)
{
	initialize_kevent(ident, EVFILT_READ, EV_ADD | EV_ONESHOT, 0, 0, NULL);
}

void KeventHandler::add_write_event(uintptr_t ident)
{
	initialize_kevent(ident, EVFILT_WRITE, EV_ADD, 0, 0, NULL);
}

/**		add_exit_event   **/
/**		@brief set_write와 유사하나 종료 신호를 보내기 위한 더미 udata를 생성하여 이벤트를 등록   **/
void KeventHandler::add_exit_event(uintptr_t ident)
{
	char k(0);

	initialize_kevent(ident, EVFILT_WRITE, EV_ADD, 0, 0, &k);
}

void KeventHandler::delete_event(const struct kevent &event)
{
	struct kevent tmp;

	EV_SET(&tmp, event.ident, event.filter, EV_DELETE, 0, 0, 0);
	kevent(kqueue_fd_, &tmp, 1, NULL, 0, NULL);
}

void KeventHandler::delete_server(uintptr_t serv_sock)
{
	struct kevent tmp;

	EV_SET(&tmp, serv_sock, EVFILT_READ, EV_DELETE, 0, 0, 0);
	kevent(kqueue_fd_, &tmp, 1, NULL, 0, NULL);
}
