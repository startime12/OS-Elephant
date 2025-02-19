# OS-Elephant
这里实现到sys_exec.
一开始是报sys_free里面的ASSERT(a->large == 0 || a->large == 1)出错
调试发现在sys_execv -> load -> segment_load -> sys_read -> file_read -> sys_free(io_buf)出现了错误
不知道什么原因导致io_buf所在area的large字段变成了负数！！！
在不断调试之后，换了一个错误，变成了缺页异常。。。
不想调了，累了，有时间再说吧。。。