set(CMAKE_CXX_STANDARD 11)
set(EXECUTABLE_OUTPUT_PATH ../)

add_executable(httpd httpd.cc EpollPoller.cc HttpConnection.cc HttpServer.cc
Poller.cc PollPoller.cc SocketsOps.cc TimerOps.cc TimerMonitor.cc)

#add_subdirectory(test)
