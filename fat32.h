/* FAT32 driver - Copyright (C) 2012 Leandro Fernández 

For USB Host Library (C) Circuits At Home, LTD. All rights reserved.

This software may be distributed and modified under the terms of the GNU
General Public License version 2 (GPL2) as published by the Free Software
Foundation and appearing in the file GPL2.TXT included in the packaging of
this file. Please note that GPL2 Section 2[b] requires that all works based
on this software must also be made publicly available under the terms of
the GPL2 ("Copyleft").

Contact information
-------------------

Leandro Fernández
Web      :  http://www.drk.com.ar
e-mail   :  drkbugs@gmail.com

Circuits At Home, LTD (http://www.circuitsathome.com)
*/

#if !defined(__FAT32_H__)
#define __FAT32_H__

#include <max3421e.h>
#include <Usb.h>
#include <masstorage.h>

#define FAT32_DEBUG 1

#include <fat32structs.h>

#define FAT32_ERR_SUCCESS   0x00
#define FAT32_ERR_LOW_LEVEL 0x01
#define FAT32_ERR_NOT_FAT32 0x02
#define FAT32_ERR_NOT_ENOUGH_MEMORY 0x03
#define FAT32_ERR_INVALID_ARGUMENT 0x04
#define FAT32_ERR_FP_BEFORE_FILE 0x05
#define FAT32_ERR_READ_EOF 0x06
#define FAT32_ERR_FILENOTFOUND 0x07

#define SEEK_SET 0x00
#define SEEK_CUR 0x01
#define SEEK_END 0x02

#define FAT32_EOC 0x0FFFFFF8
#define FAT32_BAD 0x0FFFFFF7

class FAT32DirectoryCB
{
public:
  virtual uint8_t foundEntry(const FAT32DirEntParser &entry) = 0;
};

class FAT32;

class FAT32File
{
protected:
  FAT32 &fat;
  uint32_t cluster;
  uint32_t size;
  uint32_t fp;
  FAT32File(FAT32 &f, uint32_t c, uint32_t s) : fat(f), cluster(c), size(s), fp(0) {};
  ~FAT32File() {};
public:
  uint8_t seek(uint32_t offset, uint8_t whence);
  uint16_t read(void *buf, uint16_t count);
  
  friend class FAT32;
};

class FAT32
{
private:
  BulkOnly * bulk;
  FAT32BootSectorParser bootp;
  uint8_t sub_error;

protected:
  uint32_t nextCluster(uint32_t active_cluster);
  
public:
  FAT32(BulkOnly  * bulk) : bulk(bulk), sub_error(0) {};
  ~FAT32();
  uint8_t Init();
  void ls(FAT32DirectoryCB &cb, uint32_t cluster = 0);
  uint32_t find(const char * name, uint32_t &size);
  FAT32File * open(const char * name);
  void close(FAT32File * &file);
  
  uint8_t parseCluster(uint32_t cluster, USBReadParser * parser);
  uint32_t findClusterN(uint32_t first, uint32_t n);
  void fileToSerial(uint32_t cluster, uint32_t size);
  uint8_t GetSubError() { return sub_error; };
  
  friend class FAT32File;
};

#endif // __FAT32_H__
