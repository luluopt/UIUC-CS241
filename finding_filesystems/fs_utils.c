/**
* Finding Filesystems
* CS 241 - Fall 2017
*/
#include "fs.h"
#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

file_system *open_fs(const char *path) {
    if (!path) {
        return NULL;
    }
    struct stat file_stat;
    int open_flags = O_RDONLY;
    int mmap_flags = PROT_READ;

    int fd = open(path, open_flags);
    if (fd == -1) {
        printf("Failed to open file: %s\n", path);
        exit(1);
    }
    if (fstat(fd, &file_stat) != 0) {
        printf("Failed to stat file: %s\n", path);
        exit(1);
    }
    char *file = mmap(NULL, file_stat.st_size, mmap_flags, MAP_SHARED, fd, 0);
    if (file == (void *)-1) {
        printf("Failed to map: %s\n", path);
        exit(1);
    }
    close(fd);
    superblock *metadata = (void *)file;
    file_system *my_fs = malloc(sizeof(*my_fs));
    my_fs->meta = (void *)file;
    my_fs->inode_root = (void *)(file + sizeof(superblock) + DATA_NUMBER);
    my_fs->data_root = (void *)(my_fs->inode_root + metadata->inode_count);

    return my_fs;
}

void close_fs(file_system **fs_pointer) {
    assert(fs_pointer);
    assert(*fs_pointer);
    superblock *meta = (*fs_pointer)->meta;
    munmap(meta, meta->size);
    free(*fs_pointer);
    *fs_pointer = NULL;
}

void set_data_used(file_system *fs_pointer, data_block_number data_number,
                   int used) {
    used = !!(used); // Gives a hard 1 or 0 prediction
    if (data_number < 0 || data_number >= DATA_NUMBER) {
        return;
    }
    fs_pointer->meta->data_map[data_number] = used;
}

data_block_number get_data_used(file_system *fs_pointer, int data_number) {
    if (data_number < 0 || data_number >= DATA_NUMBER) {
        return -1;
    }
    return fs_pointer->meta->data_map[data_number];
}

inode_number first_unused_inode(file_system *fs_pointer) {
    assert(fs_pointer);

    uint64_t i;
    for (i = 1; i < INODES_NUMBER; ++i) {
        int used = fs_pointer->inode_root[i].nlink;
        if (used == 0) {
            return i;
        }
    }
    return -1;
}

data_block_number first_unused_data(file_system *fs_pointer) {
    assert(fs_pointer);

    uint64_t i;
    uint64_t filled = 0;
    while (filled < DATA_NUMBER) {
        filled = 0;
        for (i = 0; i < DATA_NUMBER; ++i) {
            int used = get_data_used(fs_pointer, i);
            if (!used && rand() % 2) {
                return i;
            } else if (used) {
                filled++;
            }
        }
    }
    return -1;
}

data_block_number add_data_block_to_inode(file_system *fs_pointer,
                                          inode *node) {
    assert(fs_pointer);
    assert(node);

    int i;
    for (i = 0; i < NUM_DIRECT_INODES; ++i) {
        if (node->direct[i] == -1) {
            data_block_number first_data = first_unused_data(fs_pointer);
            if (first_data == -1) {
                return -1;
            }
            node->direct[i] = first_data;
            set_data_used(fs_pointer, first_data, 1);
            return first_data;
        }
    }
    return 0;
}

data_block_number add_data_block_to_indirect_block(file_system *fs_pointer,
                                                   data_block_number *blocks) {
    assert(fs_pointer);
    assert(blocks);

    int i;
    for (i = 0; i < NUM_DIRECT_INODES; ++i) {
        if (blocks[i] == UNASSIGNED_NODE) {
            data_block_number first_data = first_unused_data(fs_pointer);
            if (first_data == -1) {
                return -1;
            }
            blocks[i] = first_data;
            set_data_used(fs_pointer, first_data, 1);
            return first_data;
        }
    }
    return 0;
}
inode_number add_single_indirect_block(file_system *fs_pointer, inode *node) {
    assert(fs_pointer);
    assert(node);

    if (node->indirect != UNASSIGNED_NODE)
        return -1;
    data_block_number first_data = first_unused_data(fs_pointer);
    if (first_data == -1) {
        return -1;
    }
    node->indirect = first_data;
    set_data_used(fs_pointer, first_data, 1);
    node->nlink = 1;
    int i;
    data_block_number *block_array =
        (data_block_number *)(fs_pointer->data_root + first_data);
    for (i = 0; i < NUM_DIRECT_INODES; ++i) {
        block_array[i] = UNASSIGNED_NODE;
    }
    return 0;
}

inode *parent_directory(file_system *fs, char *path, char **filename) {
    assert(fs);
    assert(path);

    int len = strlen(path);
    char *endptr = path + len;
    while (*endptr != '/') {
        endptr--;
    }
    *endptr = '\0';

    if (filename) {
        *filename = endptr + 1;
    }
    inode *nody = get_inode(fs, path);
    return nody;
}

int valid_filename(const char *filename) {
    assert(filename);

    if (*filename == '\0') {
        return 0;
    }
    while (*filename) {
        if (*filename == '/') {
            return 0;
        }
        filename++;
    }
    return 1;
}

void init_inode(inode *parent, inode *init) {
    assert(parent);
    assert(init);

    init->uid = parent->uid;
    init->gid = parent->gid;
    init->mode = (TYPE_FILE << RWX_BITS_NUMBER) | (parent->mode & 0777);
    init->nlink = 1;
    time(&init->atim);
    time(&init->mtim);
    time(&init->ctim);
    init->size = 0;
    init->indirect = UNASSIGNED_NODE;
    int i;
    for (i = 0; i < NUM_DIRECT_INODES; ++i)
        init->direct[i] = UNASSIGNED_NODE;
}

inode *find_inode_named(file_system *fs, inode *root, const char *name) {
    assert(fs);
    assert(root);
    assert(name);

    uint64_t size = root->size;
    int count = 0;
    dirent node;
    data_block_number *block_array = root->direct;
    while (size > 0 && root->direct[count] != -1) {
        char *block = (char *)(fs->data_root + block_array[count]);
        char *endptr = block + sizeof(data_block);
        while (size > 0 && block < endptr) {
            make_dirent_from_string(block, &node);
            if (strcmp(name, node.name) == 0) {
                return fs->inode_root + node.inode_num;
            }
            block += FILE_NAME_ENTRY;
            size -= FILE_NAME_ENTRY;
        }
        count++;
        if (count == NUM_DIRECT_INODES) {
            if (root->indirect == UNASSIGNED_NODE) {
                break;
            }
            block_array = (data_block_number *)(fs->data_root + root->indirect);
            count = 0;
        }
    }
    return NULL;
}

inode *get_inode(file_system *fs, char *path) {
    assert(fs);
    assert(path);

    if (*path == '\0') {
        return fs->inode_root;
    }
    if (*path != '/') {
        return NULL;
    }
    char *path_cpy = strdup(path);
    char *tok = strtok(path_cpy, "/");
    inode *node = fs->inode_root;
    while (node && tok && *tok != '\0') {
        if (!is_directory(node)) {
            node = NULL;
            continue;
        }
        node = find_inode_named(fs, node, tok);
        if (node) {
            tok = strtok(NULL, "/");
        }
    }
    free(path_cpy);
    return node;
}

int is_file(inode *node) {
    return (node->mode >> RWX_BITS_NUMBER) == TYPE_FILE;
}

int is_directory(inode *node) {
    return (node->mode >> RWX_BITS_NUMBER) == TYPE_DIRECTORY;
}

int make_dirent_from_string(char *block, dirent *to_fill) {
    char inode_number[9];
    memcpy(inode_number, block + FILE_NAME_LENGTH, 8);
    inode_number[8] = '\0';
    long offset = strtoll(inode_number, NULL, 16);
    if (offset == 0) {
        perror("strtoll:");
        return 0;
    }
    to_fill->inode_num = offset;
    to_fill->name = block;
    return 1;
}
