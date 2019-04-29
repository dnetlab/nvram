
#ifndef __NVRAM_H_
#define __NVRAM_H_

#include "crc32.h"

#define MAX_NVRAM_SIZE 0x60000

/* Staging file for NVRAM */
#define NVRAM_STAGING		"/tmp/.nvram"
#define NVRAM_RO			1
#define NVRAM_RW			0

/* Helper macros */
#define NVRAM_ARRAYSIZE(a)	sizeof(a)/sizeof(a[0])
#define	NVRAM_ROUNDUP(x, y)	((((x)+((y)-1))/(y))*(y))

/* NVRAM constants */
#define NVRAM_MIN_SPACE		0x8000
#define NVRAM_MAGIC         0x20131224
#define NVRAM_VERSION		1

struct nvram_header {
	uint32_t magic;
	uint32_t len;
	uint32_t crc;	/* crc32 */
    uint8_t data[0];
} __attribute__((__packed__));

struct nvram_tuple {
	char *name;
	char *value;
	struct nvram_tuple *next;
};

struct nvram_handle {
	int fd;
	char *mmap;
	unsigned int length;
	unsigned int offset;
	struct nvram_tuple *nvram_hash[257];
	struct nvram_tuple *nvram_dead;
};

typedef struct nvram_handle nvram_handle_t;
typedef struct nvram_header nvram_header_t;
typedef struct nvram_tuple  nvram_tuple_t;


/* Get nvram header. */
nvram_header_t * nvram_header(nvram_handle_t *h);

/* Set the value of an NVRAM variable */
int nvram_set(nvram_handle_t *h, const char *name, const char *value);

/* Get the value of an NVRAM variable. */
char * nvram_get(nvram_handle_t *h, const char *name);

/* Unset the value of an NVRAM variable. */
int nvram_unset(nvram_handle_t *h, const char *name);

/* Get all NVRAM variables. */
nvram_tuple_t * nvram_getall(nvram_handle_t *h);

/* Regenerate NVRAM. */
int nvram_commit(nvram_handle_t *h);

/* Open NVRAM and obtain a handle. */
nvram_handle_t * nvram_open(const char *file, int rdonly);

/* Close NVRAM and free memory. */
int nvram_close(nvram_handle_t *h);

/* Get the value of an NVRAM variable in a safe way, use "" instead of NULL. */
#define nvram_safe_get(h, name) (nvram_get(h, name) ? : "")

/* Determine NVRAM device node. */
char * nvram_find_mtd(void);

/* Copy NVRAM contents to staging file. */
int nvram_to_staging(void);

/* Copy staging file to NVRAM device. */
int staging_to_nvram(void);

/* Check NVRAM staging file. */
char * nvram_find_staging(void);

#endif /* __NVRAM_H_ */
