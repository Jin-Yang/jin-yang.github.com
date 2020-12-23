---
title: 如何在 Linux 中查看 CPU 信息
layout: post
comments: true
language: chinese
tag: [Linux,DevOps]
keywords: cpu info, linux
description: 现在的 CPU 一般是多核，甚至包括了现在的手机，而每个核又包含了缓存。那么，在 Linux 上如何通过命令查看当前 CPU 的信息呢？
---

现在的 CPU 一般是多核，甚至包括了现在的手机，而每个核又包含了缓存，那么，在 Linux 上如何通过命令查看当前 CPU 的信息呢？

例如，常见的型号、时钟频率、核数等等，在这篇文章里会详细介绍。

<!-- more -->

## 简介

查看不同的信息使用的命令也略有区别，例如最基本的 CPU 供应商、CPU 型号、时钟频率、核数、L1~3 缓存配置等，还有 CPU 的能力，例如硬件虚拟化、MMX、SSE 等。

当然，获取不同的信息使用的命令也略有区别，这里仅介绍一些常用的命令。

## /proc/cpuinfo

包含了 CPU 相关的基本信息，信息是按照逻辑核展示的。

{% highlight text %}
processor       : 0
vendor_id       : GenuineIntel
cpu family      : 6
model           : 78
model name      : Intel(R) Core(TM) i5-6200U CPU @ 2.30GHz
stepping        : 3
microcode       : 0xd6
cpu MHz         : 2721.118
cache size      : 3072 KB
physical id     : 0
siblings        : 4
core id         : 0
cpu cores       : 2
apicid          : 0
initial apicid  : 0
fpu             : yes
fpu_exception   : yes
cpuid level     : 22
wp              : yes
flags           : fpu vme de pse tsc msr pae mce cx8 apic sep mtrr pge mca cmov pat pse36 clflush dts acpi mmx fxsr sse sse2 ss ht tm pbe syscall nx pdpe1gb rdtscp lm constant_tsc art arch_perfmon pebs bts rep_good nopl xtopology nonstop_tsc cpuid aperfmperf tsc_known_freq pni pclmulqdq dtes64 monitor ds_cpl vmx est tm2 ssse3 sdbg fma cx16 xtpr pdcm pcid sse4_1 sse4_2 x2apic movbe popcnt tsc_deadline_timer xsave avx f16c rdrand lahf_lm abm 3dnowprefetch cpuid_fault epb invpcid_single pti ssbd ibrs ibpb stibp tpr_shadow vnmi flexpriority ept vpid fsgsbase tsc_adjust bmi1 avx2 smep bmi2 erms invpcid mpx rdseed adx smap clflushopt intel_pt xsaveopt xsavec xgetbv1 xsaves dtherm ida arat pln pts hwp hwp_notify hwp_act_window hwp_epp md_clear flush_l1d
bugs            : cpu_meltdown spectre_v1 spectre_v2 spec_store_bypass l1tf mds swapgs itlb_multihit srbds
bogomips        : 4800.00
clflush size    : 64
cache_alignment : 64
address sizes   : 39 bits physical, 48 bits virtual
power management:
{% endhighlight %}

## lscpu

对于 `/proc/cpuinfo` 中的内容比较分散，每个核的大部分内容都是相同的，而 `lscpu` 命令提供了一个更加友好的格式统计方式，例如 CPU、核数等信息。

{% highlight text %}
Architecture:        x86_64
CPU op-mode(s):      32-bit, 64-bit
Byte Order:          Little Endian
CPU(s):              4
On-line CPU(s) list: 0-3
Thread(s) per core:  2
Core(s) per socket:  2
Socket(s):           1
NUMA node(s):        1
Vendor ID:           GenuineIntel
CPU family:          6
Model:               78
Model name:          Intel(R) Core(TM) i5-6200U CPU @ 2.30GHz
Stepping:            3
CPU MHz:             2511.447
CPU max MHz:         2800.0000
CPU min MHz:         400.0000
BogoMIPS:            4800.00
Virtualization:      VT-x
L1d cache:           32K
L1i cache:           32K
L2 cache:            256K
L3 cache:            3072K
NUMA node0 CPU(s):   0-3
Flags:               fpu vme de pse tsc msr pae mce cx8 apic sep mtrr pge mca cmov pat pse36 clflush dts acpi mmx fxsr sse sse2 ss ht tm pbe syscall nx pdpe1gb rdtscp lm constant_tsc art arch_perfmon pebs bts rep_good nopl xtopology nonstop_tsc cpuid aperfmperf tsc_known_freq pni pclmulqdq dtes64 monitor ds_cpl vmx est tm2 ssse3 sdbg fma cx16 xtpr pdcm pcid sse4_1 sse4_2 x2apic movbe popcnt tsc_deadline_timer xsave avx f16c rdrand lahf_lm abm 3dnowprefetch cpuid_fault epb invpcid_single pti ssbd ibrs ibpb stibp tpr_shadow vnmi flexpriority ept vpid fsgsbase tsc_adjust bmi1 avx2 smep bmi2 erms invpcid mpx rdseed adx smap clflushopt intel_pt xsaveopt xsavec xgetbv1 xsaves dtherm ida arat pln pts hwp hwp_notify hwp_act_window hwp_epp md_clear flush_l1d
{% endhighlight %}

## cpuid

有点 CPU 专用的信息收集工具，可以通过 [CPUID](https://en.wikipedia.org/wiki/CPUID) 功能获取详细的 CPU 硬件信息，包括了 CPU 类型、扩展指令、缓存信息等等。

对于 CentOS 需要从 EPEL 库中安装，默认是不会安装的，直接通过 `yum install cpuid` 即可。

{% highlight text %}
CPU 0:
   vendor_id = "GenuineIntel"
   version information (1/eax):
      processor type  = primary processor (0)
      family          = 0x6 (6)
      model           = 0xe (14)
      stepping id     = 0x3 (3)
      extended family = 0x0 (0)
      extended model  = 0x4 (4)
      (family synth)  = 0x6 (6)
      (model synth)   = 0x4e (78)
      (simple synth)  = Intel Core (unknown type) (Skylake D0) {Skylake}, 14nm
   miscellaneous (1/ebx):
      process local APIC physical ID = 0x0 (0)
      maximum IDs for CPUs in pkg    = 0x10 (16)
      CLFLUSH line size              = 0x8 (8)
      brand index                    = 0x0 (0)
... ...
{% endhighlight %}

## dmidecode

该命令会从 BIOS 的 DMI 接口数据收集关于系统硬件的具体信息，所以不会仅限于 CPU 信息。其中与 CPU 相关的信息包括了 CPU 类型、版本、核心总数、L1/L2/L3 缓存配置等等。

{% highlight text %}
... ...
Processor Information
        Socket Designation: U3E1
        Type: Central Processor
        Family: Core i5
        Manufacturer: Intel(R) Corporation
        ID: E3 06 04 00 FF FB EB BF
        Signature: Type 0, Family 6, Model 78, Stepping 3
        Flags:
                FPU (Floating-point unit on-chip)
                VME (Virtual mode extension)
                DE (Debugging extension)
                PSE (Page size extension)
                TSC (Time stamp counter)
                MSR (Model specific registers)
                PAE (Physical address extension)
                MCE (Machine check exception)
                CX8 (CMPXCHG8 instruction supported)
... ...
{% endhighlight %}

## lshw

与 `dmidecode` 命令类似，也是一个综合性硬件查询工具，同样是在 BIOS 里查询 DMI 信息，包括了总核心数和可用核心数，不过会遗漏一些信息，例如 L1/L2/L3 缓存配置。

{% highlight text %}
# lshw -class processor
  *-cpu
       description: CPU
       product: Intel(R) Core(TM) i5-6200U CPU @ 2.30GHz
       vendor: Intel Corp.
       physical id: 7
       bus info: cpu@0
       version: Intel(R) Core(TM) i5-6200U CPU @ 2.30GHz
       serial: None
       slot: U3E1
       size: 2752MHz
       capacity: 4005MHz
       width: 64 bits
       clock: 100MHz
       capabilities: lm fpu fpu_exception wp vme de pse tsc msr pae mce cx8 apic sep mtrr pge mca cmov pat pse36 clflush dts acpi mmx fxsr sse sse2 ss ht tm pbe syscall nx pdpe1gb rdtscp x86-64 constant_tsc art arch_perfmon pebs bts rep_good nopl xtopology nonstop_tsc cpuid aperfmperf tsc_known_freq pni pclmulqdq dtes64 monitor ds_cpl vmx est tm2 ssse3 sdbg fma cx16 xtpr pdcm pcid sse4_1 sse4_2 x2apic movbe popcnt tsc_deadline_timer xsave avx f16c rdrand lahf_lm abm 3dnowprefetch cpuid_fault epb invpcid_single pti ssbd ibrs ibpb stibp tpr_shadow vnmi flexpriority ept vpid fsgsbase tsc_adjust bmi1 avx2 smep bmi2 erms invpcid mpx rdseed adx smap clflushopt intel_pt xsaveopt xsavec xgetbv1 xsaves dtherm ida arat pln pts hwp hwp_notify hwp_act_window hwp_epp md_clear flush_l1d cpufreq
       configuration: cores=2 enabledcores=2 threads=4
{% endhighlight %}


## 其它

* HardInfo 一个 GUI 的图形界面，可以用来查看硬件信息。
* numactl 用来查看 NUMA 和 CPU 的布局策略，也就是 NUMA 的拓扑结构信息。
* x86info 展示基于 x86 架构的 CPU 信息。

{% highlight text %}
{% endhighlight %}
