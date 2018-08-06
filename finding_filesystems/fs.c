/**
* Finding Filesystems
* CS 241 - Fall 2017
*/
#include "format.h"
#include "fs.h"
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

void fs_ls(file_system *fs, char *path)
{
    inode *fs_i = get_inode(fs, path);

    if (!fs_i) {
        print_no_file_or_directory();
        exit(1);
    }

    // Calculate the number of blocks this inode would need
    int num_blocks = (fs_i->size + sizeof(data_block) - 1) / sizeof(data_block);
    int block_id   = -1;

    inode      *inode_root = fs->inode_root;

    data_block *block_root = fs->data_root;
    data_block *curr_block = NULL;
    char       *block_char_ptr = NULL;

    int num_entries   = fs_i->size / FILE_NAME_ENTRY;
    dirent *dir_entry = malloc(sizeof(dirent));

    int max_entries_per_block = sizeof(data_block) / FILE_NAME_ENTRY;
    int rc;

    // Read each direct block
    for (int i=0; i<num_blocks && i<NUM_DIRECT_INODES; i++) {
        block_id = fs_i->direct[i]; 
        if (block_id == UNASSIGNED_NODE) {
            // We should technically never reach here
            break;
        }

        // Get the pointer to the block, and cast it to a char*
        curr_block     = block_root + block_id; 
        block_char_ptr = (char*)curr_block;
        
        for (int j=0; j < max_entries_per_block && num_entries > 0; j++) {
            memset(dir_entry, 0, sizeof(dirent));
            rc = make_dirent_from_string(block_char_ptr + j*FILE_NAME_ENTRY, dir_entry);
            if (rc != 1) {
                fprintf(stderr, "ERROR: malformed directory entry\n");
                exit(1);
            }
            
            if (is_directory(inode_root + dir_entry->inode_num)) {
                print_directory(dir_entry->name);
            } else {
                print_file(dir_entry->name);
            }
            
            num_entries--;
        }
    }

    // Read indirect blocks,
    if (num_blocks > NUM_DIRECT_INODES) {
        // Cast the indirect block indexes to an array of ints
        data_block        *indirect_block = block_root + fs_i->indirect;
        data_block_number *level2         = (data_block_number*)indirect_block;
        
        num_blocks -= NUM_DIRECT_INODES;
        
        // Read each block
        for (int i=0; i<num_blocks && i<NUM_DIRECT_INODES; i++) {
            block_id = level2[i]; 
            if (block_id == UNASSIGNED_NODE) {
                // We should technically never reach here
                break;
            }

            // Get the pointer to the block, and cast it to a char*
            curr_block     = block_root + block_id; 
            block_char_ptr = (char*)curr_block;
            
            for (int j=0; j < max_entries_per_block && num_entries > 0; j++) {
                memset(dir_entry, 0, sizeof(dirent));
                rc = make_dirent_from_string(block_char_ptr + j*FILE_NAME_ENTRY, dir_entry);
                if (rc != 1) {
                    fprintf(stderr, "ERROR: malformed directory entry\n");
                    exit(1);
                }
                
                if (is_directory(inode_root + dir_entry->inode_num)) {
                    print_directory(dir_entry->name);
                } else {
                    print_file(dir_entry->name);
                }
                
                num_entries--;
            }
        }
    }
    
    free(dir_entry);
}

void fs_cat(file_system *fs, char *path)
{
    inode *fs_i = get_inode(fs, path);

    if (!fs_i) {
        print_no_file_or_directory();
        exit(1);
    }

    // Calculate the number of blocks this inode would need
    int num_blocks = (fs_i->size + sizeof(data_block) - 1) / sizeof(data_block);
    int block_id   = -1;
    int num_bytes  = fs_i->size;

    data_block *block_root = fs->data_root;
    data_block *curr_block = NULL;
    char       *block_char_ptr = NULL;

    int bytes_per_block = sizeof(data_block);

    // Read each direct block
    for (int i=0; i<num_blocks && i<NUM_DIRECT_INODES; i++) {
        block_id = fs_i->direct[i]; 
        if (block_id == UNASSIGNED_NODE) {
            // We should technically never reach here
            break;
        }

        // Get the pointer to the block, and cast it to a char*
        curr_block     = block_root + block_id; 
        block_char_ptr = (char*)curr_block;
        
        for (int j=0; j < bytes_per_block && num_bytes > 0; j++) {
            printf("%c", *block_char_ptr);            

            block_char_ptr++;
            num_bytes--;
        }
    }


    // Read indirect blocks,
    if (num_blocks > NUM_DIRECT_INODES) {
        // Cast the indirect block indexes to an array of ints
        data_block        *indirect_block = block_root + fs_i->indirect;
        data_block_number *level2         = (data_block_number*)indirect_block;
        
        num_blocks -= NUM_DIRECT_INODES;
        
        // Read each block
        for (int i=0; i<num_blocks && i<NUM_DIRECT_INODES; i++) {
            block_id = level2[i]; 
            if (block_id == UNASSIGNED_NODE) {
                // We should technically never reach here
                break;
            }

            // Get the pointer to the block, and cast it to a char*
            curr_block     = block_root + block_id; 
            block_char_ptr = (char*)curr_block;
            
            for (int j=0; j < bytes_per_block && num_bytes > 0; j++) {
                printf("%c", *block_char_ptr);            

                block_char_ptr++;
                num_bytes--;
            }
        }
    }
}
