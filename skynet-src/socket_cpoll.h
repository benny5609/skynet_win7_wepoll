#ifndef poll_socket_epoll_h
#define poll_socket_epoll_h

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include "wepoll.h"

static bool
sp_invalid(poll_fd efd) {
	return efd == NULL;
}

static poll_fd
sp_create() {
	return epoll_create(1024);
}

static void
sp_release(poll_fd efd) {
	closesocket(efd);
}

static int
sp_add(poll_fd efd, int sock, void *ud) {
	struct epoll_event ev;
	ev.events = EPOLLIN;
	ev.data.ptr = ud;
	if (epoll_ctl(efd, EPOLL_CTL_ADD, sock, &ev) == -1) {
		return 1;
	}
	return 0;
}

static void
sp_del(poll_fd efd, int sock) {
	epoll_ctl(efd, EPOLL_CTL_DEL, sock, NULL);
}

void
static sp_write(poll_fd efd, int sock, void *ud, bool enable) {
	struct epoll_event ev;
	ev.events = EPOLLIN | (enable ? EPOLLOUT : 0);
	ev.data.ptr = ud;
	epoll_ctl(efd, EPOLL_CTL_MOD, sock, &ev);
}

static int
sp_wait(poll_fd efd, struct event *e, int max) {
	assert(max <= 2048);
	struct epoll_event ev[2048];
	int n = epoll_wait(efd, ev, max, -1);
	int i;
	for (i = 0; i < n; i++) {
		e[i].s = ev[i].data.ptr;
		unsigned flag = ev[i].events;
		e[i].write = (flag & EPOLLOUT) != 0;
		e[i].read = (flag & (EPOLLIN | EPOLLHUP)) != 0;
		e[i].error = (flag & EPOLLERR) != 0;
		e[i].eof = false;
	}

	return n;
}

static void
sp_nonblocking(int fd) {
	u_long ul = 1;
	ioctlsocket(fd, FIONBIO, &ul);
}
#endif
