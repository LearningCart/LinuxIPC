#include <sys/timerfd.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <iostream>
#include <errno.h>
#include <string.h>
int main() {
    int fd = timerfd_create (CLOCK_MONOTONIC, TFD_NONBLOCK);
    if (fd == -1) {
        std::cerr << strerror (errno);
        return 1;
    }
    const int size = 10;
    int epollfd = epoll_create (size);
    if (epollfd == -1) {
        std::cerr << strerror (errno) << "\n";
        return 1;
    }
    epoll_event event;
    epoll_event events[size];
    /* add timer to reactor */
    event.events = EPOLLIN; /* notification is a read event */
    event.data.fd = fd; /* user data */
    int r = epoll_ctl (epollfd, EPOLL_CTL_ADD, fd, &event);
    if (r == -1) {
        std::cerr << strerror (errno) << "\n";
        close (epollfd);
        close (fd);
        return 1;
    }

    int flags = 0;
    itimerspec new_timer;
    
    new_timer.it_interval.tv_sec = 2;
    new_timer.it_interval.tv_nsec = 0;
    new_timer.it_value.tv_sec = 2;
    new_timer.it_value.tv_nsec = 0;
    itimerspec old_timer;
    int i = timerfd_settime (fd, flags, &new_timer, &old_timer);
    if (i == -1) {
        std::cerr << strerror (errno) << "\n";
        close (epollfd);
        close (fd);
        return 1;
    }
    
    while (true) {
        const int infinity = -1;
        int r = epoll_wait (epollfd, events, size, infinity);
        if (r == -1) {
            close (epollfd);
            close (fd);
            return 1;
        }
        int i = 0;
        while (i < r) {
            if (events[i].data.fd == fd) {
                size_t s = 0;
                int i = read (fd, &s, sizeof (s));
                if (i != -1) {
                    std::cout << "Timer triggered (" << s << ")\n";
                }
            }
            ++i;
        }
    }
    close(epollfd);
    close (fd);
    return 0;
}

