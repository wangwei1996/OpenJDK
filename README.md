# OpenJdk

## 介绍
&nbsp;&nbsp;分析JDK源代码，深入理解JVM(以书本为驱动，进行相应的源代码分析，深入理解),因代码中的中文注释，导致编译JDK时报错(error: unmappable character for encoding ascii),目前已初步解决，详见文档： 《OpenJdk编译问题以及解决方案集锦.md》.构建&调试见文件《005.OpenJDK/000.openJDK_8u40/OpenJdk代码调试解决方案.md》、《005.OpenJDK/000.openJDK_8u40/OpenJdk代码调试解决方案.md》

## 打好基本功

### 学习方法
&nbsp;&nbsp;自顶向下的学习方法
- 场景(什么时候用) -> 用法(怎么用) -> 原理(为什么是这么用)

### 学习方式
&nbsp;&nbsp;以书本或问题为驱动，以源码和官方文档为辅助来学习，且要时常回顾

### 源码如何分析
1. 了解源码具体的功能是什么，能够解决什么问题(有一个大概的了解)
2. 查阅资料，大概了解是如何解决这样的问题的(带着问题去看源代码)
3. 学习功能内部原理的知识，如使用了什么算法，什么数据结构。。。。
4. 学习该功能的用法，并编写Demo
5. 以Demo作为入口，来进行调试，从而分析源代码
6. 剥离出关键代码，分析关键代码

### 参考资料查询
1. Oracle:[https://docs.oracle.com/](https://docs.oracle.com/)
2. OpenJDK Wiki: [http://openjdk.java.net/](http://openjdk.java.net/)