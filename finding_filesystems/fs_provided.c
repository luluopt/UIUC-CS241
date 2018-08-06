/**
* Finding Filesystems
* CS 241 - Fall 2017
*/
#include "fs.h"
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

void fs_stat(file_system *fs, char *path) {
    inode *node = get_inode(fs, path);
    if (!node)
        return;
    printf("  File: '%s'\n", path);
    printf("  Size: %zu\t", node->size);
    printf("Blocks: %zu\t",
           (node->size + sizeof(data_block) - 1) / sizeof(data_block));

    if (is_directory(node)) {
        printf("directory\n");
    } else {
        printf("regular file\n");
    }

    printf("Links: %d\n", node->nlink);

    printf("Access: (0%o)  ", node->mode & 0777);
    printf("Uid: (%d)   ", node->uid);
    printf("Gid: (%d)\n", node->gid);

    char buffer[100];
    strftime(buffer, 100, "%F %H:%M:%S %z", localtime(&node->atim));
    printf("Access: %s\n", buffer);
    strftime(buffer, 100, "%F %H:%M:%S %z", localtime(&node->mtim));
    printf("Modify: %s\n", buffer);
    strftime(buffer, 100, "%F %H:%M:%S %z", localtime(&node->ctim));
    printf("Change: %s\n", buffer);
}
