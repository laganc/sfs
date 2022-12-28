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

// Struct for dictionary entry time
struct __attribute__((__packed__)) dir_entry_timedate_t {
    uint16_t year;
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
};

// Struct for dictionary entry
struct __attribute__((__packed__)) dir_entry_t {
    uint8_t status;
    uint32_t starting_block;
    uint32_t block_count;
    uint32_t size;
    struct dir_entry_timedate_t create_time;
    struct dir_entry_timedate_t modify_time;
    uint8_t filename[31];
    uint8_t unused[6];
};

// Function to iterate over root directory to find all files
void dir_entry(char* address, uint32_t root_dir_count, uint32_t root_start, uint16_t block_size){
    // Start is the byte of the first root directory block
    int start = root_start*block_size;
    // End is the last byte of the root directory
    int end = root_dir_count*block_size;
    address += start;

    for(int i = start; i < start+end; i+=64){
        struct dir_entry_t* de;
        de=(struct dir_entry_t*)address;

        // If directory entry is not empty
        if(de->status != 0x00){
            if(de->status == 3) printf("F ");       // If status == 3, then entry is a file
            else if(de->status == 5) printf("D ");  // If status == 5, then entry is a directory
            printf("%10d ", ntohl(de->size));       // File size
            printf("%30s ", de->filename);          // File name
            printf("%04d/%02d/%02d %02d:%02d:%02d\n", ntohs(de->create_time.year), de->create_time.month, de->create_time.day, de->create_time.hour, de->create_time.minute, de->create_time.second);
        }
        address += 64;
    }
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

    dir_entry(address, ntohl(sb->root_dir_block_count), ntohl(sb->root_dir_start_block), ntohs(sb->block_size));

    munmap(address,buffer.st_size);
    close(fd);
}
