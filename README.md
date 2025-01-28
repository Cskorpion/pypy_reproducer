# pypy_reproducer
Code to reproduce the vmprof &amp; libunwind related segfault

The segfault occurs, when vmprof calls `unw_get_proc_name_by_ip` with an address of a function from `libffi.so`, which is not exposed in the symboltable.

You need a recent, self built version of [libunwind](https://github.com/libunwind/libunwind)
And a `libffi.so` from a [PyPy nightly build](https://buildbot.pypy.org/nightly/main/), as the segfault only occurs with the shipped `libffi.so`

Then you need to set the path to `libffi` for `dlopen` in `reproducer.c`

Finally compile and run `reproducer.c`
