/* Copyright (C) 2014-2017 Ben Kurtovic <ben.kurtovic@gmail.com>
   Released under the terms of the MIT License. See LICENSE for details. */

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "save.h"
#include "mmu.h"
#include "util.h"

static const char *MAGIC = "CRATER GAMEGEAR SAVE FILE\n";
static size_t HEADER_LEN = 64;

/*
    Log an error while trying to load the save file.
*/
static void log_error(const char *action, const char *path, const char *reason)
{
    ERROR("couldn't %s save file '%s': %s", action, path, reason)
}

/*
    Log an error in a standard library function.
*/
static void log_stdlib_error(const char *action, const char *path,
    const char *func)
{
    ERROR("couldn't %s save file '%s': %s(): %s",
        action, path, func, strerror(errno))
}

/*
    Parse the header of a save file, and return whether it is valid.

    If the load succeeds and off is not NULL, it will be set to the address
    of the first byte of non-header data.
*/
static bool parse_save_header(void *ptr, size_t size, size_t *off,
    const ROM *rom, const char *path)
{
    const char *str = ptr;
    if (size < HEADER_LEN) {
        log_error("load", path, "too short");
        return false;
    }
    if (strncmp(str, MAGIC, strlen(MAGIC))) {
        log_error("load", path,
            "invalid header (was this save created by crater?)");
        return false;
    }
    str += strlen(MAGIC);

    int version;
    uint32_t prodcode;
    uint16_t checksum;
    if (sscanf(str, "%d:%06d:0x%04hX\n", &version, &prodcode, &checksum) < 3) {
        log_error("load", path, "invalid header (failed to parse)");
        return false;
    }
    if (version != 1) {
        log_error("load", path, "unknown or unsupported save file version");
        return false;
    }
    if (prodcode != rom->product_code || checksum != rom->expected_checksum) {
        log_error("load", path, "save was created for a different ROM");
        return false;
    }
    if (size != HEADER_LEN + MMU_CART_RAM_SIZE) {
        log_error("load", path, "cart RAM size is wrong; file may be corrupt");
        return false;
    }

    if (off)
        *off = HEADER_LEN;
    return true;
}

/*
    Initialize a save object, which represents persistent RAM.

    The given path will be used to store save data. If it already exists,
    it will be loaded here. The return value indicates whether the load was
    successful; if it is false, then a file exists in the save location but is
    not valid, and this save should not be used. save_free() does not need to
    be called in this case.
*/
bool save_init(Save *save, const char *path, const ROM *rom)
{
    save->path = NULL;
    save->rom = rom;
    save->map = NULL;
    save->mapsize = 0;
    save->cart_ram_offset = 0;
    save->has_cart_ram = false;

    if (!path)
        return true;

    int fd = open(path, O_RDWR);
    if (fd < 0) {
        if (errno == ENOENT) {
            save->path = cr_strdup(path);
            return true;
        }
        log_stdlib_error("load", path, "open");
        return false;
    }

    struct stat s;
    if (fstat(fd, &s) < 0) {
        close(fd);
        log_stdlib_error("load", path, "fstat");
        return false;
    }

    size_t size = s.st_size;
    void *ptr = mmap(NULL, size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
    close(fd);
    if (ptr == MAP_FAILED) {
        log_stdlib_error("load", path, "mmap");
        return false;
    }

    size_t offset;
    if (!parse_save_header(ptr, size, &offset, rom, path))
        return false;

    save->map = ptr;
    save->mapsize = size;
    save->cart_ram_offset = offset;
    save->has_cart_ram = true;
    return true;
}

/*
    Free memory previously allocated by the save.

    Will flush the save data to disk if necessary.
*/
void save_free(Save *save)
{
    free(save->path);
    if (save->map) {
        msync(save->map, save->mapsize, MS_SYNC);
        munmap(save->map, save->mapsize);
    }
}

/*
    Return whether the save has existing cartridge RAM.
*/
bool save_has_cart_ram(const Save *save)
{
    return save->has_cart_ram;
}

/*
    Return a readable and writable pointer to existing cartridge RAM.
*/
uint8_t* save_get_cart_ram(Save *save)
{
    if (!save->has_cart_ram)
        return NULL;
    return ((uint8_t*) save->map) + save->cart_ram_offset;
}

/*
    Initialize the save file with fresh cartridge RAM as appropriate.

    If the save file is already loaded, return true. Otherwise, the return
    value indicates whether the save file creation was successful.
*/
bool save_init_cart_ram(Save *save)
{
    if (save->has_cart_ram)
        return true;
    if (!save->path || save->map)  // This should not happen normally
        return false;

    DEBUG("Creating new save file at %s", save->path)

    int fd = open(save->path, O_RDWR|O_CREAT|O_EXCL, 0644);
    if (fd < 0) {
        log_stdlib_error("create", save->path, "open");
        return false;
    }

    // Write header
    static const int VERSION = 1;
    int header_len = dprintf(fd, "%s%d:%06d:0x%04hX\n", MAGIC, VERSION,
        save->rom->product_code, save->rom->expected_checksum);
    if (header_len < 0 || (unsigned) header_len > HEADER_LEN) {
        if (header_len < 0)
            log_stdlib_error("create", save->path, "dprintf");
        else
            log_error("create", save->path, "header was unexpectedly long");
        close(fd);
        unlink(save->path);
        return false;
    }

    // Zero out space for the cartridge RAM
    char buf[4096];
    memset(buf, 0, sizeof(buf));
    size_t rem = MMU_CART_RAM_SIZE + (HEADER_LEN - header_len);
    while (rem > 0) {
        ssize_t chunk = rem > sizeof(buf) ? sizeof(buf) : rem;
        if (write(fd, buf, chunk) < chunk) {
            log_stdlib_error("create", save->path, "write");
            close(fd);
            unlink(save->path);
            return false;
        }
        rem -= chunk;
    }

    // Try to MMAP
    size_t size = HEADER_LEN + MMU_CART_RAM_SIZE;
    lseek(fd, 0, SEEK_SET);
    void *ptr = mmap(NULL, size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
    close(fd);
    if (ptr == MAP_FAILED) {
        log_stdlib_error("create", save->path, "mmap");
        unlink(save->path);
        return false;
    }

    save->map = ptr;
    save->mapsize = size;
    save->cart_ram_offset = HEADER_LEN;
    save->has_cart_ram = true;
    return true;
}
