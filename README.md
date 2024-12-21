# simple_echo
一个简单的聊天室，支持多人聊天。

引入了[coroutine](https://github.com/windplusflower/coroutine)协程库，采用同步的写法实现异步的程序。

## 运行

```
make run
```
会利用tmux生成4个窗口来运行程序，左边的窗口是服务器的信息，右边的三个窗口是客户端的信息。

任意一个客户端发送消息，都会在服务器和另外两个客户端上显示，包括消息来源和消息内容。