set(CMAKE_CXX_STANDARD 11)

add_executable(echoserver echoserver.cc ../SocketsOps.cc ../Poller.cc ../PollPoller.cc ../EpollPoller.cc)

add_executable(http_request_test http_request_test.cc)

add_executable(httpcontest httpcontest.cc ../SocketsOps.cc ../Poller.cc ../PollPoller.cc ../EpollPoller.cc ../HttpConnection.cc)

add_executable(timertest timerops_test.cc ../TimerOps.cc ../Poller.cc ../PollPoller.cc ../EpollPoller.cc)

add_executable(timermonitor_test timermonitor_test.cc ../TimerOps.cc ../Poller.cc ../PollPoller.cc ../EpollPoller.cc ../TimerMonitor.cc)

