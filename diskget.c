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
    uint8_t  fs_id [8];
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

// Function to check if a file exists in file system, and if it does, copy it into current directory
void get_file(char* address, char* address_extra, uint32_t root_dir_count, uint32_t root_start, uint16_t block_size, uint32_t fat_start_block, char* new_file_name, char* file_name){
    // Start is the byte of the first root directory block
    int start = root_start*block_size;
    // End is the last byte of the root directory
    int end = root_dir_count*block_size;
    address += start;

    size_t n = sizeof(file_name);
    uint32_t current_block = -1;
    uint32_t current_file_size;
    uint32_t fat_entry;
    uint8_t file_contents[block_size];

    // Loop through all directory entries to check if file exists
    for(int i = start; i < start+end; i+=64){
        struct dir_entry_t* de;
        de=(struct dir_entry_t*)address;

        if(de->status != 0x00){
            if(memcmp(de->filename, file_name, n) == 0){
                current_block = ntohl(de->starting_block);
                current_file_size = ntohl(de->size);
            }
        }
        address += 64;
    }

    // If file doesn't exist, exit
    if(current_block == -1){
        printf("File not found.\n");
    }
    // If file exists, create new file, and write to it
    else{
        FILE *fp;
        fp = fopen(new_file_name, "w+");
        while(1){
            memcpy(&file_contents, address_extra+block_size*current_block, block_size);
            fputs(file_contents, fp);
            memcpy(&fat_entry, address_extra+fat_start_block*block_size + 4*current_block, 4);
            current_block = ntohl(fat_entry);
            if(fat_entry == -1) break;
        }
    }
}

int main(int argc, char* argv[]) {

    int fd = open(argv[1], O_RDWR);
    struct stat buffer;
    int status = fstat(fd, &buffer);

    char* new_file_name = argv[2] + 1;
    char* file_name = argv[3];

    //tamplate:   pa=mmap(addr, len, prot, flags, fildes, off);
    //c will implicitly cast void* to char*, while c++ does NOT
    void* address=mmap(NULL, buffer.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    struct superblock_t* sb;
    sb=(struct superblock_t*)address;

    get_file(address, address, ntohl(sb->root_dir_block_count), ntohl(sb->root_dir_start_block), ntohs(sb->block_size), ntohl(sb->fat_start_block), new_file_name, file_name);

    munmap(address,buffer.st_size);
    close(fd);
}
