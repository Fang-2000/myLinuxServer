#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <string.h>
#include <stdlib.h>

int main()
{
    // 打开源文件，获取文件长度并创建对应内存映射区
    int fdSource = open("english.txt", O_RDONLY);
    int len = lseek(fdSource, 0, SEEK_END);
    void *ptrSource = mmap(NULL, len, PROT_READ, MAP_SHARED, fdSource, 0);
    if (ptrSource == MAP_FAILED) {
        perror("mmap");
        exit(-1);
    }
    
    // 打开目标文件，并创建对应内存映射区
    int fdTarget = open("target.txt", O_RDWR | O_CREAT, 0664);
    // 由于目标文件是通过创建得到，所以需要扩展长度与源文件保持一致
    truncate("target.txt", len);
    // 如果不加，扩展可能失败（保险起见）
    write(fdTarget, " ", 1);
    void *ptrTarget = mmap(NULL, len, PROT_READ | PROT_WRITE, MAP_SHARED, fdTarget, 0);
    if (ptrTarget == MAP_FAILED) {
        perror("mmap");
        exit(-1);
    }
    
    // 内存拷贝
    memcpy(ptrTarget, ptrSource, len);
    
    // 回收资源
    close(fdTarget);
    close(fdSource);
    munmap(ptrTarget, len);
    munmap(ptrSource, len);

    return 0;
}
