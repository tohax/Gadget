/*
 *  Copyright (c) 2012-2013 wangsheng gao
 *  Copyright (c) 2012-2013 Anyka, Inc
 *
 *  aimer98 key's program
 *
 * See INSTALL for installation details or manually compile with
 * gcc -o dlna_key  dlna_key.c
 */

/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 * Should you need to contact me, the author, you can do so either by
 * e-mail - mail your message to <vojtech@ucw.cz>, or by paper mail:
 * Vojtech Pavlik, Simunkova 1594, Prague 8, 182 00 Czech Republic
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <pthread.h>
#include <linux/netlink.h>
#include <linux/version.h>
#include <linux/input.h>

#include <linux/ioctl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#define KEY_GPIO_DEV		"/dev/input/event0"
#define KEY_AD_DEV			"/dev/input/event1"
#define SELECT_WIFI_MODE	"/etc/init.d/select_wifi_mode.sh"
#define WIRELESS_SWITCH		"/etc/init.d/select_wifi_mode.sh"
#define WIRENET_SWITCH 		"/etc/init.d/select_wire_mode.sh"
#define RECOVER_MODE		"/etc/init.d/wifi_recover.sh"
#define WIFI_WPS_MODE		"/etc/init.d/wifi_wps.sh"
#define UPDATE_IMAGE_MODE	"/etc/init.d/update.sh /mnt/zImage"
#define SDP1_DEV_NAME       "/dev/mmcblk0p1"
#define SD_DEV_NAME         "/dev/mmcblk0"

#define KERNEL_ZIMAGE_FILE			"/mnt/zImage"
#define SQSH_ZIMAGE_FILE			"/mnt/root.sqsh4"
#define JFFS_ZIMAGE_FILE			"/mnt/root.jffs2"

#define KERNEL_ZIMAGE		1
#define SQSH_ZIMAGE			2
#define JFFS_ZIMAGE			4
#define K_S_IMAGE		(KERNEL_ZIMAGE|SQSH_ZIMAGE)
#define K_J_IMAGE		(KERNEL_ZIMAGE|JFFS_ZIMAGE)
#define S_J_IMAGE		(SQSH_ZIMAGE|JFFS_ZIMAGE)
#define K_S_J_IMAGE		(KERNEL_ZIMAGE|SQSH_ZIMAGE|JFFS_ZIMAGE)


#define WIFI_MODE			3
#define UPDATE_IMAGE		10

#define UEVENT_BUFFER_SIZE      2048

#define KEY_DEBUG(fmt...)	 //printf(fmt)

/* compare two file and make sure is it the same */
int check_file(void)
{
    int fd;
    int ret = 0;

    if ((fd = open(KERNEL_ZIMAGE_FILE, O_RDONLY)) >= 0)
        ret = KERNEL_ZIMAGE;
    else {
        printf("not have the kernel zImage in /mnt\n");
    }
    close(fd);

    if ((fd = open(SQSH_ZIMAGE_FILE, O_RDONLY)) >= 0)
        ret = ret|SQSH_ZIMAGE;
    else{
        printf("not have root.sqsh4 in /mnt\n");
    }
    close(fd);

    if ((fd = open(JFFS_ZIMAGE_FILE, O_RDONLY)) >= 0)
        ret = ret|JFFS_ZIMAGE;
    else{
        printf("not have root.jffs2 in /mnt\n");
    }
    close(fd);

    if (ret == 0)
        ret = -1;

    return ret;
}

#define I2C_DEV "/dev/i2c-0"

static inline __s32 i2c_smbus_access(int file, char read_write, __u8 command,
        int size, union i2c_smbus_data *data)
{
    struct i2c_smbus_ioctl_data args;

    args.read_write = read_write;
    args.command = command;
    args.size = size;
    args.data = data;
    return ioctl(file,I2C_SMBUS,&args);
}

static inline __s32 i2c_smbus_read_byte(int file)
{
    union i2c_smbus_data data;
    if (i2c_smbus_access(file,I2C_SMBUS_READ,0,I2C_SMBUS_BYTE,&data))
        return -1;
    else
        return 0x0FF & data.byte;
}

static inline __s32 i2c_smbus_write_byte(int file, __u8 value)
{
    return i2c_smbus_access(file,I2C_SMBUS_WRITE,value,
            I2C_SMBUS_BYTE,NULL);
}

static inline __s32 i2c_smbus_read_byte_data(int file, __u8 command)
{
    union i2c_smbus_data data;
    if (i2c_smbus_access(file,I2C_SMBUS_READ,command,
                I2C_SMBUS_BYTE_DATA,&data))
        return -1;
    else
        return 0x0FF & data.byte;
}

static inline __s32 i2c_smbus_write_byte_data(int file, __u8 command,
        __u8 value)
{
    union i2c_smbus_data data;
    data.byte = value;
    return i2c_smbus_access(file,I2C_SMBUS_WRITE,command,
            I2C_SMBUS_BYTE_DATA, &data);
}

/**
 * *  @brief       	push_key0
 * *  @author      	tohax
 * *  @date        	2018-02-05
 * *  @param[in]   	struct input_event *event
 * *  @return      	0 on success
 * */
static int push_key0(struct input_event *event)
{
    if (event->value == 0)
    {
	system("echo key0 Pressed\n");
        return 0;
    }

    return 1;
}

/**
 * *  @brief            push_key1
 * *  @author           tohax
 * *  @date             2018-02-05
 * *  @param[in]        struct input_event *event
 * *  @return           0 on success
 * */
static int push_key1(struct input_event *event)
{
    if (event->value == 0)
    {
        system("/sbin/reboot\n");
        return 0;
    }

    return 1;
}

/**
 * *  @brief            push_key2
 * *  @author           tohax
 * *  @date             2018-02-05
 * *  @param[in]        struct input_event *event
 * *  @return           0 on success
 * */
static int push_key2(struct input_event *event)
{
if (event->value == 1)
    {
	system("echo CHARGING\n");
	system("/etc/init.d/on.sh &");
}
    else if (event->value == 0)
    {
	system("echo NOT CHARGING\n");
	system("/etc/init.d/off.sh &");
	return 0;
    }
    return 1;
}

/**
 * *  @brief       do_key
 * *  @author      gao wangsheng
 * *  @date        2012-12-19
 * *  @param[in]   struct input_event *event, int key event count
 * *  @return      0 on success
 * */
static int do_key(struct input_event *key_event, int key_cnt)
{
    int i = 0;
    int ret = -1;
    struct input_event *event;

    if (key_cnt < (int) sizeof(struct input_event)) {
        printf("expected %d bytes, got %d\n", (int) sizeof(struct input_event), key_cnt);
        return -1;
    }

    for (i = 0; (i < key_cnt/sizeof(struct input_event)); i++)
    {
        event = (struct input_event *)&key_event[i];
        if (EV_KEY != event->type)
        {
            continue;
        }
        KEY_DEBUG("count = %d, code = %d, value = %d!\n", 
                key_cnt/sizeof(struct input_event), event->code, event->value);

        //printf("%s handler event:", __func__);
        switch(event->code)
        {
            case KEY_0:
                ret = push_key0(event);
                break;
            case KEY_1:
                ret = push_key1(event);
                break;
 	    case KEY_2:
                ret = push_key2(event);
                break;
    default:
                printf("%s %s: Error key code!\n", __FILE__, __func__);
                ret = -1;
                break;
        }

        if (!ret){
            break;
        }
    }
    return ret;
}

/**
 * *  @brief       program's access point
 * *  @author      gao wangsheng
 * *  @date        2012-10-15
 * *  @param[in]   not use
 * *  @return      0 on success
 * */
int main (int argc, char **argv)
{
    int ret = 0;
    int gpio_fd;
//    int ad_fd;
    pthread_t pth;
    fd_set readfds, tempfds;

    FD_ZERO(&readfds);

    if ((gpio_fd = open(KEY_GPIO_DEV, O_RDONLY)) < 0) {
        perror("Open gpio key dev fail");
        return -ENOENT;
    }
    FD_SET(gpio_fd, &readfds);

#if 0
    if ((ad_fd = open(KEY_AD_DEV, O_RDONLY)) < 0){
        perror("Open ad key dev fail");
        return -ENOENT;
    }
    FD_SET(ad_fd, &readfds);
#endif

    struct input_event evt;
    evt.value = 1;

    while (1)
    {
        int rd = 0;
        struct input_event key_event[64];

        tempfds = readfds;
        ret = select(FD_SETSIZE, &tempfds, (fd_set *)0, (fd_set *)0, (struct timeval*)0);
        if (FD_ISSET(gpio_fd, &tempfds))
        {
            rd = read(gpio_fd, key_event, sizeof(struct input_event) * sizeof(key_event));
            do_key(key_event, rd);
        }
    }

    pthread_cancel(pth);
    close(gpio_fd);
    return ret;
}
