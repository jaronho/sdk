# 功能

USB设备查看工具

# 用法

`./usb_watch [选项]`

# 选项

* `-b busNum`

总线编号，指定要查找哪个总线上的设备, 例如: 1

* `-p portNum`

端口编号, 指定要查找哪个端口上的设备, 例如: 2

# 示例

* 查看所有总线和端口的USB设备，不指定`-b`和`-p`

  `./usb_watch`

* 查看总线编号为1，端口编号为2的USB设备

  `./usb_watch -b 1 -p 2`

# 输出

输出所查找到的USB设备信息，以**JSON**格式输出。例如：

```bash
[
{"busNum":2,"portNum":4,"address":3,"classCode":9,"classDesc":"Hub","vid":"174c","pid":"3074","serial":"0000000A000C","devNodes":[]},
{"busNum":2,"portNum":3,"address":2,"classCode":9,"classDesc":"Hub","vid":"174c","pid":"3074","serial":"0000000A000C","devNodes":[]},
{"busNum":2,"portNum":0,"address":1,"classCode":9,"classDesc":"Hub","vid":"1d6b","pid":"0003","serial":"0000:06:00.0","devNodes":[]},
{"busNum":1,"portNum":4,"address":4,"classCode":9,"classDesc":"Hub","vid":"174c","pid":"2074","serial":"0000000A000C","devNodes":[]},
{"busNum":1,"portNum":3,"address":3,"classCode":9,"classDesc":"Hub","vid":"174c","pid":"2074","serial":"0000000A000C","devNodes":[]},
{"busNum":1,"portNum":2,"address":11,"classCode":3,"classDesc":"Human Interface Device","vid":"413c","pid":"301a","serial":"","devNodes":["/dev/hidraw2"]},
{"busNum":1,"portNum":0,"address":1,"classCode":9,"classDesc":"Hub","vid":"1d6b","pid":"0002","serial":"0000:06:00.0","devNodes":[]}
]
```

**Windows**下的`devNodes`节点都为空，例如：

```bash
[
{"busNum":1,"portNum":3,"address":20,"classCode":3,"classDesc":"Human Interface Device","vid":"2717","pid":"004b","serial":"","devNodes":[]},
{"busNum":1,"portNum":0,"address":0,"classCode":9,"classDesc":"Hub","vid":"8086","pid":"a0ed","serial":"","devNodes":[]},
{"busNum":1,"portNum":2,"address":53,"classCode":3,"classDesc":"Human Interface Device","vid":"1a86","pid":"9350","serial":"","devNodes":[]},
{"busNum":1,"portNum":6,"address":2,"classCode":239,"classDesc":"Miscellaneous","vid":"0c45","pid":"671e","serial":"","devNodes":[]},
{"busNum":1,"portNum":10,"address":4,"classCode":224,"classDesc":"Wireless","vid":"0cf3","pid":"e009","serial":"","devNodes":[]},
{"busNum":1,"portNum":1,"address":22,"classCode":9,"classDesc":"Hub","vid":"05e3","pid":"0608","serial":"","devNodes":[]}
]
```

