# Subscription-Tool-for-Clash  
(Clash-Subscription-Little-Helper)

## 📖 简介

*自用 + 初学c练手，自己用起来还不错，分享一下，希望有所帮助。*  

*目前实现比较简单粗暴且局限，以后有时间再更新，并届时增加MIPS/MIPSel架构的Release，以方便路由器使用。*

-------------------------------------------------------------------------------------------------------------

一个简单的 **Clash订阅更新 + 自定义规则/节点/设置** 工具。  

自动从 **订阅** 链接获取最新配置，将订阅 **节点/节点组/规则** 和 基础文件 的 **自定义节点/节点组/规则**  +  **其他所有设置** 结合，生成一个新的配置文件，供clash使用。  

<img src="https://raw.githubusercontent.com/norman06/Subscription-Tool-for-Clash/master/readme-example.png" style="zoom:40%;" />

目的是：

1. **保留所有基础文件的设置**，如端口、模式、远程管理密码等，而不会被订阅配置覆盖；

2. 仅从订阅配置中获取 **节点、节点组、规则** ，添加到基础文件相应条目已有内容之后（输出文件会另存，不会覆盖基础文件）。这样既可以使用自己的节点/节点组/规则，也添加了最新订阅的。

   （且基础文件内容会在订阅内容之前，以保证自定义的内容优先级更高。  ）

3. 使用C语言编写，使用标准库/跨平台库，希望可以跨平台使用。  

## ⛏ 编译

依赖：libcurl

```
$ sudo apt install libcurl4-openssl-dev
```

编译：

```
$ git clone https://github.com/alxt17/Clash-Subscription-Little-Helper.git
$ cd Subscription-Tool-for-Clash
$ mkdir build && cd build
$ cmake ..
$ make
```

done. 在build目录下得到```sub4clash```可执行文件。

## ⌨️ 使用方法️
1. 创建yaml基础文件，填入所有必须的基本配置，例如代理端口、代理模式等（如果不会设置，请参考订阅文件内容）。

2. （选填）需要自定义的 **proxies: / proxy-groups: / rules:** 的内容。

   如无需自定义，则在上述三处的冒号之后，换行并留空）  

3. 运行程序，需要三个必要参数，按顺序分别是：订阅url、基础配置文件、目标输出文件。

   例如：

   我的基础文件是 ```~/.config/clash/base.yaml``` ，clash的使用配置文件是 ```~/.config/clash/config.yaml``` ：

   ```./sub4clash "https://subscription.link" ~/.config/clash/base.yaml ~/.config/clash/config.yaml```  

4. 使clash重新加载配置文件，或重启clash。  

   

P.S. 

1. 建议链接使用双引号，以避免url中的特殊字符造成不正常工作 

2. 默认情况下，clash的配置文件保存在 ```~/.config/clash/config.yaml```
3. 可以在.bashrc文件中创建alias，以快速手动更新订阅。


## ⚙️ 工作原理

1. 使用curl库将订阅文件下载到本地，提取其中的 **proxies / proxy-groups / rules** 的内容，添加到队列中。
2. 读取自定义yaml模版文件，将所有内容逐行读取，并写入新的yaml文件......直到遇见 **proxies, proxy-groups和rules** 这三个条目，追加写入 **proxies / proxy-groups / rules** 队列内容，至对应条目已有内容之后。
