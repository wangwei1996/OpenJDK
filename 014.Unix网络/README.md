# Unix 网络
&nbsp;&nbsp;UNIX 网络

---
## 术语
### 1. MTU： 最大传输单元(Maximum Transmission Unit)
最大传输单元（Maximum Transmission Unit，MTU）用来通知对方所能接受数据服务单元的最大尺寸，说明发送方能够接受的有效载荷大小。 是包或帧的最大长度，一般以字节记。如果MTU过大，在碰到路由器时会被拒绝转发，因为它不能处理过大的包。如果太小，因为协议一定要在包(或帧)上加上包头，那实际传送的数据量就会过小，这样也划不来。大部分操作系统会提供给用户一个默认值，该值一般对用户是比较合适的


---
## 参考资料
1. 《UNIX 网络编程第三版 · 卷一》