#include <fcntl.h>
#include <stdint.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/mman.h>
typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t  s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

typedef float f32;
typedef double f64;

typedef u32 Hash;

#define fn static
#define global static
#define assert(x) if (__builtin_expect(!(x), 0)) { trap(); }
#define forceinline __attribute__((always_inline))
#define expect(x, b) __builtin_expect(x, b)
#define trap() __builtin_trap()
#define array_length(arr) sizeof(arr) / sizeof((arr)[0])
#define page_size (0x1000)
#define unused(x) (void)(x)
#define KB(n) ((n) * 1024)
#define MB(n) ((n) * 1024 * 1024)
#define GB(n) ((u64)(n) * 1024 * 1024 * 1024)
#define TB(n) ((u64)(n) * 1024 * 1024 * 1024 * 1024)

template<typename T>
struct DynamicList
{
    T* pointer;
    u64 count;
    DynamicList* next;
};

template<typename T, u64 count>
struct StaticList
{
    u64 length;
    StaticList* next;
    T array[count];
};

extern "C" void* memcpy(void* __restrict dst, void* __restrict src, u64 size)
{
    auto* destination = (u8*)dst;
    auto* source = (u8*)src;

    for (u64 i = 0; i < size; i += 1)
    {
        destination[i] = source[i];
    }

    return dst;
}

extern "C" void* memset(void* dst, u8 n, u64 size)
{
    auto* destination = (u8*)dst;
    for (u64 i = 0; i < size; i += 1)
    {
        destination[i] = n;
    }

    return dst;
}

fn int memcmp(const void* left, const void* right, u64 n)
{
	const u8 *l=(const u8*)left, *r=(const u8*)right;
	for (; n && *l == *r; n--, l++, r++);
	return n ? *l - *r : 0;
}

template<typename T>
forceinline fn u8 mem_equal_range(T* a, T* b, u64 count)
{
    return memcmp(a, b, count * sizeof(T)) == 0;
}

template <typename T>
forceinline fn T min(T a, T b)
{
    return a < b ? a : b;
}

template <typename T>
forceinline fn T max(T a, T b)
{
    return a > b ? a : b;
}

template<typename T>
struct Slice
{
    T* pointer;
    u64 length;

    T& operator[](u64 index)
    {
        assert(index < length);
        return pointer[index];
    }

    Slice slice(u64 start, u64 end)
    {
        return {
            .pointer = pointer + start,
            .length = end - start,
        };
    }

    forceinline u8 equal(Slice other)
    {
        if (length == other.length)
        {
            return mem_equal_range(pointer, other.pointer, length);
        }
        else
        {
            return 0;
        }
    }

    forceinline T* begin()
    {
        return pointer;
    }

    forceinline T* end()
    {
        return pointer + length;
    }

    forceinline void copy_in(Slice other)
    {
        assert(length == other.length);
        memcpy(pointer, other.pointer, sizeof(T) * other.length);
    }

    T* find(T item)
    {
        T* result = 0;

        for (T& i : *this)
        {
            if (i == item)
            {
                result = &i;
                break;
            }
        }

        return result;
    }

    u32 index(T* item)
    {
        return item - pointer;
    }

    s32 find_index(T item)
    {
        if (auto* result = find(item))
        {
            auto result_index = index(result);
            return result_index;
        }
        else
        {
            return -1;
        }
    }
};

using String = Slice<u8>;
#define strlit(s) String{ .pointer = (u8*)s, .length = sizeof(s) - 1, }
#define ch_to_str(ch) String{ .pointer = &ch, .length = 1 }

// global auto constexpr fnv_offset = 14695981039346656037ull;
// global auto constexpr fnv_prime = 1099511628211ull;

// fn Hash hash_bytes(String bytes)
// {
//     u64 result = fnv_offset;
//     for (u64 i = 0; i < bytes.length; i += 1)
//     {
//         result ^= bytes.pointer[i];
//         result *= fnv_prime;
//     }
//
//     return (Hash)result;
// }

// fn forceinline long syscall0(long n)
// {
// 	unsigned long ret;
// 	__asm__ __volatile__ ("syscall" : "=a"(ret) : "a"(n) : "rcx", "r11", "memory");
// 	return ret;
// }

fn forceinline long syscall1(long n, long a1)
{
	unsigned long ret;
	__asm__ __volatile__ ("syscall" : "=a"(ret) : "a"(n), "D"(a1) : "rcx", "r11", "memory");
	return ret;
}

fn forceinline long syscall2(long n, long a1, long a2)
{
	unsigned long ret;
	__asm__ __volatile__ ("syscall" : "=a"(ret) : "a"(n), "D"(a1), "S"(a2)
						  : "rcx", "r11", "memory");
	return ret;
}

fn forceinline long syscall3(long n, long a1, long a2, long a3)
{
	unsigned long ret;
	__asm__ __volatile__ ("syscall" : "=a"(ret) : "a"(n), "D"(a1), "S"(a2),
						  "d"(a3) : "rcx", "r11", "memory");
	return ret;
}

// fn forceinline long syscall4(long n, long a1, long a2, long a3, long a4)
// {
// 	unsigned long ret;
// 	register long r10 __asm__("r10") = a4;
// 	__asm__ __volatile__ ("syscall" : "=a"(ret) : "a"(n), "D"(a1), "S"(a2),
// 						  "d"(a3), "r"(r10): "rcx", "r11", "memory");
// 	return ret;
// }

// fn forceinline long syscall5(long n, long a1, long a2, long a3, long a4, long a5)
// {
// 	unsigned long ret;
// 	register long r10 __asm__("r10") = a4;
// 	register long r8 __asm__("r8") = a5;
// 	__asm__ __volatile__ ("syscall" : "=a"(ret) : "a"(n), "D"(a1), "S"(a2),
// 						  "d"(a3), "r"(r10), "r"(r8) : "rcx", "r11", "memory");
// 	return ret;
// }

fn forceinline long syscall6(long n, long a1, long a2, long a3, long a4, long a5, long a6)
{
	unsigned long ret;
	register long r10 __asm__("r10") = a4;
	register long r8 __asm__("r8") = a5;
	register long r9 __asm__("r9") = a6;
	__asm__ __volatile__ ("syscall" : "=a"(ret) : "a"(n), "D"(a1), "S"(a2),
						  "d"(a3), "r"(r10), "r"(r8), "r"(r9) : "rcx", "r11", "memory");
	return ret;
}

fn u8 memeq(u8* a, u8* b, u64 size)
{
    for (u64 i = 0; i < size; i += 1)
    {
        if (a[i] != b[i])
        {
            return 0;
        }
    }

    return 1;
}

enum class SyscallX86_64 : u64 {
    read = 0,
    write = 1,
    open = 2,
    close = 3,
    stat = 4,
    fstat = 5,
    lstat = 6,
    poll = 7,
    lseek = 8,
    mmap = 9,
    mprotect = 10,
    munmap = 11,
    brk = 12,
    rt_sigaction = 13,
    rt_sigprocmask = 14,
    rt_sigreturn = 15,
    ioctl = 16,
    pread64 = 17,
    pwrite64 = 18,
    readv = 19,
    writev = 20,
    access = 21,
    pipe = 22,
    select = 23,
    sched_yield = 24,
    mremap = 25,
    msync = 26,
    mincore = 27,
    madvise = 28,
    shmget = 29,
    shmat = 30,
    shmctl = 31,
    dup = 32,
    dup2 = 33,
    pause = 34,
    nanosleep = 35,
    getitimer = 36,
    alarm = 37,
    setitimer = 38,
    getpid = 39,
    sendfile = 40,
    socket = 41,
    connect = 42,
    accept = 43,
    sendto = 44,
    recvfrom = 45,
    sendmsg = 46,
    recvmsg = 47,
    shutdown = 48,
    bind = 49,
    listen = 50,
    getsockname = 51,
    getpeername = 52,
    socketpair = 53,
    setsockopt = 54,
    getsockopt = 55,
    clone = 56,
    fork = 57,
    vfork = 58,
    execve = 59,
    exit = 60,
    wait4 = 61,
    kill = 62,
    uname = 63,
    semget = 64,
    semop = 65,
    semctl = 66,
    shmdt = 67,
    msgget = 68,
    msgsnd = 69,
    msgrcv = 70,
    msgctl = 71,
    fcntl = 72,
    flock = 73,
    fsync = 74,
    fdatasync = 75,
    truncate = 76,
    ftruncate = 77,
    getdents = 78,
    getcwd = 79,
    chdir = 80,
    fchdir = 81,
    rename = 82,
    mkdir = 83,
    rmdir = 84,
    creat = 85,
    link = 86,
    unlink = 87,
    symlink = 88,
    readlink = 89,
    chmod = 90,
    fchmod = 91,
    chown = 92,
    fchown = 93,
    lchown = 94,
    umask = 95,
    gettimeofday = 96,
    getrlimit = 97,
    getrusage = 98,
    sysinfo = 99,
    times = 100,
    ptrace = 101,
    getuid = 102,
    syslog = 103,
    getgid = 104,
    setuid = 105,
    setgid = 106,
    geteuid = 107,
    getegid = 108,
    setpgid = 109,
    getppid = 110,
    getpgrp = 111,
    setsid = 112,
    setreuid = 113,
    setregid = 114,
    getgroups = 115,
    setgroups = 116,
    setresuid = 117,
    getresuid = 118,
    setresgid = 119,
    getresgid = 120,
    getpgid = 121,
    setfsuid = 122,
    setfsgid = 123,
    getsid = 124,
    capget = 125,
    capset = 126,
    rt_sigpending = 127,
    rt_sigtimedwait = 128,
    rt_sigqueueinfo = 129,
    rt_sigsuspend = 130,
    sigaltstack = 131,
    utime = 132,
    mknod = 133,
    uselib = 134,
    personality = 135,
    ustat = 136,
    statfs = 137,
    fstatfs = 138,
    sysfs = 139,
    getpriority = 140,
    setpriority = 141,
    sched_setparam = 142,
    sched_getparam = 143,
    sched_setscheduler = 144,
    sched_getscheduler = 145,
    sched_get_priority_max = 146,
    sched_get_priority_min = 147,
    sched_rr_get_interval = 148,
    mlock = 149,
    munlock = 150,
    mlockall = 151,
    munlockall = 152,
    vhangup = 153,
    modify_ldt = 154,
    pivot_root = 155,
    _sysctl = 156,
    prctl = 157,
    arch_prctl = 158,
    adjtimex = 159,
    setrlimit = 160,
    chroot = 161,
    sync = 162,
    acct = 163,
    settimeofday = 164,
    mount = 165,
    umount2 = 166,
    swapon = 167,
    swapoff = 168,
    reboot = 169,
    sethostname = 170,
    setdomainname = 171,
    iopl = 172,
    ioperm = 173,
    create_module = 174,
    init_module = 175,
    delete_module = 176,
    get_kernel_syms = 177,
    query_module = 178,
    quotactl = 179,
    nfsservctl = 180,
    getpmsg = 181,
    putpmsg = 182,
    afs_syscall = 183,
    tuxcall = 184,
    security = 185,
    gettid = 186,
    readahead = 187,
    setxattr = 188,
    lsetxattr = 189,
    fsetxattr = 190,
    getxattr = 191,
    lgetxattr = 192,
    fgetxattr = 193,
    listxattr = 194,
    llistxattr = 195,
    flistxattr = 196,
    removexattr = 197,
    lremovexattr = 198,
    fremovexattr = 199,
    tkill = 200,
    time = 201,
    futex = 202,
    sched_setaffinity = 203,
    sched_getaffinity = 204,
    set_thread_area = 205,
    io_setup = 206,
    io_destroy = 207,
    io_getevents = 208,
    io_submit = 209,
    io_cancel = 210,
    get_thread_area = 211,
    lookup_dcookie = 212,
    epoll_create = 213,
    epoll_ctl_old = 214,
    epoll_wait_old = 215,
    remap_file_pages = 216,
    getdents64 = 217,
    set_tid_address = 218,
    restart_syscall = 219,
    semtimedop = 220,
    fadvise64 = 221,
    timer_create = 222,
    timer_settime = 223,
    timer_gettime = 224,
    timer_getoverrun = 225,
    timer_delete = 226,
    clock_settime = 227,
    clock_gettime = 228,
    clock_getres = 229,
    clock_nanosleep = 230,
    exit_group = 231,
    epoll_wait = 232,
    epoll_ctl = 233,
    tgkill = 234,
    utimes = 235,
    vserver = 236,
    mbind = 237,
    set_mempolicy = 238,
    get_mempolicy = 239,
    mq_open = 240,
    mq_unlink = 241,
    mq_timedsend = 242,
    mq_timedreceive = 243,
    mq_notify = 244,
    mq_getsetattr = 245,
    kexec_load = 246,
    waitid = 247,
    add_key = 248,
    request_key = 249,
    keyctl = 250,
    ioprio_set = 251,
    ioprio_get = 252,
    inotify_init = 253,
    inotify_add_watch = 254,
    inotify_rm_watch = 255,
    migrate_pages = 256,
    openat = 257,
    mkdirat = 258,
    mknodat = 259,
    fchownat = 260,
    futimesat = 261,
    fstatat64 = 262,
    unlinkat = 263,
    renameat = 264,
    linkat = 265,
    symlinkat = 266,
    readlinkat = 267,
    fchmodat = 268,
    faccessat = 269,
    pselect6 = 270,
    ppoll = 271,
    unshare = 272,
    set_robust_list = 273,
    get_robust_list = 274,
    splice = 275,
    tee = 276,
    sync_file_range = 277,
    vmsplice = 278,
    move_pages = 279,
    utimensat = 280,
    epoll_pwait = 281,
    signalfd = 282,
    timerfd_create = 283,
    eventfd = 284,
    fallocate = 285,
    timerfd_settime = 286,
    timerfd_gettime = 287,
    accept4 = 288,
    signalfd4 = 289,
    eventfd2 = 290,
    epoll_create1 = 291,
    dup3 = 292,
    pipe2 = 293,
    inotify_init1 = 294,
    preadv = 295,
    pwritev = 296,
    rt_tgsigqueueinfo = 297,
    perf_event_open = 298,
    recvmmsg = 299,
    fanotify_init = 300,
    fanotify_mark = 301,
    prlimit64 = 302,
    name_to_handle_at = 303,
    open_by_handle_at = 304,
    clock_adjtime = 305,
    syncfs = 306,
    sendmmsg = 307,
    setns = 308,
    getcpu = 309,
    process_vm_readv = 310,
    process_vm_writev = 311,
    kcmp = 312,
    finit_module = 313,
    sched_setattr = 314,
    sched_getattr = 315,
    renameat2 = 316,
    seccomp = 317,
    getrandom = 318,
    memfd_create = 319,
    kexec_file_load = 320,
    bpf = 321,
    execveat = 322,
    userfaultfd = 323,
    membarrier = 324,
    mlock2 = 325,
    copy_file_range = 326,
    preadv2 = 327,
    pwritev2 = 328,
    pkey_mprotect = 329,
    pkey_alloc = 330,
    pkey_free = 331,
    statx = 332,
    io_pgetevents = 333,
    rseq = 334,
    pidfd_send_signal = 424,
    io_uring_setup = 425,
    io_uring_enter = 426,
    io_uring_register = 427,
    open_tree = 428,
    move_mount = 429,
    fsopen = 430,
    fsconfig = 431,
    fsmount = 432,
    fspick = 433,
    pidfd_open = 434,
    clone3 = 435,
    close_range = 436,
    openat2 = 437,
    pidfd_getfd = 438,
    faccessat2 = 439,
    process_madvise = 440,
    epoll_pwait2 = 441,
    mount_setattr = 442,
    quotactl_fd = 443,
    landlock_create_ruleset = 444,
    landlock_add_rule = 445,
    landlock_restrict_self = 446,
    memfd_secret = 447,
    process_mrelease = 448,
    futex_waitv = 449,
    set_mempolicy_home_node = 450,
    cachestat = 451,
    fchmodat2 = 452,
    map_shadow_stack = 453,
    futex_wake = 454,
    futex_wait = 455,
    futex_requeue = 456,
};

fn void* syscall_mmap(void* address, size_t length, int protection_flags, int map_flags, int fd, __off_t offset)
{
    return (void*) syscall6(static_cast<long>(SyscallX86_64::mmap), (unsigned long)address, length, protection_flags, map_flags, fd, offset);
}

fn int syscall_mprotect(void *address, size_t length, int protection_flags)
{
    return syscall3(static_cast<long>(SyscallX86_64::mprotect), (unsigned long)address, length, protection_flags);
}

fn int syscall_open(const char *file_path, int flags, int mode)
{
    return syscall3(static_cast<long>(SyscallX86_64::open), (unsigned long)file_path, flags, mode);
}

fn int syscall_fstat(int fd, struct stat *buffer)
{
    return syscall2(static_cast<long>(SyscallX86_64::fstat), fd, (unsigned long)buffer);
}

fn ssize_t syscall_read(int fd, void* buffer, size_t bytes)
{
    return syscall3(static_cast<long>(SyscallX86_64::read), fd, (unsigned long)buffer, bytes);
}

fn ssize_t syscall_write(int fd, const void *buffer, size_t bytes)
{
    return syscall3(static_cast<long>(SyscallX86_64::write), fd, (unsigned long)buffer, bytes);
}

[[noreturn]] [[gnu::cold]] fn void syscall_exit(int status)
{
    (void)syscall1(231, status);
    trap();
}

[[noreturn]] [[gnu::cold]] fn void fail()
{
    trap();
    syscall_exit(1);
}

fn void* reserve(u64 size)
{
    int protection_flags = PROT_NONE;
    int map_flags = MAP_ANONYMOUS | MAP_PRIVATE | MAP_NORESERVE;
    void* result = syscall_mmap(0, size, protection_flags, map_flags, -1, 0);
    assert(result != MAP_FAILED);
    return result;
}

fn void commit(void* address, u64 size)
{
    int result = syscall_mprotect(address, size, PROT_READ | PROT_WRITE);
    assert(result == 0);
}

fn u64 align_forward(u64 value, u64 alignment)
{
    u64 mask = alignment - 1;
    u64 result = (value + mask) & ~mask;
    return result;
}

struct Arena
{
    u64 reserved_size;
    u64 commited;
    u64 commit_position;
    u64 granularity;
    u8 reserved[4 * 8];

    global auto constexpr minimum_granularity = KB(4);
    global auto constexpr middle_granularity = MB(2);
    global auto constexpr page_granularity = page_size;
    global auto constexpr default_size = GB(4);

    fn Arena* init(u64 reserved_size, u64 granularity, u64 initial_size)
    {
        assert(initial_size % granularity == 0);
        Arena* arena = (Arena*)reserve(reserved_size);
        commit(arena, initial_size);
        *arena = {
            .reserved_size = reserved_size,
            .commited = initial_size,
            .commit_position = sizeof(Arena),
            .granularity = granularity,
        };
        return arena;
    }

    fn Arena* init_default(u64 initial_size)
    {
        return init(default_size, minimum_granularity, initial_size);
    }

    void* allocate_bytes(u64 size, u64 alignment)
    {
        u64 aligned_offset = align_forward(commit_position, alignment);
        u64 aligned_size_after = aligned_offset + size;
        if (aligned_size_after <= commited)
        {
            void* result = (u8*)this + aligned_offset;
            commit_position = aligned_size_after;
            return result;
        }
        else
        {
            trap();
        }
    }

    template<typename T>
    T* allocate_many(u64 count)
    {
        return (T*)allocate_bytes(sizeof(T) * count, alignof(T));
    }

    template<typename T>
    T* allocate_one()
    {
        return allocate_many<T>(1);
    }

    template<typename T>
    T* allocate_slice(u64 count)
    {
        return {
            .pointer = allocate_many<T>(count),
            .length = count,
        };
    }
};
static_assert(sizeof(Arena) == 64, "Arena must be cache aligned");

template<typename Destination, typename Source>
fn forceinline Destination transmute(Source source)
{
    static_assert(sizeof(Source) == sizeof(Destination));
    return *(Destination*)&source;
}

fn String file_read(Arena* arena, String path)
{
    String result = {};
    int file_descriptor = syscall_open((char*)path.pointer, 0, 0);
    assert(file_descriptor != -1);

    struct stat stat_buffer;
    int stat_result = syscall_fstat(file_descriptor, &stat_buffer);
    assert(stat_result == 0);

    u64 file_size = stat_buffer.st_size;

    result = {
        .pointer = (u8*)arena->allocate_bytes(file_size, 64),
        .length = file_size,
    };

    // TODO: big files
    ssize_t read_result = syscall_read(file_descriptor, result.pointer, result.length);
    assert(read_result >= 0);
    assert((u64)read_result == file_size);

    return result;
}

fn void print(String message)
{
    ssize_t result = syscall_write(1, message.pointer, message.length);
    assert(result >= 0);
    assert((u64)result == message.length);
}

template<typename T> struct PinnedArray;
fn void generic_pinned_array_ensure_capacity(PinnedArray<u8>* array, u32 additional_T, u32 size_of_T);
fn u8* generic_pinned_array_add_with_capacity(PinnedArray<u8>* array, u32 additional_T, u32 size_of_T);


template <typename T>
struct PinnedArray
{
    T* pointer;
    u32 length;
    u32 capacity;

    global constexpr auto granularity = page_size;
    global constexpr auto reserved_size = ((u64)GB(4) - granularity);

    // static_assert(sizeof(T) % granularity == 0);

    forceinline T& operator[](u32 index)
    {
        assert(index < length);
        return pointer[index];
    }

    forceinline void ensure_capacity(u32 additional)
    {
        auto generic_array = (PinnedArray<u8>*)(this);
        generic_pinned_array_ensure_capacity(generic_array, additional, sizeof(T));
    }

    forceinline void clear()
    {
        length = 0;
    }

    forceinline Slice<T> add_with_capacity(u32 additional)
    {
        auto generic_array = (PinnedArray<u8>*)(this);
        auto pointer = generic_pinned_array_add_with_capacity(generic_array, additional, sizeof(T));
        return {
            .pointer = (T*)pointer,
            .length = additional,
        };
    }

    forceinline Slice<T> add(u32 additional)
    {
        ensure_capacity(additional);
        auto slice = add_with_capacity(additional);
        return slice;
    }

    forceinline Slice<T> append(Slice<T> items)
    {
        assert(items.length <= 0xffffffff);
        auto slice = add(items.length);
        slice.copy_in(items);
        return slice;
    }

    forceinline T* add_one()
    {
        return add(1).pointer;
    }

    forceinline T* append_one(T item)
    {
        T* new_item = add_one();
        *new_item = item;
        return new_item;
    }

    forceinline T pop()
    {
        assert(length);
        length -= 1;
        return pointer[length];
    }

    forceinline Slice<T> slice()
    {
        return {
            .pointer = pointer,
            .length = length,
        };
    }

    T remove_swap(u32 index)
    {
        if (index >= 0 & index < length)
        {
            T item = pointer[index];
            T last = pointer[length - 1];
            pointer[index] = last;
            pop();
            return item;
        }

        trap();
    }
};

forceinline fn u32 generic_pinned_array_length(PinnedArray<u8>* array, u32 size_of_T)
{
    u32 current_length_bytes = array->length * size_of_T;
    return current_length_bytes;
}

fn void generic_pinned_array_ensure_capacity(PinnedArray<u8>* array, u32 additional_T, u32 size_of_T)
{
    u32 wanted_capacity = array->length + additional_T;
    if (array->capacity < array->length + additional_T)
    {
        if (array->capacity == 0)
        {
            assert(array->length == 0);
            assert(array->pointer == 0);
            array->pointer = static_cast<u8*>(reserve(PinnedArray<u8>::reserved_size));
        }

        u64 currently_committed_size = align_forward(array->capacity * size_of_T, array->granularity);
        u64 wanted_committed_size = align_forward(wanted_capacity * size_of_T, array->granularity);
        void* commit_pointer = array->pointer + currently_committed_size;
        u64 commit_size = wanted_committed_size - currently_committed_size;
        assert(commit_size > 0);
        commit(commit_pointer, commit_size);
        array->capacity = wanted_committed_size / size_of_T;
    }
}

fn u8* generic_pinned_array_add_with_capacity(PinnedArray<u8>* array, u32 additional_T, u32 size_of_T)
{
    u32 current_length_bytes = generic_pinned_array_length(array, size_of_T);
    assert(current_length_bytes < PinnedArray<u8>::reserved_size);
    u8* pointer = array->pointer + current_length_bytes;
    array->length += additional_T;
    return pointer;
}

template <typename K, typename V> struct PinnedHashmap;

template <typename K, typename V>
struct GetOrPut
{
    K* key;
    V* value;
    u8 existing;
};
fn GetOrPut<u8, u8> generic_pinned_hashmap_get_or_put(PinnedHashmap<u8, u8>* hashmap, u8* new_key_pointer, u32 key_size, u8* new_value_pointer, u32 value_size);

template<typename K, typename V>
struct PinnedHashmap
{
    K* keys;
    V* values;
    u32 length;
    u16 key_page_capacity;
    u16 value_page_capacity;

    global constexpr auto invalid_index = ~0u;
    global constexpr auto granularity = PinnedArray<V>::granularity;
    global constexpr auto reserved_size = PinnedArray<V>::reserved_size;

    static_assert(granularity % sizeof(K) == 0, "");
    static_assert(granularity % sizeof(V) == 0, "");

    forceinline GetOrPut<K, V> get_or_put(K key, V value)
    {
        auto* generic_hashmap = (PinnedHashmap<u8, u8>*)(this);
        auto generic_get_or_put = generic_pinned_hashmap_get_or_put(generic_hashmap, (u8*)&key, sizeof(K), (u8*)&value, sizeof(V));
        return *(GetOrPut<K, V>*)&generic_get_or_put;
    }
};

template<typename K, typename V>
using Hashmap = PinnedHashmap<K, V>;
template <typename T>
using Array = PinnedArray<T>;

// Returns the generic value pointer if the key is present
fn u32 generic_pinned_hashmap_get_index(PinnedHashmap<u8, u8>* hashmap, u8* key_pointer, u32 key_size)
{
    u32 index = hashmap->invalid_index;

    for (u32 i = 0; i < hashmap->length; i += 1)
    {
        u8* it_key_pointer = &hashmap->keys[i * key_size];
        if (memeq(it_key_pointer, key_pointer, key_size))
        {
            index = (it_key_pointer - hashmap->keys) / key_size;
            break;
        }
    }

    return index;
}

fn void generic_pinned_hashmap_ensure_capacity(PinnedHashmap<u8, u8>* hashmap, u32 key_size, u32 value_size, u32 additional_elements)
{
    if (additional_elements != 0)
    {
        if (hashmap->key_page_capacity == 0)
        {
            assert(hashmap->value_page_capacity == 0);
            hashmap->keys = (u8*)reserve(hashmap->reserved_size);
            hashmap->values = (u8*)reserve(hashmap->reserved_size);
        }

        u32 target_element_capacity = hashmap->length + additional_elements;

        {
            u32 key_byte_capacity = hashmap->key_page_capacity * hashmap->granularity;
            u32 target_byte_capacity = target_element_capacity * key_size;
            if (key_byte_capacity < target_byte_capacity)
            {
                u32 aligned_target_byte_capacity = align_forward(target_byte_capacity,  hashmap->granularity);
                void* commit_pointer = hashmap->keys + key_byte_capacity;
                u32 commit_size = aligned_target_byte_capacity - key_byte_capacity;
                commit(commit_pointer, commit_size);
                hashmap->key_page_capacity = aligned_target_byte_capacity / hashmap->granularity;
            }
        }

        {
            u32 value_byte_capacity = hashmap->value_page_capacity * hashmap->granularity;
            u32 target_byte_capacity = target_element_capacity * value_size;
            if (value_byte_capacity < target_byte_capacity)
            {
                u32 aligned_target_byte_capacity = align_forward(target_byte_capacity, hashmap->granularity);
                void* commit_pointer = hashmap->values + value_byte_capacity;
                u32 commit_size = aligned_target_byte_capacity - value_byte_capacity;
                commit(commit_pointer, commit_size);
                hashmap->value_page_capacity = aligned_target_byte_capacity / hashmap->granularity;
            }
        }
    }
}

fn GetOrPut<u8, u8> generic_pinned_hashmap_get_or_put(PinnedHashmap<u8, u8>* hashmap, u8* new_key_pointer, u32 key_size, u8* new_value_pointer, u32 value_size)
{
    u32 index = generic_pinned_hashmap_get_index(hashmap, new_key_pointer, key_size);
    if (index != hashmap->invalid_index)
    {
        trap();
    }
    else
    {
        generic_pinned_hashmap_ensure_capacity(hashmap, key_size, value_size, 1);
        u32 new_index = hashmap->length;
        hashmap->length += 1;
        u8* key_pointer = &hashmap->keys[new_index * key_size];
        u8* value_pointer = &hashmap->values[new_index * value_size];
        memcpy(key_pointer, new_key_pointer, key_size);
        memcpy(value_pointer, new_value_pointer, value_size);

        return {
            .key = key_pointer,
            .value = value_pointer,
            .existing = 0,
        };
    }
}

typedef enum FileStatus
{
    FILE_STATUS_ADDED = 0,
    FILE_STATUS_QUEUED = 1,
    FILE_STATUS_READ = 2,
    FILE_STATUS_ANALYZING = 3,
} FileStatus;

struct File
{
    String path;
    String source_code;
    FileStatus status;
};

struct SemaType;
union Type
{
    enum Id
    {
        sema,
        backend,
    };

    u64 bits:57;
    Id id:1;

    forceinline SemaType* get_sema()
    {
        assert(id == Id::sema);
        return (SemaType*)(bits);
    }

    forceinline u8 is_resolved();
};
static_assert(sizeof(Type) == 8);

enum class SemaTypeId: u8
{
    VOID,
    NORETURN,
    POINTER,
    INTEGER,
    ARRAY,
    STRUCT,
    UNION,
    COUNT,
};
global auto constexpr type_id_bit_count = 3;
static_assert(static_cast<u8>(SemaTypeId::COUNT) < (1 << type_id_bit_count), "Type bit count for id must be respected");

global auto constexpr type_flags_bit_count = 32 - (type_id_bit_count + 1);

struct SemaType
{
    u64 size;
    u64 alignment;
    SemaTypeId id : type_id_bit_count;
    u32 resolved: 1;
    u32 flags: type_flags_bit_count;
    u32 reserved;
    String name;

    u8 get_bit_count()
    {
        assert(id == SemaTypeId::INTEGER);
        u32 bit_count_mask = (1 << (type_flags_bit_count - 1)) - 1;
        u8 bit_count = flags & bit_count_mask;
        assert(bit_count <= size * 8);
        assert(bit_count <= 64);
        return bit_count;
    }

};
static_assert(sizeof(SemaType) == sizeof(u64) * 5, "Type must be 24 bytes");
forceinline u8 Type::is_resolved()
{
    return (id == Id::backend) | ((id == Id::sema) & get_sema()->resolved);
}


struct Symbol
{
    enum class Id: u8
    {
        variable,
        function,
    };

    enum class Linkage: u8
    {
        internal,
        external,
    };

    String name;
    Id id: 1;
    Linkage linkage: 1;
};

typedef enum AbiInfoKind : u8
{
    ABI_INFO_IGNORE,
    ABI_INFO_DIRECT,
    ABI_INFO_DIRECT_PAIR,
    ABI_INFO_DIRECT_COERCE,
    ABI_INFO_DIRECT_COERCE_INT,
    ABI_INFO_DIRECT_SPLIT_STRUCT_I32,
    ABI_INFO_EXPAND_COERCE,
    ABI_INFO_INDIRECT,
    ABI_INFO_EXPAND,
} AbiInfoKind;

enum class Side : u8
{
    left,
    right,
};


global auto constexpr void_type_index = 0;
global auto constexpr noreturn_type_index = 1;
global auto constexpr opaque_pointer_type_index = 2;
// global auto constexpr f32_type_offset = 3;
// global auto constexpr f64_type_offset = 4;
global auto constexpr integer_type_offset = 5;
global auto constexpr integer_type_count = 64 * 2;
global auto constexpr builtin_type_count = integer_type_count + integer_type_offset + 1;

struct Function;
struct Thread
{
    Arena* arena;
    Slice<Function> functions;
};

struct Unit
{
    // PinnedArray<File> files;
    // PinnedArray<Function> functions;
    // Arena* arena;
    // Arena* node_arena;
    // Arena* type_arena;
    // PinnedHashmap<Hash, String> identifiers;
    SemaType* builtin_types;
    u64 generate_debug_information : 1;

    SemaType* get_integer_type(u8 bit_count, u8 signedness)
    {
        auto index = integer_type_offset + signedness * 64 + bit_count - 1;
        return &builtin_types[index];
    }
};


struct NodeType
{
    enum class Id: u8
    {
        INVALID,
        BOTTOM,
        TOP,
        VOID,
        INTEGER,
        TUPLE,
        CONTROL,
        MEMORY,
        POINTER,
    };
    Id id;
    union
    {
        struct
        {
            u64 constant;
            u8 bit_count;
            u8 is_constant;
        } integer;
    };
    u8 equal(NodeType other)
    {
        if (id != other.id)
        {
            return 0;
        }

        switch (id)
        {
            default: trap();
        }
    }

    u8 is_constant()
    {
        switch (id)
        {
            case Id::VOID:
            case Id::INTEGER:
                return 1;
            default:
                trap();
        }
    }
};

union AbiInfoPayload
{
    NodeType direct;
    NodeType direct_pair[2];
    NodeType direct_coerce;
    struct
    {
        NodeType type;
        u32 alignment;
    } indirect;
};
typedef union AbiInfoPayload AbiInfoPayload;

struct AbiInfoAttributes
{
    u8 by_reg: 1;
    u8 zero_extend: 1;
    u8 sign_extend: 1;
    u8 realign: 1;
    u8 by_value: 1;
};
typedef struct AbiInfoAttributes AbiInfoAttributes;

struct AbiInfo
{
    AbiInfoPayload payload;
    u16 indices[2];
    AbiInfoAttributes attributes;
    AbiInfoKind kind;
};

struct Node;
struct Function
{
    struct Prototype
    {
        AbiInfo* argument_type_abis; // The count for this array is "original_argument_count", not "abi_argument_count"
        SemaType** original_argument_types;
        // TODO: are these needed?
        // Node::DataType* abi_argument_types;
        // u32 abi_argument_count;
        SemaType* original_return_type;
        AbiInfo return_type_abi;
        u32 original_argument_count;
        // TODO: is this needed?
        // Node::DataType abi_return_type;
        u8 varags:1;
    };

    Symbol symbol;
    Node* root_node;
    Node** parameters;
    Function::Prototype prototype;
    u32 node_count;
    u16 parameter_count;
};

struct ConstantIntData
{
    u64 value;
    Node* input;
    u32 gvn;
    u8 bit_count;
};

[[nodiscard]] fn Node* add_constant_integer(Arena* arena, ConstantIntData data);

// This is a node in the "sea of nodes" sense:
// https://en.wikipedia.org/wiki/Sea_of_nodes
struct Node
{
    enum class Id: u8
    {
        ROOT,
        PROJECTION,
        RETURN,
        CONSTANT_INT,
        INT_ADD,
        INT_SUB,
        SCOPE,
    };

    using Type = NodeType;

    PinnedArray<Node*> inputs;
    PinnedArray<Node*> outputs;
    u32 gvn;
    Type type;
    Id id;

    union
    {
        struct
        {
            u32 index;
        } projection;
        struct
        {
            Array<Hashmap<String, u16>> stack;
        } scope;
    };

    forceinline Slice<Node*> get_inputs()
    {
        return {
            .pointer = inputs.pointer,
            .length = inputs.length,
        };
    }

    forceinline Slice<Node*> get_outputs()
    {
        return {
            .pointer = outputs.pointer,
            .length = outputs.length,
        };
    }

    struct NodeData
    {
        Type type;
        Slice<Node*> inputs;
        Id id;
    };

    struct DynamicNodeData
    {
        NodeData s;
        u32 gvn;
    };

    [[nodiscard]] fn Node* add(Arena* arena, DynamicNodeData data)
    {
        auto* node = arena->allocate_one<Node>();
        *node = {
            .inputs = {},
            .outputs = {},
            .gvn = data.gvn,
            .type = data.s.type,
            .id = data.s.id,
        };

        node->inputs.append(data.s.inputs);

        for (Node* input : data.s.inputs)
        {
            if (input)
            {
                input->add_output(node);
            }
        }

        return node;
    }

    [[nodiscard]] fn Node* add_from_function(Arena* arena, Function* function, NodeData data)
    {
        auto gvn = function->node_count;
        function->node_count += 1;

        auto* node = add(arena, {
            .s = data,
            .gvn = gvn,
        });
        return node;
    }

    Node* add_output(Node* output)
    {
        outputs.append_one(output);
        return this;
    }

    // Node* add_input(Node* input)
    // {
    //     inputs.append_one(input);
    //     if (input)
    //     {
    //         input->add_output(this);
    //     }
    //     return input;
    // }

    u8 remove_output(Node* output)
    {
        s32 index = outputs.slice().find_index(output);
        assert(index != -1);
        outputs.remove_swap(index);
        return outputs.length == 0;
    }

    u8 is_pinned()
    {
        u8 is_good_id = 0;
        switch (id)
        {
        case Id::ROOT:
        case Id::RETURN:
            is_good_id = 1;
            break;
        case Id::PROJECTION:
        case Id::CONSTANT_INT:
            break;
        case Id::INT_ADD:
        case Id::INT_SUB:
            trap();
        case Id::SCOPE:
            trap();
          break;
        }

        return is_good_id | is_projection() | cfg_is_control_projection();
    }

    u8 is_projection()
    {
        switch (id)
        {
            case Id::PROJECTION:
                return 1;
            default:
                return 0;
        }
    }

    u8 cfg_is_control_projection()
    {
        return is_projection() & (type.id == Node::Type::Id::CONTROL);
    }

    u8 is_cfg_control()
    {
        switch (type.id)
        {
            case Node::Type::Id::CONTROL:
            return 1;
        case Node::Type::Id::TUPLE:
            for (Node* output : get_outputs())
            {
                if (output->cfg_is_control_projection())
                {
                    return 1;
                }
            }
        default:
            return 0;
        }
    }

    Node* idealize()
    {
        switch (id)
        {
        case Id::ROOT:
        case Id::PROJECTION:
        case Id::RETURN:
        case Id::CONSTANT_INT:
        case Id::INT_ADD:
        case Id::INT_SUB:
            return 0;
        case Id::SCOPE:
            trap();
        }
    }

    u8 is_unused()
    {
        return outputs.length == 0;
    }

    u8 is_dead()
    {
        return is_unused() & (inputs.length == 0) & (type.id == Node::Type::Id::INVALID);
    }

    void pop_inputs(Arena* arena, u32 count)
    {
        for (u32 i = 0; i < count; i += 1)
        {
            Node* old_input = inputs.pop();
            if (old_input)
            {
                if (old_input->remove_output(this))
                {
                    old_input->kill(arena);
                }
            }
        }
    }

    void kill(Arena* arena)
    {
        assert(is_unused());

        pop_inputs(arena, get_inputs().length);
        type = {};

        assert(is_dead());
    }

    static auto constexpr enable_peephole = 1;
    Node* peephole(Arena* arena, Function* function)
    {
        Node::Type type = this->type = compute();

        if (!enable_peephole)
        {
            return this;
        }

        if ((!is_constant()) & type.is_constant())
        {
            this->kill(arena);
            auto gvn = function->node_count;
            function->node_count += 1;

            auto* constant_int = Node::add(arena, {
                .s =
                {
                    .type = type,
                    .inputs = { .pointer = &function->root_node, .length = 1 },
                    .id = Node::Id::CONSTANT_INT,
                },
                .gvn = gvn,
            });
            auto* result = constant_int->peephole(arena, function);
            return result;
        }

        Node* n = idealize();
        return n ? n : this;
    }

    u8 is_constant()
    {
        switch (id)
        {
            default:
                return 0;
            case Id::CONSTANT_INT:
                return 1;
        }
    }

    Node::Type compute()
    {
        switch (id)
        {
            case Node::Id::INT_ADD:
            case Node::Id::INT_SUB:
                {
                    auto left_type = inputs[1]->type;
                    auto right_type = inputs[2]->type;
                    if ((left_type.id == Node::Type::Id::INTEGER) & (right_type.id == Node::Type::Id::INTEGER))
                    {
                        if (left_type.is_constant() & right_type.is_constant())
                        {
                            u64 result;
                            switch (id)
                            {
                            case Id::ROOT:
                            case Id::PROJECTION:
                            case Id::RETURN:
                            case Id::CONSTANT_INT:
                            case Id::SCOPE:
                                trap();
                            case Id::INT_ADD:
                                result = left_type.integer.constant + right_type.integer.constant;
                                break;
                            case Id::INT_SUB:
                                result = left_type.integer.constant - right_type.integer.constant;
                                break;
                            }

                            return Node::Type{
                                .id = Node::Type::Id::INTEGER,
                                .integer = {
                                    .constant = result,
                                    .bit_count = left_type.integer.bit_count,
                                    .is_constant = 1,
                                },
                            };
                        }
                    }
                    trap();
                }
            case Node::Id::CONSTANT_INT:
                return type;
            default:
                trap();
        }
    }
};

[[nodiscard]] fn Node* add_constant_integer(Arena* arena, ConstantIntData data)
{
    auto* constant_int = Node::add(arena, {
        .s = {
            .type =
            {
                .id = Node::Type::Id::INTEGER,
                .integer =
                {
                    .constant = data.value,
                    .bit_count = data.bit_count,
                    .is_constant = 1,
                },
            },
            .inputs = { .pointer = &data.input, .length = 1 },
            .id = Node::Id::CONSTANT_INT,
        },
        .gvn = data.gvn,
    });
    return constant_int;
}

struct WorkList
{
    using BitsetBackingType = u32;
    PinnedArray<Node*> nodes;
    PinnedArray<BitsetBackingType> bitset;

    global constexpr auto bit_count = sizeof(BitsetBackingType) * 8;

    void push(Node* node)
    {
        if (!test_and_set(node))
        {
            nodes.append_one(node);
        }
    }

    u8 test_and_set(Node* node)
    {
        BitsetBackingType gvn_word = node->gvn / bit_count;
        if (gvn_word >= bitset.capacity)
        {
            trap();
        }
        BitsetBackingType gvn_mask = 1 << (node->gvn % bit_count);
        if (bitset[gvn_word] & gvn_mask)
        {
            return 1;
        }
        else
        {
            bitset[gvn_word] |= gvn_mask;
            return 0;
        }
    }

    void ensure_capacity(u32 capacity)
    {
        u32 aligned_capacity = align_forward(capacity, bit_count);
        nodes.ensure_capacity(aligned_capacity);
        auto bitset_length = aligned_capacity / bit_count;
        unused(bitset.add(bitset_length));
    }

    void clear()
    {
        nodes.clear();
    }
};

fn u64 round_up_to_next_power_of_2(u64 n)
{
    n -= 1;
    n |= n >> 1;
    n |= n >> 2;
    n |= n >> 4;
    n |= n >> 8;
    n |= n >> 16;
    n |= n >> 32;
    n += 1;
    return n;
}

// fn Hash intern_identifier(Unit* unit, String identifier)
// {
//     Hash hash = hash_bytes(identifier);
//     (void)unit->identifiers.get_or_put(hash, identifier);
//     return hash;
// }

global String integer_names[] =
{
    strlit("u1"),
    strlit("u2"),
    strlit("u3"),
    strlit("u4"),
    strlit("u5"),
    strlit("u6"),
    strlit("u7"),
    strlit("u8"),
    strlit("u9"),
    strlit("u10"),
    strlit("u11"),
    strlit("u12"),
    strlit("u13"),
    strlit("u14"),
    strlit("u15"),
    strlit("u16"),
    strlit("u17"),
    strlit("u18"),
    strlit("u19"),
    strlit("u20"),
    strlit("u21"),
    strlit("u22"),
    strlit("u23"),
    strlit("u24"),
    strlit("u25"),
    strlit("u26"),
    strlit("u27"),
    strlit("u28"),
    strlit("u29"),
    strlit("u30"),
    strlit("u31"),
    strlit("u32"),
    strlit("u33"),
    strlit("u34"),
    strlit("u35"),
    strlit("u36"),
    strlit("u37"),
    strlit("u38"),
    strlit("u39"),
    strlit("u40"),
    strlit("u41"),
    strlit("u42"),
    strlit("u43"),
    strlit("u44"),
    strlit("u45"),
    strlit("u46"),
    strlit("u47"),
    strlit("u48"),
    strlit("u49"),
    strlit("u50"),
    strlit("u51"),
    strlit("u52"),
    strlit("u53"),
    strlit("u54"),
    strlit("u55"),
    strlit("u56"),
    strlit("u57"),
    strlit("u58"),
    strlit("u59"),
    strlit("u60"),
    strlit("u61"),
    strlit("u62"),
    strlit("u63"),
    strlit("u64"),
    strlit("s1"),
    strlit("s2"),
    strlit("s3"),
    strlit("s4"),
    strlit("s5"),
    strlit("s6"),
    strlit("s7"),
    strlit("s8"),
    strlit("s9"),
    strlit("s10"),
    strlit("s11"),
    strlit("s12"),
    strlit("s13"),
    strlit("s14"),
    strlit("s15"),
    strlit("s16"),
    strlit("s17"),
    strlit("s18"),
    strlit("s19"),
    strlit("s20"),
    strlit("s21"),
    strlit("s22"),
    strlit("s23"),
    strlit("s24"),
    strlit("s25"),
    strlit("s26"),
    strlit("s27"),
    strlit("s28"),
    strlit("s29"),
    strlit("s30"),
    strlit("s31"),
    strlit("s32"),
    strlit("s33"),
    strlit("s34"),
    strlit("s35"),
    strlit("s36"),
    strlit("s37"),
    strlit("s38"),
    strlit("s39"),
    strlit("s40"),
    strlit("s41"),
    strlit("s42"),
    strlit("s43"),
    strlit("s44"),
    strlit("s45"),
    strlit("s46"),
    strlit("s47"),
    strlit("s48"),
    strlit("s49"),
    strlit("s50"),
    strlit("s51"),
    strlit("s52"),
    strlit("s53"),
    strlit("s54"),
    strlit("s55"),
    strlit("s56"),
    strlit("s57"),
    strlit("s58"),
    strlit("s59"),
    strlit("s60"),
    strlit("s61"),
    strlit("s62"),
    strlit("s63"),
    strlit("s64"),
};

fn void unit_initialize(Unit* unit)
{
    Arena* type_arena = Arena::init(Arena::default_size, Arena::minimum_granularity, KB(64));
    SemaType* builtin_types = type_arena->allocate_many<SemaType>(builtin_type_count);

    *unit = {
        // .arena = Arena::init(Arena::default_size, Arena::minimum_granularity, KB(4)),
        // .node_arena = Arena::init(Arena::default_size, Arena::minimum_granularity, KB(64)),
        // .type_arena = type_arena,
        .builtin_types = builtin_types,
    };

    builtin_types[void_type_index] = {
        .size = 0,
        .alignment = 1,
        .id = SemaTypeId::VOID,
        .resolved = 1,
        .name = strlit("void"),
    };
    builtin_types[noreturn_type_index] = {
        .size = 0,
        .alignment = 1,
        .id = SemaTypeId::NORETURN,
        .resolved = 1,
        .name = strlit("noreturn"),
    };
    builtin_types[opaque_pointer_type_index] = {
        .size = 8,
        .alignment = 8,
        .id = SemaTypeId::POINTER,
        .resolved = 1,
        .name = strlit("*any"),
    };
    // TODO: float types
    
    u64 i;
    for (i = integer_type_offset; i < integer_type_offset + 64; i += 1)
    {
        u64 bit_count = i - integer_type_offset + 1;
        assert(bit_count >= 1 | bit_count <= 64);
        auto aligned_bit_count = round_up_to_next_power_of_2(bit_count);
        auto byte_count = max<u64>(aligned_bit_count / 8, 1);
        assert(byte_count <= bit_count);
        assert(byte_count == 1 | byte_count == 2 | byte_count == 4 | byte_count == 8);
        builtin_types[i] =
        {
            .size = byte_count,
            .alignment = byte_count,
            .id = SemaTypeId::INTEGER,
            .resolved = 1,
            .flags = static_cast<u32>(bit_count),
            .name = integer_names[bit_count - 1],
        };
    }

    for (; i < integer_type_offset + integer_type_count; i += 1)
    {
        u64 bit_count = i - (integer_type_offset + 64 - 1);
        assert(bit_count >= 1 | bit_count <= 64);
        auto aligned_bit_count = round_up_to_next_power_of_2(bit_count);
        auto byte_count = max<u64>(aligned_bit_count / 8, 1);
        assert(byte_count <= bit_count);
        assert(byte_count == 1 | byte_count == 2 | byte_count == 4 | byte_count == 8);
        builtin_types[i] =
        {
            .size = byte_count,
            .alignment = byte_count,
            .id = SemaTypeId::INTEGER,
            .resolved = 1,
            .flags = static_cast<u32>(bit_count | (1 << (type_flags_bit_count - 1))), // Signedness bit
            .name = integer_names[bit_count + 63],
        };
    }
}


static_assert(array_length(integer_names) == 128, "Integer name array must be 128 bytes");

struct Instance
{
    Arena* arena;
};
typedef struct Instance Instance;

fn Unit* instance_add_unit(Instance* instance)
{
    Unit* unit = instance->arena->allocate_one<Unit>();
    *unit = {
    };
    return unit;
}

// TODO: make it into an array
fn Thread* instance_add_thread(Instance* instance)
{
    auto* thread = instance->arena->allocate_one<Thread>();
    *thread = {
        .arena = Arena::init_default(KB(64)),
    };
    return thread;
}

fn u64 safe_flag(u64 value, u64 flag)
{
    u64 result = value & ((u64)0 - flag);
    return result;
}

fn u8 get_next_ch_safe(String file, u64 index)
{
    u64 next_index = index + 1;
    u64 is_in_range = next_index < file.length;
    u64 safe_index = safe_flag(next_index, is_in_range);
    u8 unsafe_result = file.pointer[safe_index];
    u64 safe_result = safe_flag(unsafe_result, is_in_range);
    assert(safe_result < 256);
    return (u8)safe_result;
}

fn u32 is_space(u8 ch, u8 next_ch)
{
    u32 is_comment = (ch == '/') & (next_ch == '/');
    u32 is_whitespace = ch == ' ';
    u32 is_vertical_tab = ch == 0x0b;
    u32 is_horizontal_tab = ch == '\t';
    u32 is_line_feed = ch == '\n';
    u32 is_carry_return = ch == '\r';
    u32 result = (((is_vertical_tab | is_horizontal_tab) | (is_line_feed | is_carry_return)) | (is_comment | is_whitespace));
    return result;
}

fn u64 is_lower(u8 ch)
{
    return (ch >= 'a') & (ch <= 'z');
}

fn u64 is_upper(u8 ch)
{
    return (ch >= 'A') & (ch <= 'Z');
}

fn u64 is_alphabetic(u8 ch)
{
    return is_lower(ch) | is_upper(ch);
}

fn u64 is_decimal_digit(u8 ch)
{
    return (ch >= '0') & (ch <= '9');
}

fn u64 is_hex_digit(u8 ch)
{
    return (is_decimal_digit(ch) | ((ch == 'a' | ch == 'A') | (ch == 'b' | ch == 'B'))) | (((ch == 'c' | ch == 'C') | (ch == 'd' | ch == 'D')) | ((ch == 'e' | ch == 'E') | (ch == 'f' | ch == 'F')));
}


fn u64 is_identifier_start(u8 ch)
{
    u64 alphabetic = is_alphabetic(ch);
    u64 is_underscore = ch == '_';
    return alphabetic | is_underscore;
}

fn u64 is_identifier_ch(u8 ch)
{
    u64 identifier_start = is_identifier_start(ch);
    u64 decimal = is_decimal_digit(ch);
    return identifier_start | decimal;
}

struct Parser
{
    u64 i;
    u32 line;
    u32 column;

    void skip_space(String src)
    {
        u64 original_i = i;

        if (original_i != src.length)
        {
            if (is_space(src.pointer[original_i], get_next_ch_safe(src, original_i)))
            {
                while (i < src.length)
                {
                    u64 index = i;
                    u8 ch = src.pointer[index];
                    u64 new_line = ch == '\n';
                    line += new_line;

                    if (new_line)
                    {
                        column = index + 1;
                    }

                    if (!is_space(ch, get_next_ch_safe(src, i)))
                    {
                        break;
                    }

                    u32 is_comment = src.pointer[index] == '/';
                    i += is_comment + is_comment;
                    if (is_comment)
                    {
                        while (i < src.length)
                        {
                            if (src.pointer[i] == '\n')
                            {
                                break;
                            }

                            i += 1;
                        }

                        continue;
                    }

                    i += 1;
                }
            }
        }
    }
    void expect_character(String src, u8 expected_ch)
    {
        u64 index = i;
        if (expect(index < src.length, 1))
        {
            u8 ch = src.pointer[index];
            u64 matches = ch == expected_ch;
            expect(matches, 1);
            i += matches;
            if (!matches)
            {
                print(strlit("expected character '"));
                print(ch_to_str(expected_ch));
                print(strlit("', but found '"));
                print(ch_to_str(ch));
                print(strlit("'\n"));
                fail();
            }
        }
        else
        {
            print(strlit("expected character '"));
            print(ch_to_str(expected_ch));
            print(strlit("', but found end of file\n"));
            fail();
        }
    }

    String parse_raw_identifier(String src)
    {
        u64 identifier_start_index = i;
        u64 is_string_literal = src.pointer[identifier_start_index] == '"';
        i += is_string_literal;
        u8 identifier_start_ch = src.pointer[i];
        u64 is_valid_identifier_start = is_identifier_start(identifier_start_ch);
        i += is_valid_identifier_start;

        if (expect(is_valid_identifier_start, 1))
        {
            while (i < src.length)
            {
                u8 ch = src.pointer[i];
                u64 is_identifier = is_identifier_ch(ch);
                expect(is_identifier, 1);
                i += is_identifier;

                if (!is_identifier)
                {
                    if (expect(is_string_literal, 0))
                    {
                        expect_character(src, '"');
                    }

                    String result = src.slice(identifier_start_index, i - is_string_literal);
                    return result;
                }
            }

            fail();
        }
        else
        {
            fail();
        }
    }

    typedef enum Keyword : u32
    {
        KEYWORD_COUNT,
        KEYWORD_INVALID = ~0u,
    } Keyword;

    // TODO:
    // fn Keyword parse_keyword(String identifier)
    // {
    //     Keyword result = KEYWORD_INVALID;
    //     return result;
    // }

    String parse_and_check_identifier(String src)
    {
        String identifier = parse_raw_identifier(src);
        // Keyword keyword_index = parse_keyword(identifier);
        // if (expect(keyword_index != KEYWORD_INVALID, 0))
        // {
        //     fail();
        // }

        if (expect(identifier.equal(strlit("_")), 0))
        {
            return {};
        }

        return identifier;
    }
};

// fn u32 get_line(Parser* parser)
// {
//     return parser->line + 1;
// }
//
// fn u32 get_column(Parser* parser)
// {
//     return parser->i - parser->column + 1;
// }

fn File* add_file(Arena* arena, String file_path)
{
    auto* file = arena->allocate_one<File>();
    *file = {
        .path = file_path,
    };
    return file;
}

fn void compiler_file_read(Arena* arena, File* file)
{
    assert(file->status == FILE_STATUS_ADDED || file->status == FILE_STATUS_QUEUED);
    file->source_code = file_read(arena, file->path);
    file->status = FILE_STATUS_READ;
}

global constexpr auto brace_open = '{';
global constexpr auto brace_close = '}';

global constexpr auto parenthesis_open = '(';
global constexpr auto parenthesis_close = ')';

global constexpr auto bracket_open = '[';
global constexpr auto bracket_close = ']';

global constexpr auto pointer_sign = '*';
global constexpr auto end_of_statement = ';';
global constexpr auto end_of_argument = ',';

global constexpr auto function_argument_start = parenthesis_open;
global constexpr auto function_argument_end = parenthesis_close;

global constexpr auto function_attribute_start = bracket_open;
global constexpr auto function_attribute_end = bracket_close;

global constexpr auto symbol_attribute_start = bracket_open;
global constexpr auto symbol_attribute_end = bracket_close;

global constexpr auto block_start = brace_open;
global constexpr auto block_end = brace_close;

global constexpr auto local_symbol_declaration_start = '>';

global constexpr auto array_expression_start = bracket_open;
// global constexpr auto array_expression_end = bracket_close;

global constexpr auto composite_initialization_start = brace_open;
// global constexpr auto composite_initialization_end = brace_close;

global String function_attributes[] =
{
    strlit("cc"),
};

typedef enum FunctionAttribute
{
    FUNCTION_ATTRIBUTE_CC,
    FUNCTION_ATTRIBUTE_COUNT,
} FunctionAttribute;

static_assert(array_length(function_attributes) == FUNCTION_ATTRIBUTE_COUNT, "");

global String calling_conventions[] =
{
    strlit("c"),
    strlit("custom"),
};

typedef enum CallingConvention
{
    CALLING_CONVENTION_C,
    CALLING_CONVENTION_CUSTOM,
    CALLING_CONVENTION_COUNT,
} CallingConvention;

static_assert(array_length(calling_conventions) == CALLING_CONVENTION_COUNT, "");

typedef enum GlobalSymbolAttribute
{
    GLOBAL_SYMBOL_ATTRIBUTE_EXPORT,
    GLOBAL_SYMBOL_ATTRIBUTE_EXTERN,
    GLOBAL_SYMBOL_ATTRIBUTE_COUNT,
} GlobalSymbolAttribute;

global String global_symbol_attributes[] =
{
    strlit("export"),
    strlit("extern"),
};

struct GlobalSymbolAttributes
{
    u8 exported: 1;
    u8 external: 1;
};
typedef struct GlobalSymbolAttributes GlobalSymbolAttributes;

static_assert(array_length(global_symbol_attributes) == GLOBAL_SYMBOL_ATTRIBUTE_COUNT, "");

struct Analyzer
{
    Function* function;
    Node* scope;
};

fn SemaType* analyze_type(Parser* parser, Unit* unit, String src)
{
    u64 start_index = parser->i;
    u8 start_ch = src.pointer[start_index];
    u32 array_start = start_ch == array_expression_start;
    u32 u_start = start_ch == 'u';
    u32 s_start = start_ch == 's';
    u32 float_start = start_ch == 'f';
    u32 void_start = start_ch == 'v';
    u32 pointer_start = start_ch == pointer_sign;
    u32 integer_start = u_start | s_start;
    u32 number_start = integer_start | float_start;

    if (void_start)
    {
        trap();
    }
    else if (array_start)
    {
        trap();
    }
    else if (pointer_start)
    {
        trap();
    }
    else if (number_start)
    {
        u64 expected_digit_start = start_index + 1;
        u64 i = expected_digit_start;
        u32 decimal_digit_count = 0;
        u64 top = i + 5;

        while (i < top)
        {
            u8 ch = src.pointer[i];
            u32 is_digit = is_decimal_digit(ch);
            decimal_digit_count += is_digit;
            if (!is_digit)
            {
                u32 is_alpha = is_alphabetic(ch);
                if (is_alpha)
                {
                    decimal_digit_count = 0;
                }
                break;
            }

            i += 1;
        }


        if (decimal_digit_count)
        {
            parser->i += 1;

            if (integer_start)
            {
                u64 signedness = s_start;
                u64 bit_size;
                u64 current_i = parser->i;
                assert(src.pointer[current_i] >= '0' & src.pointer[current_i] <= '9');
                switch (decimal_digit_count) {
                    case 0:
                        fail();
                    case 1:
                        bit_size = src.pointer[current_i] - '0';
                        break;
                    case 2:
                        bit_size = (src.pointer[current_i] - '0') * 10 + (src.pointer[current_i + 1] - '0');
                        break;
                    default:
                        fail();
                }
                parser->i += decimal_digit_count;

                assert(!is_decimal_digit(src.pointer[parser->i]));

                if (bit_size)
                {
                    auto* result = unit->get_integer_type(bit_size, signedness);
                    return result;
                }
                else
                {
                    fail();
                }
            }
            else if (float_start)
            {
                trap();
            }
            else
            {
                trap();
            }
        }
        else
        {
            fail();
        }
    }

    trap();
}

fn u64 parse_hex(String string)
{
    u64 value = 0;
    for (u8 ch : string)
    {
        u8 byte;
        auto is_decimal = (ch >= '0') & (ch <= '9');
        auto is_lower_hex = (ch >= 'a') & (ch <= 'f');
        auto is_upper_hex = (ch >= 'A') & (ch <= 'F');
        if (is_decimal)
        {
            byte = ch - '0';
        }
        else if (is_lower_hex)
        {
            byte = ch - 'a' + 10;
        }
        else if (is_upper_hex)
        {
            byte = ch - 'A' + 10;
        }
        else
        {
            fail();
        }

        value = (value << 4) | (byte & 0x0f);
    }

    return value;
}

fn u64 parse_decimal(String string)
{
    u64 value = 0;
    for (u8 ch : string)
    {
        assert(((ch >= '0') & (ch <= '9')));
        value = (value * 10) + (ch - '0');
    }

    return value;
}



[[nodiscard]] fn Node* parse_constant_integer(Parser* parser, Arena* arena, String src, SemaType* type, u32 gvn, Node* input)
{
    u64 value = 0;
    auto starting_index = parser->i;
    auto starting_ch = src[starting_index];

    if (starting_ch == '0')
    {
        auto follow_up_character = src[parser->i + 1];
        auto is_hex_start = follow_up_character == 'x';
        auto is_octal_start = follow_up_character == 'o';
        auto is_bin_start = follow_up_character == 'b';
        auto is_prefixed_start = is_hex_start | is_octal_start | is_bin_start;
        auto follow_up_alpha = is_alphabetic(follow_up_character);
        auto follow_up_digit = is_decimal_digit(follow_up_character);
        auto is_valid_after_zero = is_space(follow_up_character, get_next_ch_safe(src, follow_up_character)) | (!follow_up_digit and !follow_up_alpha);

        if (is_prefixed_start) {
            enum class IntegerPrefix {
                hexadecimal,
                octal,
                binary,
            };
            IntegerPrefix prefix;
            switch (follow_up_character) {
                case 'x': prefix = IntegerPrefix::hexadecimal; break;
                case 'o': prefix = IntegerPrefix::octal; break;
                case 'b': prefix = IntegerPrefix::binary; break;
                default: fail();
            };

            parser->i += 2;

            auto start = parser->i;

            switch (prefix) {
                case IntegerPrefix::hexadecimal:
                    {
                        while (is_hex_digit(src[parser->i])) {
                            parser->i += 1;
                        }

                        auto slice = src.slice(start, parser->i);
                        value = parse_hex(slice);
                    }
                case IntegerPrefix::octal: 
                    trap();
                case IntegerPrefix::binary:
                    trap();
            }
        } else if (is_valid_after_zero) {
            value = 0;
            parser->i += 1;
        } else {
            fail();
        }
    }
    else
    {
        while (is_decimal_digit(src[parser->i]))
        {
            parser->i += 1;
        }

        auto slice = src.slice(starting_index, parser->i);
        value = parse_decimal(slice);
    }

    Node* result = add_constant_integer(arena, {
        .value = value,
        .input = input,
        .gvn = gvn,
        .bit_count = type->get_bit_count(),
    });

    return result;
}

[[nodiscard]] fn Node* analyze_single_expression(Analyzer* analyzer, Parser* parser, Unit* unit, Arena* arena, String src, SemaType* type, Side side)
{
    unused(side);
    enum class Unary
    {
        NONE,
        ONE_COMPLEMENT,
        NEGATION,
    };
    auto unary_operation = Unary::NONE;
    auto* function = analyzer->function;

    auto original_starting_ch_index = parser->i;
    u8 original_starting_ch = src[original_starting_ch_index];

    switch (src[parser->i])
    {
        case '\'':
            trap();
        case '"':
            trap();
        case '-':
            trap();
        case '~':
            trap();
        case '#':
            trap();
        case composite_initialization_start:
            trap();
        case array_expression_start:
            trap();
        default:
            assert(is_decimal_digit(original_starting_ch) | is_identifier_start(original_starting_ch));
            break;
    }

    auto starting_ch_index = parser->i; 
    u8 starting_ch = src[starting_ch_index];
    auto is_digit = is_decimal_digit(starting_ch);
    auto is_identifier = is_identifier_start(starting_ch);

    // auto line = get_line(parser);
    // auto column = get_column(parser);

    if (is_digit)
    {
        SemaType* integer_type;
        if (type)
        {
            integer_type = type;
        }
        else
        {
            switch (unary_operation)
            {
            case Unary::NONE:
                integer_type = unit->get_integer_type(64, 0);
                break;
            case Unary::ONE_COMPLEMENT:
                fail();
            case Unary::NEGATION:
                fail();
            }
        }

        if (integer_type->id != SemaTypeId::INTEGER)
        {
            fail();
        }

        auto gvn = function->node_count;
        function->node_count += 1;
        Node* constant_int = parse_constant_integer(parser, arena, src, integer_type, gvn, function->root_node);

        return constant_int;
    }
    else if (is_identifier)
    {
        trap();
    }
    else
    {
        fail();
    }
}

[[nodiscard]] fn Node* analyze_expression(Analyzer* analyzer, Parser* parser, Unit* unit, Arena* arena, String src, SemaType* type, Side side)
{
    enum class CurrentOperation
    {
        NONE,
        ADD,
        ADD_ASSIGN,
        SUB,
        SUB_ASSIGN,
    };

    u64 iterations = 0;
    SemaType* iteration_type = type;
    auto current_operation = CurrentOperation::NONE;
    Node* previous_node = 0;

    while (1)
    {
        if ((iterations == 0) & !iteration_type)
        {
            trap();
        }

        // u32 line = get_line(parser);
        // u32 column = get_column(parser);
        Node* current_node;
        if (src[parser->i] == '(')
        {
            trap();
        }
        else
        {
            current_node = analyze_single_expression(analyzer, parser, unit, arena, src, iteration_type, side);
        }

        parser->skip_space(src);

        switch (current_operation)
        {
        case CurrentOperation::NONE:
            previous_node = current_node;
            break;
        case CurrentOperation::ADD:
        case CurrentOperation::SUB:
            {
                Node::Id id;
                switch (current_operation)
                {
                case CurrentOperation::NONE:
                    trap();
                case CurrentOperation::ADD:
                    id = Node::Id::INT_ADD;
                    break;
                case CurrentOperation::SUB:
                    id = Node::Id::INT_SUB;
                    break;
                case CurrentOperation::ADD_ASSIGN:
                case CurrentOperation::SUB_ASSIGN:
                    trap();
                }

                Node* inputs[] = {
                    0,
                    previous_node,
                    current_node,
                };

                auto* binary = Node::add_from_function(arena, analyzer->function, {
                    .type = current_node->type,
                    .inputs = { .pointer = inputs, .length = array_length(inputs), },
                    .id = id,
                });

                previous_node = binary;
            } break;
        default:
            trap();
        }

        previous_node = previous_node->peephole(arena, analyzer->function);

        auto original_index = parser->i;
        u8 original = src[original_index];

        switch (original)
        {
            case end_of_statement:
            case end_of_argument:
            case parenthesis_close:
            case bracket_close:
                return previous_node;
            case '+':
                current_operation = CurrentOperation::ADD;
                parser->i += 1;

                switch (src[parser->i])
                {
                    case '=':
                        current_operation = CurrentOperation::ADD_ASSIGN;
                        parser->i += 1;
                        break;
                    default:
                        break;
                }
                break;
            case '-':
                current_operation = CurrentOperation::SUB;
                parser->i += 1;

                switch (src[parser->i])
                {
                    case '=':
                        current_operation = CurrentOperation::SUB_ASSIGN;
                        parser->i += 1;
                        break;
                    default:
                        break;
                }
                break;
            default:
                trap();
        }

        parser->skip_space(src);

        iterations += 1;
    }
}

fn void push_scope(Analyzer* analyzer)
{
    analyzer->scope->scope.stack.append_one({});
}

fn void pop_scope(Analyzer* analyzer)
{
    analyzer->scope->scope.stack.pop();
}

fn Node* define_variable(Analyzer* analyzer, Arena* arena, String name, Node* node)
{
    auto* stack = &analyzer->scope->scope.stack;
    assert(stack->length);
    auto* last = &stack->pointer[stack->length - 1];
    if (last->get_or_put(name, analyzer->scope->inputs.length).existing)
    {
        return 0;
    }

    return analyzer->scope->add_output(node);
}

fn Node* analyze_local_block(Analyzer* analyzer, Parser* parser, Unit* unit, Arena* arena, String src)
{
    push_scope(analyzer);
    parser->expect_character(src, block_start);

    Node* node = 0;
    while (1)
    {
        parser->skip_space(src);

        if (src[parser->i] == block_end)
        {
            break;
        }

        auto statement_start_index = parser->i;
        u8 statement_start_ch = src[statement_start_index];

        Node* statement_node = 0;

        if (is_identifier_start(statement_start_ch))
        {
            String identifier = parser->parse_raw_identifier(src);
            if (identifier.equal(strlit("return")))
            {
                parser->skip_space(src);

                auto* return_value = analyze_expression(analyzer, parser, unit, arena, src, analyzer->function->prototype.original_return_type, Side::right);
                parser->expect_character(src, ';');

                Function* function = analyzer->function;

                Node* inputs[] =
                {
                    function->root_node,
                    return_value,
                };

                Node* ret_node = Node::add_from_function(arena, function, {
                    .type = { .id = Node::Type::Id::CONTROL },
                    .inputs = { .pointer = inputs, .length = array_length(inputs) },
                    .id = Node::Id::RETURN,
                });
                statement_node = ret_node;
            }

            if (!statement_node)
            {
                print(strlit("statement: '"));
                print(identifier);
                print(strlit("'\n"));
                trap();
            }
        }
        else
        {
            switch (statement_start_ch)
            {
                case local_symbol_declaration_start:
                    {
                        parser->i += 1;

                        parser->skip_space(src);

                        String name = parser->parse_and_check_identifier(src);

                        u8 has_local_attributes = src[parser->i] == symbol_attribute_start;
                        parser->i += has_local_attributes;

                        if (has_local_attributes)
                        {
                            // TODO: local attributes
                            fail();
                        }

                        parser->skip_space(src);

                        struct LocalResult
                        {
                            Node* node;
                            SemaType* type;
                        };

                        LocalResult local_result = {};
                        switch (src[parser->i])
                        {
                            case ':':
                                {
                                    parser->i += 1;
                                    parser->skip_space(src);

                                    SemaType* type = analyze_type(parser, unit, src);

                                    parser->skip_space(src);
                                    parser->expect_character(src, '=');
                                    parser->skip_space(src);

                                    auto* initial_node = analyze_expression(analyzer, parser, unit, arena, src, type, Side::right);
                                    if (!define_variable(analyzer, arena, name, initial_node))
                                    {
                                        fail();
                                    }
                                    local_result = {
                                        .node = initial_node,
                                        .type = type,
                                    };
                                } break;
                            case '=': trap();
                            default: fail();
                        }

                        parser->skip_space(src);
                        parser->expect_character(src, ';');

                        statement_node = local_result.node;
                    } break;
                case block_start:
                    {
                        statement_node = analyze_local_block(analyzer, parser, unit, arena, src);
                    } break;
                default:
                    trap();
            }
        }

        if (statement_node)
        {
            node = statement_node;
        }
    }

    parser->expect_character(src, block_end);

    pop_scope(analyzer);

    return node;
}

typedef enum SystemVClass
{
    SYSTEMV_CLASS_NONE,
    SYSTEMV_CLASS_MEMORY,
    SYSTEMV_CLASS_INTEGER,
    SYSTEMV_CLASS_SSE,
    SYSTEMV_CLASS_SSEUP,
} SystemVClass;

struct SystemVClassification
{
    SystemVClass v[2];
};
typedef struct SystemVClassification SystemVClassification;

struct SystemVRegisterCount
{
    u32 gp_registers;
    u32 sse_registers;
};
typedef struct SystemVRegisterCount SystemVRegisterCount;

fn SystemVClassification systemv_classify(SemaType* type, u64 base_offset)
{
    SystemVClassification result;
    u32 is_memory = base_offset >= 8;
    u32 current_index = is_memory;
    result.v[current_index] = SYSTEMV_CLASS_MEMORY;
    result.v[!current_index] = SYSTEMV_CLASS_NONE;

    switch (type->id)
    {
        case SemaTypeId::VOID:
            trap();
        case SemaTypeId::NORETURN:
            trap();
        case SemaTypeId::POINTER:
            trap();
        case SemaTypeId::INTEGER:
        {
            u8 bit_count = type->get_bit_count();
            switch (bit_count)
            {
                case 8: case 16: case 32: case 64:
                    result.v[current_index] = SYSTEMV_CLASS_INTEGER;
                    break;
                default:
                    trap();
            }
        } break;
        case SemaTypeId::COUNT:
            trap();
        default:
            trap();
    }

    return result;
}

fn u8 contains_no_user_data(SemaType* type, u64 start, u64 end)
{
    unused(end);
    if (type->size <= start)
    {
        return 1;
    }

    switch (type->id)
    {
        case SemaTypeId::ARRAY:
            trap();
        case SemaTypeId::STRUCT:
            trap();
        case SemaTypeId::UNION:
            trap();
        default:
            return 0;
        case SemaTypeId::COUNT:
            trap();
    }
}

fn SemaType* systemv_get_int_type_at_offset(SemaType* type, u64 offset, SemaType* source_type, u64 source_offset)
{
    unused(source_type);

    switch (type->id)
    {
        case SemaTypeId::VOID:
            trap();
        case SemaTypeId::NORETURN:
            trap();
        case SemaTypeId::POINTER:
            trap();
        case SemaTypeId::INTEGER:
        {
            u8 bit_count = type->get_bit_count();
            switch (bit_count)
            {
                case 8: case 16: case 32: case 64:
                    if (offset == 0)
                    {
                        u64 start = source_offset + type->size;
                        u64 end = source_offset + 8;
                        if (contains_no_user_data(type, start, end))
                        {
                            return type;
                        }
                        trap();
                    }
                    else
                    {
                        trap();
                    }
                default:
                    trap();
            }
            trap();
        } break;
        case SemaTypeId::COUNT:
            trap();
        case SemaTypeId::ARRAY:
            trap();
        case SemaTypeId::STRUCT:
            trap();
        case SemaTypeId::UNION:
            trap();
    }
}

fn void analyze_function(Parser* parser, Thread* thread, Unit* unit, String src)
{
    parser->expect_character(src, 'f');
    parser->expect_character(src, 'n');

    parser->skip_space(src);
    
    u64 has_function_attributes = src.pointer[parser->i] == function_attribute_start;
    parser->i += has_function_attributes;

    CallingConvention calling_convention = CALLING_CONVENTION_CUSTOM;

    if (has_function_attributes)
    {
        u64 mask = 0;

        while (1)
        {
            parser->skip_space(src);

            if (src.pointer[parser->i] == function_attribute_end)
            {
                break;
            }

            String attribute_candidate = parser->parse_raw_identifier(src);

            u64 attribute_i;
            for (attribute_i = 0; attribute_i < array_length(function_attributes); attribute_i += 1)
            {
                String function_attribute_string = function_attributes[attribute_i];
                if (attribute_candidate.equal(function_attribute_string))
                {
                    if (mask & (1 << attribute_i))
                    {
                        fail();
                    }

                    auto function_attribute = static_cast<FunctionAttribute>(attribute_i);
                    mask |= (1 << attribute_i);

                    switch (function_attribute)
                    {
                        case FUNCTION_ATTRIBUTE_CC:
                            {
                                parser->skip_space(src);
                                parser->expect_character(src, '(');
                                parser->skip_space(src);
                                parser->expect_character(src, '.');
                                String candidate_cc = parser->parse_raw_identifier(src);
                                parser->skip_space(src);
                                parser->expect_character(src, ')');

                                u64 cc_i;
                                for (cc_i = 0; cc_i < array_length(calling_conventions); cc_i += 1)
                                {
                                    String calling_convention_string = calling_conventions[cc_i];

                                    if (calling_convention_string.equal(candidate_cc))
                                    {
                                        calling_convention = static_cast<CallingConvention>(cc_i);
                                        break;
                                    }
                                }

                                if (cc_i == array_length(calling_conventions))
                                {
                                    fail();
                                }
                            } break;
                        default:
                            trap();
                    }

                    break;
                }
            }

            if (attribute_i == array_length(function_attributes))
            {
                fail();
            }

            parser->skip_space(src);

            u8 after_ch = src.pointer[parser->i];
            switch (after_ch)
            {
                case function_attribute_end: break;
                default: fail();
            }
        }

        parser->expect_character(src, function_attribute_end);

        parser->skip_space(src);
    }

    String name = parser->parse_and_check_identifier(src);

    parser->skip_space(src);

    u64 has_global_attributes = src.pointer[parser->i] == symbol_attribute_start;
    parser->i += has_global_attributes;

    GlobalSymbolAttributes symbol_attributes = {};
    if (has_global_attributes)
    {
        u64 mask = 0;

        while (1)
        {
            parser->skip_space(src);

            if (src.pointer[parser->i] == symbol_attribute_end)
            {
                break;
            }

            String candidate_attribute = parser->parse_raw_identifier(src);
            parser->skip_space(src);
            switch (src.pointer[parser->i])
            {
                case symbol_attribute_end:
                    break;
                case ',':
                    parser->i += 1;
                    break;
                default:
                    fail();
            }

            u64 attribute_i;
            for (attribute_i = 0; attribute_i < array_length(global_symbol_attributes); attribute_i += 1)
            {
                String attribute_string = global_symbol_attributes[attribute_i];
                if (attribute_string.equal(candidate_attribute))
                {
                    if (mask & (1 << attribute_i))
                    {
                        fail();
                    }

                    mask |= 1 << attribute_i;

                    auto attribute = static_cast<GlobalSymbolAttribute>(attribute_i);

                    switch (attribute)
                    {
                    case GLOBAL_SYMBOL_ATTRIBUTE_EXPORT:
                        symbol_attributes.exported = 1;
                        break;
                    case GLOBAL_SYMBOL_ATTRIBUTE_EXTERN:
                        symbol_attributes.external = 1;
                        break;
                    default:
                        trap();
                    }
                    break;
                }
            }

            if (attribute_i == array_length(global_symbol_attributes))
            {
                fail();
            }
        }

        parser->expect_character(src, symbol_attribute_end);

        parser->skip_space(src);
    }

    if (symbol_attributes.exported & symbol_attributes.external)
    {
        fail();
    }

    parser->expect_character(src, function_argument_start);

    while (1)
    {
        parser->skip_space(src);

        if (src.pointer[parser->i] == function_argument_end)
        {
            break;
        }

        // TODO: function arguments in function definition
        trap();
    }

    parser->expect_character(src, function_argument_end);

    parser->skip_space(src);
    PinnedArray<SemaType*> original_argument_types = {};

    SemaType* original_return_type = analyze_type(parser, unit, src);

    parser->skip_space(src);

    switch (calling_convention)
    {
    case CALLING_CONVENTION_C:
        {
            // First process the return type ABI
            AbiInfo return_type_abi = {};
            {
                SystemVClassification return_type_classes = systemv_classify(original_return_type, 0);
                assert(return_type_classes.v[1] != SYSTEMV_CLASS_MEMORY | return_type_classes.v[0] == SYSTEMV_CLASS_MEMORY);
                assert(return_type_classes.v[1] != SYSTEMV_CLASS_SSEUP | return_type_classes.v[0] == SYSTEMV_CLASS_SSE);
                SemaType* low_part = 0;
                switch (return_type_classes.v[0])
                {
                    case SYSTEMV_CLASS_INTEGER:
                        {
                            SemaType* result_type = systemv_get_int_type_at_offset(original_return_type, 0, original_return_type, 0);
                            if (return_type_classes.v[1] == SYSTEMV_CLASS_NONE & original_return_type->get_bit_count() < 32)
                            {
                                trap();
                            }

                            low_part = result_type;
                        } break;
                    default:
                        trap();
                }
                assert(low_part);

                SemaType* high_part = 0;
                switch (return_type_classes.v[1])
                {
                    case SYSTEMV_CLASS_NONE:
                        break;
                    case SYSTEMV_CLASS_MEMORY:
                        trap();
                    case SYSTEMV_CLASS_INTEGER:
                        trap();
                    case SYSTEMV_CLASS_SSE:
                        trap();
                    case SYSTEMV_CLASS_SSEUP:
                        trap();
                }

                if (high_part)
                {
                    trap();
                }
                else
                {
                    // TODO:
                    u8 is_type = 1;
                    if (is_type)
                    {
                        if (low_part == original_return_type)
                        {
                            return_type_abi =
                            {
                                .kind = ABI_INFO_DIRECT,
                            };
                        }
                        else
                        {
                            trap();
                        }
                    }
                    else
                    {
                        trap();
                    }
                }
            }

            // Now process the ABI for argument types
            
            PinnedArray<AbiInfo> argument_type_abis = {};
            // u32 abi_argument_type_count = 0;
            {
                SystemVRegisterCount available_registers = {
                    .gp_registers = 6,
                    .sse_registers = 8,
                };
                
                available_registers.gp_registers -= return_type_abi.kind == ABI_INFO_INDIRECT;

                // TODO: return by reference
                u8 return_by_reference = 0;
                if (return_by_reference)
                {
                    trap();
                }

                for (u32 original_argument_index = 0; original_argument_index < original_argument_types.length; original_argument_index += 1)
                {
                    trap();
                }
            }

            switch (return_type_abi.kind)
            {
            case ABI_INFO_IGNORE: case ABI_INFO_DIRECT:
                break;
            case ABI_INFO_DIRECT_PAIR:
                trap();
            case ABI_INFO_DIRECT_COERCE:
                trap();
            case ABI_INFO_DIRECT_COERCE_INT:
                trap();
            case ABI_INFO_DIRECT_SPLIT_STRUCT_I32:
                trap();
            case ABI_INFO_EXPAND_COERCE:
                trap();
            case ABI_INFO_INDIRECT:
                trap();
            case ABI_INFO_EXPAND:
                trap();
            }

            // assert(abi_argument_type_count == 0);
            // TODO: reserve memory for them
            // Slice<Node::DataType> abi_argument_types = {};
            for (u32 i = 0; i < argument_type_abis.length; i += 1)
            {
                trap();
            }

            // TODO: put them into an array?
            auto* function = thread->arena->allocate_one<Function>();

            *function = {
                .symbol = {
                    .name = name,
                    .id = Symbol::Id::function,
                    .linkage = symbol_attributes.external ? Symbol::Linkage::external : Symbol::Linkage::internal,
                },
                .root_node = 0,
                .parameters = thread->arena->allocate_many<Node*>(argument_type_abis.length),
                .prototype = {
                    .argument_type_abis = argument_type_abis.pointer,
                    .original_argument_types = original_argument_types.pointer,
                    .original_return_type = original_return_type,
                    .return_type_abi = return_type_abi,
                    .original_argument_count = original_argument_types.length,
                    .varags = 0,
                },
                .node_count = 0,
                .parameter_count = (u16)argument_type_abis.length,
            };

            function->root_node = Node::add_from_function(thread->arena, function, {
                .type = { .id = Node::Type::Id::TUPLE },
                .id = Node::Id::ROOT,
            });

            switch (symbol_attributes.external)
            {
                case 0:
                    {
                        Analyzer analyzer = {
                            .function = function,
                            .scope = Node::add_from_function(thread->arena, function, {
                                .type = { .id = Node::Type::Id::BOTTOM },
                                .inputs = { .pointer = &function->root_node, .length = 1 },
                                .id = Node::Id::SCOPE,
                            }),
                        };
                        analyzer.scope->scope.stack = {};
                        analyze_local_block(&analyzer, parser, unit, thread->arena, src);
                        // TODO: remove hack
                        thread->functions = {
                            .pointer = function,
                            .length = 1,
                        };
                    } break;
                case 1:
                    trap();
            }
        } break;
    case CALLING_CONVENTION_CUSTOM:
        trap();
        break;
    case CALLING_CONVENTION_COUNT:
        trap();
        break;
    }
}

fn void unit_file_analyze(Thread* thread, Unit* unit, File* file)
{
    compiler_file_read(thread->arena, file);

    Parser parser = {};
    String src = file->source_code;

    while (1) 
    {
        parser.skip_space(src);

        if (parser.i >= src.length)
        {
            break;
        }

        // u32 line = get_line(&parser);
        // u32 column = get_column(&parser);
        u64 declaration_start_index = parser.i;
        u8 declaration_start_ch = src.pointer[declaration_start_index];

        switch (declaration_start_ch)
        {
            case '>':
                trap();
                break;
            case 'f':
                if (get_next_ch_safe(src, declaration_start_index) == 'n')
                {
                    analyze_function(&parser, thread, unit, src);
                }
                else
                {
                    fail();
                }
                break;
            default:
                fail();
        }
    }
}

global Instance instance;

// fn Node* instruction_selection(Node* node)
// {
//     switch (node->id)
//     {
//     case Node::Id::PROJECTION:
//         return node;
//     case Node::Id::ROOT:
//         {
//             return node;
//         }
//     case Node::Id::RETURN:
//         trap();
//     case Node::Id::CONSTANT_INT:
//         trap();
//       break;
//     }
//     trap();
// }

// fn void function_codegen(Function* function)
// {
//     WorkList helper = {};
//     helper.ensure_capacity(function->node_count);
//
//     helper.push(function->root_node);
//     PinnedArray<Node*> pins = {};
//
//     u64 i = 0;
//     while (i < helper.nodes.length)
//     {
//         Node* node = helper.nodes[i];
//         i += 1;
//
//         if (node->is_pinned() & !node->is_projection())
//         {
//             pins.append_one(node);
//         }
//
//         for (Output& output : node->get_outputs())
//         {
//             helper.push(output.node);
//         }
//     }
//
//     helper.clear();
//
//     WorkList walker = {};
//     walker.ensure_capacity(function->node_count);
//
//     for (Node* pin_node : pins.slice())
//     {
//         walker.push(pin_node);
//
//         while (walker.nodes.length > 0)
//         {
//             Node* node = walker.nodes.pop();
//
//             if (!node->is_projection() & (node->output_count == 0))
//             {
//                 helper.push(node);
//                 continue;
//             }
//
//             if (node->data_type.id == Node::Type::Id::MEMORY)
//             {
//                 trap();
//             }
//
//             Node* new_node = instruction_selection(node);
//             if (new_node && new_node != node)
//             {
//                 trap();
//             }
//
//             u16 input_i = node->input_count;
//             while (input_i > 0)
//             {
//                 input_i -= 1;
//
//                 if (node->inputs[input_i])
//                 {
//                     trap();
//                 }
//             }
//
//             // TODO: region
//         }
//     }
//
//
//
//     trap();
// }

String test_file_paths[] = {
    strlit("tests/first/main.nat"),
    strlit("tests/constant_prop/main.nat"),
    strlit("tests/simple_variable_declaration/main.nat"),
};

extern "C" void entry_point()
{
    instance.arena = Arena::init(Arena::default_size, Arena::minimum_granularity, KB(4));

    for (String test_file_path : test_file_paths)
    {
        print(test_file_path);
        print(strlit("... "));
        Unit* unit = instance_add_unit(&instance);
        unit_initialize(unit);
        Thread* thread = instance_add_thread(&instance);
        File* file = add_file(thread->arena, test_file_path);
        unit_file_analyze(thread, unit, file);
        print(strlit("[\x1b[32mOK\x1b[0m]\n"));
    }

    print(strlit("\x1b[32mTESTS SUCCEEDED!\x1b[0m\n"));
}
