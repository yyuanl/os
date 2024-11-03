
#### ubuntu24 降级gcc版本到4.4
- 添加源:/etc/apt/sources.list.d/ubuntu.sources
```
Types: deb
URIs: http://dk.archive.ubuntu.com/ubuntu/
Suites: noble-security
Components: xenial main restricted universe multiverse
Signed-By: /usr/share/keyrings/ubuntu-archive-keyring.gpg

Types: deb
URIs: http://dk.archive.ubuntu.com/ubuntu/
Suites: noble-security
Components: xenial main restricted universe multiverse
Signed-By: /usr/share/keyrings/ubuntu-archive-keyring.gpg
```


- 升级源:sudo apt update
  - 报错：
  W: GPG 错误：http://dk.archive.ubuntu.com/ubuntu trusty-updates InRelease: 由于没有公钥，无法验证下列签名： NO_PUBKEY 40976EAF437D05B5 NO_PUBKEY 3B4FE6ACC0B21F32
  - 解决：
    - sudo apt-key adv --keyserver keyserver.ubuntu.com --recv-keys 40976EAF437D05B5
    - sudo apt-key adv --keyserver keyserver.ubuntu.com --recv-keys 3B4FE6ACC0B21F32
- 安装gcc4.4:sudo apt-get install gcc-4.4
- 设置优先级：sudo update-alternatives  --install /usr/bin/gcc gcc /usr/bin/gcc-4.4 1


gcc -c -o main.o /opt/os/version5/kernel/main.c && ld main.o -Ttext 0xc0001500 -e main -o /opt/bin/kernel.bin

- 可重定位文件：包含了elf头和节头表，为链接准备
- 可执行文件：包含elf头和程序头表，链接过程就把多个节合并成一个段
  - elf头记录了程序头表的信息
  - 程序头表是记录了程序段的信息列表
  - elf头+程序头表+main函数

- elf header:
  
    ![](../asset/elf_header.png)
  - 遍历程序头表：
    - e_phentsize:程序头表表项的大小，在elf header后42字节处
    - e_phoff，程序头表在文件中偏移量，在elf header后28字节处
    - e_phnum，程序头表表项的个数，在elf header后44字节处
  - 程序头表项：
    - p_type，类型，PT_NULL是跳过该段
    - p_filesz，指明本段在文件中的大小
    - p_offset，本段相对于文件起始的偏移量
    - p_vaddr，段要被拷贝到那个地址上，将来好让cpu执行
    - 