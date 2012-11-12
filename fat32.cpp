#include <fat32.h>

uint8_t FAT32::Init() 
{
  uint8_t r;
 
  // Read MBR
  r = bulk->Read(0, sizeof(fat32_boot_t), (uint8_t*)&bootp, 1);
  if (r) {
    sub_error = r;
    return FAT32_ERR_LOW_LEVEL;
  }
  
  // Validate
  if (!bootp.valid) {
    return FAT32_ERR_NOT_FAT32;
  }
  
  #ifdef FAT32_DEBUG
  Serial.print("Bytes per sector: ");
  Serial.println(bootp.bytesPerSector);
  Serial.print("Sectors per cluster: ");
  Serial.println(bootp.sectorsPerCluster);
  Serial.print("Reserved sectors: ");
  Serial.println(bootp.reservedSectorCount);
  Serial.print("Total sectors: ");
  Serial.println(bootp.totalSectors32);
  Serial.print("FAT sectors: ");
  Serial.println(bootp.sectorsPerFat32);
  Serial.print("Cluster of the ROOT directory: ");
  Serial.println(bootp.fat32RootCluster);
  Serial.print("FSInfo sector: ");
  Serial.println(bootp.fat32FSInfo);
  #endif
  
  return FAT32_ERR_SUCCESS;
}

FAT32::~FAT32()
{
}

uint32_t FAT32::nextCluster(uint32_t active_cluster)
{
  uint32_t fat_offset = active_cluster * 4;
  uint32_t fat_sector = bootp.reservedSectorCount + (fat_offset / bootp.bytesPerSector);
  FAT32FATSectorParser sp(fat_offset % bootp.bytesPerSector);
  uint8_t r = bulk->Read(fat_sector, bootp.bytesPerSector, (uint8_t*)&sp, 1);
  if (r)
    return 0;
  
  return sp.value; 
}

void FAT32::cat(uint32_t cluster)
{
  FAT32FileParser fp(5706);
  
  do {
    if (cluster != 0x0FFFFFF7)
      bulk->Read(bootp.firstDataSector + bootp.sectorsPerCluster*(cluster-2), bootp.sectorsPerCluster*512, (uint8_t*)&fp, 1);
    cluster = nextCluster(cluster);
  } while(!fp.done && cluster < 0x0FFFFFF8 && cluster);
  
}


void FAT32::ls(FAT32DirectoryCB &cb, uint32_t cluster)
{
  FAT32DirEntParser p(cb);
  
  do {
    if (cluster != 0x0FFFFFF7)
      bulk->Read(bootp.firstDataSector + bootp.sectorsPerCluster*(bootp.fat32RootCluster+cluster-2), bootp.sectorsPerCluster*512, (uint8_t*)&p, 1);
    cluster = nextCluster(cluster);
  } while(!p.abort && cluster < 0x0FFFFFF8 && cluster);
}

class FindFileCB : public FAT32DirectoryCB
{
  const char * file;
public:
  uint32_t cluster;
public:
  FindFileCB(const char * f) : file(f), cluster(0) {};
  virtual uint8_t foundEntry(const FAT32DirEntParser &entry) {
    if (cluster != 0)
      return 1; // already found
    uint8_t found = 1;
    for(uint8_t i=0; i<11; i++)
      found = found && (file[i] == entry.name[i]);
    if (found) {
      cluster = entry.firstClusterHigh << 16;
      cluster |= entry.firstClusterLow;
      return 1; // abort
    }
  };
};


uint32_t FAT32::find(const char * name)
{
  FindFileCB cb(name);
  ls(cb, 0);
  return cb.cluster;
}

void FAT32::dump()
{
  /*
  if (bootsec->bootSignature == EXTENDED_BOOT_SIG) {
    Serial.print("Volume serial number: ");
    Serial.println(bootsec->volumeSerialNumber);
    Serial.print("Volume label: ");
    Serial.println(bootsec->volumeLabel);
    Serial.print("Type: ");
    Serial.println(bootsec->fileSystemType);
  }
  */

  /*
  // OJO!!!
  uint8_t block[512];
  memset(block, 0, 512);
  block[0] = 0x00;
  block[1] = 0x01;
  block[2] = 0x02;
  block[3] = 0x03;
  block[4] = 0x04;
  block[5] = 0x05;
  r = bulk->Write(0, 512, block, 0);
  if (r) {
    Serial.print("ERROR: ");
    Serial.println(r);
    const SenseData s = bulk->GetLastSenseData();
    Serial.print("Sense: ");
    Serial.println(s.ErrorCode);
    Serial.println(s.AdditionalSenseCode);
    Serial.println(s.AdditionalSenseCodeQualifier);
  }  
  
  HexDumper<USBReadParser, uint16_t, uint16_t>		HexDump;
  r = bulk->Read(0, 512, (uint8_t*)&HexDump, 1);
  if (r) {
    Serial.print("ERROR: ");
    Serial.println(r);
    const SenseData s = bulk->GetLastSenseData();
    Serial.println("Sense");
    Serial.println(s.Valid);
    Serial.println((uint32_t)s.Information);
    Serial.println(s.SenseKey);
    Serial.println(s.AdditionalSenseCode);
    Serial.println(s.AdditionalSenseCodeQualifier);
  }
  */
}

// Utility and parsers
void FAT32DirEntParser::Parse(const uint16_t len, const uint8_t *pbuf, const uint16_t &offset) {
  uint16_t i;
  uint8_t sos = offset % 32;
  
  if (abort)
    return;
  
  for(i = 0; i<len; i++) {
    if (sos+i == 0) {
      ignore = *(pbuf+i) == DIR_NAME_DELETED;
      #ifdef FAT32_DEBUG
      if (ignore)
        Serial.println("DirEntry ignored.");
      #endif
      if (*(pbuf+i) == DIR_NAME_FREE) {
        abort = ignore = 1;
        #ifdef FAT32_DEBUG
        Serial.println("DirEntry finished.");
        #endif
      }
    }
    
    if (!ignore) {
      if (sos+i >= 0 && offset+i <=10) name[sos+i] = ((char)*(pbuf+i));
      if (sos+i == 11) {
        attributes = *(pbuf+i);
        ignore = attributes == DIR_ATT_LONG_NAME; // Long entry will be ignored
        #ifdef FAT32_DEBUG
        if  (((attributes & DIR_ATT_LONG_NAME_MASK) != DIR_ATT_LONG_NAME) && (name[0] != DIR_NAME_DELETED))
        {
          if ((attributes & (DIR_ATT_DIRECTORY | DIR_ATT_VOLUME_ID)) == 0x00)
            Serial.println("Found a file.");
          else if ((attributes & (DIR_ATT_DIRECTORY | DIR_ATT_VOLUME_ID)) == DIR_ATT_DIRECTORY)
            Serial.println("Found a directory.");
          else if ((attributes & (DIR_ATT_DIRECTORY | DIR_ATT_VOLUME_ID)) == DIR_ATT_VOLUME_ID)
            Serial.println("Found a volume label.");
          else
            Serial.println("Found an invalid directory entry.");
        }
        #endif
        
      }
      if (sos+i == 20) firstClusterHigh = *(pbuf+i);
      if (sos+i == 21) firstClusterHigh |= *(pbuf+i) << 8;
      if (sos+i == 26) firstClusterLow = *(pbuf+i);
      if (sos+i == 27) firstClusterLow |= *(pbuf+i) << 8;
      if (sos+i == 28) fileSize = *(pbuf+i);
      if (sos+i == 29) fileSize |= *(pbuf+i) << 8;
      if (sos+i == 30) fileSize |= *(pbuf+i) << 16;
      if (sos+i == 31) fileSize |= *(pbuf+i) << 24;
      
      // Notify
      if (sos+i == 31) {
        abort = cb.foundEntry(*this);
      }
      
      // DEBUG
      #ifdef FAT32_DEBUG
      if (sos+i == 0) Serial.print("Name: ");
      if (sos+i >= 0 && sos+i <=10) Serial.print((char)*(pbuf+i));
      if (sos+i == 10) Serial.println("");
      if (sos+i == 31) {
        Serial.print("FC: ");
        Serial.print(firstClusterHigh);
        Serial.println(firstClusterLow);
        Serial.print("Size: ");
        Serial.print(fileSize);
      }
      #endif
    }
  }
}
  


