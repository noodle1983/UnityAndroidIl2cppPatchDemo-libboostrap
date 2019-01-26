# 1. 先扯一下背景
我们最初的热更方案出自项目里的周大神之手，将cs另外编译成一自命名dll，改cs代码对应meta文件里的dll文件名，将其指向自命名dll，用firstpass.dll做热更逻辑。这样热更在首包里可以在预置上挂含复杂数据的脚本，但在热更的文件中，如果有新的挂载脚本，其序列化数据会被丢失。这个问题由来已久，不得其解。机缘巧合，网上漏出了一份Unity4的源代码（学习目的），2018年初休息的时间里借此把反序列化的部分看了一遍，以期找出bug的根源和workaround的办法。workaround没找着，倒是把反序列化的结构给弄清楚了。后来回原公司新项目，Unity4升Unity2017，meta文件的格式变了，原来的方案失效。然后调研了(x)lua之类的方案，和我们的传统做法格格不入，遂花了些时间整出这么一套东西。

搞这么一套东西，就是在黑暗中摸索（源码太老，只能知道个大概的设计），原来的方案还挺复杂的。公司没了之后，总结回顾，发现有更直接的方法。遂重整了一遍，和原来已大不一样。整理的过程中发现老的方案已经不工作了，深感如果没项目用，这东西也就烂在我手上，最合适的还是开源，多人用才能与时俱进。后来各种忙，有网友来咨询，我也大概指了个方向，说等我开源。如今工作基本有了着落，终于有点时间写写，整理MIT开源。

# 2. 上架安全问题
其实不说大家都能猜到，这是基于hook的设计，大白话说就是自己黑自己。

如果是一般的hook，指的是hook其他进程，难点在于root权限的获取和ptrace后对各种指令集的注入。

但是如果是自己hook自己，步骤如下，上面的问题全没有。经过编译优化和符号裁剪后，最后的结果就是对某个内存地址赋了个值。

```
//1. 读取so，分析结构，拿到动态链接相对地址----怎么分析库是自己选的，优化后仅看的出读取了so文件
//2. 读取mmap文件，获取so在内存中的绝对基址----优化后仅看的出是读取了mmap文件
//3. 找到so中引用libc的符号动态地址，替换----就是(*addr) = new_func
```
So, 都是合法的操作，虽然没在各大平台试过，但感觉，没必要拦。

# 3. 总体设计
![design](https://github.com/noodle1983/private/raw/master/CommonBlog/bootstrap.png)

如图，libbootstrap拦截了dlopen操作，将libil2cpp的加载重定向到新的libil2cpp; 也拦截了读取apk的操作，并根据读取的offset，将内容映射到新的patch文件或老apk中。

# 4. 难点
由总体设计可以看出，代码功能上主要有两块，一块是hook，一块是将老apk和patch文件重组成一虚拟的apk。难点也就是这两块，外加gdb调试，如果有信心，维护不难。

# 4.1. hook
hook原来是用elfio分析elf文件，然后根据[网上的原理](https://www.apriorit.com/dev-blog/181-elf-hook)进行函数地址替换。后来arm64不work了，改了[爱奇艺的xhook](https://github.com/iqiyi/xHook/blob/master/README.zh-CN.md)。这部分前人做了很多，不杵就行。

# 4.2. 重组新apk
这块需要写点代码和调试，还得抹黑试着hook libc的接口，需要有点耐心。

因为apk是标准的zip文件，zip文件是按文件分块存储，最后跟着目录表。如果读者有印象，我们patch文件也是制作成了zip文件，从各个zip文件中提取文件，替换或新增（删除的文件还是保留，不用理，反正访问不到）整理出一份新的目录表，这个就是新的apk了，也不用另外存，浪费空间，存下映射表就行，动态读取。

# 4.3. gdb调试Android
这也是个难点，参考[gdb调试Android动态库脚本](https://noodle1983.github.io/2018/12/29/gdb_Android_solib/)

# 5. 编译
ndk环境设好，双击build_android.bat。

# 6. 最后再闲扯一下
上面说的是il2cpp的patch，其实如果改成dll，原理上也是没什么问题的。没改也是因为il2cpp就够用了，我们项目也编译dll做验证，快，但正式发布还用il2cpp。

关于基于lua的patch，不错的设计，还是那句话，够用就好。如果是因为不熟悉而避开Unity的原生语言（最近的了解），那就是捡了芝麻，丢了西瓜了。看看Unity新弄的东西，感觉Unity要全面lua化，还是很困难：
1. [ecs开发范式](https://unity3d.com/cn/learn/tutorials/topics/scripting/introduction-ecs)
2. [Unity多线程系统](https://unity.com/cn/unity/features/job-system-ECS)

最后，有bug说一声，我会抽空看；如果谁fork了一份代码，修了bug，也可以和我说一声，我会merge回来。

# 7. 随缘

[PayPal:https://www.paypal.me/noodle1983](https://www.paypal.me/noodle1983)

Or Alipay:![avatar](https://raw.githubusercontent.com/noodle1983/private/master/qr_icon/noodle1983_ali.png)
Or Wechat:![avatar](https://raw.githubusercontent.com/noodle1983/private/master/qr_icon/noodle1983_wx.png)



