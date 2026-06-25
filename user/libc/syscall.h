#ifndef USER_SYSCALL_H
#define USER_SYSCALL_H

#define SYS_EXIT    0
#define SYS_READ    1
#define SYS_WRITE   2
#define SYS_OPEN    3
#define SYS_CLOSE   4
#define SYS_GETPID  5

static inline int syscall0(int num) {
    int ret;
    __asm__ volatile(
        "int $0x80"
        : "=a"(ret)
        : "a"(num)
    );
    return ret;
}

static inline int syscall1(int num, int arg1) {
    int ret;
    __asm__ volatile(
        "int $0x80"
        : "=a"(ret)
        : "a"(num), "b"(arg1)
    );
    return ret;
}

static inline int syscall2(int num, int arg1, int arg2) {
    int ret;
    __asm__ volatile(
        "int $0x80"
        : "=a"(ret)
        : "a"(num), "b"(arg1), "c"(arg2)
    );
    return ret;
}

static inline int syscall3(int num, int arg1, int arg2, int arg3) {
    int ret;
    __asm__ volatile(
        "int $0x80"
        : "=a"(ret)
        : "a"(num), "b"(arg1), "c"(arg2), "d"(arg3)
    );
    return ret;
}

static inline void exit(int code) {
    syscall1(SYS_EXIT, code);
}

static inline int write(int fd, const char* buf, int count) {
    return syscall3(SYS_WRITE, fd, (int)buf, count);
}

static inline int read(int fd, char* buf, int count) {
    return syscall3(SYS_READ, fd, (int)buf, count);
}

static inline int getpid(void) {
    return syscall0(SYS_GETPID);
}

#endif
