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

void FAT32::fileToSerial(uint32_t cluster, uint32_t size)
{
  FAT32FileToSerialParser fp(size);
  
  do {
    if (cluster != FAT32_BAD)
      bulk->Read(bootp.firstDataSector + bootp.sectorsPerCluster*(cluster-2), bootp.sectorsPerCluster*512, (uint8_t*)&fp, 1);
    cluster = nextCluster(cluster);
  } while(!fp.done && cluster < FAT32_EOC && cluster);
  
}

void FAT32::ls(FAT32DirectoryCB &cb, uint32_t cluster)
{
  FAT32DirEntParser p(cb);
  
  do {
    if (cluster != FAT32_BAD)
      bulk->Read(bootp.firstDataSector + bootp.sectorsPerCluster*(bootp.fat32RootCluster+cluster-2), bootp.sectorsPerCluster*512, (uint8_t*)&p, 1);
    cluster = nextCluster(cluster);
  } while(!p.abort && cluster < FAT32_EOC && cluster);
}

class FindFileCB : public FAT32DirectoryCB
{
  const char * file;
public:
  uint32_t cluster;
  uint32_t size;
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
      size = entry.fileSize;
      return 1; // abort
    }
  };
};

uint32_t FAT32::find(const char * name, uint32_t &size)
{
  FindFileCB cb(name);
  ls(cb, 0);
  size = cb.size;
  return cb.cluster;
}

uint32_t FAT32::findClusterN(uint32_t first, uint32_t n)
{
  uint16_t c = 0;
  uint32_t found = first;
  while(n > c && found < FAT32_EOC) {
    found = nextCluster(found);
    ++c;
  }
  return found;
}

uint8_t FAT32::parseCluster(uint32_t cluster, USBReadParser * parser)
{
  sub_error = bulk->Read(bootp.firstDataSector + bootp.sectorsPerCluster*(cluster-2), bootp.sectorsPerCluster*bootp.bytesPerSector, (uint8_t*)&parser, 1);
  return (sub_error)?FAT32_ERR_LOW_LEVEL:FAT32_ERR_SUCCESS;
}

FAT32File * FAT32::open(const char * name)
{
  return new FAT32File(*this, 0, 0);
}
void FAT32::close(FAT32File * &file)
{
  delete file;
  file = NULL;
}

/////////////////////////////
// FAT32File

uint16_t FAT32File::read(void * buf, uint16_t count)
{
  if (fp + count >= size)
    return FAT32_ERR_READ_EOF;
  uint32_t cluster_number = fp / (fat.bootp.sectorsPerCluster * fat.bootp.bytesPerSector);
  FAT32FileToBufferParser f2b(fp % (fat.bootp.sectorsPerCluster * fat.bootp.bytesPerSector), count, (uint8_t*)buf);
  uint8_t r = fat.parseCluster(fat.findClusterN(this->cluster, cluster_number), &f2b);
  // TODO: span over clusters
  if (r)
    return r;
  fp += count;
  return FAT32_ERR_SUCCESS;
}

uint8_t FAT32File::seek(uint32_t offset, uint8_t whence)
{
  switch(whence) {
  case SEEK_SET:
    fp = offset;
    break;
  case SEEK_CUR:
    fp += offset;
    break;
  case SEEK_END:
    if (size < offset)
      return FAT32_ERR_FP_BEFORE_FILE; // Can't go before first byte
    fp = size-offset;
    
    break;
  default:
    return FAT32_ERR_INVALID_ARGUMENT;
  }
  
  return FAT32_ERR_SUCCESS;
}

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

// Utility and parsers
void FAT32DirEntParser::Parse(const uint16_t len, const uint8_t *pbuf, const uint16_t &offset) {
  uint16_t i, j;
  
  if (abort)
    return;
  
  for(i = 0; i<len; i++) {
    uint8_t ep = (offset + i) % 32; // Entry position
    
    if (ep == 0) {
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
      if (ep >= 0 && ep <=10) name[ep] = ((char)*(pbuf+i));
      if (ep == 11) {
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
      if (ep == 20) firstClusterHigh = *(pbuf+i);
      if (ep == 21) firstClusterHigh |= *(pbuf+i) << 8;
      if (ep == 26) firstClusterLow = *(pbuf+i);
      if (ep == 27) firstClusterLow |= *(pbuf+i) << 8;
      if (ep == 28) fileSize = *(pbuf+i);
      if (ep == 29) fileSize |= *(pbuf+i) << 8;
      if (ep == 30) fileSize |= *(pbuf+i) << 16;
      if (ep == 31) fileSize |= *(pbuf+i) << 24;
      
      // Notify
      if (ep == 31) {
        abort = cb.foundEntry(*this);
      }
      
      // DEBUG
      #ifdef FAT32_DEBUG
      if (ep == 0) Serial.print("Name: ");
      if (ep >= 0 && ep <=10) Serial.print((char)*(pbuf+i));
      if (ep == 10) Serial.println("");
      if (ep == 31) {
        Serial.print("FC: ");
        Serial.print(firstClusterHigh);
        Serial.println(firstClusterLow);
        Serial.print("Size: ");
        Serial.print(fileSize);
      }
      #endif
    } // for i
  } 
}
  


