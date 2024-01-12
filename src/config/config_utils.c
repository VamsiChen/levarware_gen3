/**
 * @brief: 	config_utils.c - Filesystem utilities for the configuration datastore manager 
 *
 * @notes:  Helper files for filesystem operations. Lifted, cleaned and pruned
 *          from Levaware Gen2 which intern was previously lifted from open source (see below)
 *  
 * 	Copyright (c) 2023 Reliance Foundry Co. Ltd.
 *  Copyright (c) 2019 Peter Bigot Consulting, LLC
 *
 *  SPDX-License-Identifier: Apache-2.0
 *
 */

// includes for nrf system
#include <zephyr.h>
#include <fs/fs.h>
#include <fs/littlefs.h>

// includes for application
#include  "config_internal.h"
#include "bsp/sys_wrapper.h"

// A change of the name of the littlefs storage symbol
// This is to avoid compiler error on flash_map_pm.h, line#29
// A similar issue is posted here: 
// https://devzone.nordicsemi.com/f/nordic-q-a/74859/not-able-to-combine-openthread-mcuboot-and-littlefs/308671
FS_LITTLEFS_DECLARE_DEFAULT_CONFIG(storage);

// create storage and initialize the file mount information
static struct fs_mount_t lfs_storage_mnt = {
    .type = FS_LITTLEFS,
    .fs_data = &storage,
    .storage_dev = (void *)FLASH_AREA_ID(littlefs_storage),
    .mnt_point = "/lfs",
};

// mp pointer is used as a mount flag
static struct fs_mount_t *mp = NULL;

/** 
* @brief    save_to_file - save buffer to file
*
* @param    content  buffer to write to file
* @param    content_len length of buffer
* @param    filename 
*
* @return   bytes_written
*
* @note     aborts if error occured
*/
int save_to_file(char *content, size_t content_len, char *file_name)
{
    char fname[LFS_NAME_MAX ];
    int rc;
    struct fs_file_t file;      // file object

    rc = 0;
    snprintk(fname, sizeof(fname), "%s/%s", mp->mnt_point, file_name);

    // init the file object
    fs_file_t_init(&file);

    rc = fs_open(&file, fname, FS_O_CREATE | FS_O_WRITE);
    if (rc < 0)
    {
        erabort("save_to_file - failed to open");
        return 0;
    }

    // this is to check, if the written buffer is larger or shorter than the previous file size
    // if not, then truncate or expend to the new size
    off_t file_len = -1;
    struct fs_dirent file_ent;
    rc = fs_stat(fname, &file_ent);
    if (rc == 0)
    {
        // file exists if size > 0
        file_len = file_ent.size;
    }

    if (file_len != content_len)
    {
        rc = fs_truncate(&file, content_len);
        if (rc)
        {
            erabort("save_to_file - truncate");
            return 0;
        }
    }

    rc = fs_write(&file, content, content_len);
    if (rc >= 0)
    {
        // success 
        fs_close(&file);
        return rc;
    }
    else
    {
        // failure
        erabort("save_to_file - failed to write");
        fs_close(&file);
        return 0;
    }
}

/** 
* @brief    read_file - read file
*
* @param    buffer  buffer to read into
* @param    buf_len length of buffer
* @param    filename 
*
* @return   bytes_read  number of bytes read from file
*
* @note     aborts if filename malformed
*/
int read_file(char *output_buf, int output_buf_len, char *file_name)
{
    char fname[LFS_NAME_MAX];
    int rc;
    int bytesread;

    struct fs_file_t file;      // file object
    struct fs_dirent file_ent;  // information on file

    bytesread = 0;              // assume no bytes read

    snprintk(fname, sizeof(fname), "%s/%s", mp->mnt_point, file_name);

    // init the file object
    fs_file_t_init(&file);

    rc = fs_open(&file, fname, FS_O_READ);
    if (rc < 0)
    {
        erabort("read_file - unable to open file");
        return 0;
    }

    // check size first
    rc = fs_stat(fname, &file_ent);
    if (rc)
    {
        erabort("read_file - failed to get stat");
    }

    rc = fs_read(&file, output_buf, output_buf_len);
    if (rc >= 0)
        bytesread = rc;
    else 
        erabort("read_file - failed to read");
 
    fs_close(&file);

    return bytesread;
}


/** 
* @brief    is_file_exists - Checks if file exists
*
* @param    filename    pointer to filename
*
* @return   exists      true if the file exists, else false
*
* @note     aborts if filename malformed
*/
bool is_file_exists(char *file_name)
{
    char fname[LFS_NAME_MAX];
    int rc;
    bool exists;
    struct fs_dirent config_ent;

    exists = false;     // assume the file does not exist
    snprintk(fname, sizeof(fname), "%s/%s", mp->mnt_point, file_name);

    rc = fs_stat(fname, &config_ent);
    if (rc == 0)
    {
        // file exists if size > 0
        if (config_ent.size > 0)
            exists = true;
    }
    else if (rc == ENOENT)
    {
        //return 0 if no config file exist
       exists = false;
    }
    else
    {
        erabort("is_file_exists");
        exists = false;
    }

    return exists;
}


/** 
* @brief    config_init_lfs - Init the filesystem used for the Configuration datastore
*
* @param    void
*
* @return   none
*
* @note     will abort if fails to init properly
*/
void config_fs_init(void)
{
    int rc;

    // check if filesystem is mounted
    if (mp == NULL)
    {
        mp = &lfs_storage_mnt;
    }  

    // mount filesystem
    rc = fs_mount(mp);
    if (rc < 0)
        erabort("config_init_fs - failed to mount");
}
