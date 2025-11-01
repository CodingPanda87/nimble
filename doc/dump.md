# dump 

异常捕获说明

## linux

### "crash.log"文件

```bash
/home/xxx/work/nb/build/libnb_d.so(_Z12crashHandleri+0x1f)[0x7ffff77f7baf]
/lib/x86_64-linux-gnu/libc.so.6(+0x45330)[0x7ffff6512330]
/home/xxx/work/nb/build/test_plugin_QtUI.so(_ZZN7TestWndC1EP7QWidgetENKUlvE_clEv+0x14)[0x7ffff1908074]
/home/xxx/work/nb/build/test_plugin_QtUI.so(_ZN9QtPrivate11FunctorCallINS_11IndexesListIJEEENS_4ListIJEEEvZN7TestWndC1EP7QWidgetEUlvE_E4callERS8_PPv+0x19)[0x7ffff1908049]
/home/xxx/work/nb/build/test_plugin_QtUI.so(_ZN9QtPrivate7FunctorIZN7TestWndC1EP7QWidgetEUlvE_Li0EE4callINS_4ListIJEEEvEEvRS4_PvPSA_+0x21)[0x7ffff1908011]
/home/xxx/work/nb/build/test_plugin_QtUI.so(_ZN9QtPrivate18QFunctorSlotObjectIZN7TestWndC1EP7QWidgetEUlvE_Li0ENS_4ListIJEEEvE4implEiPNS_15QSlotObjectBaseEP7QObjectPPvPb+0x91)[0x7ffff1907f71]
/lib/x86_64-linux-gnu/libQt5Core.so.5(+0x312e16)[0x7ffff6d88e16]
/lib/x86_64-linux-gnu/libQt5Widgets.so.5(_ZN15QAbstractButton7clickedEb+0x44)[0x7ffff7b5e6e4]
/lib/x86_64-linux-gnu/libQt5Widgets.so.5(+0x26796f)[0x7ffff7b5e96f]
/lib/x86_64-linux-gnu/libQt5Widgets.so.5(+0x2695b0)[0x7ffff7b605b0]
/lib/x86_64-linux-gnu/libQt5Widgets.so.5(_ZN15QAbstractButton17mouseReleaseEventEP11QMouseEvent+0xf8)[0x7ffff7b607b8]
/lib/x86_64-linux-gnu/libQt5Widgets.so.5(_ZN7QWidget5eventEP6QEvent+0x278)[0x7ffff7aa7df8]
/lib/x86_64-linux-gnu/libQt5Widgets.so.5(_ZN19QApplicationPrivate13notify_helperEP7QObjectP6QEvent+0x85)[0x7ffff7a62d45]
/lib/x86_64-linux-gnu/libQt5Widgets.so.5(_ZN12QApplication6notifyEP7QObjectP6QEvent+0x1290)[0x7ffff7a6b6b0]
/lib/x86_64-linux-gnu/libQt5Core.so.5(_ZN16QCoreApplication15notifyInternal2EP7QObjectP6QEvent+0x128)[0x7ffff6d4e118]
/lib/x86_64-linux-gnu/libQt5Widgets.so.5(_ZN19QApplicationPrivate14sendMouseEventEP7QWidgetP11QMouseEventS1_S1_PS1_R8QPointerIS0_Ebb+0x1d4)[0x7ffff7a69874]
/lib/x86_64-linux-gnu/libQt5Widgets.so.5(+0x1caa39)[0x7ffff7ac1a39]
/lib/x86_64-linux-gnu/libQt5Widgets.so.5(+0x1cdfbf)[0x7ffff7ac4fbf]
/lib/x86_64-linux-gnu/libQt5Widgets.so.5(_ZN19QApplicationPrivate13notify_helperEP7QObjectP6QEvent+0x85)[0x7ffff7a62d45]
/lib/x86_64-linux-gnu/libQt5Core.so.5(_ZN16QCoreApplication15notifyInternal2EP7QObjectP6QEvent+0x128)[0x7ffff6d4e118]
/lib/x86_64-linux-gnu/libQt5Gui.so.5(_ZN22QGuiApplicationPrivate17processMouseEventEPN29QWindowSystemInterfacePrivate10MouseEventE+0x80b)[0x7ffff712ea3b]
/lib/x86_64-linux-gnu/libQt5Gui.so.5(_ZN22QWindowSystemInterface22sendWindowSystemEventsE6QFlagsIN10QEventLoop17ProcessEventsFlagEE+0xac)[0x7ffff7100bfc]
/lib/x86_64-linux-gnu/libQt5XcbQpa.so.5(+0x75d06)[0x7ffff3797d06]
/lib/x86_64-linux-gnu/libglib-2.0.so.0(+0x5d5c5)[0x7ffff5b2d5c5]
/lib/x86_64-linux-gnu/libglib-2.0.so.0(+0xbc737)[0x7ffff5b8c737]
/lib/x86_64-linux-gnu/libglib-2.0.so.0(g_main_context_iteration+0x33)[0x7ffff5b2ca63]
/lib/x86_64-linux-gnu/libQt5Core.so.5(_ZN20QEventDispatcherGlib13processEventsE6QFlagsIN10QEventLoop17ProcessEventsFlagEE+0x69)[0x7ffff6dab279]
/lib/x86_64-linux-gnu/libQt5Core.so.5(_ZN10QEventLoop4execE6QFlagsINS_17ProcessEventsFlagEE+0x13b)[0x7ffff6d4ca7b]
/lib/x86_64-linux-gnu/libQt5Core.so.5(_ZN16QCoreApplication4execEv+0x98)[0x7ffff6d553e8]
/home/xxx/work/nb/build/StarterQUI_d(+0x1869d)[0x55555556c69d]
/lib/x86_64-linux-gnu/libc.so.6(+0x2a1ca)[0x7ffff64f71ca]
/lib/x86_64-linux-gnu/libc.so.6(__libc_start_main+0x8b)[0x7ffff64f728b]
/home/xxx/work/nb/build/StarterQUI_d(+0x17595)[0x55555556b595]
```

### **addr2line** 命令查找源码行号

```bash
addr2line -e /home/xxx/work/nb/build/test_plugin_QtUI.so -f -C -i 0x7ffff1908074
```

### "resolve_dump.sh" 脚本

```bash
resolve_dump.sh crash.log /home/xxx/work/nb/build/test_plugin_QtUI.so
```