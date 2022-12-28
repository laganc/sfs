#include <stdio.h>
#include <stdint.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <netinet/in.h>

// Struct for superblock
struct __attribute__((__packed__)) superblock_t {
    uint8_t   fs_id [8];
    uint16_t block_size;
    uint32_t file_system_block_count;
    uint32_t fat_start_block;
    uint32_t fat_block_count;
    uint32_t root_dir_start_block;
    uint32_t root_dir_block_count;
};

// Function to iterate over FAT table to find free, reserved, and allocated FAT entries
void iterateFAT(char* address, uint32_t fat_block_count, uint32_t fat_start, uint16_t block_size, uint32_t block_count){
    int reserved = 0;
    int allocated = 0;

    // Start is the byte of the first FAT block
    int start = fat_start*block_size;
    // End is the last byte of the FAT
    int end = fat_block_count*block_size;

    uint32_t ptr;

    // Loop over entire FAT, get 4 bytes at a time
    for(int i = start; i < start+end; i+=4){
        memcpy(&ptr, address+i, 4);

        // If FAT entry is empty, do nothing
        if(ntohl(ptr) == 0x00);
        // If FAT entry is 1, it is reserved
        else if(ntohl(ptr) == 0x00000001) reserved++;
        // If FAT entry is greater than 1, it is allocated
        else if (ntohl(ptr) > 0x00000001) allocated++;
    }
    printf("Free Blocks: %d\nReserved Blocks: %d\nAllocated Blocks: %d\n", block_count - reserved - allocated, reserved, allocated);
}

int main(int argc, char* argv[]) {

    int fd = open(argv[1], O_RDWR);
    struct stat buffer;
    int status = fstat(fd, &buffer);

    //tamplate:   pa=mmap(addr, len, prot, flags, fildes, off);
    //c will implicitly cast void* to char*, while c++ does NOT
    void* address=mmap(NULL, buffer.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    struct superblock_t* sb;
    sb=(struct superblock_t*)address;
    printf("Super block information:\n");
    printf("Block size: %d\n", ntohs(sb->block_size));
    printf("Block count: %d\n", ntohl(sb->file_system_block_count));
    printf("FAT starts: %d\n", ntohl(sb->fat_start_block));
    printf("FAT blocks: %d\n", ntohl(sb->fat_block_count));
    printf("Root directory start: %d\n", ntohl(sb->root_dir_start_block));
    printf("Root directory blocks: %d\n", ntohl(sb->root_dir_block_count));

    printf("\nFAT information:\n");
    iterateFAT(address, ntohl(sb->fat_block_count), ntohl(sb->fat_start_block), ntohs(sb->block_size), ntohl(sb->file_system_block_count));

    munmap(address,buffer.st_size);
    close(fd);
}
