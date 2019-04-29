
/*
 * 参考 opnewrt bcm nvram实现修改而来，用来适配dni image config读写
 * 主要修改nvram头，校验方式等。
 *
 * TODO: nvram header 校验
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "nvram.h"

static nvram_handle_t *nvram_open_rdonly()
{
	char *file = NULL;

    file = nvram_find_staging();
	if(file == NULL)
    {   
		file = nvram_find_mtd();
    }
    
	if(file != NULL)
    {
		nvram_handle_t *h = nvram_open(file, NVRAM_RO);
		if(strcmp(file, NVRAM_STAGING))
        {      
			free(file);
        }
        
        return h;
	}

	return NULL;
}

static nvram_handle_t *nvram_open_staging(void)
{
	if(nvram_find_staging() != NULL || nvram_to_staging() == 0)
    {   
		return nvram_open(NVRAM_STAGING, NVRAM_RW);
    }
    
	return NULL;
}

static int do_init(nvram_handle_t *nvram)
{
    if(nvram_find_staging() == NULL)
    {
        return nvram_to_staging();
    }

    return -1;
}

static int do_info(nvram_handle_t *nvram)
{
    uint32_t crc = 0;
	nvram_header_t *hdr = NULL;

    hdr = nvram_header(nvram);
    crc = calc_crc32(hdr->data, hdr->len - 4);
    
	/* Show info */
	printf("Magic:         0x%08X\n", hdr->magic);
	printf("Length:        0x%08X\n", hdr->len);
	printf("CRC32:         0x%08X (calculated: 0x%08X)\n", hdr->crc, crc);
	printf("%i bytes used / %i bytes available (%.2f%%)\n",
		hdr->len, nvram->length - nvram->offset - hdr->len,
		(100.00 / (double)(nvram->length - nvram->offset)) * (double)hdr->len);

	return 0;
}

static int do_show(nvram_handle_t *nvram)
{
	int stat = 1;
	nvram_tuple_t *t;

	if((t = nvram_getall(nvram)) != NULL)
	{
		while( t )
		{
			printf("%s=%s\n", t->name, t->value);
			t = t->next;
		}
		stat = 0;
	}

	return stat;
}

static int do_get(nvram_handle_t *nvram, const char *var)
{
	const char *val;
	int stat = 1;

	if((val = nvram_get(nvram, var)) != NULL)
	{
		printf("%s\n", val);
		stat = 0;
	}

	return stat;
}

static int do_set(nvram_handle_t *nvram, const char *pair)
{
	char *val = strstr(pair, "=");
	char var[strlen(pair)];
	int stat = 1;

	if(val != NULL)
	{
		memset(var, 0, sizeof(var));
		strncpy(var, pair, (int)(val - pair));
		stat = nvram_set(nvram, var, (char *)(val + 1));
	}

	return stat;
}

static int do_unset(nvram_handle_t *nvram, const char *var)
{
	return nvram_unset(nvram, var);
}

static void usage(void)
{
	fprintf(stderr,
		"Usage:\n"
		"   nvram init\n"
		"   nvram show\n"
		"   nvram info\n"
		"   nvram get variable\n"
		"   nvram set variable=value [set ...]\n"
		"   nvram unset variable [unset ...]\n"
		"   nvram commit\n"
	);
}

int main( int argc, const char *argv[] )
{
    int i = 0;
	int ret = 0;
	int commit = 0;
	int write = 0;
	int done = 0;
	nvram_handle_t *nvram;

	if(argc < 2) 
    {
		usage();
		return 1;
	}

	/* Ugly... iterate over arguments to see whether we can expect a write */
	if((!strcmp(argv[1], "set")  && 2 < argc) ||
		(!strcmp(argv[1], "unset") && 2 < argc) ||
		 !strcmp(argv[1], "commit"))
    {   
		write = 1;
        fprintf(stdout, "NVRAM: write operation not permitted!\n");
        return -1;
    }

	nvram = write ? nvram_open_staging() : nvram_open_rdonly();
         
	if(nvram != NULL && argc > 1)
	{
		for(i = 1; i < argc; i ++)
		{            
		    if (!strcmp(argv[i], "init"))
            {
                ret = do_init(nvram);
                done ++;
            }      
			else if (!strcmp(argv[i], "show"))
			{
				ret = do_show(nvram);
				done ++;
			}
			else if (!strcmp(argv[i], "info"))
			{
				ret = do_info(nvram);
				done ++;
			}
			else if (!strcmp(argv[i], "get") || !strcmp(argv[i], "unset") || !strcmp(argv[i], "set"))
			{
				if ((i + 1) < argc)
				{
					switch(argv[i ++][0])
					{
						case 'g':
							ret = do_get(nvram, argv[i]);
							break;
						case 'u':
							ret = do_unset(nvram, argv[i]);
							break;
						case 's':
							ret = do_set(nvram, argv[i]);
							break;
					}
					done ++;
				}
				else
				{
					fprintf(stderr, "Command '%s' requires an argument!\n", argv[i]);
					done = 0;
					break;
				}
			}
			else if (!strcmp(argv[i], "commit"))
			{
				commit = 1;
				done ++;
			}
			else
			{
				fprintf(stderr, "Unknown option '%s' !\n", argv[i]);
				done = 0;
				break;
			}
		}

		if(write)
        {      
			ret = nvram_commit(nvram);
        }
        
		nvram_close(nvram);

		if(commit)
        {      
			ret = staging_to_nvram();
        }
    }

	if(!nvram)
	{
		fprintf(stderr,
			"Could not open nvram! Possible reasons are:\n"
			"	- No device found (/proc not mounted or no nvram present)\n"
			"	- Insufficient permissions to open mtd device\n"
			"	- Insufficient memory to complete operation\n"
			"	- Memory mapping failed or not supported\n"
			"	- Nvram magic not found in specific nvram partition\n"
		);

		ret = 1;
	}
	else if(!done)
	{
		usage();
		ret = 1;
	}

	return ret;
}
