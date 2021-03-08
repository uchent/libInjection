#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dlfcn.h>
#include <stdarg.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <fcntl.h>

void* handle;
static int (*old_closedir)(DIR* dirp) = NULL;
static DIR* (*old_opendir)(const char* name) = NULL;
static struct dirent* (*old_readdir)(DIR* dirp) = NULL;
static int (*old_creat)(const char* path, mode_t mode) = NULL;
static int (*old_open)(const char* path, int oflag, ...) = NULL;
static ssize_t (*old_read)(int fd, void* buf, size_t nbyte) = NULL;
static ssize_t (*old_write)(int fd, const void* buf, size_t count) = NULL;
static int (*old_dup)(int oldfd) = NULL;
static int (*old_dup2)(int oldfd, int newfd) = NULL;
static int (*old_close)(int fd) = NULL;
static int (*old_lstat)(int ver, const char *path, struct stat* buf) = NULL;
static int (*old_stat)(int ver, const char* paht, struct stat* buf) = NULL;
static ssize_t (*old_pwrite)(int fd, const void* buf, size_t count, off_t offset);
static FILE* (*old_fopen)(const char* filename, const char* mode) = NULL;
static int (*old_fclose)(FILE* stream) = NULL;
static size_t (*old_fread)(void* ptr, size_t size, size_t count, FILE* stream) = NULL;
static size_t (*old_fwrite)(const void* ptr, size_t size, size_t count, FILE* stream) = NULL;
static int (*old_fgetc)(FILE* stream);
static char* (*old_fgets)(char* str, int num, FILE* stream) = NULL;
static int (*old_fscanf)(FILE* stream, const char* format, ...) = NULL;
static int (*old_fprintf)(void* stream, const char* format, ...) = NULL;
static int (*old_chdir)(const char* path) = NULL;
static int (*old_chown)(const char* path, uid_t owner, gid_t group) = NULL;
static int (*old_chmod)(const char* path, mode_t mode) = NULL;
static int (*old_remove)(const char* filename) = NULL;
static int (*old_rename)(const char* old, const char* new)= NULL;
static int (*old_link)(const char* path1, const char* path2) = NULL;
static int (*old_unlink)(const char* path) = NULL;
static ssize_t (*old_readlink)(const char* path, char* buf, size_t bufsiz) = NULL;
static int (*old_symlink)(const char* path1, const char* path2) = NULL;
static int (*old_mkdir)(const char* path, mode_t mode) = NULL;
static int (*old_rmdir)(const char* path) = NULL;



__attribute__((constructor)) static void before_main()
{
    handle = dlopen("libc.so.6", RTLD_LAZY);
    
    old_closedir = (int(*)())dlsym(handle, "closedir");
    old_opendir = (DIR*(*)())dlsym(handle, "opendir");
    old_readdir = (struct dirent*(*)())dlsym(handle, "readdir");
    old_creat = (int(*)())dlsym(handle, "creat");
    old_open = (int(*)(const char* path, int oflag, ...))dlsym(handle, "open");
    old_read = (ssize_t(*)())dlsym(handle, "read");
    old_write = (ssize_t(*)())dlsym(handle, "write");
    old_dup = (int(*)())dlsym(handle, "dup");
    old_dup2 = (int(*)())dlsym(handle, "dup2");
    old_close = (int(*)())dlsym(handle, "close");
    old_lstat = (int(*)())dlsym(handle, "__lxstat");
    old_stat = (int(*)())dlsym(handle, "__xstat");
    old_pwrite = (ssize_t(*)())dlsym(handle, "pwrite");
    old_fopen = (FILE*(*)())dlsym(handle, "fopen");
    old_fclose = (int(*)())dlsym(handle, "fclose");
    old_fread = (size_t(*)())dlsym(handle, "fread");
    old_fwrite = (size_t(*)())dlsym(handle, "fwrite");
    old_fgetc = (int(*)())dlsym(handle, "fgetc");
    old_fgets = (char*(*)())dlsym(handle, "fgets");
    old_fscanf = (int(*)(FILE* stream, const char* format, ...))dlsym(handle, "__isoc99_fscanf");
    old_fprintf = (int(*)(void* stream, const char* format, ...))dlsym(handle, "fprintf");
    old_chdir  = (int(*)())dlsym(handle, "chdir");
    old_chown = (int(*)())dlsym(handle, "chown");
    old_chmod = (int(*)())dlsym(handle, "chmod");
    old_remove = (int(*)())dlsym(handle, "remove");
    old_rename = (int(*)())dlsym(handle, "rename");
    old_link = (int(*)())dlsym(handle, "link");
    old_unlink = (int(*)())dlsym(handle, "unlink");
    old_readlink = (ssize_t(*)())dlsym(handle, "readlink");
    old_symlink = (int(*)())dlsym(handle, "symlink");
    old_mkdir = (int(*)())dlsym(handle, "mkdir");
    old_rmdir = (int(*)())dlsym(handle, "rmdir");
    
    dlclose(handle);

    char* output_path = getenv("MONITOR_OUTPUT");
    if(output_path != NULL){
        int output_fd = old_open(output_path, O_RDWR | O_CREAT, 0666);
        old_dup2(output_fd, STDERR_FILENO);
    }
}

char* getpath(FILE* stream)
{
    int fd = fileno(stream);
    if (fd == 0)
        return "<STDIN>";
    else if (fd == 1)
        return "<STDOUT>";
    else if (fd == 2)
        return "<STDERR>";
    else
    {
        char path[1024];
        char result[1024];
        memset(path, '\0', sizeof(path));
        memset(result, '\0', sizeof(result));
        sprintf(path, "%s%d", "/proc/self/fd/", fd);
        old_readlink(path, result, sizeof(result)-1);
        char* ret = result;
        return ret;
    }
}

char* getpath2(int fd)
{
    if (fd == 0)
        return "<STDIN>";
    else if (fd == 1)
        return "<STDOUT>";
    else if (fd == 2)
        return "<STDERR>";
    else
    {
        char path[1024];
        char result[1024];
        memset(path, '\0', sizeof(path));
        memset(result, '\0', sizeof(result));
        sprintf(path, "%s%d", "/proc/self/fd/", fd);
        old_readlink(path, result, sizeof(result)-1);
        char* ret = result;
        return ret;
    }
}

char* getDIRpath(DIR* dirp){
    char path[1024];
    char result[1024];
    char cwd[1024];
    memset(path, '\0', sizeof(path));
    memset(result, '\0', sizeof(result));
    memset(cwd, '\0', sizeof(cwd));
    getcwd(cwd, sizeof(cwd));
    sprintf(path, "%s%d", "/proc/self/fd/", dirfd(dirp));
    readlinkat(dirfd(dirp), path, result, sizeof(result)-1);
    if(strcmp(result, cwd) == 0)
        strcpy(result, ".");

    char* ret = result;
    return ret;
}

int closedir(DIR* dirp){
    char* path = getDIRpath(dirp);
    int ret = old_closedir(dirp);
    old_fprintf(stderr, "closedir(\"%s\") = %d\n", path, ret);
    return ret;
}

DIR* opendir(const char* name)
{
    DIR* ret = old_opendir(name);
    old_fprintf(stderr, "opendir(\"%s\") = %p\n", name, ret);
    return ret;
}


struct dirent* readdir(DIR* dirp){
    char* path = getDIRpath(dirp);
    struct dirent* ret = old_readdir(dirp);
    if(ret == NULL)
        old_fprintf(stderr, "readdir(\"%s\") = %s\n", path, "NULL");
    else
        old_fprintf(stderr, "readdir(\"%s\") = %s\n", path, ret->d_name);
    return ret;
}



int creat(const char* path, mode_t mode)
{
    int ret = old_creat(path, mode);
    old_fprintf(stderr, "creat(\"%s\", %d) = %d\n", path, mode, ret);
    return ret;
}

int open(const char* path, int oflag, ...)
{
    va_list ap;
    va_start(ap, oflag);
    mode_t a1 = va_arg(ap, mode_t);

    int ret =old_open(path, oflag, a1);
    old_fprintf(stderr, "open(\"%s\", %d, ...) = %d\n", path, oflag, ret);
    return ret;
}

ssize_t read(int fd, void* buf, size_t nbyte)
{
    char* path = getpath2(fd);
    ssize_t ret = old_read(fd, buf, nbyte);
    old_fprintf(stderr, "read(\"%s\", %p, %d) = %d\n", path, buf ,nbyte, ret);
    return ret;
}

ssize_t write(int fd, const void* buf, size_t count)
{
    char* path = getpath2(fd);
    ssize_t ret = old_write(fd, buf, count);
    old_fprintf(stderr, "write(\"%s\", %p, %d) = %d\n", path, buf, count, ret);
    return ret;
}

int dup(int oldfd)
{
    char* path = getpath2(oldfd);
    int ret = old_dup(oldfd);
    old_fprintf(stderr, "dup(\"%s\") = %d\n", path, ret);
    return ret;
}

int dup2(int oldfd, int newfd)
{
    char* path = getpath2(oldfd);
    int ret = old_dup2(oldfd, newfd);
    old_fprintf(stderr, "dup2(\"%s\", %d) = %d\n", path, newfd, ret);
    return ret;
}

int close(int fd)
{
    char* path = getpath2(fd);
    int ret = old_close(fd);
    old_fprintf(stderr, "close(\"%s\") = %d\n", path, ret);
    return ret;
}

int __lxstat (int ver, const char *path, struct stat* buf)
{
    int ret = old_lstat(ver, path, buf);
    old_fprintf(stderr, "lstat(\"%s\", %p {mode=%o, size=%d}) = %d\n", path, buf, buf->st_mode, buf->st_size, ret);
    return ret;
}

int __xstat(int ver, const char* path, struct stat* buf)
{
    int ret = old_stat(ver, path, buf);
    old_fprintf(stderr, "stat(\"%s\", %p {mode=%o, size=%d}) = %d\n", path, buf, buf->st_mode, buf->st_size, ret);
    return ret;
}

ssize_t pwrite(int fd, const void* buf, size_t count, off_t offset)
{
    char* path = getpath2(fd);
    ssize_t ret = old_pwrite(fd, buf, count, offset);
    old_fprintf(stderr, "pwrite(\"%s\", %p, %ld, %ld) = %d\n", path, buf, count, offset, ret);
    return ret;
}

FILE* fopen(const char* filename, const char* mode)
{   
    if(old_fopen != NULL)
    {
        FILE* ret = old_fopen(filename, mode);
        old_fprintf(stderr, "fopen(\"%s\", \"%s\") = %p\n", filename, mode, ret);
        return ret;
    }
    return 0;
}

int fclose(FILE* stream)
{
    char* path = getpath(stream);
    int ret = old_fclose(stream);
    old_fprintf(stderr, "fclose(\"%s\") = %d\n", path, ret);
    return ret;
}

size_t fread(void* ptr, size_t size, size_t count, FILE* stream)
{
    char* path = getpath(stream);
    size_t ret = old_fread(ptr, size, count, stream);
    old_fprintf(stderr, "fread(%p, %ld, %ld, \"%s\") = %ld\n", ptr, size, count, path, ret);
    return ret;
}

size_t fwrite(const void* ptr, size_t size, size_t count, FILE* stream)
{
    char* path = getpath(stream);
    old_fprintf(stderr, "fwrite(%p, %ld, %ld, \"%s\") = %ld\n", ptr, size, count, path, count);
    return old_fwrite(ptr, size, count, stream);
}

int fgetc(FILE* stream)
{
    char* path = getpath(stream);
    int ret = old_fgetc(stream);
    old_fprintf(stderr, "fgetc(\"%s\") = %d\n", path, ret);
    return ret;
}

char* fgets(char* str, int num, FILE* stream)
{
    char* path = getpath(stream);
    char* ret = old_fgets(str, num, stream);
    old_fprintf(stderr, "fgets(%p, %d, \"%s\") = \"%s\"\n",str, num, path, ret);
    return ret;
}

int __isoc99_fscanf(FILE * stream, const char * fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    int* a1 = va_arg(ap, int*);
    
    char* result = getpath(stream);
    int ret = old_fscanf(stream, fmt, a1);
    old_fprintf(stderr,"scanf(\"%s\", \"%s\", ...) = %d\n",result, fmt, ret);
    return ret;
}

int fprintf (FILE * stream, const char * fmt, ...)
{
    char* path = getpath(stream);
    va_list ap;
    va_start(ap, fmt);
    int a1 = va_arg(ap, int);
    int a2 = va_arg(ap, int);
    int a3 = va_arg(ap, int);
    int a4 = va_arg(ap, int);
    int a5 = va_arg(ap, int);
    
    int ret = old_fprintf(stream, fmt, a1, a2, a3, a4, a5);
    old_fprintf(stderr, "fprintf (\"%s\", \"%s\", ...) = %d\n", path, fmt, ret);

    return ret;
}

int chdir(const char* path)
{
    int ret = old_chdir(path);
    old_fprintf(stderr, "chdir(\"%s\") = %d\n", path, ret);
    return ret;
}

int chown(const char* path, uid_t owner, gid_t group)
{
    int ret = old_chown(path, owner, group);
    old_fprintf(stderr, "chown(\"%s\", %d, %d) = %d\n", path, owner, group, ret);
    return ret;
}

int chmod(const char* path, mode_t mode)
{
    int ret = old_chmod(path, mode);
    old_fprintf(stderr, "chmod(\"%s\", %d) = %d\n", path, mode, ret);
    return ret;
}

int remove(const char* filename)
{
    int ret = old_remove(filename);
    old_fprintf(stderr, "remove(\"%s\") = %d\n", filename, ret);
    return ret;
}

int rename (const char* old, const char* new)
{
    int ret = old_rename(old, new);
    old_fprintf(stderr, "rename(\"%s\", \"%s\") = %d\n", old, new, ret);
    return ret;
}

int link(const char* path1, const char* path2)
{
    int ret = old_link(path1, path2);
    old_fprintf(stderr, "link(\"%s\", \"%s\") = %d\n", path1, path2, ret);
    return ret;
}

int unlink(const char* path)
{
    int ret = old_unlink(path);
    old_fprintf(stderr, "unlink(\"%s\") = %d\n", path, ret);
    return ret;
}

ssize_t readlink(const char* path, char* buf, size_t bufsiz){
    if(old_readlink != NULL)
    {
        ssize_t ret = old_readlink(path, buf, bufsiz);
        old_fprintf(stderr, "readlink(\"%s\", \"%s\", %ld) = %ld\n", path, buf, bufsiz, ret);
        return ret;
    } 
    return 0;
}

int symlink(const char* path1, const char* path2)
{
    int ret = old_symlink(path1, path2);
    old_fprintf(stderr, "symlink(\"%s\", \"%s\") = %d\n", path1, path2, ret);
    return ret;
}

int mkdir(const char* path, mode_t mode)
{
    int ret = old_mkdir(path, mode);
    old_fprintf(stderr, "mkdir(\"%s\", %d) = %d\n", path, mode, ret);
    return ret;
}

int rmdir(const char* path)
{
    int ret = old_rmdir(path);
    old_fprintf(stderr, "rmdir(\"%s\") = %d\n", path, ret);
    return ret;
}