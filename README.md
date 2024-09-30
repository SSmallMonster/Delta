# Delta
Delta intercepts data and divert it to different storage directories.

## Usage

### Compile and Test

1. Compile libwrite_hook

```shell
$ gcc -g -shared -fPIC -pthread -ldl -w -o libwrite_hook.so lib/libwrite_v3.c -Ddebug
```

2. Compile Test Code

Compile with sharelibrary, this will use intercepted syscall open/write 
```shell
$ gcc large_write.c -o large_write -g -L. -lwrite_hook -Wl,-rpath=. -lpthread -Daccl
```

Or, you can directly compile
```shell
$ gcc -o large_write_no_accl large_write.c
```

3. Test and Compare 

```shell
$ ./large_write
...

$ ./large_write_no_accl
...
```

