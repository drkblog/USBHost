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

// This file has been taken and changed from Arduino SdFat Library 
// Copyright (C) 2012 by William Greiman

/** Value for byte 510 of boot block or MBR */
uint8_t const BOOTSIG0 = 0X55;
/** Value for byte 511 of boot block or MBR */
uint8_t const BOOTSIG1 = 0XAA;
/** Value for bootSignature field int FAT/FAT32 boot sector */
uint8_t const EXTENDED_BOOT_SIG = 0X29;
//------------------------------------------------------------------------------
/**
 * \struct partitionTable
 * \brief MBR partition table entry
 *
 * A partition table entry for a MBR formatted storage device.
 * The MBR partition table has four entries.
 */
struct partitionTable {
          /**
           * Boot Indicator . Indicates whether the volume is the active
           * partition.  Legal values include: 0X00. Do not use for booting.
           * 0X80 Active partition.
           */
  uint8_t  boot;
          /**
            * Head part of Cylinder-head-sector address of the first block in
            * the partition. Legal values are 0-255. Only used in old PC BIOS.
            */
  uint8_t  beginHead;
          /**
           * Sector part of Cylinder-head-sector address of the first block in
           * the partition. Legal values are 1-63. Only used in old PC BIOS.
           */
  unsigned beginSector : 6;
           /** High bits cylinder for first block in partition. */
  unsigned beginCylinderHigh : 2;
          /**
           * Combine beginCylinderLow with beginCylinderHigh. Legal values
           * are 0-1023.  Only used in old PC BIOS.
           */
  uint8_t  beginCylinderLow;
          /**
           * Partition type. See defines that begin with PART_TYPE_ for
           * some Microsoft partition types.
           */
  uint8_t  type;
          /**
           * head part of cylinder-head-sector address of the last sector in the
           * partition.  Legal values are 0-255. Only used in old PC BIOS.
           */
  uint8_t  endHead;
          /**
           * Sector part of cylinder-head-sector address of the last sector in
           * the partition.  Legal values are 1-63. Only used in old PC BIOS.
           */
  unsigned endSector : 6;
           /** High bits of end cylinder */
  unsigned endCylinderHigh : 2;
          /**
           * Combine endCylinderLow with endCylinderHigh. Legal values
           * are 0-1023.  Only used in old PC BIOS.
           */
  uint8_t  endCylinderLow;
           /** Logical block address of the first block in the partition. */
  uint32_t firstSector;
           /** Length of the partition, in blocks. */
  uint32_t totalSectors;
};
/** Type name for partitionTable */
typedef struct partitionTable part_t;
//------------------------------------------------------------------------------
/**
 * \struct masterBootRecord
 *
 * \brief Master Boot Record
 *
 * The first block of a storage device that is formatted with a MBR.
 */
struct masterBootRecord {
           /** Code Area for master boot program. */
  uint8_t  codeArea[440];
           /** Optional Windows NT disk signature. May contain boot code. */
  uint32_t diskSignature;
           /** Usually zero but may be more boot code. */
  uint16_t usuallyZero;
           /** Partition tables. */
  part_t   part[4];
           /** First MBR signature byte. Must be 0X55 */
  uint8_t  mbrSig0;
           /** Second MBR signature byte. Must be 0XAA */
  uint8_t  mbrSig1;
};
/** Type name for masterBootRecord */
typedef struct masterBootRecord mbr_t;
//------------------------------------------------------------------------------
/**
 * \struct fat32_boot
 *
 * \brief Boot sector for a FAT32 volume.
 *
 */
struct fat32_boot {
         /**
          * The first three bytes of the boot sector must be valid,
          * executable x 86-based CPU instructions. This includes a
          * jump instruction that skips the next nonexecutable bytes.
          */
  uint8_t jump[3];
         /**
          * This is typically a string of characters that identifies
          * the operating system that formatted the volume.
          */
  char    oemId[8];
          /**
           * The size of a hardware sector. Valid decimal values for this
           * field are 512, 1024, 2048, and 4096. For most disks used in
           * the United States, the value of this field is 512.
           */
  uint16_t bytesPerSector;
          /**
           * Number of sectors per allocation unit. This value must be a
           * power of 2 that is greater than 0. The legal values are
           * 1, 2, 4, 8, 16, 32, 64, and 128.  128 should be avoided.
           */
  uint8_t  sectorsPerCluster;
          /**
           * The number of sectors preceding the start of the first FAT,
           * including the boot sector. Must not be zero
           */
  uint16_t reservedSectorCount;
          /**
           * The number of copies of the FAT on the volume.
           * The value of this field is always 2.
           */
  uint8_t  fatCount;//16
          /**
           * FAT12/FAT16 only. For FAT32 volumes, this field must be set to 0.
           */
  uint16_t rootDirEntryCount;
          /**
           * For FAT32 volumes, this field must be 0.
           */
  uint16_t totalSectors16;
          /**
           * This dates back to the old MS-DOS 1.x media determination and is
           * no longer usually used for anything.  0xF8 is the standard value
           * for fixed (nonremovable) media. For removable media, 0xF0 is
           * frequently used. Legal values are 0xF0 or 0xF8-0xFF.
           */
  uint8_t  mediaType;
          /**
           * On FAT32 volumes this field must be 0, and sectorsPerFat32
           * contains the FAT size count.
           */
  uint16_t sectorsPerFat16;
           /** Sectors per track for interrupt 0x13. Not used otherwise. */
  uint16_t sectorsPerTrack;
           /** Number of heads for interrupt 0x13.  Not used otherwise. */
  uint16_t headCount;
          /**
           * Count of hidden sectors preceding the partition that contains this
           * FAT volume. This field is generally only relevant for media
           * visible on interrupt 0x13.
           */
  uint32_t hidddenSectors;
          /**
           * Contains the total number of sectors in the FAT32 volume.
           */
  uint32_t totalSectors32;
         /**
           * Count of sectors occupied by one FAT on FAT32 volumes.
           */
  uint32_t sectorsPerFat32;
          /**
           * This field is only defined for FAT32 media and does not exist on
           * FAT12 and FAT16 media.
           * Bits 0-3 -- Zero-based number of active FAT.
           *             Only valid if mirroring is disabled.
           * Bits 4-6 -- Reserved.
           * Bit 7	-- 0 means the FAT is mirrored at runtime into all FATs.
	         *        -- 1 means only one FAT is active; it is the one referenced
	         *             in bits 0-3.
           * Bits 8-15 	-- Reserved.
           */
  uint16_t fat32Flags;
          /**
           * FAT32 version. High byte is major revision number.
           * Low byte is minor revision number. Only 0.0 define.
           */
  uint16_t fat32Version;
          /**
           * Cluster number of the first cluster of the root directory for FAT32.
           * This usually 2 but not required to be 2.
           */
  uint32_t fat32RootCluster;
          /**
           * Sector number of FSINFO structure in the reserved area of the
           * FAT32 volume. Usually 1.
           */
  uint16_t fat32FSInfo;
          /**
           * If nonzero, indicates the sector number in the reserved area
           * of the volume of a copy of the boot record. Usually 6.
           * No value other than 6 is recommended.
           */
  uint16_t fat32BackBootBlock;
          /**
           * Reserved for future expansion. Code that formats FAT32 volumes
           * should always set all of the bytes of this field to 0.
           */
  uint8_t  fat32Reserved[12];
           /**
            * Related to the BIOS physical drive number. Floppy drives are
            * identified as 0x00 and physical hard disks are identified as
            * 0x80, regardless of the number of physical disk drives.
            * Typically, this value is set prior to issuing an INT 13h BIOS
            * call to specify the device to access. The value is only
            * relevant if the device is a boot device.
            */
  uint8_t  driveNumber;
           /** used by Windows NT - should be zero for FAT */
  uint8_t  reserved1;
           /** 0X29 if next three fields are valid */
  uint8_t  bootSignature;
           /**
            * A random serial number created when formatting a disk,
            * which helps to distinguish between disks.
            * Usually generated by combining date and time.
            */
  uint32_t volumeSerialNumber;
           /**
            * A field once used to store the volume label. The volume label
            * is now stored as a special file in the root directory.
            */
  char     volumeLabel[11];
           /**
            * A text field with a value of FAT32.
            */
  char     fileSystemType[8];
           /** X86 boot code */
  uint8_t  bootCode[420];
           /** must be 0X55 */
  uint8_t  bootSectorSig0;
           /** must be 0XAA */
  uint8_t  bootSectorSig1;
};
/** Type name for FAT32 Boot Sector */
typedef struct fat32_boot fat32_boot_t;

class FAT32BootSectorParser : public USBReadParser
{
public:
  uint8_t valid : 1;
  uint8_t reserved : 7;
  uint16_t bytesPerSector;
  uint8_t sectorsPerCluster;
  uint16_t reservedSectorCount;
  uint32_t totalSectors32;
  uint32_t sectorsPerFat32;
  uint16_t fat32Flags;
  uint32_t fat32RootCluster;
  uint16_t fat32FSInfo;
  uint16_t firstDataSector;
  
public:
  FAT32BootSectorParser() : valid(0), firstDataSector(0) { };
	virtual void Parse(const uint16_t len, const uint8_t *pbuf, const uint16_t &offset) {
	  uint16_t i;
	  for(i = 0; i<len; i++) {
	    if (offset+i == 11) bytesPerSector = *(pbuf+i);
	    if (offset+i == 12) bytesPerSector |= *(pbuf+i) << 8;
	    if (offset+i == 13) sectorsPerCluster = *(pbuf+i);
	    if (offset+i == 14) reservedSectorCount = *(pbuf+i);
	    if (offset+i == 15) reservedSectorCount |= *(pbuf+i) << 8;
	    if (offset+i == 32) totalSectors32 = *(pbuf+i);
	    if (offset+i == 33) totalSectors32 |= *(pbuf+i) << 8;
	    if (offset+i == 34) totalSectors32 |= *(pbuf+i) << 16;
	    if (offset+i == 35) totalSectors32 |= *(pbuf+i) << 24;
	    if (offset+i == 36) sectorsPerFat32 = *(pbuf+i);
	    if (offset+i == 37) sectorsPerFat32 |= *(pbuf+i) << 8;
	    if (offset+i == 38) sectorsPerFat32 |= *(pbuf+i) << 16;
	    if (offset+i == 39) sectorsPerFat32 |= *(pbuf+i) << 24;
	    if (offset+i == 40) fat32Flags = *(pbuf+i);
	    if (offset+i == 41) fat32Flags |= *(pbuf+i) << 8;
	    if (offset+i == 44) fat32RootCluster = *(pbuf+i);
	    if (offset+i == 45) fat32RootCluster |= *(pbuf+i) << 8;
	    if (offset+i == 46) fat32RootCluster |= *(pbuf+i) << 16;
	    if (offset+i == 47) fat32RootCluster |= *(pbuf+i) << 24;
	    if (offset+i == 48) fat32FSInfo = *(pbuf+i);
	    if (offset+i == 49) fat32FSInfo |= *(pbuf+i) << 8;
	    if (offset+i == 510) valid = *(pbuf+i) == 0x55;
	    if (offset+i == 511) {
	      valid = valid && (*(pbuf+i)) == 0xAA;
	      if (valid)
	        firstDataSector = reservedSectorCount + (2 * sectorsPerFat32);
	    }
	    // DEBUG
      #ifdef FAT32_DEBUG
	    if (offset+i == 3) Serial.print("OEMID: ");
	    if (offset+i >= 3 && offset+i <=10) Serial.print((char)*(pbuf+i));
	    if (offset+i == 10) Serial.println("");
	    #endif
	  }
	};
};

//------------------------------------------------------------------------------
/** Lead signature for a FSINFO sector */
uint32_t const FSINFO_LEAD_SIG = 0x41615252;
/** Struct signature for a FSINFO sector */
uint32_t const FSINFO_STRUCT_SIG = 0x61417272;
/**
 * \struct fat32_fsinfo
 *
 * \brief FSINFO sector for a FAT32 volume.
 *
 */
struct fat32_fsinfo {
           /** must be 0X52, 0X52, 0X61, 0X41 */
  uint32_t  leadSignature;
           /** must be zero */
  uint8_t  reserved1[480];
           /** must be 0X72, 0X72, 0X41, 0X61 */
  uint32_t  structSignature;
          /**
           * Contains the last known free cluster count on the volume.
           * If the value is 0xFFFFFFFF, then the free count is unknown
           * and must be computed. Any other value can be used, but is
           * not necessarily correct. It should be range checked at least
           * to make sure it is <= volume cluster count.
           */
  uint32_t freeCount;
          /**
           * This is a hint for the FAT driver. It indicates the cluster
           * number at which the driver should start looking for free clusters.
           * If the value is 0xFFFFFFFF, then there is no hint and the driver
           * should start looking at cluster 2.
           */
  uint32_t nextFree;
           /** must be zero */
  uint8_t  reserved2[12];
           /** must be 0X00, 0X00, 0X55, 0XAA */
  uint8_t  tailSignature[4];
};
/** Type name for FAT32 FSINFO Sector */
typedef struct fat32_fsinfo fat32_fsinfo_t;
//------------------------------------------------------------------------------
// End Of Chain values for FAT entries
/** FAT12 end of chain value used by Microsoft. */
uint16_t const FAT12EOC = 0XFFF;
/** Minimum value for FAT12 EOC.  Use to test for EOC. */
uint16_t const FAT12EOC_MIN = 0XFF8;
/** FAT16 end of chain value used by Microsoft. */
uint16_t const FAT16EOC = 0XFFFF;
/** Minimum value for FAT16 EOC.  Use to test for EOC. */
uint16_t const FAT16EOC_MIN = 0XFFF8;
/** FAT32 end of chain value used by Microsoft. */
uint32_t const FAT32EOC = 0X0FFFFFFF;
/** Minimum value for FAT32 EOC.  Use to test for EOC. */
uint32_t const FAT32EOC_MIN = 0X0FFFFFF8;
/** Mask a for FAT32 entry. Entries are 28 bits. */
uint32_t const FAT32MASK = 0X0FFFFFFF;
//------------------------------------------------------------------------------
/**
 * \struct directoryEntry
 * \brief FAT short directory entry
 *
 * Short means short 8.3 name, not the entry size.
 *  
 * Date Format. A FAT directory entry date stamp is a 16-bit field that is 
 * basically a date relative to the MS-DOS epoch of 01/01/1980. Here is the
 * format (bit 0 is the LSB of the 16-bit word, bit 15 is the MSB of the 
 * 16-bit word):
 *   
 * Bits 9-15: Count of years from 1980, valid value range 0-127 
 * inclusive (1980-2107).
 *   
 * Bits 5-8: Month of year, 1 = January, valid value range 1-12 inclusive.
 *
 * Bits 0-4: Day of month, valid value range 1-31 inclusive.
 *
 * Time Format. A FAT directory entry time stamp is a 16-bit field that has
 * a granularity of 2 seconds. Here is the format (bit 0 is the LSB of the 
 * 16-bit word, bit 15 is the MSB of the 16-bit word).
 *   
 * Bits 11-15: Hours, valid value range 0-23 inclusive.
 * 
 * Bits 5-10: Minutes, valid value range 0-59 inclusive.
 *      
 * Bits 0-4: 2-second count, valid value range 0-29 inclusive (0 - 58 seconds).
 *   
 * The valid time range is from Midnight 00:00:00 to 23:59:58.
 */
struct directoryEntry {
           /** Short 8.3 name.
            *
            * The first eight bytes contain the file name with blank fill.
            * The last three bytes contain the file extension with blank fill.
            */
  uint8_t  name[11];
          /** Entry attributes.
           *
           * The upper two bits of the attribute byte are reserved and should
           * always be set to 0 when a file is created and never modified or
           * looked at after that.  See defines that begin with DIR_ATT_.
           */
  uint8_t  attributes;
          /**
           * Reserved for use by Windows NT. Set value to 0 when a file is
           * created and never modify or look at it after that.
           */
  uint8_t  reservedNT;
          /**
           * The granularity of the seconds part of creationTime is 2 seconds
           * so this field is a count of tenths of a second and its valid
           * value range is 0-199 inclusive. (WHG note - seems to be hundredths)
           */
  uint8_t  creationTimeTenths;
           /** Time file was created. */
  uint16_t creationTime;
           /** Date file was created. */
  uint16_t creationDate;
          /**
           * Last access date. Note that there is no last access time, only
           * a date.  This is the date of last read or write. In the case of
           * a write, this should be set to the same date as lastWriteDate.
           */
  uint16_t lastAccessDate;
          /**
           * High word of this entry's first cluster number (always 0 for a
           * FAT12 or FAT16 volume).
           */
  uint16_t firstClusterHigh;
           /** Time of last write. File creation is considered a write. */
  uint16_t lastWriteTime;
           /** Date of last write. File creation is considered a write. */
  uint16_t lastWriteDate;
           /** Low word of this entry's first cluster number. */
  uint16_t firstClusterLow;
           /** 32-bit unsigned holding this file's size in bytes. */
  uint32_t fileSize;
};
//------------------------------------------------------------------------------
// Definitions for directory entries
//
/** Type name for directoryEntry */
typedef struct directoryEntry dir_t;
/** escape for name[0] = 0XE5 */
uint8_t const DIR_NAME_0XE5 = 0X05;
/** name[0] value for entry that is free after being "deleted" */
uint8_t const DIR_NAME_DELETED = 0XE5;
/** name[0] value for entry that is free and no allocated entries follow */
uint8_t const DIR_NAME_FREE = 0X00;
/** file is read-only */
uint8_t const DIR_ATT_READ_ONLY = 0X01;
/** File should hidden in directory listings */
uint8_t const DIR_ATT_HIDDEN = 0X02;
/** Entry is for a system file */
uint8_t const DIR_ATT_SYSTEM = 0X04;
/** Directory entry contains the volume label */
uint8_t const DIR_ATT_VOLUME_ID = 0X08;
/** Entry is for a directory */
uint8_t const DIR_ATT_DIRECTORY = 0X10;
/** Old DOS archive bit for backup support */
uint8_t const DIR_ATT_ARCHIVE = 0X20;
/** Test value for long name entry.  Test is
  (d->attributes & DIR_ATT_LONG_NAME_MASK) == DIR_ATT_LONG_NAME. */
uint8_t const DIR_ATT_LONG_NAME = 0X0F;
/** Test mask for long name entry */
uint8_t const DIR_ATT_LONG_NAME_MASK = 0X3F;
/** defined attribute bits */
uint8_t const DIR_ATT_DEFINED_BITS = 0X3F;

class FAT32DirectoryCB;
class FAT32DirEntParser : public USBReadParser
{
public:
  uint8_t  name[11];
  uint8_t  attributes;
  uint16_t firstClusterHigh;
  uint16_t firstClusterLow;
  uint32_t fileSize;
  uint8_t ignore : 1;
  uint8_t abort : 1;
  uint8_t reserved : 6;
protected:
  FAT32DirectoryCB &cb;
public:
  FAT32DirEntParser(FAT32DirectoryCB &cb) : cb(cb), attributes(0), ignore(0), abort(0) { };
	virtual void Parse(const uint16_t len, const uint8_t *pbuf, const uint16_t &offset); // Defined in fat32.cpp
};

class FAT32FATSectorParser : public USBReadParser
{
public:
  uint8_t done : 1;
  uint8_t reserved : 7;
  uint8_t position;
  uint32_t value;
  
public:
  FAT32FATSectorParser(uint8_t position) : done(0), position(position) { };
	virtual void Parse(const uint16_t len, const uint8_t *pbuf, const uint16_t &offset) {
	  if (done)
	    return;
	  
	  uint16_t i;
	  for(i = 0; i<len; i++) {
	    if (offset+i == position) value = *(pbuf+i);
	    if (offset+i == position+1) value |= *(pbuf+i) << 8;
	    if (offset+i == position+2) value |= *(pbuf+i) << 16;
	    if (offset+i == position+3) {
	      value |= *(pbuf+i) << 24;
	      value &= 0x0FFFFFFF; // mask for FAT32
	      done = 1;
	    }
	  }
	}
};

class FAT32FileToSerialParser : public USBReadParser
{
public:
  uint8_t done : 1;
  uint8_t reserved : 7;
  uint32_t size;
  uint32_t parsed;
  
public:
  FAT32FileToSerialParser(uint32_t size) : done(0), parsed(0), size(size) { };
	virtual void Parse(const uint16_t len, const uint8_t *pbuf, const uint16_t &offset) {
	  if (done)
	    return;
	  
	  uint16_t i;
	  for(i = 0; i<len; i++) {
	    ++parsed;
	    if (parsed <= size)
	      Serial.print((char)*(pbuf+i));
	    else
	      done = 1;
	  }
	}
};

class FAT32FileToBufferParser : public USBReadParser
{
public:
  uint8_t done : 1;
  uint8_t reserved : 7;
  uint16_t size;
  uint16_t position;
  uint16_t left;
  uint8_t * buffer;
  
public:
  FAT32FileToBufferParser(uint16_t position, uint16_t size, uint8_t * buffer) : done(0), position(position), buffer(buffer), size(size), left(size) { };
  void resetTo(uint16_t position) { buffer+=(size-left); this->position = position; size = left; };
	virtual void Parse(const uint16_t len, const uint8_t *pbuf, const uint16_t &offset) {
	  if (done)
	    return;

	  uint16_t i;
	  if (offset + len >= position)
      for(i = 0; i<len; i++) {
        if (offset + i >= position && offset + i < position + size) {
          buffer[i - (position - offset)] = pbuf[i];
          --left;
        }
        if (!left)
          done = 1;
      }
	}
};

/** Directory entry is part of a long name
 * \param[in] dir Pointer to a directory entry.
 *
 * \return true if the entry is for part of a long name else false.
 */
static inline uint8_t DIR_IS_LONG_NAME(const dir_t* dir) {
  return (dir->attributes & DIR_ATT_LONG_NAME_MASK) == DIR_ATT_LONG_NAME;
}
/** Mask for file/subdirectory tests */
uint8_t const DIR_ATT_FILE_TYPE_MASK = (DIR_ATT_VOLUME_ID | DIR_ATT_DIRECTORY);
/** Directory entry is for a file
 * \param[in] dir Pointer to a directory entry.
 *
 * \return true if the entry is for a normal file else false.
 */
static inline uint8_t DIR_IS_FILE(const dir_t* dir) {
  return (dir->attributes & DIR_ATT_FILE_TYPE_MASK) == 0;
}
/** Directory entry is for a subdirectory
 * \param[in] dir Pointer to a directory entry.
 *
 * \return true if the entry is for a subdirectory else false.
 */
static inline uint8_t DIR_IS_SUBDIR(const dir_t* dir) {
  return (dir->attributes & DIR_ATT_FILE_TYPE_MASK) == DIR_ATT_DIRECTORY;
}
/** Directory entry is for a file or subdirectory
 * \param[in] dir Pointer to a directory entry.
 *
 * \return true if the entry is for a normal file or subdirectory else false.
 */
static inline uint8_t DIR_IS_FILE_OR_SUBDIR(const dir_t* dir) {
  return (dir->attributes & DIR_ATT_VOLUME_ID) == 0;
}
