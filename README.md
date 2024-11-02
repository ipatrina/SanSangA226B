# 三丧A226B — 安卓root从入门到出师

"三丧A226B"是一个安卓OEM安全机制探究项目。

项目旨在通过对特定型号数字移动电话机进行数据模型分析，探究现代安卓操作系统正向提权的通用概念和原理，实现对话机设备REE操作系统层面的自主完全控制。

项目将通过搭载 Android 13 操作系统的三星电子 Samsung Galaxy A22 5G (SM-A226B) 智能手机，对"BL解锁"、"安卓root"等常见提权需求的动机和底层原理进行系统性探究，从而得到适用于安卓生态全部智能手机的普遍思想。

# 禁止废话

**为什么要研究这些，是不是闲的？**

市场上的确已经有非常成熟的安卓手机root生态链条。

在现代安卓root需求中，Magisk几乎占据了全部市场。由于Magisk root生态链的高效存在，以至人们一提及root好像就要和Magisk绑定。在这种"先BL解锁，再Magisk开路，下游模块一应俱全"的场景下，人们似乎早已将root的本质抛掷脑后。

人们并不关心什么是root、什么是解BL锁，以及它们如何被设计和实现。人们只知道解锁后刷Magisk进行root能够解除许多限制，对手机做自己想做的事。

---

**Magisk方案不是挺好的，还瞎折腾什么？**

root的初衷本是解除OEM厂商的限制，对REE操作系统进行定制。

通俗地说，在root前，你的手机不是自己的，而是OEM厂商的。而使用Magisk进行root后，你的手机依然不是自己的，而是Magisk的。是Magisk通过一系列复杂的运行机制，获取到安卓操作系统的root权限，并对系统组件及手机APP进行管理。

安卓操作系统采用Linux内核，而root权限属于Linux内核特性，从来不专指使APP获得更高权限。而Linux权限机制本身并不复杂。在这种概念混淆的生态环境下，对于一些基本的Linux提权需求，例如刷写分区、系统调试和管理、驻守自定义进程，使用Magisk的修改方案就会显得大张旗鼓。一些用户可能只是为了实现对手机底层工作逻辑进行定制，与APP提权并无关联。只有使用REE操作系统原生特性进行最小改动，不驻留第三方修改方案，才会真正获得一部"属于自己控制的手机"。

此外，如果不能理解BL锁、安卓root的一系列本质，即使在技术上解锁了手机，在思想上会仍然受制于OEM及上游厂商、Magisk及下游开发者创造的概念，无法具备独立思考并解决不同需求问题的条件。

---

**手机root后需不需要伪装？**

我们先来谈一谈为什么使用Magisk时需要伪装。本项目以Magisk为例进行讲解，但是对于类似的诸如SuperSU的root方案，许多概念也同样适用。

人们都知道安装Magisk后要使用模块隐藏Magisk存在的痕迹，是因为用户希望一些APP能够获得更高的操作系统权限，但另一些APP不希望在操作系统提权后的不安全环境下运行。

为了使APP能够进行提权、模块正常工作，Magisk会以root及内核权限驻守在操作系统后台并在需要时提供帮助。但任何APP都需要有一个途径进行提权(一般是su)，且Magisk方案及模块运行时会产生一些系统痕迹。

APP若要探测"系统是否已root"，实则是在探测运行环境是否存在Magisk等修改方案。因为APP工作在Dalvik虚拟机内，且即使是在JNI层也受到了Linux权限机制和SELinux策略的双重限制，可用的探测方式并不多：无非就是试探性地进行提权(su)，或查找操作系统内存在的一切标记和痕迹。

而如果使用了符合Linux内核及安卓操作系统运行逻辑的原生修改方式提权，不使用第三方root方案，则权限管理APP"从没来过"、Magisk服务"从没来过"，又哪里来的APP提权入口(su)和Magisk运行特征呢。

---

**这听起来跟KernelSU的思想很像啊？**

是的。

除了KernelSU挑内核以外，最大的区别就是：你的手机这下变成KernelSU的了，依然不是你自己的。

---

**为什么选择三星手机进行这些研究？**

正如项目介绍所述，这个项目绝非是为了服务某一特定机型，而是为了对市场上的全部安卓智能手机提供一些通用的概念和思想支持。这就需要我们综合全方面因素，寻找一个代表机型进行实践。通常来说，此类需求一般要求机型具有市场及各方面的代表性地位。

自从苹果和安卓两大生态"一统天下"的那一天起，你会发现，在许多西方国家的电商网站上，手机销售分类只有：苹果、三星和其他。

几年以后，由于谷歌"亲儿子"打入市场，时至今日，这些电商网站的手机销售分类变成了：苹果、三星、谷歌和其他。

三星电子在全球视角下，安卓手机供销产业链的历史悠久、市场最为成熟。因此一般世界主流观点认为，三星是最具代表的安卓手机品牌，工业设计更为原生和规范。

而当我们了解三星手机的一系列机制后，其他品牌安卓手机的实现也相差无几。

---

**为什么选择联发科处理器进行这些研究？**

在挑选SoC处理器时，同样需要寻找具有市场代表性地位的产品。

在智能手机SoC厂商中，以市场占有率来看，基本可以将厂商分为：联发科、高通、苹果和其他。

联发科部分相对较新的安卓手机SoC处理器存在BootROM漏洞，允许执行任意代码、读写闪存数据，可使用开源项目mtkclient进行攻击，在需要时可以更加底层地建立和分析数据模型。

https://github.com/bkerler/mtkclient

而高通不存在同级别近期漏洞。

---

**为什么选择A22 5G这个型号进行这些研究？**

Samsung Galaxy A22 5G (SM-A226B) 在细致全面的探究下，被我们发现是最适合探索安卓生态引导流程的三星智能手机。

首先，它采用了存在BootROM漏洞的联发科SoC处理器。

第二，获取root权限后，我们可能希望对5G移动网络技术在安卓系统上的应用进行进一步探索，并可能存在LK以上层级的数据读写需要。因此，我们需要一部联发科5G手机。

第三，对于许多存在BootROM漏洞的联发科手机，需要拆机短接测试点才能进入BootROM下载模式(EDL)。而支持联发科BootROM漏洞的5G SoC手机中，又有一部分同时支持联发科Preloader漏洞。使用Preloader漏洞可以在不拆机的情况下，使Preloader运行崩溃从而自动跳转进入BootROM下载模式(EDL)。

满足以上条件后，仅剩SoC型号为联发科MT6833的以下机型可供挑选：

- Samsung Galaxy A13 5G

- Samsung Galaxy A14 5G

- Samsung Galaxy A22 5G

- Samsung Galaxy F42 5G

- Samsung Galaxy M13 5G

第四，根据三星公司的市场规划，S系列为全球市场的高端机型，A系列为全球市场的中端机型。而其他诸如C、F、M系列针对区域市场，可筛除。

三星公司对于机型尾号市场区域定义一般如下：

- B - 国际版 (除以下列出的全部国家或地区)

- U - 美国运营商合约机

- U1 - 美国无运营商锁版

- W - 加拿大版

- E - 南美洲版

- 0 - 中国、日本版

- N - 韩国版

最后，三星公司不允许所有美国版手机进行官方渠道OEM解锁。尽管使用mtkclient可以无视官方渠道限制，直接刷写seccfg分区中的Bootloader锁定标记，但这并不利于我们探究OEM解锁原理。而A13 5G及A14 5G在手机交易市场的主要货源为美国版。

结合所有条件，最终选择唯一可用的 Samsung Galaxy A22 5G (SM-A226B) 型号作为实验机型。

---

**什么是嵌入式系统平台？**

当我们提到嵌入式设备的时候，跟一般的计算机设备不太一样。

在电脑中，CPU厂商就是专门做芯片。然后主板、操作系统、应用程序等，全都是其他厂商的事情。软硬件相对独立。

而对于嵌入式SoC而言，除了造出芯片，他们的工作是要涵盖启动初始化到适配操作系统：软硬件、全功能、全流程的。

一颗SoC要管所有的事情，设备运行的所有环节都要有SoC厂商的文件和功能支撑。

每个SoC厂商都有自己的一套设计，底层逻辑差不多，具体实现上存在差异而已。

SoC厂商要设计引导程序、适配操作系统、适配外设、设计驱动，而一切的核心指向芯片自身功能。

比如一颗SoC芯片专为安卓智能手机设计，只能提供出用于适配安卓系统及智能手机相关外设的代码和文件，那可能就跑不了Ubuntu，因为缺少引导和内核层面的运行支持。

由于SoC芯片对软硬件运行环境的局限性和捆绑性，我们就把这称之为一个嵌入式系统平台，例如高通平台、联发科平台、海思平台等。

机顶盒SoC厂商有时连操作系统源代码都一起提供给OEM，恰恰是因为SoC厂商需要将操作系统对SoC功能和外设进行适配，加入适合的平台文件到特定操作系统版本。

安卓手机SoC厂商倒是不需要将操作系统捆在一起交付，但也有安卓的公共接口和规范需要遵循，才能用来引导特定版本的AOSP代码，并提供适用于特定版本安卓系统的平台文件。

所以，谷歌AOSP的许多设计，要各SoC厂商支持才能够实现。OEM也不是想挑选什么操作系统都可以，这同样需要SoC平台适配。

---

**手机闪存的存储逻辑跟电脑硬盘有啥区别？**

首先我们来看看，在暂不考虑独立TEE芯片的情况下，一部手机的主板上都有哪里能存放非易失性数据：

- SoC芯片内部 (BootROM)

- 闪存 (EMMC/UFS Flash)

没了。

SoC芯片内部有一个微型非易失性存储区域，叫做BootROM，存储内容由SoC厂商在生产芯片时固化写入，出厂后数据无法更改。BootROM存储的是芯片reset后最先执行的AArch64指令，它的存储容量有个几百KB就够了。

手机上其他的非易失性数据都存储在可删改的闪存，容量有128GB/256GB/512GB等。现代安卓手机闪存的类型有：

- EMMC

- UFS

没了。

对于现代电脑上的机械硬盘或固态硬盘，我们知道，其拥有唯一的"硬件分区"，例如64GB硬盘的扇区编号为0-124702719，一般采用GPT分区表对这个唯一的硬件分区进行不同用途的划分。

而对于一块手机闪存来说，它可以根据OEM厂商的设定，分为多个存储区域，工作起来就像好几块不同的"HDD/SDD"，而每一块"HDD/SSD"的设计用途都不太一样，这被称之为多个不同的"硬件分区"。

对于EMMC闪存来说，最多存在8个硬件分区：

- BOOT1

- BOOT2

- RPMB

- GP1

- GP2

- GP3

- GP4

- USER

那么对于UFS闪存来说，最多存在32个硬件分区：

- LU0 - 31

- 额外再加一个RPMB LU

在手机闪存中：

- BOOT1的作用是，存储Preloader代码，也就是BootROM的引导代码跳转到闪存上的引导代码的所在存储区域。

- BOOT2的作用跟BOOT1一样，作为一个备份。具体用不用BOOT2这个备份硬件分区、怎么个用法，由SoC/OEM厂商决定。反正闪存标准设计了这个概念。

- RPMB的作用是，安全存储。一般就是给TEE的专属存储区域，否则TEE的数据往哪里存嘞？RPMB的读取不受限制，但提供数据完整性检查机制。向RPMB写入数据需要密钥认证。

- USER的作用是：跟传统的HDD/SSD硬盘的那唯一一个存储区域一样。

我更愿意理解为硬件分区只是名字不一样，除RPMB外，实际都是性质相同、大小不同的存储空间。

每个硬件分区的扇区编号都是从0开始的，相当于一块独立的HDD/SSD硬盘的用法。如果是USER用途，则安卓手机平台也一般使用GPT分区表。但也可以选择不用：每个硬件分区实则跟一块电脑硬盘的概念一样。BOOT1和BOOT2一般为4MB大小，RPMB一般为16MB大小，USER的大小就是闪存剩余可用的空间。

对于UFS闪存而言，除RPMB作为"第33个硬件分区"外，BOOT1和BOOT2均由SoC/OEM厂商指定一个LU编号，剩下的LU都是EMMC概念中USER的用途，称为"普通LU"。需要创建几个普通LU也是由SoC/OEM厂商决定。

对于EMMC闪存而言，GP在设计用途上和USER几乎无差别。EMMC手机一般只会使用BOOT1、BOOT2、RPMB和USER硬件分区，这也是EMMC的出厂划分。所以EMMC出厂的时候，4个General Purpose Partition (GP)是不存在的，需要时由OEM厂商按需创建。一般没人用GP，真的没人用。

---

**主流SoC厂商的安卓LU规划方案？**

高通UFS

- LU0 - 安卓系统和用户数据硬件分区 (persist, cache, frp, system, vendor, userdata等)，普通LU

- LU1 - BOOT1 (高通平台Preloader)

- LU2 - BOOT2 (高通平台Preloader)

- LU3 - 高通平台配置参数硬件分区 (cdt, ddr等)，普通LU

- LU4 - 高通平台和Linux内核数据硬件分区 (tz, modem, abl, boot, recovery等)，普通LU

- LU5 - 高通平台基带参数硬件分区 (modemst1, modemst2, fsg, fsc等)，普通LU

- RPMB LU - 现代手机都会用到的TEE安全存储

联发科UFS

- LU0 - BOOT1

- LU1 - BOOT2

- LU2 - 普通LU

- RPMB LU - 现代手机都会用到的TEE安全存储

*高通/联发科EMMC*

- *除了BOOT1、BOOT2、RPMB以外，GPT分区都往USER里扔*

因此，这就解释了为什么在搭载高通SoC的安卓UFS手机中，能够看到/dev/sda、/dev/sdb、/dev/sdc、/dev/sdd、/dev/sde、/dev/sdf这六块"硬盘"的存在：并不是主板上搭载了六块闪存，而是一块UFS闪存被高通分为了6个不同用途的LU。从本质上讲，如果把这些零散的分区做成像联发科使用一整个LU2那样，或是像EMMC使用唯一的USER硬件分区那样，也行。只是高通不想。

虽然安卓手机的GPT分区数量众多，且这些GPT分区有时被做在了多个硬件分区中，有时只做在一个硬件分区，但是在平台、TEE、基带、安卓系统等不同的启动阶段，每一阶段的程序只会关心和使用自己的分区，只是它们共享了同一个GPT分区表或同一块闪存罢了。

---

**安卓的引导流程是什么？**

要透彻地理解BL锁和root，我们必须理解嵌入式设备的引导流程框架。我并不喜欢使用ARM官方框架来说明问题，这对解答疑惑没什么太大帮助。

我更愿意用启动阶段来描述这件事情。请注意，由于这里研究的是联发科处理器，所以在项目中，我会用联发科的叫法来称呼每一启动阶段及相关概念。到了高通平台上，是换汤不换药的，可能换了个名字罢了。另外，每一启动阶段都会用非对称加密算法校验下一阶段的镜像签名，所以现代安卓手机并不容易被黑客攻击。

- 第0阶段 - BootROM (PBL)：SoC执行BootROM中的代码。是否需要验证SBL签名的标记、以及用于验证SBL签名的公钥指纹，被OEM厂商在出厂时烧写在SoC的OTP区域里，也就是烧保险丝。

- 第1阶段 - Preloader (SBL)：BootROM加载并执行闪存中BOOT1的代码。但是取决于不同SoC厂商的设计，也可能用到BOOT2作A/B备份。一般只用BOOT1。SBL初始化平台外设，然后依次验证并加载闪存"USER硬件分区/普通LU"中TEE分区、LK分区的镜像。此时显示屏还没初始化，不亮屏，所以这个时候你就可以知道砖机卡在哪个启动阶段了。后面的启动镜像也都是从闪存"USER硬件分区/普通LU"中的某个或某些GPT分区中加载。

- 第2阶段 - Bootloader (LK)：亮屏了，显示LOGO。LK加载Modem(基带)。LK判断按键组合决定下一步做什么。如果触发Fastboot按键组合，则停留在LK，加载LK中的Fastboot刷机功能(三星手机使用了Odin替代Fastboot)。如果触发BootROM下载模式(EDL)按键组合，则向特定寄存器写一个"进入EDL"标记并reset SoC。如果触发Recovery模式组合按键，则第3阶段加载recovery镜像而非boot镜像。如果啥都没按，直接加载boot镜像。当然，如果啥都没按，但是misc分区等位置存在特定标记，也会执行像按键一样的操作。如果发现解了BL锁，同样是在LK这里显示警告画面。在Kernel加载期间，同样是显示LK的LOGO画面。

- 第3阶段 - kernel：启动Linux内核。recovery镜像是一个独立的微型内核和操作系统。而正常启动的boot镜像才是加载完整的安卓系统，依赖system、vendor、userdata等分区。boot和recovery镜像的结构是一样的，里面存放了kernel和ramdisk，ramdisk里存放了init1和fstab。kernel加载init1的时候不需要校验，因为LK已经把整个boot镜像校验过了。

- 第4阶段 - ramdisk (init1)：初始化Linux环境。验证并挂载分区。

- 第5阶段 - system (init2)：加载SELinux策略。显示bootanimation动画。加载完整的操作系统组件，进入桌面。

---

**BL锁是解在了哪里？**

LK。

LK在加载boot或recovery镜像的时候，如果检测到"BL已解锁"，则不校验镜像的签名。

boot和recovery镜像的签名使用谷歌AVB实现，存储在vbmeta中。LK首先校验vbmeta的签名，然后通过vbmeta读取并校验boot/recovery的镜像签名。

顾名思义，BL锁就是Bootloader锁。LK是Bootloader，所以LK的锁被去掉了。BL解锁后，从LK以下的第3阶段开始，都是可以自由修改的。LK本身不能被修改，会受到SBL的校验。

---

**BL锁标记是怎么设置的？**

LK当然需要通过一个标记来判断BL解锁状态。

LK其实是由SoC厂商实现，但是开放源代码给OEM厂商修改的一个东西。不过，改不改完全就是看OEM厂商想不想。

LK集成的功能其实很多：SoC厂商自有组件的初始化、SoC厂商的基带启动、BL锁标记的读取和写入、谷歌的Fastboot协议、谷歌的AVB实现。而Fastboot这种东西当然也可以被三星噶掉然后换成自家的Odin协议和LOGO。

虽然OEM锁(BL锁)这个东西由谷歌设计了一些规范，但是是由各平台(SoC厂商)自己去具体实现这些思想的，不是谷歌，也不是OEM厂商。当然，由于LK代码开源，OEM厂商他可以改。

所以不光是判定BL是否已解锁，就连"解锁"、"回锁"操作都是需要在LK中完成的，因为LK下面的阶段就全是安卓那套东西了，跟平台安全启动环境就脱钩了。

好好想一想，一般手机解BL锁的时候，用的"fastboot oem unlock"命令中的fastboot，是在哪个启动阶段？对了，LK。

好好想一想，三星手机回解BL锁的时候，用的Odin模式按键操作，是在哪个启动阶段？对了，还是LK。

所以LK上头的东西是不会让你动的。解了BL锁无非就是让你动LK下面的REE部分罢了。

LK上面的TEE、SBL什么的，你控制不了。LK自身的工作逻辑你都控制不了、被LK加载的镜像结构你也控制不了。解BL锁后无非就是可以按照boot/recovery的格式打包一个自己的镜像罢了。

由于LK下面已经是失信环境，所以BL锁相关操作都是在LK中完成的。

---

**BL锁的标记存放在了哪里？**

这就完全由SoC厂商自由发挥咯。

联发科一般使用"USER/普通LU"中的seccfg分区存放解锁标记。在seccfg这个GPT分区的第13字节处，0x02表示默认状态(上锁)，0x03表示BL已解锁，0x04表示BL已上锁。如果你觉得把闪存吹下来用编程器一改就万事大吉了，那就真的是做美梦了。seccfg的内容是用SoC的一机一密密钥签名过的，附在状态值后面。但如果出于某种原因，你已经得到了你要的状态的签名数据(一串32字节的加密哈希值)的话，那把闪存吹下来确实是可以随便改BL锁状态的。

高通早期的实现跟联发科类似，标记存放在devinfo分区，并且还没有任何校验。但是对于现在的高通LK代码而言，这种办法只适用于没有烧掉"安全启动"保险丝(指没有使能安全启动功能)的高通手机，否则高通会使用RPMB存取标记。说白了就是把devinfo从USER搬到了RPMB里。

其实不论是像联发科存放在USER硬件分区并用SoC安全算法签名，还是像高通使用RPMB硬件分区，BL锁标记实现的安全原理和安全程度是一样的，因为它们都由硬件安全实现，这也是为什么谷歌需要叫SoC厂商用LK实现谷歌的AVB和谷歌提出的BL锁这套东西。

另外，从原理上讲，如果SoC/OEM厂商愿意用烧保险丝(SoC OTP)的办法也是可以做BL锁标记的，只是解锁了以后还怎么回锁呢？所以RPMB靠谱多了，起码还能回锁。同样，如果OEM厂商愿意，可以改一改BL锁这部分的LK源代码。

用户动不了LK，因为LK要OEM签名。

---

**三星解锁后KNOX熔断是怎么回事？**

LK烧保险丝。

以三星手机为例，LK在校验vbmeta, boot, recovery等GPT分区的签名时，如果发现签名校验失败，就会熔断"保修"保险丝。值得注意的是，"BL已解锁"标记的存在并不会导致三星的LK去熔断"保修"保险丝，验证第3阶段启动镜像(vbmeta, boot等)的签名无效时才会。"KNOX熔断"后，由于TEE工作在第1阶段，我们无法控制KNOX读取"保修"保险丝的状态。KNOX见到"保修"被熔断就罢工，所以KNOX功能就失效了。这些都完全取决于OEM厂商的设计。

---

**保险丝是怎么烧的？**

在各启动阶段，包括Linux内核加载后，都会有SoC平台功能的函数接口可以被程序调用。内核加载后就该叫平台驱动了。

平台函数接口可以发送指令，控制芯片熔断OTP保险丝。

---

**安卓设置中的OEM解锁开关又是怎么回事？**

根据谷歌的游戏规则，OEM厂商不能默认允许用户在LK中操作BL锁的状态。

也就是说，虽然LK控制着BL锁的开关，但是"用户可以用LK去操作BL锁开关"这件事情又存在另一个控制开关。

这个开关被称作"OEM解锁"开关。主要是用来批准LK处理BL解锁指令，不针对BL回锁指令。

所以一共有两个开关。

手机出厂时，"OEM解锁"开关和"BL锁"开关，均为关闭状态。

用户需要先在安卓系统中启用"OEM解锁"开关，再进入LK，发送控制"BL锁"开关的指令"fastboot oem unlock"，完成BL解锁。

如果LK发现"OEM解锁"开关的状态为关，那就不接受控制"BL锁"开关的指令，不调用SoC平台函数处理对BL锁标记的修改动作。

---

**我在开发者选项中找不到OEM解锁开关，可以强开吗？**

我们刚刚知道了"BL锁"开关的存储位置。那么"OEM解锁"开关又存在了哪里呢？存在了安卓"ro.frp.pst"属性所指向的那个分区的最后一个字节。一般是指向"persistent"分区。

persistent分区中的最后一个字节，若为0x00，表示"OEM解锁"开关为关；若为0x01，表示"OEM解锁"开关为开。

对于三星手机，"OEM解锁"选项必须在联网后才会在开发者选项界面中出现。这是因为，三星有自己的KNOX安全管理方案。如果一台设备被KNOX限制，例如属于某个公司的资产，那么就不会允许BL解锁。因此，当每部三星手机恢复出厂设置后，KNOX状态会被重置。而"设置"APP通过读取KNOX状态来决定是否显示"OEM解锁"选项。新机需要联网更新KNOX状态，才能使隐藏的"OEM解锁"选项出现。有趣的是，KNOX状态不是在"格式化"过程中被重置的，而是在OOBE中的FRP环节被重置的。

直接改写闪存数据同样可以控制"OEM解锁"开关，而不一定在安卓设置中的开发者选项界面操作，因为这个界面中的开关实现的是相同的效果。

若不使用开发者选项界面，强行修改persistent分区的最后一个字节，在安卓系统内需要获取到system用户(uid=1000)权限才能dd这个分区，并会受到SELinux规则限制。OEM官方的"设置"APP自然具备这些条件。在安卓系统外则可随意改写。

persistent分区存在安卓Framework层实现的数据校验机制。分区的前32字节存储的是整个分区内容的SHA-256哈希结果，在计算时前32位填零。

LK就是读取persistent分区的最后一个字节来决定是否允许"OEM解锁"，即是否允许接受BL锁状态变更指令。有趣的是，LK似乎并不关心persistent分区的SHA-256哈希校验结果，只关心最后一个字节的值。当安卓系统的Framework发现persistent分区的SHA-256校验结果不对时才会重置persistent分区的内容，即填零，也就关闭了"OEM解锁"开关。

---

**为什么有的手机能自由回解BL锁，有的不能？**

OEM可以定制LK中BL锁的相关工作方式。它不给你调用SoC平台改写BL锁标记的函数就好了。

对于不支持BL锁功能的手机，OEM可以让Fastboot无视"OEM解锁"开关的状态。

对于不支持解锁的手机，OEM就让Fastboot不处理"解锁"指令。就算OEM的Fastboot接受该指令，LK实际不去调用SoC平台函数。

对于不支持回锁的手机，OEM就让Fastboot不处理"上锁"指令。就算OEM的Fastboot接受该指令，LK实际不去调用SoC平台函数。

对于需要解锁码解锁的手机，Fastboot就用OEM写的算法验证解锁码后，再调用SoC平台函数。

---

**什么是强解BL锁？**

使用非SoC/OEM提供的官方方式或途径把BL锁标记设置为解锁或锁定状态值，就叫强解BL锁。

官方方式一定是通过向LK发送Fastboot指令或者Odin按键一类的方式。然后LK去给你调用SoC平台写BL锁标记的函数。

如果你能直接访问闪存的特定分区，更改这个BL锁标记的值，就强解了BL锁。但一般这个标记是被硬件安全保护的，例如联发科seccfg同时存储了哈希签名值，或高通使用RPMB进行标记值的读取和写入认证。

一般来说，你不能强解BL锁是因为你既无法不拆机修改闪存中的BL锁标记值，也无法通过硬件安全校验对BL锁标记进行存入(RPMB)或签名(USER)。当然，你更无法修改LK，使SoC/OEM的这套机制失效。

如果存在SoC平台(BootROM/Preloader/LK)漏洞，则允许通过漏洞，不通过SoC/OEM设计的官方途径，注入代码，调用BL标记硬件安全相关的平台函数。

---

**为什么有的手机强解BL锁后，修改的ROM依然不能启动？**

尽管一般OEM厂商并不会动SoC平台跟BL锁有关的LK代码，但不代表他们不可以动。

OEM修改一下代码逻辑就可以了：不论BL是否已解锁，最终到验证boot/recovery镜像签名环节，无视标记，强制执行。

---

**LK真的好讨厌啊！**

是的。

---

**既然有的手机型号不让解锁，为什么OEM还不动LK代码，防止强解的BL生效？**

代码不是你想动，想动就能动。

的确，从理论上讲，所有的OEM确实可以直接删掉SoC平台BL锁的相关代码，使Fastboot的回解锁命令永远无效、使LK始终验证boot/recovery镜像的签名。

谷歌的胃口很大。安卓现在已经不仅仅是一个操作系统，倒更像是一整套完全的解决方案。谷歌以某种形式牵制着上游SoC厂商和下游OEM厂商。SoC、安卓、OEM三方都是一根绳上的蚂蚱，而AOSP也早早就被栓上去了，或者说成为了谷歌霸权安卓产业的工具，不可能像原来一样纯正了。

这就导致，无论是在哪一启动阶段的程序和代码，上到平台引导，下到安卓系统内部，都是藕断丝连的。

OEM稍微改一点平台或谷歌的逻辑，就可能产生蝴蝶效应，严重会导致操作系统无法工作、谷歌认证无法通过。

所以平台和谷歌的设计还是少动为妙。更何况谷歌已经给OEM设计好了"对于各种BL锁的需要，你应该怎么做，按我说的做，不需要你自己去动这部分逻辑"。

要说起来，OEM可能都不知道BL锁标记具体是怎么被平台存储的。只有我们会在这里执拗，所以这个项目叫做"三丧"呢。OEM只需要调用各种平台函数获取和更新BL锁状态就行了，甚至就连这些代码都是已经被平台写好的：OEM买来芯片，直接编译LK源代码就行了，知道那么多干吗。

这也就是为什么即使有些OEM成天嚷嚷着不让解锁，但各品牌手机的BL锁相关处理逻辑却很少能见到改动过的，然后网上那套撬锁理论好像就通用了。

---

**BL锁状态变更后会格式化userdata是怎么回事？**

这里面其实是两回事。

第一回事，如果用官解BL，那么LK调用SoC平台函数后，同时会写misc或等效标记，通知Recovery清除用户数据。

那么如果是强解BL，不通过LK直接改写BL状态值，当然就不会触发清除用户数据。

然后就会出现第二回事。现代安卓手机的userdata分区都是加密的，然后TEE里存储了根密钥。TEE检测到设备状态变更后，例如BL锁状态变化、boot分区签名状态变化，就会选择拒绝提供这个根密钥，直到恢复至密钥创建时的状态。

于是，init1检测到/data挂载不上，就会写misc或等效标记通知Recovery启动失败了，并重启。然后Recovery就会问你要不要试一下格式化？

当然，你也可以选择改改fstab，让userdata分区不加密或者不依赖TEE地加密。但是你改不了TEE。不过，一些APP会在运行时检测userdata分区是否已加密。

---

**BL解锁后开机会显示告警。手机启动画面怎么改？**

在三星手机中，LK至init1阶段的LOGO及告警图片存放在up_param分区。up_param分区在BL解锁后并不会被LK验证签名。

init2阶段的bootanimation存放在system分区。

---

**刷机救砖是怎么实现的？**

事实上，在很多启动阶段中，都提供刷机维护功能。

第0阶段(PBL)、第1阶段(SBL)、第2阶段(LK)、第3阶段(Recovery)都可能会提供刷机方式。

---

**PBL的刷机功能是如何实现的？**

所有SoC厂商的BootROM代码中，都内置了采用USB或UART等传输协议的刷机功能。在满足特定条件下，BootROM代码会使设备会进入BootROM下载模式，而非正常引导至下一阶段。一般这种情况被统称为紧急下载(EDL)模式。

高通手机芯片的EDL通信协议是Sahara协议，联发科手机芯片是SP协议，海思机顶盒芯片是HiBurn协议。

厂商设计EDL模式的原因是用来救砖或初始化闪存数据，因为EDL模式并不依赖闪存数据就可以独立运行。一般当BootROM检测到无论出于何种原因，例如闪存数据为空、SBL损坏、主板或闪存的EDL测试点被短接、SBL异常崩溃，也就是SBL无法正常被加载和运行的情况下，就会停留在BootROM的代码中并自动进入EDL模式。

而对于EDL模式来说，其实它最大的作用是从外部加载一个临时的SBL。

BootROM的代码非常精简，所以并不会包含完整的刷机功能。而会进入EDL的原因通常是因为SBL启动异常。所以只要有一个临时的SBL用来刷机就可以了。SBL里面才会有包括闪存驱动在内的完整的刷机功能支持。

高通的EDL专用的SBL叫做firehose编程器文件，联发科EDL专用的SBL叫做DA，海思机顶盒EDL用的SBL就是"海思fastboot"本尊。我们管这种特殊的SBL都叫DA好了。

DA也是需要签名验证的，跟PBL验证SBL没有任何区别。因为DA的本质是一个用途不同的临时SBL罢了。

一般的DA都会允许不受限制地刷写闪存中的任意位置，RPMB除外。这也恰恰是为什么大家都热衷于EDL，因为使用DA的读写效果跟把闪存吹下来用编程器搞没什么区别。那为什么要用编程器？纯粹的性价比问题。

也正是由于DA的这种设计目的，DA本身并不会关心GPT分区表，需要自己提供要读写的闪存扇区位置。

高通手机只需要一个拥有签名的DA(firehose编程器文件)即可使用EDL工具无限制地读写非RPMB硬件分区。除非OEM对DA进行了修改，在刷写过程中需要其他方式授权。

联发科手机则可能需要USB链路、DA签名、来自OEM的DA运行时授权这三者同时存在。

海思机顶盒并没有任何限制，一定程度上是因为需要拆机访问UART串口作为EDL传输协议。

所以，PC先把DA传进处于EDL模式的手机。然后PC与DA通信就可以实现刷写闪存数据了。

---

**听说有些手机很难进入EDL模式？**

是的。

早期，可能通过按键组合，LK就会给你跳转到EDL。

但后来OEM厂商发现这个功能好像不太需要，把这个功能去了。于是需要拆机短接测试点才可以进入EDL模式。

另外如何获得OEM签名后的DA也是一大问题。

因此鼓励使用LK刷机。

---

**SBL的刷机功能是如何实现的？**

Preloader自身一般也提供一些刷机和维护功能。

因为SBL工作正常的情况下手机是不会进入EDL模式的。启动直接进SBL。外加即使是进了EDL模式最终还是由某个SBL来实现具体刷机。所以SBL和DA这两个东西本身是有重叠之处的，在海思机顶盒平台更是完全一致的东西。

联发科手机平台是，一旦SBL发现连接了USB接口，则在Preloader阶段停留2秒，等待指令。若无来自USB端口的"进入Preloader模式"的指令则继续引导。

海思机顶盒平台是，如果SBL阶段接收到特殊的UART串口指令，则进入SBL中的HiBurn刷机模式。

EDL顾名思义，是紧急情况下的救砖入口。而SBL才是SoC平台预留的正常情况下的维护入口。

进入Preloader模式后，根据平台实现的不同，可能由SBL自身提供刷机功能，也可能给你跳转到EDL模式刷机，还可能提供其他类型的平台维护功能，例如进入Preloader中的工程模式。

---

**LK的刷机功能是如何实现的？**

与一般SoC平台的DA设计是能够自由读写闪存任意位置不同，LK会读取GPT分区表获取各分区名称。

未解BL锁的手机可能不允许使用Fastboot刷入分区数据。

即使BL解锁后的手机，LK也会按照分区名称进行限制，只能向特定分区写入数据。不允许提取分区数据。

但无论是DA、Fastboot还是编程器，读写闪存扇区的道理都是一样的，只是各途径有不同权限的问题。

---

**Recovery的刷机功能是如何实现的？**

Recovery就是一个小型的精简安卓系统。内核会根据GPT分区表对每个分区生成设备文件。

Recovery实际上就是用来刷使用安卓方式签名后的update.zip。原理上跟自己在操作系统中dd没什么区别。

---

**安卓系统有了root，是不是就可以dd闪存的任意位置了？**

调用闪存驱动确实不受限制。

但是OEM可以在LK中可以维护一个黑名单，对特定扇区进行写保护。因此即使使用安卓root权限调用闪存驱动成功，LK实际不会向闪存改写保护位置中的数据。

这就是"平台"：内核功能的实现需要平台架构支撑。

事实上，这个黑名单设计主要在联发科平台出现，并由联发科维护分区列表。此功能默认启用，OEM厂商可以按需修改。

---

**我怎么感觉联发科比高通做得更安全？**

是的。

---

**那为什么联发科反而曝出BootROM漏洞？**

设计得安全不等于实现得缜密。

---

**联发科漏洞的基本原理是什么？**

EDL模式的USB协商阶段发送攻击数据，使安全启动、EDL和DA相关鉴权暂时失效。然后发送自定义DA获得任意代码执行能力。

在SBL能够正常启动的情况下，如果想要不拆机进入EDL模式，则在SBL阶段等待USB指令2秒期间发送"进入Preloader模式"，攻击DA使其崩溃，根据平台设计会自动退入EDL模式。进入EDL后再使用上述的攻击方式。

---

**联发科漏洞为什么能强解BL锁？**

通过上述方法获得任意代码执行能力后，调用硬件安全函数计算并存储seccfg分区数据的校验哈希值。

随后可以选择用DA手动读写seccfg分区数据，进行BL锁状态的备份或强制变更。

这样强解能够生效的前提是OEM没有改过LK的相关逻辑，否则LK直接无视BL锁标记。

---

**所以一共有多少刷机途径？**

在EDL和Preloader模式下，安卓手机平台一般在PC上显示为USB模拟出的COM串口。

- 第0阶段 - BootROM的EDL模式。联发科串口在PC中显示为"MediaTek USB VCOM"。高通则是9008。联发科在EDL模式加载DA完成后，串口名称变为"MediaTek DA USB VCOM"。

- 第1阶段 - Preloader模式。联发科串口名称为"MediaTek Preloader USB VCOM"。高通则是9006。联发科进入Preloader的工程模式后，串口名称变为"MediaTek PreLoader USB VCOM (Android)"。

所以通过串口名称就可以判断手机处于哪个刷机模式了。每个平台的每个模式都有对应的PC端工具，使用时可能需要各种认证。

- 第2阶段 - Bootloader模式。这是安卓体系内的叫法，实际一般就是指Fastboot刷机模式。三星手机没有Fastboot，被替换为了三星自己的Odin。

- 第3阶段 - Recovery模式。加载Linux内核了。可以用ADB Sideload或者TF卡加载update.zip。

---

**高通的9008和9006模式是什么意思？**

USB设备ID。16进制。

---

**OEM能改SBL吗？**

一般不能。

PBL和SBL都属于平台的私有资产。

有些SBL提供给OEM时都是编译好的文件。

一般OEM只能签名SBL。

所以平台的Preloader功能会存在于各OEM厂商销售的产品中，就连漏洞也是一样。

---

**所有SoC厂商都会用BOOT1存放Preloader吗？**

并不是。

只是在安卓手机SoC的设计中，PBL似乎从一个固定位置(BOOT1硬件分区)加载SBL，要更加规范一些。

在海思机顶盒SoC平台中，只用到了USER硬件分区来存储一切。有TEE OS的情况下可能会使用RPMB。但BOOT1、BOOT2都是空数据。GP照例是未划分状态，不存在GP。

事实上，海思机顶盒SoC平台将SBL、LK的功能做到了一起，称为"海思fastboot"，跟安卓的Fastboot刷机没啥关系，只是名字一样。海思不使用GPT分区表，而是使用"海思bootargs"分区表，同样这跟传递Linux内核参数用的bootargs也没啥关系，只是名字一样。

---

**高通平台怎么称呼不同的启动阶段？**

SBL叫XBL。LK叫ABL或者aboot。其他的都差不多。

SBL这种东西其实可以细分为更多阶段(SBL1, SBL2等)，只是这对我们理解安卓引导原理没什么帮助，所以我就统一叫SBL。高通也意识到了这一点，所以把SBL的各细致阶段直接整合了一个叫XBL的东西，这个东西就是我一直在说的SBL。

现代高通的ABL都采用了UEFI实现。也就是在第2阶段，实则被加载的是高通UEFI，然后再由UEFI去启动ABL、Fastboot/Odin等UEFI应用。高通用UEFI替代了传统的LK。并且UEFI作为第2阶段引导方案是未来行业主流。

---

**不同启动阶段的程序都是谁的设计？**

PBL加载SBL。SBL加载ATF(TEE)、LK。LK判断按键或misc、判断和操作BL状态、加载Modem、加载kernel。Fastboot/Odin位于LK自身。

PBL、SBL、LK都是SoC平台的东西。在LK中，平台会结合谷歌的东西，或者说是平台对于谷歌东西的实现。LK往下都是谷歌的东西，包括谷歌对Linux的定制。OEM也可自行定制LK及以下的所有东西。

TEE镜像、Modem镜像一般都是SoC厂商自己的代码。TEE OS、Modem并行运行于REE环境，即与安卓系统并行运行，相互之间存在通信接口。

---

**各启动阶段的镜像都由谁来签名？**

OEM。

从第1阶段到第5阶段的启动镜像，全都是由OEM签名。

平台的文件(SBL、TEE、LK、Modem等)使用平台的签名方法。LK加载的vbmeta也用平台的签名方法。

boot、recovery、system、vendor等安卓镜像使用谷歌AVB签名方法。

但都是用OEM的私钥。

手机出厂时，OEM会向SoC的OTP烧录密钥和一些标志位，也会向闪存烧录RPMB密钥。OEM一般都会熔断SoC"安全启动"标志位，使能安全启动功能。否则PBL不验证SBL签名，然后就不用有"三丧A226B"这个项目了。

---

**各大OEM厂商似乎很喜欢谷歌这套东西，把运行环境掌控得死死的？**

不然谷歌不给你认证。

---

**SoC平台为什么也要按照谷歌的想法做？**

不然谷歌不给OEM认证。

芯片卖给谁去啊？

---

**没了谷歌，很多厂子还造不出个智能手机了？**

是的。

---

**照这样看，OEM的发挥空间其实很小？**

是的。

底层的东西都动不了。都是被平台和谷歌牵着鼻子走的。需要很多的商业授权。

能动的也就是花里胡哨的名字、APP、Framework、交互和界面这些表层的东西，给人们最直观的感受说，好像每个品牌的手机都大不相同。

实际上安卓生态的架构都一样。要么叫"安卓生态"呢。

不然每个OEM都从底层架构自由发挥，哪儿来的那么多通用的BL解锁和安卓root的概念呢？Magisk都要分成几十个版本了；OEM锁都能分成BL锁、CL锁、DL锁和EL锁了。

---

**TEE是一个独立的芯片吗？**

不需要是。

现代ARM架构处理器支持TrustZone技术。简单来说，芯片自身可以将来自TEE OS和REE OS的指令和内存数据进行隔离。而EMMC/UFS闪存又提供了RPMB给TEE存东西使。所以SoC + 闪存就足够搭建一个TEE环境了。

就像REE OS(boot, system分区)存在了"USER/普通LU"中一样，TEE OS(tee分区)也存在"USER/普通LU"中。由SBL带起TEE环境，由LK带起REE环境，并行运行即可。REE(安卓系统)和TEE之间存在通信接口。

但有一些OEM厂商对设备安全有更高的追求，那么他们当然可以选择投入更多成本，为手机添加一块独立的TEE安全芯片，一般支持独立的安全运算和安全存储。反正最后都是用户买单。

---

**搞清楚了这些，所以究竟要怎么root？**

安卓有一套自己的RC启动服务管理机制，类似于发行版Linux中的systemd机制。单纯为了获取到"Android Linux"的root权限没那么复杂。

根据Linux内核的共性，init具有天生的root权限。而init执行着安卓RC机制。

BL解锁后，在"/system/etc/init"目录下添加一个自己的安卓RC服务就可以了。创建"/system/etc/init/local.rc"：

```
on boot
    symlink /mnt/vendor/efs/wifi/.mac.info /efs/wifi/.mac.info
    chmod 0777 /efs/wifi/.mac.info

on property:dev.bootcomplete=1
    start local

service local /system/bin/sh /system/etc/rc.local
    class main
    seclabel u:r:shell:s0
    user root
    group root
    disabled
    oneshot
```

其实这就足够了："/system/etc/rc.local"脚本会在每次开机时获得继承型Linux root权限。"on boot"字段是用来解决A22 5G后期一个特定的Wi-Fi问题，在一般的安卓手机上不需要。

一般来说，我们希望"rc.local"脚本能够灵活地执行随时变更的内容。但为了应对手机恢复出厂设置后，/data、/cache、/sdcard目录手动访问受限的问题，要用system分区的脚本带一下。创建"/system/etc/rc.local"：

```
#! /system/bin/sh

/system/bin/sleep 5

if [ -f /storage/emulated/0/rc.local ]; then
    /system/bin/cp /storage/emulated/0/rc.local /mnt/rc.local
    /system/bin/sh /mnt/rc.local
fi

if [ -f /data/rc.local ]; then
    /system/bin/cp /data/rc.local /mnt/rc.local
    /system/bin/sh /mnt/rc.local
fi
```

这下好了，优先带起/sdcard中的脚本。"sleep 5"是为了等待/sdcard挂载。如果/sdcard开机时挂载不上，例如用户没有输入锁屏PIN密码，则用/data中的脚本替代。

对于这种设计，每次OOBE后可以直接用Chrome浏览器APP下载脚本到/sdcard，然后重启手机，先拿一次临时权限，再用它到/data里面做长期配置。

反正这个"rc.local"启动脚本会继承来自init的root权限。剩下的，你想咋整就咋整。

---

**脚本是有权限了，可怎么用来进入交互式root命令行？**

万能的BusyBox。telnetd，我的最爱。

rc.local脚本里带起一个"busybox telnetd -l /system/bin/sh"进程，用电脑Telnet连接到手机root命令行，做自己想做的事。

需要先找一个安卓AArch64版本的BusyBox，放到/sdcard下。然后，示例脚本"/sdcard/rc.local"如下：

```
#! /system/bin/sh

/system/bin/touch /mnt/rc.flag
/system/bin/cp /storage/emulated/0/busybox /data/local/tmp/busybox
/system/bin/chmod +x /data/local/tmp/busybox
/data/local/tmp/busybox telnetd -F -l /system/bin/sh
```

touch一个标记是为了在telnetd带不起来的时候，好歹能用ADB看一下这脚本到底有没有被执行。

需要加"-F"是因为，根据安卓RC机制的特性，如果主进程退出，子进程就也退出了。所以必须得让telnetd在前台挂着。但这是相对于RC的前台。而从操作系统的视角上看，每个RC服务都是在后台运行着。

同时，rc.local脚本所带起的内容，都是属于RC服务的一部分。RC服务不会被安卓系统的性能优化机制所随意管控，例如暂停、中止或释放内存，始终会稳定驻守在操作系统后台运行。

---

**这么简单就可以了？**

当然不是。

自建RC服务，需要能够自主修改system分区，并会受到安卓SELinux安全机制的管控。

---

**要怎么去理解SELinux这个东西？**

传统的Linux内核只有一种权限管理机制，就是基于用户(uid)和组(gid)的权限管控。我喜欢把它称为"用户控制"。

所谓Linux系统中的root用户或root权限，实则跟"root"这个单词无关，而是当uid = 0的时候，自动获得了系统最高权限。叫什么名字都无所谓。

除了uid = 0的情况下，其他的uid甭管名字听着有多唬人，什么"system"、什么"shell"，实际上只要uid不为0，都是普通用户。只要uid为0，就是特权用户。

你的uid是多少，就只能干你的uid被允许的事情；你的uid是0，就可以干所有的事情。这就是"用户控制"机制。

但"用户控制"对于一个主体的权限管理不够细致化。他们认为仅凭uid、gid、rwx权限这几样东西去关联一个目标主体还远远不够。

SELinux提供了另一种权限管理机制，我喜欢把它称为"条件控制"。

每一个主体都会被标记一个上下文。SELinux规则文件存储了哪些上文能够访问哪些下文、只允许执行哪些动作。

在传统的Linux系统中，只要通过"用户控制"，进程就可以访问目标主体。

而在SELinux开启的系统中，要同时通过"用户控制"和"条件控制"，即同时具备相应的uid和上下文条件，进程才可以访问目标主体。

SELinux工作形态是"用户控制"的一个hook。

OEM会强制启用安卓的SELinux功能。对啊，不然谷歌不给你认证。

而关闭它就不要想了。首先，内核有没有把SELinux的开关噶掉都是一回事，新一点的手机系统早就噶了这个内核功能开关，只能是开启状态。其次，许多APP如果检测到SELinux不是enforce状态，都会拒绝运行。

安卓SELinux给root带来的最大影响就是，虽然从init继承了root权限，但所有动作被SELinux规则拒绝得死死的，没有一个脚本能被带起来。是啊，有了uid = 0的权限这没错，但是带不起任何东西，郁闷不？因为你带起自定义脚本或进程的行为，只是用权限继承的方式通过了"用户控制"，却无法通过"条件控制"。

---

**既通不过，还不能关，咋解决？**

改内核。

内核可执行文件位于boot镜像中。

SELinux的相关内核函数，在判定控制结果为"拒绝"时，会调用avc_denied()内核函数来进行最终的处理方式确认。

```
static noinline int avc_denied(u32 ssid, u32 tsid, u16 tclass, u32 requested, unsigned flags, struct av_decision *avd)
{
	if (flags & AVC_STRICT)
		return -EACCES;
	if (selinux_enforcing && !(avd->flags & AVD_FLAGS_PERMISSIVE))
		return -EACCES;
	avc_update_node(AVC_CALLBACK_GRANT, requested, ssid, tsid, tclass, avd->seqno);
	return 0;
}
```

修改内核可执行文件的AArch64汇编指令，直接跳过两个if判断条件即可。

SELinux还是开着的，没错。SELinux也拒绝了动作，没错。但，拒绝的处理方式为始终放行(返回值为0)。

然后root就完事了。

---

**说了那么半天BL锁，为什么root的篇幅这么短？**

不解锁怎么root？

满世界都是签名验证。BL解锁了才能在LK阶段打破信任链。

root本身本来就很简单，真正有门道的是OEM锁(BL锁)。

如果单说root，倒是Magisk这种东西把事情搞复杂了。但是Magisk能够实现的效果也不仅仅是root这么简单，它早已成为了一套完整的安卓改机方案，有着自己庞大的生态，而root不过是其中一个小小的必要环节。所以其实把使用Magisk叫做"root一部安卓手机"并不妥善，倒不如叫做"Magisk一部安卓手机"。

事情刚好反过来了。你以为解BL锁是很简单的事情，拨开"OEM解锁"开关并使用fastboot命令就好了，实际上BL锁的门道才像上面说的一样复杂；你以为安卓root是很复杂的事情，是要Magisk、TWRP这种规模性技术团队研发的工具才能实现的事情，实际上就是加两个文本文件、改两条内核AArch64指令的事。

---

**Magisk是如何实现的root？**

Magisk的思想是到处hook。所以Magisk只需要修改boot镜像即可。因为它把init都hook了，然后再去hook别的东西，什么SELinux策略、RC服务、system目录之类的。如果只是简单要个root执行权限，这动静未免有点太大。

Magisk直接自己写了个init，修改ramdisk使内核先行加载自己的init，然后再去调用OEM的init。由于init分不同阶段，那么就hook两次。等于先要加载Magisk的init1，然后加载原厂init1，再加载Magisk的init2，再加载原厂init2。所以Magisk的root权限还真就是自己翻江倒海给出来的。这就是为什么说你的手机从OEM的变成Magisk的了。

---

**所以到底什么情况下该用Magisk？**

Magisk存在的最大意义是针对安卓应用。它这么费劲扒拉地hook半天，搞得它自己"就是"操作系统，就是为了对付各种安卓应用。

因此如果你的目标是安卓应用而不是原生层，那么请直接使用Magisk，不要尝试使用本项目的思想，它们的定位和目标都是天壤之别。在这种情况下，你需要的正是一台Magisk控制的手机。

---

**root后还需要刷第三方Recovery吗？**

先说一下为什么大家都依赖TWRP，无外乎就是两点。

第一，刷模块方便。TWRP是配套Magisk一起用的，有什么模块都可以自动或手动进去刷。

第二，不懂项目中涉及的基础，不会自主操作，只能按照提示和教程操作。

否则，还有什么比直接修改内核、dd闪存、运行时编辑EXT4分区资源更痛快的呢。Recovery这种东西根本用不上。

---

**直接用telnetd敞开root命令行是不是非常不安全？**

是的。

不过一般手机只能通过局域网访问监听的TCP端口。但局域网也不一定安全，尤其是到了公共场合连接Wi-Fi。

因此我写了一个简单的登录程序，在通过密码鉴权后，才会跳转到sh命令行。该程序使用了DJB2算法来加密密码。telnetd的入口指向编译好的文件即可。

我们大家都不喜欢编译。这种设计的好处在于，只需要编译一次。若要修改密码，只需要使用UltraEdit一类的Hex文本编辑软件，编辑ELF文件中的DJB2哈希值即可。可以用Python生成DJB2哈希值。

---

**如何快速地编译一个安卓ELF？**

r15c版NDK，我的最爱。

虽然有点老，但编译方便。无需任何其他安装，解压就能用的gcc编译器。输出的ELF，上到安卓13+，下到安卓5，通吃，直接无压力运行。

https://github.com/android/ndk/wiki/unsupported-downloads#r15c

在Windows操作系统中就可以轻松编译安卓ELF。示例编译命令如下：

```
"C:\Program Files\android-ndk-r15c\toolchains\arm-linux-androideabi-4.9\prebuilt\windows-x86_64\bin\arm-linux-androideabi-gcc.exe" -pie -fPIE --sysroot="C:\Program Files\android-ndk-r15c\platforms\android-24\arch-arm" login.c -o login.elf
```
---

**拿到一部安卓手机后应该先做什么？**

备份。

在可能的情况下，备份闪存除RPMB外的所有硬件分区。

值得注意的是，网络上的思想一般都是在独立备份各个GPT分区。实际上更为合理的办法是将整个硬件分区作为一个整体进行数据备份，不需要被GPT这种软分区表切得七零八碎，也就是俗称的全盘备份。需要的时候从备份中二进制切出对应分区的部分即可。

因此，如果可以拿到DA或dd权限，则应将每一个硬件分区备份为一个文件。例如：boot1.bin、boot2.bin、user.bin。备份时可以跳过userdata分区的扇区区段，因为没有用，还浪费时间和存储空间。

许多GPT分区中的数据为该部手机的专有数据，例如基带参数，无法从其他手机或固件包中复制来恢复。因此，把每部手机都做好备份非常重要。

---

**RC方式root后怎么隐藏痕迹？**

一般不需要隐藏。因为不存在类似Magisk的"root解决方案"的痕迹。

但有几点可以酌情优化：

- 隐藏su进程。这是因为用telnet登录命令行时，Linux会创建root所有的pts接口。每次登录后用chmod把"/dev/pts/*"的所有者改成别人就行了。

- getprop中有关BL锁状态的值。

还有几点优化不了：

- SELinux会允许neverallow条目。这是因为我们把内核改了，而avc_denied()中没有空间做更细致的汇编指令修改，只能放行所有动作。不过这一点一般不会作为APP判定运行环境有风险的因素。但一定是通过不了谷歌认证的原因。

- BL未锁定、TEE损坏。

---

**如何修改安卓属性中BL锁状态的值？**

```
setprop ro.boot.vbmeta.device_state locked
setprop ro.boot.verifiedbootstate green
setprop ro.boot.flash.locked 1
setprop ro.boot.veritymode enforcing
setprop ro.boot.warranty_bit 0
setprop ro.warranty_bit 0
setprop ro.vendor.boot.warranty_bit 0
setprop vendor.boot.vbmeta.device_state locked
```

而能够使用setprop修改ro值的前提是，修改init2文件的AArch64汇编指令，让setprop执行时不检查SELinux安全策略、且允许覆写ro属性。

一个有意思的坑是，由于init是不使用avc_*()来执行SELinux策略的，所以我们的内核修改对init无效，只对一般进程有效。因此修改过后的init文件要用chcon打上原始init的SELinux标签，否则安卓系统无法启动。同样因此，init中setprop时的SELinux策略检查函数也要修改，使其始终返回0为允许。

但是现在一般的APP已经不使用getprop的方式来检查BL锁锁定状态，转而使用Play Integrity API (原SafetyNet)。这种方法是基于尝试与TEE实际通信进行判定。这就是为什么"BL未锁定、TEE损坏"我们自己不好优化，工作量太大。有相关需求应该选择Magisk。

且APP判定运行环境风险一般是基于root痕迹，而不是检测BL锁或者TEE状态。因此优化这些属性值的意义不大。

---

**BL解锁不等于root？**

是的。

BL解锁只是允许LK不验证将要启动的内核镜像的签名。

内核及安卓系统有没有能够获取root权限的第三方进程或改动，又是另外一码事。

这也就是为什么解锁BL并不会导致三星熔断，修改内核或安卓系统文件才会。常见的三星熔断触发原因主要基于对boot或vbmeta分区的改动。

```
Set Warranty Bit : vbmeta
Set Warranty Bit : boot
```

---

**LK如何将BL锁状态传递进安卓系统？**

传递内核命令行参数。

cmdline中存在"androidboot.verifiedbootstate=green"为未解锁，"androidboot.verifiedbootstate=orange"为已解锁。

---

**能不能把内核改掉，向后传递green？**

可以。但系统不一定还能启动。

内核初始化早期会将LK传递的cmdline的值，复制到另外一块内存区域，进行存储。我们就劫持这个地方的内存。

```
0x0000000000000000:  B4 E7 8C 52    movz w20, #0x673d
0x0000000000000004:  74 56 04 79    strh w20, [x19, #0x22a]
0x0000000000000008:  54 AE 8C 52    movz w20, #0x6572
0x000000000000000c:  74 5A 04 79    strh w20, [x19, #0x22c]
0x0000000000000010:  B4 CC 8D 52    movz w20, #0x6e65
0x0000000000000014:  74 5E 04 79    strh w20, [x19, #0x22e]
0x0000000000000018:  14 04 84 52    movz w20, #0x2020
0x000000000000001c:  74 62 04 79    strh w20, [x19, #0x230]
```

在这个例子中，X19寄存器存储的是kernel复制好的cmdline的基址。"=orange"起始于(X19+554字节)处。

当我们用这种方法覆写内存为"=orange"后，也就是等同于啥也没改的情况下，系统可以正常启动。

而覆写为"=green"或其他字母后，系统无法启动。

疑似存在内核或驱动层面的其他校验机制。但细究这个问题意义不大，因为安卓属性可以在系统内部改，而且改了也没啥用，几乎没有APP用这种方式判定BL锁状态。

---

**自己手动root都要改哪些分区？**

- vbmeta - 将第123字节改为0x03。

- boot - 修改内核并重新打包boot镜像。

- super - 修改super中的system分区并重新打包super。可以在这个过程中顺便把system分区从只读改成可读写。

---

**如何修改内核？**

如果有内核符号表的情况下，使用IDA分析内核会简单得多。

使用root权限提取内核符号表的步骤如下：

```
echo 0 > /proc/sys/kernel/kptr_restrict

cat /proc/kallsyms > kernel_syms.txt
```

然后用IDA加载这个脚本，导入kernel_syms.txt就可以分析了：

https://github.com/gmh5225/IDA-KallsymsSymbolRenamer

但这样提取就存在一个先有鸡还是先有蛋的问题。要不咱还是先用Magisk root一次？

让我来放一个avc_denied()函数的AArch64的指令模型，事情可能会简单得多：

![Function avc_denied()](https://thumbs2.imgbox.com/c3/3c/BxXcZJxJ_t.png)

---

**如何解包打包boot镜像？**

在Debian中使用这个项目：

https://github.com/ravindu644/Kitchen

示例过程如下：

```
bash kitchen unpack boot.img

rm ./workspace/kernel

wget -O ./workspace/kernel http://192.168.1.2:8000/kernel-modified

bash kitchen repack
```

---

**如何解包打包super镜像？**

在Debian中使用这些步骤：

https://blog.senyuuri.info/posts/2022-04-27-patching-android-super-images

示例过程如下：

```
simg2img super.img super-raw.img

./imjtool super-raw.img extract

cd extracted

fallocate -l 4.3G system.img

/sbin/resize2fs system.img

mkdir /system

mount -t ext4 -o loop system.img /system

cat > /system/system/etc/init/local.rc <<EOF
on boot
    symlink /mnt/vendor/efs/wifi/.mac.info /efs/wifi/.mac.info
    chmod 0777 /efs/wifi/.mac.info

on property:dev.bootcomplete=1
    start local

service local /system/bin/sh /system/etc/rc.local
    class main
    seclabel u:r:shell:s0
    user root
    group root
    disabled
    oneshot
EOF

chcon --reference /system/system/etc/init/hw/init.rc /system/system/etc/init/local.rc

cat > /system/system/etc/rc.local <<EOF
#! /system/bin/sh

/system/bin/sleep 5

if [ -f /storage/emulated/0/rc.local ]; then
    /system/bin/cp /storage/emulated/0/rc.local /mnt/rc.local
    /system/bin/sh /mnt/rc.local
fi

if [ -f /data/rc.local ]; then
    /system/bin/cp /data/rc.local /mnt/rc.local
    /system/bin/sh /mnt/rc.local
fi
EOF

chcon --reference /system/system/etc/init/hw/init.rc /system/system/etc/rc.local

umount /system

rm -r /system

/sbin/e2fsck -yf system.img

../lpmake --metadata-size 65536 --device-size 7239368704 --metadata-slots 2 --group main:7148904448 --partition system:none:4652879872:main --partition odm:none:4349952:main --partition product:none:1397633024:main --partition vendor:none:1094041600:main --image system=./system.img --image odm=./odm.img --image product=./product.img --image vendor=./vendor.img --sparse --output ../super-out.img

cd ..
```

另外，如果system分区使用了EROFS而不是EXT4，则需要搜索适用于EROFS的相关处理命令。

---

**如何下载到三星手机官方固件？**

使用以下项目，可以通过三星官方FUS服务器下载工厂固件：

https://github.com/MatrixEditor/samloader3

---

**固件包中存在userdata分区镜像的意义是什么？**

这是工厂固件。工厂固件是用来生产新机用的。

尽管恢复出厂设置，格式化userdata分区后，/data会被清空，然后再经过OOBE，手机也可以用。

但是userdata镜像里可以提供一吨的新机预装APP，让赞助商高兴。否则预装APP就只能存在system分区，齁占地方的。

这就是为什么我每次OOBE后，时而能在A22 5G上看到谷歌全家桶，时而不能。能的情况一定是用Odin刷的工厂包，不能的情况一定是用的Recovery格式化。

---

**如何找到一个靠谱的Odin刷机工具？**

认准XDA论坛。

https://xdaforums.com/t/patched-odin-3-13-1.3762572

---

**如何自己创建Odin用的刷机文件？**

把命名好的分区镜像文件打包成tar就可以了。

---

**三星的版本号看起来好乱，有什么讲究？**

看版本号的倒数第五位。

倒数第五位是递增的，从0开始，到8、9、A、B、C、D、E、F这样。

例如：版本号"A226BZHSADXH1"实为版本10。

这个版本号是SoC版本熔断标记保险丝的版本号。所以会存在多个软件发行版本倒数第五位相同的情况，也就是三星认为没必要熔断一个版本标记的时候，保险丝还是省着点用。

---

**防降级熔断如何工作？**

PBL会验证SBL的版本号是否大于等于OTP中保存的版本号，否则直接停留在EDL。对于EDL加载DA来说也是一样的道理。

如果已经升级，版本号OTP熔断状态已经变更，再拿原来的备份全盘恢复，肯定会黑砖，自动进EDL。

另外，SBL当然也会验证要加载的LK版本是否匹配。

---

**黑砖要怎么救？**

如果能使用EDL加载DA刷写闪存分区的话，就拿固件包里的分区镜像一通乱刷，应该就能亮屏了。

亮屏之后用Fastboot或者Recovery刷固件包就可以了。

否则除了用编程器好像真没什么好办法。

所以重点在于，不要折腾成黑砖，会很难搞。至少要保证LK能用。

---

**说的是禁止废话，但你的废话好像很多？**

概念多。

在了解这些概念的情况下，就会发现root的思路其实挺简单，然后操作起来又是另外一回事罢了。所以Magisk一个原本很复杂的东西，由于其"傻瓜式操作"的实践特点，才会理所当然地成为"安卓root"的代名词。

---

**上面提到的A22 5G的Wi-Fi问题是怎么回事？**

由于修改了内核，使SELinux不再阻止任何动作，似乎触发了A22 5G操作系统的一个逻辑bug。

修改内核后Wi-Fi的MAC地址会丢失，变成每次开机随机生成的MAC地址。

这个bug的成因，似乎是因为，安卓系统中某个Wi-Fi相关的组件，会优先读取"/efs/wifi/.mac.info"中存储的MAC地址。如果访问失败，也就是被预设的SELinux策略默认拒绝后，再尝试使用"/mnt/vendor/efs/wifi/.mac.info"配置。访问前者成功就造成了系统根本不会去理会后者，而平台驱动实则将MAC地址从基带参数加载后存入后者。

所以解决这个问题的思路，就是要让前者无法被访问...... 才怪！

在实践中，有一股来自疑似其他RC服务的力量，使前者永远有"用户控制"权限被访问。只有存在"条件控制"的情况下，前者才能够无法被访问。而"条件控制"功能已经被我们砍废了。

那就换个思路，开机时把前者直接symlink到后者即可。实现后，A22 5G Wi-Fi工作一切正常。

# 你说的不对 / 我还有问题

提个Issue咯。
