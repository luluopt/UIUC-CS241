/**
 * Mapping Memory Lab
 * CS 241 - Fall 2017
 */
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "mmap.h"

int read_1_byte_4k_file() {
  char *four_kb_test_file_path = "4k_file.bin";
  FILE *fptr = fopen(four_kb_test_file_path, "w+");
  assert(fptr);
  for (int i = 0; i < PAGE_SIZE; i++) {
    char test_char = i;
    fwrite(&test_char, 1, 1, fptr);
  }

  fclose(fptr);

  mmap *four_kb_mmap = mmap_create(four_kb_test_file_path);

  char buff[1];
  size_t bytes_read = mmap_read(four_kb_mmap, buff, 0, 1);
  assert(bytes_read == 1);
  assert(buff[0] == 0);
  unlink(four_kb_test_file_path);
  return 0;
}

int read_4k_byte_4k_file() {
  char *four_kb_test_file_path = "4k_file.bin";
  FILE *fptr = fopen(four_kb_test_file_path, "w+");
  assert(fptr);
  for (int i = 0; i < PAGE_SIZE; i++) {
    char test_char = i;
    fwrite(&test_char, 1, 1, fptr);
  }

  fclose(fptr);

  mmap *four_kb_mmap = mmap_create(four_kb_test_file_path);

  char buff[PAGE_SIZE];
  size_t bytes_read = mmap_read(four_kb_mmap, buff, 0, PAGE_SIZE);
  assert(bytes_read == PAGE_SIZE);
  for (int i = 0; i < PAGE_SIZE; i++) {
    char test_char = i;
    assert(buff[i] == test_char);
  }

  unlink(four_kb_test_file_path);
  munmap(four_kb_mmap);
  return 0;
}

int read_8k_byte_8k_file() {
  char *eight_kb_test_file_path = "8k_file.bin";
  FILE *fptr = fopen(eight_kb_test_file_path, "w+");
  assert(fptr);
  for (int i = 0; i < 2 * PAGE_SIZE; i++) {
    char test_char = i;
    fwrite(&test_char, 1, 1, fptr);
  }

  fclose(fptr);

  mmap *eight_kb_mmap = mmap_create(eight_kb_test_file_path);

  char buff[2 * PAGE_SIZE];
  size_t bytes_read = mmap_read(eight_kb_mmap, buff, 0, 2 * PAGE_SIZE);
  assert(bytes_read == 2 * PAGE_SIZE);
  for (int i = 0; i < 2 * PAGE_SIZE; i++) {
    char test_char = i;
    assert(buff[i] == test_char);
  }

  unlink(eight_kb_test_file_path);
  munmap(eight_kb_mmap);
  return 0;
}

int read_4k_byte_8k_file_2k_offset() {
  char *eight_kb_test_file_path = "8k_file.bin";
  FILE *fptr = fopen(eight_kb_test_file_path, "w+");
  assert(fptr);
  for (int i = 0; i < 2 * PAGE_SIZE; i++) {
    char test_char = i;
    fwrite(&test_char, 1, 1, fptr);
  }

  fclose(fptr);

  mmap *eight_kb_mmap = mmap_create(eight_kb_test_file_path);

  char buff[PAGE_SIZE];
  size_t bytes_read = mmap_read(eight_kb_mmap, buff, PAGE_SIZE / 2, PAGE_SIZE);
  assert(bytes_read == PAGE_SIZE);
  for (int i = 0; i < PAGE_SIZE; i++) {
    char test_char = i + (PAGE_SIZE / 2);
    assert(buff[i] == test_char);
  }

  unlink(eight_kb_test_file_path);
  munmap(eight_kb_mmap);
  return 0;
}

int write_1_byte_4k_file() {
  char *four_kb_test_file_path = "4k_file.bin";
  FILE *fptr = fopen(four_kb_test_file_path, "w+");
  assert(fptr);
  for (int i = 0; i < PAGE_SIZE; i++) {
    char test_char = i;
    fwrite(&test_char, 1, 1, fptr);
  }

  fclose(fptr);

  mmap *four_kb_mmap = mmap_create(four_kb_test_file_path);

  char test_char = 'a';
  size_t bytes_written = mmap_write(four_kb_mmap, &test_char, 0, 1);
  assert(bytes_written == 1);
  char buff[1];
  size_t bytes_read = mmap_read(four_kb_mmap, buff, 0, 1);
  assert(bytes_read == 1);
  assert(buff[0] == 'a');
  munmap(four_kb_mmap);
  fptr = fopen(four_kb_test_file_path, "r");
  fread(buff, 1, 1, fptr);
  assert(buff[0] == 'a');
  fclose(fptr);
  unlink(four_kb_test_file_path);
  return 0;
}

int write_4k_byte_4k_file() {
  char *four_kb_test_file_path = "4k_file.bin";
  FILE *fptr = fopen(four_kb_test_file_path, "w+");
  assert(fptr);
  for (int i = 0; i < PAGE_SIZE; i++) {
    char test_char = i;
    fwrite(&test_char, 1, 1, fptr);
  }

  fclose(fptr);

  mmap *four_kb_mmap = mmap_create(four_kb_test_file_path);

  char test_char = 'a';
  for (int i = 0; i < PAGE_SIZE; i++) {
    size_t bytes_written = mmap_write(four_kb_mmap, &test_char, i, 1);
    assert(bytes_written == 1);
    char buff[1];
    size_t bytes_read = mmap_read(four_kb_mmap, buff, i, 1);
    assert(bytes_read == 1);
    assert(buff[0] == 'a');
  }

  munmap(four_kb_mmap);
  char buff[PAGE_SIZE];
  fptr = fopen(four_kb_test_file_path, "r");
  fread(buff, 1, PAGE_SIZE, fptr);
  for (int i = 0; i < PAGE_SIZE; i++) {
    assert(buff[i] == 'a');
  }

  fclose(fptr);
  unlink(four_kb_test_file_path);
  return 0;
}

int write_4k_byte_8k_file_2k_offset() {
  char *eight_kb_test_file_path = "8k_file.bin";
  FILE *fptr = fopen(eight_kb_test_file_path, "w+");
  assert(fptr);
  for (int i = 0; i < 2 * PAGE_SIZE; i++) {
    char test_char = i;
    fwrite(&test_char, 1, 1, fptr);
  }

  fclose(fptr);

  mmap *eight_kb_mmap = mmap_create(eight_kb_test_file_path);

  for (int i = 0; i < PAGE_SIZE; i++) {
    char test_char = 'a';
    size_t bytes_written =
        mmap_write(eight_kb_mmap, &test_char, PAGE_SIZE / 2 + i, 1);
    assert(bytes_written == 1);
  }

  char buff[2 * PAGE_SIZE];
  size_t bytes_read = mmap_read(eight_kb_mmap, buff, 0, sizeof(buff));
  assert(bytes_read == sizeof(buff));

  for (int i = 0; i < PAGE_SIZE / 2; i++) {
    char test_char = i;
    assert(buff[i] == test_char);
  }

  for (int i = PAGE_SIZE / 2; i < PAGE_SIZE + PAGE_SIZE / 2; i++) {
    char test_char = 'a';
    assert(buff[i] == test_char);
  }

  for (int i = PAGE_SIZE + PAGE_SIZE / 2; i < 2 * PAGE_SIZE; i++) {
    char test_char = i;
    assert(buff[i] == test_char);
  }

  munmap(eight_kb_mmap);

  char buff2[sizeof(buff)];
  fptr = fopen(eight_kb_test_file_path, "r");
  fread(buff2, 1, sizeof(buff), fptr);
  for (int i = 0; i < 2 * PAGE_SIZE; i++) {
    assert(buff[i] == buff2[i]);
  }
  fclose(fptr);
  unlink(eight_kb_test_file_path);
  return 0;
}

int write_only_dirty_pages_back() {
  char *eight_kb_test_file_path = "8k_file.bin";
  FILE *fptr = fopen(eight_kb_test_file_path, "w+");
  assert(fptr);
  for (int i = 0; i < 2 * PAGE_SIZE; i++) {
    char test_char = i;
    fwrite(&test_char, 1, 1, fptr);
  }

  fclose(fptr);

  mmap *eight_kb_mmap = mmap_create(eight_kb_test_file_path);

  // read all bytes to load both pages
  char buff[2 * PAGE_SIZE];
  size_t bytes_read = mmap_read(eight_kb_mmap, buff, 0, sizeof(buff));
  assert(bytes_read == sizeof(buff));

  // write only to the second page
  bytes_read = mmap_read(eight_kb_mmap, buff, 0, sizeof(buff));
  assert(bytes_read == sizeof(buff));

  for (int i = 0; i < PAGE_SIZE / 2; i++) {
    char test_char = i;
    assert(buff[i] == test_char);
  }

  for (int i = PAGE_SIZE / 2; i < PAGE_SIZE + PAGE_SIZE / 2; i++) {
    char test_char = 'a';
    assert(buff[i] == test_char);
  }

  for (int i = PAGE_SIZE + PAGE_SIZE / 2; i < 2 * PAGE_SIZE; i++) {
    char test_char = i;
    assert(buff[i] == test_char);
  }

  munmap(eight_kb_mmap);

  char buff2[sizeof(buff)];
  fptr = fopen(eight_kb_test_file_path, "r");
  fread(buff2, 1, sizeof(buff), fptr);
  for (int i = 0; i < 2 * PAGE_SIZE; i++) {
    assert(buff[i] == buff2[i]);
  }

  unlink(eight_kb_test_file_path);
  return 0;
}

int main(int argc, char *argv[]) {
  int result = 13;
  // Please add more test cases
  if (argc != 2) {
    fprintf(stderr, "%s\n", "Needs test number");
    return result;
  }

  int test_number = atoi(argv[1]);
  switch (test_number) {
  default:
    fprintf(stderr, "%s\n", "Invalid test number");
    break;
  case 1:
    result = read_1_byte_4k_file();
    break;
  case 2:
    result = read_4k_byte_4k_file();
    break;
  case 3:
    result = read_8k_byte_8k_file();
    break;
  case 4:
    result = read_4k_byte_8k_file_2k_offset();
    break;
  case 5:
    result = write_1_byte_4k_file();
    break;
  case 6:
    result = write_4k_byte_4k_file();
    break;
  case 7:
    result = write_4k_byte_8k_file_2k_offset();
    break;
  }

  return result;
}
