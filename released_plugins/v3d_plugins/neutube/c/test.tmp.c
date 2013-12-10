# 1 "tz_utilities.c"
# 1 "<built-in>"
# 1 "<command-line>"
# 1 "tz_utilities.c"





# 1 "/usr/include/sys/time.h" 1 3 4
# 67 "/usr/include/sys/time.h" 3 4
# 1 "/usr/include/sys/cdefs.h" 1 3 4
# 417 "/usr/include/sys/cdefs.h" 3 4
# 1 "/usr/include/sys/_symbol_aliasing.h" 1 3 4
# 418 "/usr/include/sys/cdefs.h" 2 3 4
# 494 "/usr/include/sys/cdefs.h" 3 4
# 1 "/usr/include/sys/_posix_availability.h" 1 3 4
# 495 "/usr/include/sys/cdefs.h" 2 3 4
# 68 "/usr/include/sys/time.h" 2 3 4
# 1 "/usr/include/sys/_types.h" 1 3 4
# 33 "/usr/include/sys/_types.h" 3 4
# 1 "/usr/include/machine/_types.h" 1 3 4
# 32 "/usr/include/machine/_types.h" 3 4
# 1 "/usr/include/i386/_types.h" 1 3 4
# 37 "/usr/include/i386/_types.h" 3 4
typedef signed char __int8_t;



typedef unsigned char __uint8_t;
typedef short __int16_t;
typedef unsigned short __uint16_t;
typedef int __int32_t;
typedef unsigned int __uint32_t;
typedef long long __int64_t;
typedef unsigned long long __uint64_t;

typedef long __darwin_intptr_t;
typedef unsigned int __darwin_natural_t;
# 70 "/usr/include/i386/_types.h" 3 4
typedef int __darwin_ct_rune_t;





typedef union {
 char __mbstate8[128];
 long long _mbstateL;
} __mbstate_t;

typedef __mbstate_t __darwin_mbstate_t;


typedef long int __darwin_ptrdiff_t;





typedef long unsigned int __darwin_size_t;





typedef __builtin_va_list __darwin_va_list;





typedef int __darwin_wchar_t;




typedef __darwin_wchar_t __darwin_rune_t;


typedef int __darwin_wint_t;




typedef unsigned long __darwin_clock_t;
typedef __uint32_t __darwin_socklen_t;
typedef long __darwin_ssize_t;
typedef long __darwin_time_t;
# 33 "/usr/include/machine/_types.h" 2 3 4
# 34 "/usr/include/sys/_types.h" 2 3 4
# 58 "/usr/include/sys/_types.h" 3 4
struct __darwin_pthread_handler_rec
{
 void (*__routine)(void *);
 void *__arg;
 struct __darwin_pthread_handler_rec *__next;
};
struct _opaque_pthread_attr_t { long __sig; char __opaque[56]; };
struct _opaque_pthread_cond_t { long __sig; char __opaque[40]; };
struct _opaque_pthread_condattr_t { long __sig; char __opaque[8]; };
struct _opaque_pthread_mutex_t { long __sig; char __opaque[56]; };
struct _opaque_pthread_mutexattr_t { long __sig; char __opaque[8]; };
struct _opaque_pthread_once_t { long __sig; char __opaque[8]; };
struct _opaque_pthread_rwlock_t { long __sig; char __opaque[192]; };
struct _opaque_pthread_rwlockattr_t { long __sig; char __opaque[16]; };
struct _opaque_pthread_t { long __sig; struct __darwin_pthread_handler_rec *__cleanup_stack; char __opaque[1168]; };
# 94 "/usr/include/sys/_types.h" 3 4
typedef __int64_t __darwin_blkcnt_t;
typedef __int32_t __darwin_blksize_t;
typedef __int32_t __darwin_dev_t;
typedef unsigned int __darwin_fsblkcnt_t;
typedef unsigned int __darwin_fsfilcnt_t;
typedef __uint32_t __darwin_gid_t;
typedef __uint32_t __darwin_id_t;
typedef __uint64_t __darwin_ino64_t;

typedef __darwin_ino64_t __darwin_ino_t;



typedef __darwin_natural_t __darwin_mach_port_name_t;
typedef __darwin_mach_port_name_t __darwin_mach_port_t;
typedef __uint16_t __darwin_mode_t;
typedef __int64_t __darwin_off_t;
typedef __int32_t __darwin_pid_t;
typedef struct _opaque_pthread_attr_t
   __darwin_pthread_attr_t;
typedef struct _opaque_pthread_cond_t
   __darwin_pthread_cond_t;
typedef struct _opaque_pthread_condattr_t
   __darwin_pthread_condattr_t;
typedef unsigned long __darwin_pthread_key_t;
typedef struct _opaque_pthread_mutex_t
   __darwin_pthread_mutex_t;
typedef struct _opaque_pthread_mutexattr_t
   __darwin_pthread_mutexattr_t;
typedef struct _opaque_pthread_once_t
   __darwin_pthread_once_t;
typedef struct _opaque_pthread_rwlock_t
   __darwin_pthread_rwlock_t;
typedef struct _opaque_pthread_rwlockattr_t
   __darwin_pthread_rwlockattr_t;
typedef struct _opaque_pthread_t
   *__darwin_pthread_t;
typedef __uint32_t __darwin_sigset_t;
typedef __int32_t __darwin_suseconds_t;
typedef __uint32_t __darwin_uid_t;
typedef __uint32_t __darwin_useconds_t;
typedef unsigned char __darwin_uuid_t[16];
typedef char __darwin_uuid_string_t[37];
# 69 "/usr/include/sys/time.h" 2 3 4
# 1 "/usr/include/Availability.h" 1 3 4
# 141 "/usr/include/Availability.h" 3 4
# 1 "/usr/include/AvailabilityInternal.h" 1 3 4
# 142 "/usr/include/Availability.h" 2 3 4
# 70 "/usr/include/sys/time.h" 2 3 4
# 78 "/usr/include/sys/time.h" 3 4
# 1 "/usr/include/sys/_structs.h" 1 3 4
# 88 "/usr/include/sys/_structs.h" 3 4
struct timespec
{
 __darwin_time_t tv_sec;
 long tv_nsec;
};







struct timeval
{
 __darwin_time_t tv_sec;
 __darwin_suseconds_t tv_usec;
};
# 183 "/usr/include/sys/_structs.h" 3 4

typedef struct fd_set {
 __int32_t fds_bits[((((1024) % ((sizeof(__int32_t) * 8))) == 0) ? ((1024) / ((sizeof(__int32_t) * 8))) : (((1024) / ((sizeof(__int32_t) * 8))) + 1))];
} fd_set;



static __inline int
__darwin_fd_isset(int _n, const struct fd_set *_p)
{
 return (_p->fds_bits[_n/(sizeof(__int32_t) * 8)] & (1<<(_n % (sizeof(__int32_t) * 8))));
}
# 79 "/usr/include/sys/time.h" 2 3 4



typedef __darwin_time_t time_t;




typedef __darwin_suseconds_t suseconds_t;






struct itimerval {
 struct timeval it_interval;
 struct timeval it_value;
};
# 144 "/usr/include/sys/time.h" 3 4
struct timezone {
 int tz_minuteswest;
 int tz_dsttime;
};
# 187 "/usr/include/sys/time.h" 3 4
struct clockinfo {
 int hz;
 int tick;
 int tickadj;
 int stathz;
 int profhz;
};




# 1 "/usr/include/time.h" 1 3 4
# 66 "/usr/include/time.h" 3 4
# 1 "/usr/include/_types.h" 1 3 4
# 39 "/usr/include/_types.h" 3 4
typedef int __darwin_nl_item;
typedef int __darwin_wctrans_t;

typedef __uint32_t __darwin_wctype_t;
# 67 "/usr/include/time.h" 2 3 4


# 1 "/usr/include/_structs.h" 1 3 4
# 24 "/usr/include/_structs.h" 3 4
# 1 "/usr/include/sys/_structs.h" 1 3 4
# 25 "/usr/include/_structs.h" 2 3 4
# 70 "/usr/include/time.h" 2 3 4







typedef __darwin_clock_t clock_t;




typedef __darwin_size_t size_t;







struct tm {
 int tm_sec;
 int tm_min;
 int tm_hour;
 int tm_mday;
 int tm_mon;
 int tm_year;
 int tm_wday;
 int tm_yday;
 int tm_isdst;
 long tm_gmtoff;
 char *tm_zone;
};
# 113 "/usr/include/time.h" 3 4
extern char *tzname[];


extern int getdate_err;

extern long timezone __asm("_" "timezone" );

extern int daylight;


char *asctime(const struct tm *);
clock_t clock(void) __asm("_" "clock" );
char *ctime(const time_t *);
double difftime(time_t, time_t);
struct tm *getdate(const char *);
struct tm *gmtime(const time_t *);
struct tm *localtime(const time_t *);
time_t mktime(struct tm *) __asm("_" "mktime" );
size_t strftime(char * restrict, size_t, const char * restrict, const struct tm * restrict) __asm("_" "strftime" );
char *strptime(const char * restrict, const char * restrict, struct tm * restrict) __asm("_" "strptime" );
time_t time(time_t *);


void tzset(void);



char *asctime_r(const struct tm * restrict, char * restrict);
char *ctime_r(const time_t *, char *);
struct tm *gmtime_r(const time_t * restrict, struct tm * restrict);
struct tm *localtime_r(const time_t * restrict, struct tm * restrict);


time_t posix2time(time_t);



void tzsetwall(void);
time_t time2posix(time_t);
time_t timelocal(struct tm * const);
time_t timegm(struct tm * const);



int nanosleep(const struct timespec *, struct timespec *) __asm("_" "nanosleep" );


# 199 "/usr/include/sys/time.h" 2 3 4





int adjtime(const struct timeval *, struct timeval *);
int futimes(int, const struct timeval *);
int lutimes(const char *, const struct timeval *) __attribute__((visibility("default")));
int settimeofday(const struct timeval *, const struct timezone *);


int getitimer(int, struct itimerval *);
int gettimeofday(struct timeval * restrict, void * restrict);

# 1 "/usr/include/sys/_select.h" 1 3 4
# 39 "/usr/include/sys/_select.h" 3 4
int select(int, fd_set * restrict, fd_set * restrict,
  fd_set * restrict, struct timeval * restrict)




  __asm("_" "select" "$1050")




  ;
# 214 "/usr/include/sys/time.h" 2 3 4

int setitimer(int, const struct itimerval * restrict,
  struct itimerval * restrict);
int utimes(const char *, const struct timeval *);


# 7 "tz_utilities.c" 2

# 1 "/usr/include/string.h" 1 3 4
# 79 "/usr/include/string.h" 3 4

void *memchr(const void *, int, size_t);
int memcmp(const void *, const void *, size_t);
void *memcpy(void *, const void *, size_t);
void *memmove(void *, const void *, size_t);
void *memset(void *, int, size_t);
char *strcat(char *, const char *);
char *strchr(const char *, int);
int strcmp(const char *, const char *);
int strcoll(const char *, const char *);
char *strcpy(char *, const char *);
size_t strcspn(const char *, const char *);
char *strerror(int) __asm("_" "strerror" );
size_t strlen(const char *);
char *strncat(char *, const char *, size_t);
int strncmp(const char *, const char *, size_t);
char *strncpy(char *, const char *, size_t);
char *strpbrk(const char *, const char *);
char *strrchr(const char *, int);
size_t strspn(const char *, const char *);
char *strstr(const char *, const char *);
char *strtok(char *, const char *);
size_t strxfrm(char *, const char *, size_t);

# 113 "/usr/include/string.h" 3 4

char *strtok_r(char *, const char *, char **);

# 125 "/usr/include/string.h" 3 4

int strerror_r(int, char *, size_t);
char *strdup(const char *);
void *memccpy(void *, const void *, int, size_t);

# 139 "/usr/include/string.h" 3 4

char *stpcpy(char *, const char *);
char *stpncpy(char *, const char *, size_t) __attribute__((visibility("default")));
char *strndup(const char *, size_t) __attribute__((visibility("default")));
size_t strnlen(const char *, size_t) __attribute__((visibility("default")));
char *strsignal(int sig);

# 155 "/usr/include/string.h" 3 4
typedef __darwin_ssize_t ssize_t;



void *memmem(const void *, size_t, const void *, size_t) __attribute__((visibility("default")));
void memset_pattern4(void *, const void *, size_t) __attribute__((visibility("default")));
void memset_pattern8(void *, const void *, size_t) __attribute__((visibility("default")));
void memset_pattern16(void *, const void *, size_t) __attribute__((visibility("default")));

char *strcasestr(const char *, const char *);
char *strnstr(const char *, const char *, size_t);
size_t strlcat(char *, const char *, size_t);
size_t strlcpy(char *, const char *, size_t);
void strmode(int, char *);
char *strsep(char **, const char *);


void swab(const void * restrict, void * restrict, ssize_t);







# 1 "/usr/include/strings.h" 1 3 4
# 71 "/usr/include/strings.h" 3 4



int bcmp(const void *, const void *, size_t) ;
void bcopy(const void *, void *, size_t) ;
void bzero(void *, size_t) ;
char *index(const char *, int) ;
char *rindex(const char *, int) ;


int ffs(int);
int strcasecmp(const char *, const char *);
int strncasecmp(const char *, const char *, size_t);





int ffsl(long) __attribute__((visibility("default")));
int fls(int) __attribute__((visibility("default")));
int flsl(long) __attribute__((visibility("default")));


# 1 "/usr/include/string.h" 1 3 4
# 95 "/usr/include/strings.h" 2 3 4
# 181 "/usr/include/string.h" 2 3 4
# 190 "/usr/include/string.h" 3 4
# 1 "/usr/include/secure/_string.h" 1 3 4
# 32 "/usr/include/secure/_string.h" 3 4
# 1 "/usr/include/secure/_common.h" 1 3 4
# 33 "/usr/include/secure/_string.h" 2 3 4
# 58 "/usr/include/secure/_string.h" 3 4
static __inline void *
__inline_memcpy_chk (void *__dest, const void *__src, size_t __len)
{
  return __builtin___memcpy_chk (__dest, __src, __len, __builtin_object_size (__dest, 0));
}






static __inline void *
__inline_memmove_chk (void *__dest, const void *__src, size_t __len)
{
  return __builtin___memmove_chk (__dest, __src, __len, __builtin_object_size (__dest, 0));
}






static __inline void *
__inline_memset_chk (void *__dest, int __val, size_t __len)
{
  return __builtin___memset_chk (__dest, __val, __len, __builtin_object_size (__dest, 0));
}






static __inline char *
__inline_strcpy_chk (char *restrict __dest, const char *restrict __src)
{
  return __builtin___strcpy_chk (__dest, __src, __builtin_object_size (__dest, 2 > 1));
}







static __inline char *
__inline_stpcpy_chk (char *__dest, const char *__src)
{
  return __builtin___stpcpy_chk (__dest, __src, __builtin_object_size (__dest, 2 > 1));
}






static __inline char *
__inline_stpncpy_chk (char *restrict __dest, const char *restrict __src,
        size_t __len)
{
  return __builtin___stpncpy_chk (__dest, __src, __len, __builtin_object_size (__dest, 2 > 1));
}







static __inline char *
__inline_strncpy_chk (char *restrict __dest, const char *restrict __src,
        size_t __len)
{
  return __builtin___strncpy_chk (__dest, __src, __len, __builtin_object_size (__dest, 2 > 1));
}






static __inline char *
__inline_strcat_chk (char *restrict __dest, const char *restrict __src)
{
  return __builtin___strcat_chk (__dest, __src, __builtin_object_size (__dest, 2 > 1));
}







static __inline char *
__inline_strncat_chk (char *restrict __dest, const char *restrict __src,
        size_t __len)
{
  return __builtin___strncat_chk (__dest, __src, __len, __builtin_object_size (__dest, 2 > 1));
}
# 191 "/usr/include/string.h" 2 3 4
# 9 "tz_utilities.c" 2
# 1 "/usr/include/stdio.h" 1 3 4
# 73 "/usr/include/stdio.h" 3 4
typedef __darwin_va_list va_list;
# 85 "/usr/include/stdio.h" 3 4
typedef __darwin_off_t fpos_t;
# 96 "/usr/include/stdio.h" 3 4
struct __sbuf {
 unsigned char *_base;
 int _size;
};


struct __sFILEX;
# 130 "/usr/include/stdio.h" 3 4
typedef struct __sFILE {
 unsigned char *_p;
 int _r;
 int _w;
 short _flags;
 short _file;
 struct __sbuf _bf;
 int _lbfsize;


 void *_cookie;
 int (*_close)(void *);
 int (*_read) (void *, char *, int);
 fpos_t (*_seek) (void *, fpos_t, int);
 int (*_write)(void *, const char *, int);


 struct __sbuf _ub;
 struct __sFILEX *_extra;
 int _ur;


 unsigned char _ubuf[3];
 unsigned char _nbuf[1];


 struct __sbuf _lb;


 int _blksize;
 fpos_t _offset;
} FILE;


extern FILE *__stdinp;
extern FILE *__stdoutp;
extern FILE *__stderrp;

# 238 "/usr/include/stdio.h" 3 4

void clearerr(FILE *);
int fclose(FILE *);
int feof(FILE *);
int ferror(FILE *);
int fflush(FILE *);
int fgetc(FILE *);
int fgetpos(FILE * restrict, fpos_t *);
char *fgets(char * restrict, int, FILE *);



FILE *fopen(const char * restrict, const char * restrict) __asm("_" "fopen" );

int fprintf(FILE * restrict, const char * restrict, ...) __attribute__((__format__ (__printf__, 2, 3)));
int fputc(int, FILE *);
int fputs(const char * restrict, FILE * restrict) __asm("_" "fputs" );
size_t fread(void * restrict, size_t, size_t, FILE * restrict);
FILE *freopen(const char * restrict, const char * restrict,
                 FILE * restrict) __asm("_" "freopen" );
int fscanf(FILE * restrict, const char * restrict, ...) __attribute__((__format__ (__scanf__, 2, 3)));
int fseek(FILE *, long, int);
int fsetpos(FILE *, const fpos_t *);
long ftell(FILE *);
size_t fwrite(const void * restrict, size_t, size_t, FILE * restrict) __asm("_" "fwrite" );
int getc(FILE *);
int getchar(void);
char *gets(char *);
void perror(const char *);
int printf(const char * restrict, ...) __attribute__((__format__ (__printf__, 1, 2)));
int putc(int, FILE *);
int putchar(int);
int puts(const char *);
int remove(const char *);
int rename (const char *, const char *);
void rewind(FILE *);
int scanf(const char * restrict, ...) __attribute__((__format__ (__scanf__, 1, 2)));
void setbuf(FILE * restrict, char * restrict);
int setvbuf(FILE * restrict, char * restrict, int, size_t);
int sprintf(char * restrict, const char * restrict, ...) __attribute__((__format__ (__printf__, 2, 3)));
int sscanf(const char * restrict, const char * restrict, ...) __attribute__((__format__ (__scanf__, 2, 3)));
FILE *tmpfile(void);
char *tmpnam(char *);
int ungetc(int, FILE *);
int vfprintf(FILE * restrict, const char * restrict, va_list) __attribute__((__format__ (__printf__, 2, 0)));
int vprintf(const char * restrict, va_list) __attribute__((__format__ (__printf__, 1, 0)));
int vsprintf(char * restrict, const char * restrict, va_list) __attribute__((__format__ (__printf__, 2, 0)));

# 296 "/usr/include/stdio.h" 3 4




char *ctermid(char *);





FILE *fdopen(int, const char *) __asm("_" "fdopen" );

int fileno(FILE *);

# 318 "/usr/include/stdio.h" 3 4

int pclose(FILE *);



FILE *popen(const char *, const char *) __asm("_" "popen" );


# 340 "/usr/include/stdio.h" 3 4

int __srget(FILE *);
int __svfscanf(FILE *, const char *, va_list) __attribute__((__format__ (__scanf__, 2, 0)));
int __swbuf(int, FILE *);








static __inline int __sputc(int _c, FILE *_p) {
 if (--_p->_w >= 0 || (_p->_w >= _p->_lbfsize && (char)_c != '\n'))
  return (*_p->_p++ = _c);
 else
  return (__swbuf(_c, _p));
}
# 377 "/usr/include/stdio.h" 3 4

void flockfile(FILE *);
int ftrylockfile(FILE *);
void funlockfile(FILE *);
int getc_unlocked(FILE *);
int getchar_unlocked(void);
int putc_unlocked(int, FILE *);
int putchar_unlocked(int);



int getw(FILE *);
int putw(int, FILE *);


char *tempnam(const char *, const char *) __asm("_" "tempnam" );

# 414 "/usr/include/stdio.h" 3 4
typedef __darwin_off_t off_t;



int fseeko(FILE *, off_t, int);
off_t ftello(FILE *);





int snprintf(char * restrict, size_t, const char * restrict, ...) __attribute__((__format__ (__printf__, 3, 4)));
int vfscanf(FILE * restrict, const char * restrict, va_list) __attribute__((__format__ (__scanf__, 2, 0)));
int vscanf(const char * restrict, va_list) __attribute__((__format__ (__scanf__, 1, 0)));
int vsnprintf(char * restrict, size_t, const char * restrict, va_list) __attribute__((__format__ (__printf__, 3, 0)));
int vsscanf(const char * restrict, const char * restrict, va_list) __attribute__((__format__ (__scanf__, 2, 0)));

# 445 "/usr/include/stdio.h" 3 4

int dprintf(int, const char * restrict, ...) __attribute__((__format__ (__printf__, 2, 3))) __attribute__((visibility("default")));
int vdprintf(int, const char * restrict, va_list) __attribute__((__format__ (__printf__, 2, 0))) __attribute__((visibility("default")));
ssize_t getdelim(char ** restrict, size_t * restrict, int, FILE * restrict) __attribute__((visibility("default")));
ssize_t getline(char ** restrict, size_t * restrict, FILE * restrict) __attribute__((visibility("default")));









extern const int sys_nerr;
extern const char *const sys_errlist[];

int asprintf(char **, const char *, ...) __attribute__((__format__ (__printf__, 2, 3)));
char *ctermid_r(char *);
char *fgetln(FILE *, size_t *);
const char *fmtcheck(const char *, const char *);
int fpurge(FILE *);
void setbuffer(FILE *, char *, int);
int setlinebuf(FILE *);
int vasprintf(char **, const char *, va_list) __attribute__((__format__ (__printf__, 2, 0)));
FILE *zopen(const char *, const char *, int);





FILE *funopen(const void *,
                 int (*)(void *, char *, int),
                 int (*)(void *, const char *, int),
                 fpos_t (*)(void *, fpos_t, int),
                 int (*)(void *));

# 499 "/usr/include/stdio.h" 3 4
# 1 "/usr/include/secure/_stdio.h" 1 3 4
# 45 "/usr/include/secure/_stdio.h" 3 4
extern int __sprintf_chk (char * restrict, int, size_t,
     const char * restrict, ...)
  ;




extern int __snprintf_chk (char * restrict, size_t, int, size_t,
      const char * restrict, ...)
  ;





extern int __vsprintf_chk (char * restrict, int, size_t,
      const char * restrict, va_list)
  ;




extern int __vsnprintf_chk (char * restrict, size_t, int, size_t,
       const char * restrict, va_list)
  ;
# 500 "/usr/include/stdio.h" 2 3 4
# 10 "tz_utilities.c" 2
# 1 "/usr/include/stdlib.h" 1 3 4
# 65 "/usr/include/stdlib.h" 3 4
# 1 "/usr/include/sys/wait.h" 1 3 4
# 79 "/usr/include/sys/wait.h" 3 4
typedef enum {
 P_ALL,
 P_PID,
 P_PGID
} idtype_t;






typedef __darwin_pid_t pid_t;




typedef __darwin_id_t id_t;
# 116 "/usr/include/sys/wait.h" 3 4
# 1 "/usr/include/sys/signal.h" 1 3 4
# 73 "/usr/include/sys/signal.h" 3 4
# 1 "/usr/include/sys/appleapiopts.h" 1 3 4
# 74 "/usr/include/sys/signal.h" 2 3 4







# 1 "/usr/include/machine/signal.h" 1 3 4
# 32 "/usr/include/machine/signal.h" 3 4
# 1 "/usr/include/i386/signal.h" 1 3 4
# 39 "/usr/include/i386/signal.h" 3 4
typedef int sig_atomic_t;
# 55 "/usr/include/i386/signal.h" 3 4
# 1 "/usr/include/i386/_structs.h" 1 3 4
# 56 "/usr/include/i386/signal.h" 2 3 4
# 33 "/usr/include/machine/signal.h" 2 3 4
# 82 "/usr/include/sys/signal.h" 2 3 4
# 148 "/usr/include/sys/signal.h" 3 4
# 1 "/usr/include/sys/_structs.h" 1 3 4
# 57 "/usr/include/sys/_structs.h" 3 4
# 1 "/usr/include/machine/_structs.h" 1 3 4
# 29 "/usr/include/machine/_structs.h" 3 4
# 1 "/usr/include/i386/_structs.h" 1 3 4
# 38 "/usr/include/i386/_structs.h" 3 4
# 1 "/usr/include/mach/i386/_structs.h" 1 3 4
# 43 "/usr/include/mach/i386/_structs.h" 3 4
struct __darwin_i386_thread_state
{
    unsigned int __eax;
    unsigned int __ebx;
    unsigned int __ecx;
    unsigned int __edx;
    unsigned int __edi;
    unsigned int __esi;
    unsigned int __ebp;
    unsigned int __esp;
    unsigned int __ss;
    unsigned int __eflags;
    unsigned int __eip;
    unsigned int __cs;
    unsigned int __ds;
    unsigned int __es;
    unsigned int __fs;
    unsigned int __gs;
};
# 89 "/usr/include/mach/i386/_structs.h" 3 4
struct __darwin_fp_control
{
    unsigned short __invalid :1,
        __denorm :1,
    __zdiv :1,
    __ovrfl :1,
    __undfl :1,
    __precis :1,
      :2,
    __pc :2,





    __rc :2,






             :1,
      :3;
};
typedef struct __darwin_fp_control __darwin_fp_control_t;
# 147 "/usr/include/mach/i386/_structs.h" 3 4
struct __darwin_fp_status
{
    unsigned short __invalid :1,
        __denorm :1,
    __zdiv :1,
    __ovrfl :1,
    __undfl :1,
    __precis :1,
    __stkflt :1,
    __errsumm :1,
    __c0 :1,
    __c1 :1,
    __c2 :1,
    __tos :3,
    __c3 :1,
    __busy :1;
};
typedef struct __darwin_fp_status __darwin_fp_status_t;
# 191 "/usr/include/mach/i386/_structs.h" 3 4
struct __darwin_mmst_reg
{
 char __mmst_reg[10];
 char __mmst_rsrv[6];
};
# 210 "/usr/include/mach/i386/_structs.h" 3 4
struct __darwin_xmm_reg
{
 char __xmm_reg[16];
};
# 232 "/usr/include/mach/i386/_structs.h" 3 4
struct __darwin_i386_float_state
{
 int __fpu_reserved[2];
 struct __darwin_fp_control __fpu_fcw;
 struct __darwin_fp_status __fpu_fsw;
 __uint8_t __fpu_ftw;
 __uint8_t __fpu_rsrv1;
 __uint16_t __fpu_fop;
 __uint32_t __fpu_ip;
 __uint16_t __fpu_cs;
 __uint16_t __fpu_rsrv2;
 __uint32_t __fpu_dp;
 __uint16_t __fpu_ds;
 __uint16_t __fpu_rsrv3;
 __uint32_t __fpu_mxcsr;
 __uint32_t __fpu_mxcsrmask;
 struct __darwin_mmst_reg __fpu_stmm0;
 struct __darwin_mmst_reg __fpu_stmm1;
 struct __darwin_mmst_reg __fpu_stmm2;
 struct __darwin_mmst_reg __fpu_stmm3;
 struct __darwin_mmst_reg __fpu_stmm4;
 struct __darwin_mmst_reg __fpu_stmm5;
 struct __darwin_mmst_reg __fpu_stmm6;
 struct __darwin_mmst_reg __fpu_stmm7;
 struct __darwin_xmm_reg __fpu_xmm0;
 struct __darwin_xmm_reg __fpu_xmm1;
 struct __darwin_xmm_reg __fpu_xmm2;
 struct __darwin_xmm_reg __fpu_xmm3;
 struct __darwin_xmm_reg __fpu_xmm4;
 struct __darwin_xmm_reg __fpu_xmm5;
 struct __darwin_xmm_reg __fpu_xmm6;
 struct __darwin_xmm_reg __fpu_xmm7;
 char __fpu_rsrv4[14*16];
 int __fpu_reserved1;
};


struct __darwin_i386_avx_state
{
 int __fpu_reserved[2];
 struct __darwin_fp_control __fpu_fcw;
 struct __darwin_fp_status __fpu_fsw;
 __uint8_t __fpu_ftw;
 __uint8_t __fpu_rsrv1;
 __uint16_t __fpu_fop;
 __uint32_t __fpu_ip;
 __uint16_t __fpu_cs;
 __uint16_t __fpu_rsrv2;
 __uint32_t __fpu_dp;
 __uint16_t __fpu_ds;
 __uint16_t __fpu_rsrv3;
 __uint32_t __fpu_mxcsr;
 __uint32_t __fpu_mxcsrmask;
 struct __darwin_mmst_reg __fpu_stmm0;
 struct __darwin_mmst_reg __fpu_stmm1;
 struct __darwin_mmst_reg __fpu_stmm2;
 struct __darwin_mmst_reg __fpu_stmm3;
 struct __darwin_mmst_reg __fpu_stmm4;
 struct __darwin_mmst_reg __fpu_stmm5;
 struct __darwin_mmst_reg __fpu_stmm6;
 struct __darwin_mmst_reg __fpu_stmm7;
 struct __darwin_xmm_reg __fpu_xmm0;
 struct __darwin_xmm_reg __fpu_xmm1;
 struct __darwin_xmm_reg __fpu_xmm2;
 struct __darwin_xmm_reg __fpu_xmm3;
 struct __darwin_xmm_reg __fpu_xmm4;
 struct __darwin_xmm_reg __fpu_xmm5;
 struct __darwin_xmm_reg __fpu_xmm6;
 struct __darwin_xmm_reg __fpu_xmm7;
 char __fpu_rsrv4[14*16];
 int __fpu_reserved1;
 char __avx_reserved1[64];
 struct __darwin_xmm_reg __fpu_ymmh0;
 struct __darwin_xmm_reg __fpu_ymmh1;
 struct __darwin_xmm_reg __fpu_ymmh2;
 struct __darwin_xmm_reg __fpu_ymmh3;
 struct __darwin_xmm_reg __fpu_ymmh4;
 struct __darwin_xmm_reg __fpu_ymmh5;
 struct __darwin_xmm_reg __fpu_ymmh6;
 struct __darwin_xmm_reg __fpu_ymmh7;
};
# 402 "/usr/include/mach/i386/_structs.h" 3 4
struct __darwin_i386_exception_state
{
 __uint16_t __trapno;
 __uint16_t __cpu;
 __uint32_t __err;
 __uint32_t __faultvaddr;
};
# 422 "/usr/include/mach/i386/_structs.h" 3 4
struct __darwin_x86_debug_state32
{
 unsigned int __dr0;
 unsigned int __dr1;
 unsigned int __dr2;
 unsigned int __dr3;
 unsigned int __dr4;
 unsigned int __dr5;
 unsigned int __dr6;
 unsigned int __dr7;
};
# 454 "/usr/include/mach/i386/_structs.h" 3 4
struct __darwin_x86_thread_state64
{
 __uint64_t __rax;
 __uint64_t __rbx;
 __uint64_t __rcx;
 __uint64_t __rdx;
 __uint64_t __rdi;
 __uint64_t __rsi;
 __uint64_t __rbp;
 __uint64_t __rsp;
 __uint64_t __r8;
 __uint64_t __r9;
 __uint64_t __r10;
 __uint64_t __r11;
 __uint64_t __r12;
 __uint64_t __r13;
 __uint64_t __r14;
 __uint64_t __r15;
 __uint64_t __rip;
 __uint64_t __rflags;
 __uint64_t __cs;
 __uint64_t __fs;
 __uint64_t __gs;
};
# 509 "/usr/include/mach/i386/_structs.h" 3 4
struct __darwin_x86_float_state64
{
 int __fpu_reserved[2];
 struct __darwin_fp_control __fpu_fcw;
 struct __darwin_fp_status __fpu_fsw;
 __uint8_t __fpu_ftw;
 __uint8_t __fpu_rsrv1;
 __uint16_t __fpu_fop;


 __uint32_t __fpu_ip;
 __uint16_t __fpu_cs;

 __uint16_t __fpu_rsrv2;


 __uint32_t __fpu_dp;
 __uint16_t __fpu_ds;

 __uint16_t __fpu_rsrv3;
 __uint32_t __fpu_mxcsr;
 __uint32_t __fpu_mxcsrmask;
 struct __darwin_mmst_reg __fpu_stmm0;
 struct __darwin_mmst_reg __fpu_stmm1;
 struct __darwin_mmst_reg __fpu_stmm2;
 struct __darwin_mmst_reg __fpu_stmm3;
 struct __darwin_mmst_reg __fpu_stmm4;
 struct __darwin_mmst_reg __fpu_stmm5;
 struct __darwin_mmst_reg __fpu_stmm6;
 struct __darwin_mmst_reg __fpu_stmm7;
 struct __darwin_xmm_reg __fpu_xmm0;
 struct __darwin_xmm_reg __fpu_xmm1;
 struct __darwin_xmm_reg __fpu_xmm2;
 struct __darwin_xmm_reg __fpu_xmm3;
 struct __darwin_xmm_reg __fpu_xmm4;
 struct __darwin_xmm_reg __fpu_xmm5;
 struct __darwin_xmm_reg __fpu_xmm6;
 struct __darwin_xmm_reg __fpu_xmm7;
 struct __darwin_xmm_reg __fpu_xmm8;
 struct __darwin_xmm_reg __fpu_xmm9;
 struct __darwin_xmm_reg __fpu_xmm10;
 struct __darwin_xmm_reg __fpu_xmm11;
 struct __darwin_xmm_reg __fpu_xmm12;
 struct __darwin_xmm_reg __fpu_xmm13;
 struct __darwin_xmm_reg __fpu_xmm14;
 struct __darwin_xmm_reg __fpu_xmm15;
 char __fpu_rsrv4[6*16];
 int __fpu_reserved1;
};


struct __darwin_x86_avx_state64
{
 int __fpu_reserved[2];
 struct __darwin_fp_control __fpu_fcw;
 struct __darwin_fp_status __fpu_fsw;
 __uint8_t __fpu_ftw;
 __uint8_t __fpu_rsrv1;
 __uint16_t __fpu_fop;


 __uint32_t __fpu_ip;
 __uint16_t __fpu_cs;

 __uint16_t __fpu_rsrv2;


 __uint32_t __fpu_dp;
 __uint16_t __fpu_ds;

 __uint16_t __fpu_rsrv3;
 __uint32_t __fpu_mxcsr;
 __uint32_t __fpu_mxcsrmask;
 struct __darwin_mmst_reg __fpu_stmm0;
 struct __darwin_mmst_reg __fpu_stmm1;
 struct __darwin_mmst_reg __fpu_stmm2;
 struct __darwin_mmst_reg __fpu_stmm3;
 struct __darwin_mmst_reg __fpu_stmm4;
 struct __darwin_mmst_reg __fpu_stmm5;
 struct __darwin_mmst_reg __fpu_stmm6;
 struct __darwin_mmst_reg __fpu_stmm7;
 struct __darwin_xmm_reg __fpu_xmm0;
 struct __darwin_xmm_reg __fpu_xmm1;
 struct __darwin_xmm_reg __fpu_xmm2;
 struct __darwin_xmm_reg __fpu_xmm3;
 struct __darwin_xmm_reg __fpu_xmm4;
 struct __darwin_xmm_reg __fpu_xmm5;
 struct __darwin_xmm_reg __fpu_xmm6;
 struct __darwin_xmm_reg __fpu_xmm7;
 struct __darwin_xmm_reg __fpu_xmm8;
 struct __darwin_xmm_reg __fpu_xmm9;
 struct __darwin_xmm_reg __fpu_xmm10;
 struct __darwin_xmm_reg __fpu_xmm11;
 struct __darwin_xmm_reg __fpu_xmm12;
 struct __darwin_xmm_reg __fpu_xmm13;
 struct __darwin_xmm_reg __fpu_xmm14;
 struct __darwin_xmm_reg __fpu_xmm15;
 char __fpu_rsrv4[6*16];
 int __fpu_reserved1;
 char __avx_reserved1[64];
 struct __darwin_xmm_reg __fpu_ymmh0;
 struct __darwin_xmm_reg __fpu_ymmh1;
 struct __darwin_xmm_reg __fpu_ymmh2;
 struct __darwin_xmm_reg __fpu_ymmh3;
 struct __darwin_xmm_reg __fpu_ymmh4;
 struct __darwin_xmm_reg __fpu_ymmh5;
 struct __darwin_xmm_reg __fpu_ymmh6;
 struct __darwin_xmm_reg __fpu_ymmh7;
 struct __darwin_xmm_reg __fpu_ymmh8;
 struct __darwin_xmm_reg __fpu_ymmh9;
 struct __darwin_xmm_reg __fpu_ymmh10;
 struct __darwin_xmm_reg __fpu_ymmh11;
 struct __darwin_xmm_reg __fpu_ymmh12;
 struct __darwin_xmm_reg __fpu_ymmh13;
 struct __darwin_xmm_reg __fpu_ymmh14;
 struct __darwin_xmm_reg __fpu_ymmh15;
};
# 751 "/usr/include/mach/i386/_structs.h" 3 4
struct __darwin_x86_exception_state64
{
    __uint16_t __trapno;
    __uint16_t __cpu;
    __uint32_t __err;
    __uint64_t __faultvaddr;
};
# 771 "/usr/include/mach/i386/_structs.h" 3 4
struct __darwin_x86_debug_state64
{
 __uint64_t __dr0;
 __uint64_t __dr1;
 __uint64_t __dr2;
 __uint64_t __dr3;
 __uint64_t __dr4;
 __uint64_t __dr5;
 __uint64_t __dr6;
 __uint64_t __dr7;
};
# 39 "/usr/include/i386/_structs.h" 2 3 4
# 48 "/usr/include/i386/_structs.h" 3 4
struct __darwin_mcontext32
{
 struct __darwin_i386_exception_state __es;
 struct __darwin_i386_thread_state __ss;
 struct __darwin_i386_float_state __fs;
};


struct __darwin_mcontext_avx32
{
 struct __darwin_i386_exception_state __es;
 struct __darwin_i386_thread_state __ss;
 struct __darwin_i386_avx_state __fs;
};
# 86 "/usr/include/i386/_structs.h" 3 4
struct __darwin_mcontext64
{
 struct __darwin_x86_exception_state64 __es;
 struct __darwin_x86_thread_state64 __ss;
 struct __darwin_x86_float_state64 __fs;
};


struct __darwin_mcontext_avx64
{
 struct __darwin_x86_exception_state64 __es;
 struct __darwin_x86_thread_state64 __ss;
 struct __darwin_x86_avx_state64 __fs;
};
# 127 "/usr/include/i386/_structs.h" 3 4
typedef struct __darwin_mcontext64 *mcontext_t;
# 30 "/usr/include/machine/_structs.h" 2 3 4
# 58 "/usr/include/sys/_structs.h" 2 3 4
# 75 "/usr/include/sys/_structs.h" 3 4
struct __darwin_sigaltstack
{
 void *ss_sp;
 __darwin_size_t ss_size;
 int ss_flags;
};
# 128 "/usr/include/sys/_structs.h" 3 4
struct __darwin_ucontext
{
 int uc_onstack;
 __darwin_sigset_t uc_sigmask;
 struct __darwin_sigaltstack uc_stack;
 struct __darwin_ucontext *uc_link;
 __darwin_size_t uc_mcsize;
 struct __darwin_mcontext64 *uc_mcontext;



};
# 218 "/usr/include/sys/_structs.h" 3 4
typedef struct __darwin_sigaltstack stack_t;
# 227 "/usr/include/sys/_structs.h" 3 4
typedef struct __darwin_ucontext ucontext_t;
# 149 "/usr/include/sys/signal.h" 2 3 4
# 157 "/usr/include/sys/signal.h" 3 4
typedef __darwin_pthread_attr_t pthread_attr_t;




typedef __darwin_sigset_t sigset_t;
# 172 "/usr/include/sys/signal.h" 3 4
typedef __darwin_uid_t uid_t;


union sigval {

 int sival_int;
 void *sival_ptr;
};





struct sigevent {
 int sigev_notify;
 int sigev_signo;
 union sigval sigev_value;
 void (*sigev_notify_function)(union sigval);
 pthread_attr_t *sigev_notify_attributes;
};


typedef struct __siginfo {
 int si_signo;
 int si_errno;
 int si_code;
 pid_t si_pid;
 uid_t si_uid;
 int si_status;
 void *si_addr;
 union sigval si_value;
 long si_band;
 unsigned long __pad[7];
} siginfo_t;
# 286 "/usr/include/sys/signal.h" 3 4
union __sigaction_u {
 void (*__sa_handler)(int);
 void (*__sa_sigaction)(int, struct __siginfo *,
         void *);
};


struct __sigaction {
 union __sigaction_u __sigaction_u;
 void (*sa_tramp)(void *, int, int, siginfo_t *, void *);
 sigset_t sa_mask;
 int sa_flags;
};




struct sigaction {
 union __sigaction_u __sigaction_u;
 sigset_t sa_mask;
 int sa_flags;
};
# 348 "/usr/include/sys/signal.h" 3 4
typedef void (*sig_t)(int);
# 365 "/usr/include/sys/signal.h" 3 4
struct sigvec {
 void (*sv_handler)(int);
 int sv_mask;
 int sv_flags;
};
# 384 "/usr/include/sys/signal.h" 3 4
struct sigstack {
 char *ss_sp;
 int ss_onstack;
};
# 406 "/usr/include/sys/signal.h" 3 4

void (*signal(int, void (*)(int)))(int);

# 117 "/usr/include/sys/wait.h" 2 3 4
# 1 "/usr/include/sys/resource.h" 1 3 4
# 77 "/usr/include/sys/resource.h" 3 4
# 1 "/usr/include/sys/_structs.h" 1 3 4
# 78 "/usr/include/sys/resource.h" 2 3 4
# 89 "/usr/include/sys/resource.h" 3 4
typedef __uint64_t rlim_t;
# 151 "/usr/include/sys/resource.h" 3 4
struct rusage {
 struct timeval ru_utime;
 struct timeval ru_stime;
# 162 "/usr/include/sys/resource.h" 3 4
 long ru_maxrss;

 long ru_ixrss;
 long ru_idrss;
 long ru_isrss;
 long ru_minflt;
 long ru_majflt;
 long ru_nswap;
 long ru_inblock;
 long ru_oublock;
 long ru_msgsnd;
 long ru_msgrcv;
 long ru_nsignals;
 long ru_nvcsw;
 long ru_nivcsw;


};
# 222 "/usr/include/sys/resource.h" 3 4
struct rlimit {
 rlim_t rlim_cur;
 rlim_t rlim_max;
};
# 244 "/usr/include/sys/resource.h" 3 4

int getpriority(int, id_t);

int getiopolicy_np(int, int) __attribute__((visibility("default")));

int getrlimit(int, struct rlimit *) __asm("_" "getrlimit" );
int getrusage(int, struct rusage *);
int setpriority(int, id_t, int);

int setiopolicy_np(int, int, int) __attribute__((visibility("default")));

int setrlimit(int, const struct rlimit *) __asm("_" "setrlimit" );

# 118 "/usr/include/sys/wait.h" 2 3 4
# 193 "/usr/include/sys/wait.h" 3 4
# 1 "/usr/include/machine/endian.h" 1 3 4
# 35 "/usr/include/machine/endian.h" 3 4
# 1 "/usr/include/i386/endian.h" 1 3 4
# 99 "/usr/include/i386/endian.h" 3 4
# 1 "/usr/include/sys/_endian.h" 1 3 4
# 124 "/usr/include/sys/_endian.h" 3 4
# 1 "/usr/include/libkern/_OSByteOrder.h" 1 3 4
# 66 "/usr/include/libkern/_OSByteOrder.h" 3 4
# 1 "/usr/include/libkern/i386/_OSByteOrder.h" 1 3 4
# 44 "/usr/include/libkern/i386/_OSByteOrder.h" 3 4
static inline
__uint16_t
_OSSwapInt16(
    __uint16_t _data
)
{
    return ((_data << 8) | (_data >> 8));
}

static inline
__uint32_t
_OSSwapInt32(
    __uint32_t _data
)
{

    return __builtin_bswap32(_data);




}


static inline
__uint64_t
_OSSwapInt64(
    __uint64_t _data
)
{
    return __builtin_bswap64(_data);
}
# 67 "/usr/include/libkern/_OSByteOrder.h" 2 3 4
# 125 "/usr/include/sys/_endian.h" 2 3 4
# 100 "/usr/include/i386/endian.h" 2 3 4
# 36 "/usr/include/machine/endian.h" 2 3 4
# 194 "/usr/include/sys/wait.h" 2 3 4







union wait {
 int w_status;



 struct {

  unsigned int w_Termsig:7,
    w_Coredump:1,
    w_Retcode:8,
    w_Filler:16;







 } w_T;





 struct {

  unsigned int w_Stopval:8,
    w_Stopsig:8,
    w_Filler:16;






 } w_S;
};
# 254 "/usr/include/sys/wait.h" 3 4

pid_t wait(int *) __asm("_" "wait" );
pid_t waitpid(pid_t, int *, int) __asm("_" "waitpid" );

int waitid(idtype_t, id_t, siginfo_t *, int) __asm("_" "waitid" );


pid_t wait3(int *, int, struct rusage *);
pid_t wait4(pid_t, int *, int, struct rusage *);


# 66 "/usr/include/stdlib.h" 2 3 4

# 1 "/usr/include/alloca.h" 1 3 4
# 35 "/usr/include/alloca.h" 3 4

void *alloca(size_t);

# 68 "/usr/include/stdlib.h" 2 3 4
# 81 "/usr/include/stdlib.h" 3 4
typedef __darwin_ct_rune_t ct_rune_t;




typedef __darwin_rune_t rune_t;






typedef __darwin_wchar_t wchar_t;



typedef struct {
 int quot;
 int rem;
} div_t;

typedef struct {
 long quot;
 long rem;
} ldiv_t;


typedef struct {
 long long quot;
 long long rem;
} lldiv_t;
# 134 "/usr/include/stdlib.h" 3 4
extern int __mb_cur_max;
# 144 "/usr/include/stdlib.h" 3 4

void abort(void) __attribute__((__noreturn__));
int abs(int) __attribute__((__const__));
int atexit(void (*)(void));
double atof(const char *);
int atoi(const char *);
long atol(const char *);

long long
  atoll(const char *);

void *bsearch(const void *, const void *, size_t,
     size_t, int (*)(const void *, const void *));
void *calloc(size_t, size_t);
div_t div(int, int) __attribute__((__const__));
void exit(int) __attribute__((__noreturn__));
void free(void *);
char *getenv(const char *);
long labs(long) __attribute__((__const__));
ldiv_t ldiv(long, long) __attribute__((__const__));

long long
  llabs(long long);
lldiv_t lldiv(long long, long long);

void *malloc(size_t);
int mblen(const char *, size_t);
size_t mbstowcs(wchar_t * restrict , const char * restrict, size_t);
int mbtowc(wchar_t * restrict, const char * restrict, size_t);
int posix_memalign(void **, size_t, size_t) __attribute__((visibility("default")));
void qsort(void *, size_t, size_t,
     int (*)(const void *, const void *));
int rand(void);
void *realloc(void *, size_t);
void srand(unsigned);
double strtod(const char *, char **) __asm("_" "strtod" );
float strtof(const char *, char **) __asm("_" "strtof" );
long strtol(const char *, char **, int);
long double
  strtold(const char *, char **) ;

long long
  strtoll(const char *, char **, int);

unsigned long
  strtoul(const char *, char **, int);

unsigned long long
  strtoull(const char *, char **, int);

int system(const char *) __asm("_" "system" );
size_t wcstombs(char * restrict, const wchar_t * restrict, size_t);
int wctomb(char *, wchar_t);


void _Exit(int) __attribute__((__noreturn__));
long a64l(const char *);
double drand48(void);
char *ecvt(double, int, int *restrict, int *restrict);
double erand48(unsigned short[3]);
char *fcvt(double, int, int *restrict, int *restrict);
char *gcvt(double, int, char *);
int getsubopt(char **, char * const *, char **);
int grantpt(int);

char *initstate(unsigned, char *, size_t);



long jrand48(unsigned short[3]);
char *l64a(long);
void lcong48(unsigned short[7]);
long lrand48(void);
char *mktemp(char *);
int mkstemp(char *);
long mrand48(void);
long nrand48(unsigned short[3]);
int posix_openpt(int);
char *ptsname(int);
int putenv(char *) __asm("_" "putenv" );
long random(void);
int rand_r(unsigned *);

char *realpath(const char * restrict, char * restrict) __asm("_" "realpath" "$DARWIN_EXTSN");



unsigned short
 *seed48(unsigned short[3]);
int setenv(const char *, const char *, int) __asm("_" "setenv" );

void setkey(const char *) __asm("_" "setkey" );



char *setstate(const char *);
void srand48(long);

void srandom(unsigned);



int unlockpt(int);

int unsetenv(const char *) __asm("_" "unsetenv" );






# 1 "/usr/include/machine/types.h" 1 3 4
# 35 "/usr/include/machine/types.h" 3 4
# 1 "/usr/include/i386/types.h" 1 3 4
# 70 "/usr/include/i386/types.h" 3 4
# 1 "/usr/include/i386/_types.h" 1 3 4
# 71 "/usr/include/i386/types.h" 2 3 4







typedef signed char int8_t;

typedef unsigned char u_int8_t;


typedef short int16_t;

typedef unsigned short u_int16_t;


typedef int int32_t;

typedef unsigned int u_int32_t;


typedef long long int64_t;

typedef unsigned long long u_int64_t;


typedef int64_t register_t;






typedef __darwin_intptr_t intptr_t;



typedef unsigned long uintptr_t;




typedef u_int64_t user_addr_t;
typedef u_int64_t user_size_t;
typedef int64_t user_ssize_t;
typedef int64_t user_long_t;
typedef u_int64_t user_ulong_t;
typedef int64_t user_time_t;
typedef int64_t user_off_t;







typedef u_int64_t syscall_arg_t;
# 36 "/usr/include/machine/types.h" 2 3 4
# 256 "/usr/include/stdlib.h" 2 3 4


typedef __darwin_dev_t dev_t;




typedef __darwin_mode_t mode_t;



u_int32_t
  arc4random(void);
void arc4random_addrandom(unsigned char * , int );
void arc4random_buf(void * , size_t ) __attribute__((visibility("default")));
void arc4random_stir(void);
u_int32_t
  arc4random_uniform(u_int32_t ) __attribute__((visibility("default")));







char *cgetcap(char *, const char *, int);
int cgetclose(void);
int cgetent(char **, char **, const char *);
int cgetfirst(char **, char **);
int cgetmatch(const char *, const char *);
int cgetnext(char **, char **);
int cgetnum(char *, const char *, long *);
int cgetset(const char *);
int cgetstr(char *, const char *, char **);
int cgetustr(char *, const char *, char **);

int daemon(int, int) __asm("_" "daemon" "$1050") __attribute__((deprecated,visibility("default")));
char *devname(dev_t, mode_t);
char *devname_r(dev_t, mode_t, char *buf, int len);
char *getbsize(int *, long *);
int getloadavg(double [], int);
const char
 *getprogname(void);

int heapsort(void *, size_t, size_t,
     int (*)(const void *, const void *));




int mergesort(void *, size_t, size_t,
     int (*)(const void *, const void *));




void psort(void *, size_t, size_t,
     int (*)(const void *, const void *)) __attribute__((visibility("default")));




void psort_r(void *, size_t, size_t, void *,
     int (*)(void *, const void *, const void *)) __attribute__((visibility("default")));




void qsort_r(void *, size_t, size_t, void *,
     int (*)(void *, const void *, const void *));
int radixsort(const unsigned char **, int, const unsigned char *,
     unsigned);
void setprogname(const char *);
int sradixsort(const unsigned char **, int, const unsigned char *,
     unsigned);
void sranddev(void);
void srandomdev(void);
void *reallocf(void *, size_t);

long long
  strtoq(const char *, char **, int);
unsigned long long
  strtouq(const char *, char **, int);

extern char *suboptarg;
void *valloc(size_t);







# 11 "tz_utilities.c" 2
# 1 "/usr/include/math.h" 1 3 4
# 28 "/usr/include/math.h" 3 4
# 1 "/usr/include/architecture/i386/math.h" 1 3 4
# 49 "/usr/include/architecture/i386/math.h" 3 4
 typedef float float_t;
 typedef double double_t;
# 108 "/usr/include/architecture/i386/math.h" 3 4
extern int __math_errhandling ( void );
# 128 "/usr/include/architecture/i386/math.h" 3 4
extern int __fpclassifyf(float );
extern int __fpclassifyd(double );
extern int __fpclassify (long double);
# 163 "/usr/include/architecture/i386/math.h" 3 4
 static __inline__ int __inline_isfinitef (float ) __attribute__ ((always_inline));
 static __inline__ int __inline_isfinited (double ) __attribute__ ((always_inline));
 static __inline__ int __inline_isfinite (long double) __attribute__ ((always_inline));
 static __inline__ int __inline_isinff (float ) __attribute__ ((always_inline));
 static __inline__ int __inline_isinfd (double ) __attribute__ ((always_inline));
 static __inline__ int __inline_isinf (long double) __attribute__ ((always_inline));
 static __inline__ int __inline_isnanf (float ) __attribute__ ((always_inline));
 static __inline__ int __inline_isnand (double ) __attribute__ ((always_inline));
 static __inline__ int __inline_isnan (long double) __attribute__ ((always_inline));
 static __inline__ int __inline_isnormalf (float ) __attribute__ ((always_inline));
 static __inline__ int __inline_isnormald (double ) __attribute__ ((always_inline));
 static __inline__ int __inline_isnormal (long double) __attribute__ ((always_inline));
 static __inline__ int __inline_signbitf (float ) __attribute__ ((always_inline));
 static __inline__ int __inline_signbitd (double ) __attribute__ ((always_inline));
 static __inline__ int __inline_signbit (long double) __attribute__ ((always_inline));

 static __inline__ int __inline_isinff( float __x ) { return __builtin_fabsf(__x) == __builtin_inff(); }
 static __inline__ int __inline_isinfd( double __x ) { return __builtin_fabs(__x) == __builtin_inf(); }
 static __inline__ int __inline_isinf( long double __x ) { return __builtin_fabsl(__x) == __builtin_infl(); }
 static __inline__ int __inline_isfinitef( float __x ) { return __x == __x && __builtin_fabsf(__x) != __builtin_inff(); }
 static __inline__ int __inline_isfinited( double __x ) { return __x == __x && __builtin_fabs(__x) != __builtin_inf(); }
 static __inline__ int __inline_isfinite( long double __x ) { return __x == __x && __builtin_fabsl(__x) != __builtin_infl(); }
 static __inline__ int __inline_isnanf( float __x ) { return __x != __x; }
 static __inline__ int __inline_isnand( double __x ) { return __x != __x; }
 static __inline__ int __inline_isnan( long double __x ) { return __x != __x; }
 static __inline__ int __inline_signbitf( float __x ) { union{ float __f; unsigned int __u; }__u; __u.__f = __x; return (int)(__u.__u >> 31); }
 static __inline__ int __inline_signbitd( double __x ) { union{ double __f; unsigned int __u[2]; }__u; __u.__f = __x; return (int)(__u.__u[1] >> 31); }
 static __inline__ int __inline_signbit( long double __x ){ union{ long double __ld; struct{ unsigned int __m[2]; short __sexp; }__p; }__u; __u.__ld = __x; return (int) (((unsigned short) __u.__p.__sexp) >> 15); }
 static __inline__ int __inline_isnormalf( float __x ) { float fabsf = __builtin_fabsf(__x); if( __x != __x ) return 0; return fabsf < __builtin_inff() && fabsf >= 1.17549435e-38F; }
 static __inline__ int __inline_isnormald( double __x ) { double fabsf = __builtin_fabs(__x); if( __x != __x ) return 0; return fabsf < __builtin_inf() && fabsf >= 2.2250738585072014e-308; }
 static __inline__ int __inline_isnormal( long double __x ) { long double fabsf = __builtin_fabsl(__x); if( __x != __x ) return 0; return fabsf < __builtin_infl() && fabsf >= 3.36210314311209350626e-4932L; }
# 253 "/usr/include/architecture/i386/math.h" 3 4
extern double acos( double );
extern float acosf( float );

extern double asin( double );
extern float asinf( float );

extern double atan( double );
extern float atanf( float );

extern double atan2( double, double );
extern float atan2f( float, float );

extern double cos( double );
extern float cosf( float );

extern double sin( double );
extern float sinf( float );

extern double tan( double );
extern float tanf( float );

extern double acosh( double );
extern float acoshf( float );

extern double asinh( double );
extern float asinhf( float );

extern double atanh( double );
extern float atanhf( float );

extern double cosh( double );
extern float coshf( float );

extern double sinh( double );
extern float sinhf( float );

extern double tanh( double );
extern float tanhf( float );

extern double exp ( double );
extern float expf ( float );

extern double exp2 ( double );
extern float exp2f ( float );

extern double expm1 ( double );
extern float expm1f ( float );

extern double log ( double );
extern float logf ( float );

extern double log10 ( double );
extern float log10f ( float );

extern double log2 ( double );
extern float log2f ( float );

extern double log1p ( double );
extern float log1pf ( float );

extern double logb ( double );
extern float logbf ( float );

extern double modf ( double, double * );
extern float modff ( float, float * );

extern double ldexp ( double, int );
extern float ldexpf ( float, int );

extern double frexp ( double, int * );
extern float frexpf ( float, int * );

extern int ilogb ( double );
extern int ilogbf ( float );

extern double scalbn ( double, int );
extern float scalbnf ( float, int );

extern double scalbln ( double, long int );
extern float scalblnf ( float, long int );

extern double fabs( double );
extern float fabsf( float );

extern double cbrt( double );
extern float cbrtf( float );

extern double hypot ( double, double );
extern float hypotf ( float, float );

extern double pow ( double, double );
extern float powf ( float, float );

extern double sqrt( double );
extern float sqrtf( float );

extern double erf( double );
extern float erff( float );

extern double erfc( double );
extern float erfcf( float );






extern double lgamma( double );
extern float lgammaf( float );

extern double tgamma( double );
extern float tgammaf( float );

extern double ceil ( double );
extern float ceilf ( float );

extern double floor ( double );
extern float floorf ( float );

extern double nearbyint ( double );
extern float nearbyintf ( float );

extern double rint ( double );
extern float rintf ( float );

extern long int lrint ( double );
extern long int lrintf ( float );

extern double round ( double );
extern float roundf ( float );

extern long int lround ( double );
extern long int lroundf ( float );



    extern long long int llrint ( double );
    extern long long int llrintf ( float );
    extern long long int llround ( double );
    extern long long int llroundf ( float );


extern double trunc ( double );
extern float truncf ( float );

extern double fmod ( double, double );
extern float fmodf ( float, float );

extern double remainder ( double, double );
extern float remainderf ( float, float );

extern double remquo ( double, double, int * );
extern float remquof ( float, float, int * );

extern double copysign ( double, double );
extern float copysignf ( float, float );

extern double nan( const char * );
extern float nanf( const char * );

extern double nextafter ( double, double );
extern float nextafterf ( float, float );

extern double fdim ( double, double );
extern float fdimf ( float, float );

extern double fmax ( double, double );
extern float fmaxf ( float, float );

extern double fmin ( double, double );
extern float fminf ( float, float );

extern double fma ( double, double, double );
extern float fmaf ( float, float, float );

extern long double acosl(long double);
extern long double asinl(long double);
extern long double atanl(long double);
extern long double atan2l(long double, long double);
extern long double cosl(long double);
extern long double sinl(long double);
extern long double tanl(long double);
extern long double acoshl(long double);
extern long double asinhl(long double);
extern long double atanhl(long double);
extern long double coshl(long double);
extern long double sinhl(long double);
extern long double tanhl(long double);
extern long double expl(long double);
extern long double exp2l(long double);
extern long double expm1l(long double);
extern long double logl(long double);
extern long double log10l(long double);
extern long double log2l(long double);
extern long double log1pl(long double);
extern long double logbl(long double);
extern long double modfl(long double, long double *);
extern long double ldexpl(long double, int);
extern long double frexpl(long double, int *);
extern int ilogbl(long double);
extern long double scalbnl(long double, int);
extern long double scalblnl(long double, long int);
extern long double fabsl(long double);
extern long double cbrtl(long double);
extern long double hypotl(long double, long double);
extern long double powl(long double, long double);
extern long double sqrtl(long double);
extern long double erfl(long double);
extern long double erfcl(long double);






extern long double lgammal(long double);

extern long double tgammal(long double);
extern long double ceill(long double);
extern long double floorl(long double);
extern long double nearbyintl(long double);
extern long double rintl(long double);
extern long int lrintl(long double);
extern long double roundl(long double);
extern long int lroundl(long double);



    extern long long int llrintl(long double);
    extern long long int llroundl(long double);


extern long double truncl(long double);
extern long double fmodl(long double, long double);
extern long double remainderl(long double, long double);
extern long double remquol(long double, long double, int *);
extern long double copysignl(long double, long double);
extern long double nanl(const char *);
extern long double nextafterl(long double, long double);
extern double nexttoward(double, long double);
extern float nexttowardf(float, long double);
extern long double nexttowardl(long double, long double);
extern long double fdiml(long double, long double);
extern long double fmaxl(long double, long double);
extern long double fminl(long double, long double);
extern long double fmal(long double, long double, long double);
# 507 "/usr/include/architecture/i386/math.h" 3 4
extern double __inf( void );
extern float __inff( void );
extern long double __infl( void );
extern float __nan( void );


extern double j0 ( double );

extern double j1 ( double );

extern double jn ( int, double );

extern double y0 ( double );

extern double y1 ( double );

extern double yn ( int, double );

extern double scalb ( double, double );
# 543 "/usr/include/architecture/i386/math.h" 3 4
extern int signgam;
# 558 "/usr/include/architecture/i386/math.h" 3 4
extern long int rinttol ( double );


extern long int roundtol ( double );
# 570 "/usr/include/architecture/i386/math.h" 3 4
struct exception {
 int type;
 char *name;
 double arg1;
 double arg2;
 double retval;
};
# 29 "/usr/include/math.h" 2 3 4
# 12 "tz_utilities.c" 2

# 1 "/usr/include/dirent.h" 1 3 4
# 65 "/usr/include/dirent.h" 3 4
# 1 "/usr/include/sys/dirent.h" 1 3 4
# 82 "/usr/include/sys/dirent.h" 3 4
typedef __darwin_ino_t ino_t;






#pragma pack(4)
# 101 "/usr/include/sys/dirent.h" 3 4
#pragma pack()
# 115 "/usr/include/sys/dirent.h" 3 4
struct dirent { __uint64_t d_ino; __uint64_t d_seekoff; __uint16_t d_reclen; __uint16_t d_namlen; __uint8_t d_type; char d_name[1024]; };
# 66 "/usr/include/dirent.h" 2 3 4


struct _telldir;


typedef struct {
 int __dd_fd;
 long __dd_loc;
 long __dd_size;
 char *__dd_buf;
 int __dd_len;
 long __dd_seek;
 long __dd_rewind;
 int __dd_flags;
 __darwin_pthread_mutex_t __dd_lock;
 struct _telldir *__dd_td;
} DIR;
# 102 "/usr/include/dirent.h" 3 4


int alphasort(const void *, const void *) __asm("_" "alphasort" "$INODE64");

int closedir(DIR *) __asm("_" "closedir" );

int getdirentries(int, char *, int, long *)





      __asm("_getdirentries_is_not_available_when_64_bit_inodes_are_in_effect")



;

DIR *opendir(const char *) __asm("_" "opendir" "$INODE64" );

DIR *__opendir2(const char *, int) __asm("_" "__opendir2" "$INODE64" );

struct dirent *readdir(DIR *) __asm("_" "readdir" "$INODE64");
int readdir_r(DIR *, struct dirent *, struct dirent **) __asm("_" "readdir_r" "$INODE64");
void rewinddir(DIR *) __asm("_" "rewinddir" "$INODE64" );

int scandir(const char *, struct dirent ***,
    int (*)(struct dirent *), int (*)(const void *, const void *)) __asm("_" "scandir" "$INODE64");





void seekdir(DIR *, long) __asm("_" "seekdir" "$INODE64" );
long telldir(DIR *) __asm("_" "telldir" "$INODE64" );

# 14 "tz_utilities.c" 2







# 1 "/usr/include/regex.h" 1 3 4
# 68 "/usr/include/regex.h" 3 4
typedef __darwin_off_t regoff_t;






typedef struct {
 int re_magic;
 size_t re_nsub;
 const char *re_endp;
 struct re_guts *re_g;
} regex_t;

typedef struct {
 regoff_t rm_so;
 regoff_t rm_eo;
} regmatch_t;
# 135 "/usr/include/regex.h" 3 4

int regcomp(regex_t * restrict, const char * restrict, int) __asm("_" "regcomp" );
size_t regerror(int, const regex_t * restrict, char * restrict, size_t);




int regexec(const regex_t * restrict, const char * restrict, size_t,
     regmatch_t __pmatch[ restrict], int);
void regfree(regex_t *);

# 22 "tz_utilities.c" 2

# 1 "/usr/include/errno.h" 1 3 4
# 23 "/usr/include/errno.h" 3 4
# 1 "/usr/include/sys/errno.h" 1 3 4
# 74 "/usr/include/sys/errno.h" 3 4

extern int * __error(void);


# 24 "/usr/include/errno.h" 2 3 4
# 24 "tz_utilities.c" 2
# 1 "tz_utilities.h" 1
# 11 "tz_utilities.h"
# 1 "./include/tz_cdefs.h" 1
# 10 "./include/tz_cdefs.h"
# 1 "./include/neurolabi_config.h" 1
# 11 "./include/tz_cdefs.h" 2
# 124 "./include/tz_cdefs.h"
static const double NaN = (0.0 / 0.0);
# 138 "./include/tz_cdefs.h"
static const double Infinity = (1.0 / 0.0);
# 12 "tz_utilities.h" 2
# 1 "./include/tz_stdint.h" 1



# 1 "./include/neurolabi_config.h" 1
# 5 "./include/tz_stdint.h" 2
# 20 "./include/tz_stdint.h"
# 1 "/usr/include/stdint.h" 1 3 4
# 40 "/usr/include/stdint.h" 3 4
typedef unsigned char uint8_t;




typedef unsigned short uint16_t;




typedef unsigned int uint32_t;




typedef unsigned long long uint64_t;



typedef int8_t int_least8_t;
typedef int16_t int_least16_t;
typedef int32_t int_least32_t;
typedef int64_t int_least64_t;
typedef uint8_t uint_least8_t;
typedef uint16_t uint_least16_t;
typedef uint32_t uint_least32_t;
typedef uint64_t uint_least64_t;



typedef int8_t int_fast8_t;
typedef int16_t int_fast16_t;
typedef int32_t int_fast32_t;
typedef int64_t int_fast64_t;
typedef uint8_t uint_fast8_t;
typedef uint16_t uint_fast16_t;
typedef uint32_t uint_fast32_t;
typedef uint64_t uint_fast64_t;
# 97 "/usr/include/stdint.h" 3 4
typedef long int intmax_t;
# 106 "/usr/include/stdint.h" 3 4
typedef long unsigned int uintmax_t;
# 21 "./include/tz_stdint.h" 2



# 1 "/usr/include/sys/types.h" 1 3 4
# 84 "/usr/include/sys/types.h" 3 4
typedef unsigned char u_char;
typedef unsigned short u_short;
typedef unsigned int u_int;

typedef unsigned long u_long;


typedef unsigned short ushort;
typedef unsigned int uint;


typedef u_int64_t u_quad_t;
typedef int64_t quad_t;
typedef quad_t * qaddr_t;

typedef char * caddr_t;
typedef int32_t daddr_t;






typedef u_int32_t fixpt_t;


typedef __darwin_blkcnt_t blkcnt_t;




typedef __darwin_blksize_t blksize_t;




typedef __darwin_gid_t gid_t;





typedef __uint32_t in_addr_t;




typedef __uint16_t in_port_t;
# 141 "/usr/include/sys/types.h" 3 4
typedef __darwin_ino64_t ino64_t;






typedef __int32_t key_t;
# 157 "/usr/include/sys/types.h" 3 4
typedef __uint16_t nlink_t;
# 176 "/usr/include/sys/types.h" 3 4
typedef int32_t segsz_t;
typedef int32_t swblk_t;
# 245 "/usr/include/sys/types.h" 3 4
typedef __darwin_useconds_t useconds_t;
# 260 "/usr/include/sys/types.h" 3 4
# 1 "/usr/include/sys/_structs.h" 1 3 4
# 261 "/usr/include/sys/types.h" 2 3 4




typedef __int32_t fd_mask;
# 322 "/usr/include/sys/types.h" 3 4
typedef __darwin_pthread_cond_t pthread_cond_t;



typedef __darwin_pthread_condattr_t pthread_condattr_t;



typedef __darwin_pthread_mutex_t pthread_mutex_t;



typedef __darwin_pthread_mutexattr_t pthread_mutexattr_t;



typedef __darwin_pthread_once_t pthread_once_t;



typedef __darwin_pthread_rwlock_t pthread_rwlock_t;



typedef __darwin_pthread_rwlockattr_t pthread_rwlockattr_t;



typedef __darwin_pthread_t pthread_t;






typedef __darwin_pthread_key_t pthread_key_t;





typedef __darwin_fsblkcnt_t fsblkcnt_t;




typedef __darwin_fsfilcnt_t fsfilcnt_t;
# 25 "./include/tz_stdint.h" 2
# 45 "./include/tz_stdint.h"
typedef int8_t byte_t;
typedef int16_t word_t;
# 13 "tz_utilities.h" 2



# 1 "../lib/genelib/src/utilities.h" 1
# 74 "../lib/genelib/src/utilities.h"
void *Guarded_Malloc(size_t size, const char *routine);
void *Guarded_Realloc(void *array, size_t size, char *routine);
char *Guarded_Strdup(char *string, char *routine);
FILE *Guarded_Fopen(const char *name, const char *options, const char *routine);
# 86 "../lib/genelib/src/utilities.h"
void Process_Arguments(int argc, char *argv[], char *spec[], int no_escapes);
# 97 "../lib/genelib/src/utilities.h"
char *Program_Name();

int Get_Repeat_Count(char *name);
int Is_Arg_Matched(char *name, ...);

int Get_Int_Arg (char *name, ...);
double Get_Double_Arg(char *name, ...);
char *Get_String_Arg(char *name, ...);







void Print_Argument_Usage(FILE *file, int no_escapes);
# 17 "tz_utilities.h" 2

typedef uint8_t tz_uint8;
typedef int8_t tz_int8;
typedef uint16_t tz_uint16;
typedef int16_t tz_int16;
typedef uint32_t tz_uint32;
typedef int32_t tz_int32;
typedef uint64_t tz_uint64;
typedef int64_t tz_int64;
typedef float tz_float32;
typedef double tz_float64;





typedef enum {FLOAT64_TD=0,FLOAT32_TD,INT64_TD,UINT64_TD,INT32_TD,
       UINT32_TD,INT16_TD,UINT16_TD,INT8_TD,UINT8_TD,
       DOUBLE_TD=0,FLOAT_TD,LLONG_TD,ULLONG_TD,LONG_TD,ULONG_TD,
       SHORT_TD,USHORT_TD,CHAR_TD,UCHAR_TD,
       INT_TD,UINT_TD} Data_Type_t;

const static int gDataSize[] = {8,4,8,8,4,4,2,2,1,1,4,4};
# 49 "tz_utilities.h"
static inline float sqr(float a) { return a * a; }
static inline double dsqr(double a) { return a * a; }
static inline double dmax2(double a,double b) { return (a>b ? a : b); }
static inline double dmin2(double a,double b) { return (a<b ? a : b); }
static inline float fmax2(float a,float b) { return (a>b ? a : b); }
static inline float fmin2(float a,float b) { return (a<b ? a : b); }
static inline long lmax2(long a,long b) { return (a>b ? a : b); }
static inline long lmin2(long a,long b) { return (a<b ? a : b); }
static inline int imax2(int a,int b) { return (a>b ? a : b); }
static inline int imin2(int a,int b) { return (a<b ? a : b); }
# 68 "tz_utilities.h"
static inline double dmax3 (double a, double b, double c) { return (a)>=(b) ? (a)>=(c)?(a):(c) : (b)>=(c)?(b):(c); }
static inline double dmin3 (double a, double b, double c) { return (a)<=(b) ? (a)<=(c)?(a):(c) : (b)<=(c)?(b):(c); }
static inline double fmax3 (float a, float b, float c) { return (a)>=(b) ? (a)>=(c)?(a):(c) : (b)>=(c)?(b):(c); }
static inline double fmin3 (float a, float b, float c) { return (a)<=(b) ? (a)<=(c)?(a):(c) : (b)<=(c)?(b):(c); }
static inline double lmax3 (long a, long b, long c) { return (a)>=(b) ? (a)>=(c)?(a):(c) : (b)>=(c)?(b):(c); }
static inline double lmin3 (long a, long b, long c) { return (a)<=(b) ? (a)<=(c)?(a):(c) : (b)<=(c)?(b):(c); }
static inline double imax3 (int a, int b, int c) { return (a)>=(b) ? (a)>=(c)?(a):(c) : (b)>=(c)?(b):(c); }
static inline double imin3 (int a, int b, int c) { return (a)<=(b) ? (a)<=(c)?(a):(c) : (b)<=(c)?(b):(c); }
# 104 "tz_utilities.h"
static inline int valid_type(Data_Type_t type) { return ((int)type>=0 &&
        (int)type<12); }





void tic();






tz_int64 toc();





void ptoc();





void pmtoc(const char *msg);






int fexist(const char* filename);






int dexist(const char *path);





int fcopy(const char *source, const char *target);






const char *fextn(const char* filename);






int fhasext(const char* filename, const char *ext);







char* fname(const char *path, char *name);







char* dname(const char *path, char *name);






int dir_fnum(const char *dir_name, const char *ext);







int dir_fnum_p(const char *dir_name, const char *pattern);







int dir_fnum_s(const char *dir_name, const char *pattern);




int dir_fnum_pair(const char *dir_name, const char *pattern, int *start,
    int *end);





size_t fsize(const char *filename);






int fcmp(const char *filepath1, const char *filepath2);




int eqstr(const char *str1, const char *str2);





void rmpath(const char *path);






char* fullpath(const char *path1, const char *path2, char *path);






char* fullpath_e(const char *path1, const char *path2, const char *ext,
   char *path);







void safe_free(void **ptr, void (*free_fun) (void *ptr));
# 313 "tz_utilities.h"
char* double_binstr(double x, char str[]);



int tz_isnan(double x);





int tz_isinf(double x);


extern char* fgetln(FILE *stream, size_t *len);



double Value_Trend(double *value, int n);






int Show_Version(int argc, char *argv[], const char *ver);



void Print_Arguments(int argc, char *argv[]);





uint32_t Hexstr_To_Uint(char *str);



char* Uint_To_Hexstr(uint32_t num, char *str);



void Fprint_File_Binary(FILE *fp, size_t n, FILE *out);

void *Guarded_Calloc(size_t count, size_t size, const char *routine);






void pow2decomp(int n, int *k, int *m);
int pow2sum(int k, int m);

void Print_Argument_Spec(const char *spec[]);





void Memset_Pattern4(void *b, const void *pattern4, size_t len);





int Is_Valid_Array_Index(size_t index);


# 25 "tz_utilities.c" 2
# 1 "tz_string.h" 1
# 10 "tz_string.h"
# 1 "./include/tz_cdefs.h" 1
# 11 "tz_string.h" 2
# 1 "tz_string_defs.h" 1
# 10 "tz_string_defs.h"
# 1 "./include/tz_cdefs.h" 1
# 11 "tz_string_defs.h" 2


# 25 "tz_string_defs.h"
typedef struct _String_Workspace {

  char *array;
  char *iterator;
  int size;
} String_Workspace;

void Default_String_Workspace(String_Workspace *sw);

void Construct_String_Workspace(String_Workspace * sw, int size);
void Clean_String_Workspace(String_Workspace * sw);

String_Workspace* Make_String_Workspace();


String_Workspace* New_String_Workspace();
String_Workspace *Copy_String_Workspace(const String_Workspace *sw);
void Pack_String_Workspace(String_Workspace *sw);
void Free_String_Workspace(String_Workspace *sw);
void Kill_String_Workspace(String_Workspace *sw);
void Reset_String_Workspace();
int String_Workspace_Usage();

int Save_String_Workspace(String_Workspace *sw, char *file_path);
String_Workspace* Load_String_Workspace(char *file_path);


void String_Workspace_Realloc(String_Workspace *sw, int size);
void String_Workspace_Grow(String_Workspace *sw);




# 12 "tz_string.h" 2


# 24 "tz_string.h"
void strtrim(char *str);





void strrmspc(char *str);
# 39 "tz_string.h"
char* strsplit(char *str, char delim, int pos);


int tz_isspace(char c);

int tz_issemicolon(char c);
int tz_iscoma(char c);
int tz_islinebreak(char c);
int tz_isdlm(char c);






int String_First_Integer(const char *str);







int String_Last_Integer(const char *str);

int Is_Integer(const char *str);
int Is_Float(const char *str);
int Is_Space(const char *str);







int String_Ends_With(const char *str, const char *end);







int String_Starts_With(const char *str, const char *start);





void fprint_space(FILE *fp, int n);
# 102 "tz_string.h"
int Read_Word(FILE *fp, char *str, int n);
int Read_Word_D(FILE *fp, char *str, int n, int (*is_dlm) (char));

int Count_Word_D(char *str, int (*is_dlm) (char));
int Count_Word_P(char *str, int (*is_dlm) (char),
   int (*is_pattern)(const char *));
int Count_Number_D(char *str, int (*is_dlm) (char));
int Count_Integer_D(char *str, int (*is_dlm) (char));







int* String_To_Integer_Array(const char *str, int *array, int *n);







double* String_To_Double_Array(const char *str, double *array, int *n);







char* Read_Param(FILE *fp, const char* var, String_Workspace *sw);





char* Read_Line(FILE *fp, String_Workspace *sw);






int File_Line_Number(const char *path, int count_empty);




# 26 "tz_utilities.c" 2

# 1 "/usr/include/sys/stat.h" 1 3 4
# 79 "/usr/include/sys/stat.h" 3 4
# 1 "/usr/include/sys/_structs.h" 1 3 4
# 80 "/usr/include/sys/stat.h" 2 3 4
# 153 "/usr/include/sys/stat.h" 3 4
struct ostat {
 __uint16_t st_dev;
 ino_t st_ino;
 mode_t st_mode;
 nlink_t st_nlink;
 __uint16_t st_uid;
 __uint16_t st_gid;
 __uint16_t st_rdev;
 __int32_t st_size;
 struct timespec st_atimespec;
 struct timespec st_mtimespec;
 struct timespec st_ctimespec;
 __int32_t st_blksize;
 __int32_t st_blocks;
 __uint32_t st_flags;
 __uint32_t st_gen;
};
# 225 "/usr/include/sys/stat.h" 3 4
struct stat { dev_t st_dev; mode_t st_mode; nlink_t st_nlink; __darwin_ino64_t st_ino; uid_t st_uid; gid_t st_gid; dev_t st_rdev; struct timespec st_atimespec; struct timespec st_mtimespec; struct timespec st_ctimespec; struct timespec st_birthtimespec; off_t st_size; blkcnt_t st_blocks; blksize_t st_blksize; __uint32_t st_flags; __uint32_t st_gen; __int32_t st_lspare; __int64_t st_qspare[2]; };
# 264 "/usr/include/sys/stat.h" 3 4
struct stat64 { dev_t st_dev; mode_t st_mode; nlink_t st_nlink; __darwin_ino64_t st_ino; uid_t st_uid; gid_t st_gid; dev_t st_rdev; struct timespec st_atimespec; struct timespec st_mtimespec; struct timespec st_ctimespec; struct timespec st_birthtimespec; off_t st_size; blkcnt_t st_blocks; blksize_t st_blksize; __uint32_t st_flags; __uint32_t st_gen; __int32_t st_lspare; __int64_t st_qspare[2]; };
# 428 "/usr/include/sys/stat.h" 3 4


int chmod(const char *, mode_t) __asm("_" "chmod" );
int fchmod(int, mode_t) __asm("_" "fchmod" );
int fstat(int, struct stat *) __asm("_" "fstat" "$INODE64");
int lstat(const char *, struct stat *) __asm("_" "lstat" "$INODE64");
int mkdir(const char *, mode_t);
int mkfifo(const char *, mode_t);
int stat(const char *, struct stat *) __asm("_" "stat" "$INODE64");
int mknod(const char *, mode_t, dev_t);
mode_t umask(mode_t);



struct _filesec;
typedef struct _filesec *filesec_t;


int chflags(const char *, __uint32_t);
int chmodx_np(const char *, filesec_t);
int fchflags(int, __uint32_t);
int fchmodx_np(int, filesec_t);
int fstatx_np(int, struct stat *, filesec_t) __asm("_" "fstatx_np" "$INODE64");
int lchflags(const char *, __uint32_t) __attribute__((visibility("default")));
int lchmod(const char *, mode_t) __attribute__((visibility("default")));
int lstatx_np(const char *, struct stat *, filesec_t) __asm("_" "lstatx_np" "$INODE64");
int mkdirx_np(const char *, filesec_t);
int mkfifox_np(const char *, filesec_t);
int statx_np(const char *, struct stat *, filesec_t) __asm("_" "statx_np" "$INODE64");
int umaskx_np(filesec_t) __attribute__((deprecated,visibility("default")));



int fstatx64_np(int, struct stat64 *, filesec_t) __attribute__((deprecated,visibility("default")));
int lstatx64_np(const char *, struct stat64 *, filesec_t) __attribute__((deprecated,visibility("default")));
int statx64_np(const char *, struct stat64 *, filesec_t) __attribute__((deprecated,visibility("default")));
int fstat64(int, struct stat64 *) __attribute__((deprecated,visibility("default")));
int lstat64(const char *, struct stat64 *) __attribute__((deprecated,visibility("default")));
int stat64(const char *, struct stat64 *) __attribute__((deprecated,visibility("default")));




# 28 "tz_utilities.c" 2

# 1 "tz_error.h" 1
# 12 "tz_error.h"
# 1 "./include/tz_cdefs.h" 1
# 13 "tz_error.h" 2












static const int SUCCESS = 0;






enum {
  ERROR_IO_OPEN = 1024,
  ERROR_IO_WRITE,
  ERROR_IO_READ,
  ERROR_IO_FORMAT,
  ERROR_DATA_TYPE,
  ERROR_DATA_COMPTB,
  ERROR_MALLOC,
  ERROR_POINTER_NULL,
  ERROR_DATA_VALUE,
  ERROR_CODE_BUG,
  ERROR_NA_FUNC,
  EXCEPTION_THROWN,
  ERROR_UNKNOWN,
  ERROR_ASSERT_FAILED,
  ERROR_OBSOLETE_FUNC,
  ERROR_PART_FUNC,
  ERROR_UNTESTED_FUNC,
  TRACE_FLAG,
  ERROR_OTHER
};





const char* tz_errmsg(int error_type);




int Get_Error_Code();



void tz_error(int error_type, const char* file, int line, const char* func);
void tz_warn(int error_type, const char* file, int line, const char* func);
# 173 "tz_error.h"

# 30 "tz_utilities.c" 2


static struct timeval start_time,end_time;

void tic()
{

  gettimeofday(&start_time,((void *)0));

}

tz_int64 toc()
{

  gettimeofday(&end_time,((void *)0));
  return (end_time.tv_sec-start_time.tv_sec) * 1000 +
    (end_time.tv_usec-start_time.tv_usec)/1000;



}

void ptoc()
{
  printf("Time elapsed: %llu ms\n", toc());
}

void pmtoc(const char *msg)
{
  if (msg == ((void *)0)) {
    ptoc();
  } else {
    printf("%llu : %s\n", toc(), msg);
  }
}

int fexist(const char* filename)
{
  FILE* fp = fopen(filename,"r");

  if(fp==((void *)0))
    return 0;
  else {
    fclose(fp);
    return 1;
  }
}

int dexist(const char *path)
{

  struct stat st;
  if ((stat(path, &st) == 0) && ((((st.st_mode) & 0170000) == 0040000))) {
    return 1;
  }




  return 0;
}
int fcopy(const char *source, const char *target)
{
  FILE *fp = fopen(source, "rb");
  if (fp == ((void *)0)) {
    return -1;
  }

  FILE *fp2 = fopen(target, "wb");
  if (fp2 == ((void *)0)) {
    fclose(fp);
    return -1;
  }

  while (!feof(fp)) {
    char ch = fgetc(fp);
    if (!feof(fp)) {
      fputc(ch, fp2);
    }
  }

  fclose(fp2);

  return 0;
}

const char *fextn(const char* filename)
{
  int len = strlen(filename);
  int i;
  for (i = len - 1; i >=0; i--) {
    if (filename[i] == '.') {
      return filename + i + 1;
    }
  }

  return ((void *)0);
}

char* fname(const char *path, char *name)
{
  if (path == ((void *)0)) {
    return ((void *)0);
  }

  int len = strlen(path);

  if (len == 0) {
    return ((void *)0);
  }

  if (len == 1) {
    if (path[0] == '.') {
      return ((void *)0);
    }
  }

  char pathsep = '/';
  if (path[len-1] == pathsep || path[len-1] == '\\') {
    return ((void *)0);
  }

  int start = -1;
  int end = -1;
  int dot_pos = len;
  int pathsep_pos = -1;
  int i;
  for (i = len - 1; i >=0; i--) {
    if (dot_pos == len) {
      if (path[i] == '.') {
 dot_pos = i;
      }
    }
    if (pathsep_pos < 0) {
      if (path[i] == pathsep || path[i] == '\\') {
 pathsep_pos = i;
      }
    }
  }

  if (pathsep_pos >= 0) {
    if (dot_pos == pathsep_pos + 1) {
      if (dot_pos == len - 1) {
 return ((void *)0);
      }
      dot_pos = len;
    }
  }

  if (dot_pos < pathsep_pos) {
    dot_pos = len;
  }

  start = pathsep_pos + 1;
  end = dot_pos - 1;

  int new_len = end - start + 1;

  if (name == ((void *)0)) {
    name = (char*) malloc(sizeof(char) * (new_len + 1));
  }

  ((__builtin_object_size (name, 0) != (size_t) -1) ? __builtin___memcpy_chk (name, path + start, new_len, __builtin_object_size (name, 0)) : __inline_memcpy_chk (name, path + start, new_len));
  name[new_len] = '\0';

  return name;
}

char* dname(const char *path, char *name)
{
  if (path == ((void *)0)) {
    return ((void *)0);
  }

  int len = strlen(path);

  if (len == 0) {
    return ((void *)0);
  }

  if (len == 1) {
    if (path[0] == '.') {
      return ((void *)0);
    }
  }

  char pathsep = '/';
  if (path[len-1] == pathsep || path[len-1] == '\\') {
    return ((void *)0);
  }

  int start = -1;
  int end = -1;
  int dot_pos = len;
  int pathsep_pos = -1;
  int i;
  for (i = len - 1; i >=0; i--) {
    if (dot_pos == len) {
      if (path[i] == '.') {
 dot_pos = i;
      }
    }
    if (pathsep_pos < 0) {
      if (path[i] == pathsep || path[i] == '\\') {
 pathsep_pos = i;
      }
    }
  }

  if (pathsep_pos >= 0) {
    if (dot_pos == pathsep_pos + 1) {
      if (dot_pos == len - 1) {
 return ((void *)0);
      }
      dot_pos = len;
    }
  }

  if (dot_pos < pathsep_pos) {
    dot_pos = len;
  }

  start = pathsep_pos + 1;
  end = dot_pos - 1;

  int new_len = start;

  if (name == ((void *)0)) {
    name = (char*) malloc(sizeof(char) * (new_len + 1));
  }

  if (new_len > 0) {
    ((__builtin_object_size (name, 0) != (size_t) -1) ? __builtin___memcpy_chk (name, path, new_len, __builtin_object_size (name, 0)) : __inline_memcpy_chk (name, path, new_len));
  }

  name[new_len] = '\0';

  return name;
}

int fhasext(const char* filename, const char *ext)
{
  if (filename == ((void *)0)) {
    return 0;
  }

  const char *sext = fextn(filename);
  if ((sext == ((void *)0)) && (ext == ((void *)0))) {
    return 1;
  } else if ((sext != ((void *)0)) && (ext != ((void *)0))) {
    if (strcmp(sext, ext) == 0) {
      return 1;
    }
  }

  return 0;
}

int dir_fnum(const char *dir_name, const char *ext)
{
  if (dir_name == ((void *)0)) {
    return 0;
  }

  int n = 0;
  DIR *dir = opendir(dir_name);
  if (dir == ((void *)0)) {
    perror(strerror((*__error())));
    return 0;
  }

  struct dirent *ent = readdir(dir);
  while (ent != ((void *)0)) {
    const char *file_ext = fextn(ent->d_name);
    if (file_ext != ((void *)0)) {
      if (strcmp(file_ext, ext) ==0) {
 n++;
      }
    }
    ent = readdir(dir);
  }
  closedir(dir);

  return n;
}

int dir_fnum_p(const char *dir_name, const char *pattern)
{
  if (dir_name == ((void *)0)) {
    return 0;
  }

  int n = 0;
  DIR *dir = opendir(dir_name);
  struct dirent *ent = readdir(dir);

  regex_t preg;
  regcomp(&preg, pattern, 0000);

  while (ent != ((void *)0)) {
    if (regexec(&preg, ent->d_name, 0, ((void *)0), 0000) ==0) {
      n++;
    }
    ent = readdir(dir);
  }
  closedir(dir);

  regfree(&preg);

  return n;
}

int dir_fnum_s(const char *dir_name, const char *pattern)
{
  if (dir_name == ((void *)0)) {
    return 0;
  }

  int n = 0;
  DIR *dir = opendir(dir_name);
  struct dirent *ent = readdir(dir);

  regex_t preg;
  regcomp(&preg, pattern, 0000);

  while (ent != ((void *)0)) {
    if (regexec(&preg, ent->d_name, 0, ((void *)0), 0000) ==0) {
      int id = String_Last_Integer(ent->d_name);
      if (n < id) {
 n = id;
      }
    }
    ent = readdir(dir);
  }
  closedir(dir);

  regfree(&preg);

  return n;
}

int dir_fnum_pair(const char *dir_name, const char *pattern, int *start,
    int *end)
{
  if (dir_name == ((void *)0)) {
    return 0;
  }

  DIR *dir = opendir(dir_name);
  struct dirent *ent = readdir(dir);

  regex_t preg;
  regcomp(&preg, pattern, 0000);

  *start = -1;
  *end = -1;

  while (ent != ((void *)0)) {
    if (regexec(&preg, ent->d_name, 0, ((void *)0), 0000) ==0) {
      int id = String_Last_Integer(ent->d_name);
      if (*start < 0) {
        *start = id;
        *end = id;
      } else {
        if (*start > id) {
          *start = id;
        }
        if (*end < id) {
          *end = id;
        }
      }
    }
    ent = readdir(dir);
  }
  closedir(dir);

  regfree(&preg);

  return ((*start) >= 0);
}

char* fullpath(const char *path1, const char *path2, char *path)
{
  return fullpath_e(path1, path2, ((void *)0), path);
}

char* fullpath_e(const char *path1, const char *path2, const char *ext,
   char *path)
{
  if (path == ((void *)0)) {
    path = (char*) malloc(sizeof(char) * (strlen(path1) + strlen(path2) + 2));
  }

  int len = strlen(path1);
  if (len > 0) {
    ((__builtin_object_size (path, 0) != (size_t) -1) ? __builtin___strcpy_chk (path, path1, __builtin_object_size (path, 2 > 1)) : __inline_strcpy_chk (path, path1));
    if (path1[len - 1] != '/') {
      path[len] = '/';
      path[len + 1] = '\0';
    }
    ((__builtin_object_size (path, 0) != (size_t) -1) ? __builtin___strcat_chk (path, path2, __builtin_object_size (path, 2 > 1)) : __inline_strcat_chk (path, path2));
  } else {
    ((__builtin_object_size (path, 0) != (size_t) -1) ? __builtin___strcpy_chk (path, path2, __builtin_object_size (path, 2 > 1)) : __inline_strcpy_chk (path, path2));
  }

  if (ext != ((void *)0)) {
    if (ext[0] == '.') {
      ext++;
    }
    len = strlen(path);
    path[len] = '.';
    path[len+1] = '\0';
    ((__builtin_object_size (path, 0) != (size_t) -1) ? __builtin___strcat_chk (path, ext, __builtin_object_size (path, 2 > 1)) : __inline_strcat_chk (path, ext));
  }

  return path;
}

void safe_free(void **ptr, void (*free_fun) (void *))
{
  if (free_fun == ((void *)0)) {
    free(*ptr);
  } else {
    free_fun(*ptr);
  }

  *ptr = ((void *)0);
}


char* double_binstr(double x, char str[])
{
  union {
    long long c;
    double d;
  } tmp_x;

  tmp_x.d = x;

  long long y = 1;
  int nbit = sizeof(double) * 8;
  int i;
  for (i = nbit - 1; i >= 0; i--) {
    if ((y & tmp_x.c) == 0) {
      str[i] = '0';
    } else {
      str[i] = '1';
    }
    y <<= 1;
  }
  str[nbit] = '\0';

  return str;
}

int tz_isnan(double x)
{
  return ( sizeof (x) == sizeof(float ) ? __inline_isnanf((float)(x)) : sizeof (x) == sizeof(double) ? __inline_isnand((double)(x)) : __inline_isnan ((long double)(x)));
}

int tz_isinf(double x)
{
  return (x == Infinity) || (x == -Infinity);
}
# 516 "tz_utilities.c"
double Value_Trend(double *scores, int n)
{
  int i;
  double sxi = 0.0;
  double sx2 = 0.0;
  double sx = 0.0;
  for (i = 0; i < n; i++) {
    sx += scores[i];
    sxi += scores[i] * (double) i;
    sx2 += scores[i] * scores[i];
  }

  return (sxi - sx * (n-1) / 2.0) /
    (sqrt((sx2 * n - sx * sx) * (n - 1) * (n + 1) / 12.0));
}

int Show_Version(int argc, char *argv[], const char *ver)
{
  int show_ver = 0;
  if (argc == 2) {
    if (strcmp(argv[1], "-ver") == 0) {
      show_ver = 1;
    }
  }

  if (show_ver == 1) {
    printf("%s Version %s\n", argv[0], ver);



    printf("Author: Ting Zhao\n");
    printf("Janelia Farm Research Campus\n");
  }

  return show_ver;
}

void Print_Arguments(int argc, char *argv[])
{
  int i;
  for (i = 0; i < argc; i++) {
    printf("%s ", argv[i]);
  }
  printf("\n");
}

static inline uint8_t hexchar2num(char c)
{
  if ((c >= '0') && (c <= '9')) {
    return c - '0';
  } else if ((c >= 'a') && (c <= 'f')){
    return c - 'a' + 10;
  } else if ((c >= 'A') && (c <= 'F')){
    return c - 'A' + 10;
  } else {
    return 255;
  }
}

uint32_t Hexstr_To_Uint(char *str)
{
  int len = strlen(str);

  { if (!((len > 0) && (len % 2 == 0))) { fprintf(__stderrp,"Assertion failed in the function %s.\n", __func__); fprintf(__stderrp, "  %s:%d: %s\n", "tz_utilities.c", 579, "invalid string"); tz_error(ERROR_ASSERT_FAILED, "tz_utilities.c", 579, __func__); } };

  uint32_t num;
  uint8_t *mem = (uint8_t*) &num;

  len /= 2;
  int i;
  for (i = 0; i < len; i++) {
    mem[i] = (hexchar2num(str[i*2]) << 4) + hexchar2num(str[i*2+1]);
  }

  for (i = len; i < 4; i++) {
    mem[i] = 0;
  }

  return num;
}

static inline char num2hexchar(uint8_t num)
{
  if (num < 10) {
    return num + '0';
  } else if (num < 16) {
    return num - 10 + 'A';
  } else {
    return 'n';
  }
}

char* Uint_To_Hexstr(uint32_t num, char *str)
{
  uint8_t *mem = (uint8_t*) &num;
  int i;
  for (i = 0; i < 4; i++) {
    uint8_t b = mem[i];
    str[3*i] = num2hexchar((b & 0xF0)>>4);
    str[3*i+1] = num2hexchar(b & 0x0F);
    str[3*i+2] = ' ';
  }
  str[11] = '\0';

  return str;
}

inline static char* uint8_to_hexstr(uint8_t value, char *str)
{
  str[0] = num2hexchar((value & 0xF0)>>4);
  str[1] = num2hexchar(value & 0x0F);

  return str;
}

void Fprint_File_Binary(FILE *fp, size_t n, FILE *out)
{
  if (fp == ((void *)0)) {
    return;
  }

  uint8_t value;
  char str[3];
  str[2] = '\0';
  size_t i;
  for (i = 0; i < n; i++) {
    fread(&value, 1, 1, fp);
    if (!feof(fp)) {
      fprintf(out, "%s ", uint8_to_hexstr(value, str));
    } else {
      fprintf(out, "##");
      break;
    }
  }
  fprintf(out, "\n");
}

size_t fsize(const char *filename)
{

  struct stat buf;
  stat(filename, &buf);
  return buf.st_size;
# 671 "tz_utilities.c"
}

int fcmp(const char *filepath1, const char *filepath2)
{
  if (strcmp(filepath1, filepath2) == 0) {
    return 0;
  }

  FILE *fp1 = Guarded_Fopen((char*) filepath1, "rb", (char*) __func__);
  FILE *fp2 = Guarded_Fopen((char*) filepath2, "rb", (char*) __func__);

  int diff = 0;

  size_t n1, n2;
  char ch1, ch2;

  while (!feof(fp1) && !feof(fp2)) {
    ch1 = '\0';
    ch2 = '\0';
    n1 = fread(&ch1, 1, 1, fp1);
    n2 = fread(&ch2, 1, 1, fp2);
    if ((n1 == 1) && (n2 == 1)) {
      if (ch1 != ch2) {
        diff = 1;
        break;
      }
    } else {
      if (feof(fp1) != feof(fp2)) {
        diff = 2;
      }
      break;
    }
  }

  fclose(fp1);
  fclose(fp2);

  return diff;
}

void *Guarded_Calloc(size_t count, size_t size, const char *routine)
{ void *p;

  p = calloc(count, size);
  if (p == ((void *)0))
    { fprintf(__stderrp,"\nError in %s:\n",routine);
      fprintf(__stderrp,"   Out of memory\n");
      exit (1);
    }
  return (p);
}






void pow2decomp(int n, int *k, int *m)
{
  *k = 0;
  int pn = 2;
  n += 1;
  while (pn <= n) {
    (*k)++;
    pn *= 2;
  }

  pn /= 2;

  *m = n - pn;
}


int pow2sum(int k, int m)
{
  int result = 1;
  int i;
  for (i = 0; i < k; i++) {
    result *= 2;
  }

  return result - 1 + m;
}

void Print_Argument_Spec(const char *spec[])
{
  while (*spec) {
    printf("%s\n", *(spec++));
  }
}

void Memset_Pattern4(void *b, const void *pattern4, size_t len)
{
  size_t count = len / 4;
  size_t i;
  char *cb = (char*) b;
  for (i = 0; i < count; i++) {
    ((__builtin_object_size (cb, 0) != (size_t) -1) ? __builtin___memcpy_chk (cb, pattern4, 4, __builtin_object_size (cb, 0)) : __inline_memcpy_chk (cb, pattern4, 4));
    cb += 4;
  }

  count = len % 4;
  if (count > 0) {
    ((__builtin_object_size (cb, 0) != (size_t) -1) ? __builtin___memcpy_chk (cb, pattern4, count, __builtin_object_size (cb, 0)) : __inline_memcpy_chk (cb, pattern4, count));
  }
}

void rmpath(const char *path)
{
  char cmd[500];
  __builtin___sprintf_chk (cmd, 0, __builtin_object_size (cmd, 2 > 1), "rm -rf %s", path);
  system(cmd);
}


int eqstr(const char *str1, const char *str2)
{
  if ((str1 == ((void *)0)) && (str2 == ((void *)0))) {
    return 1;
  } else if ((str1 == ((void *)0)) && (str2 == ((void *)0))) {
    return 0;
  } else {
    if (strcmp(str1, str2) == 0) {
      return 1;
    }
  }

  return 0;
}

int Is_Valid_Array_Index(size_t index)
{
  return (index != 2147483647);
}
