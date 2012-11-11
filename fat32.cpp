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
  
    ls();
    cat(3);

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


void FAT32::ls()
{
  FAT32DirEntParser p;
  
  //HexDumper<USBReadParser, uint16_t, uint16_t>		HexDump;
  uint32_t cluster = 0;
  do {
    if (cluster != 0x0FFFFFF7)
      bulk->Read(bootp.firstDataSector + bootp.sectorsPerCluster*(bootp.fat32RootCluster+cluster-2), bootp.sectorsPerCluster*512, (uint8_t*)&p, 1);
    cluster = nextCluster(cluster);
  } while(!p.abort && cluster < 0x0FFFFFF8 && cluster);
  
  Serial.println("DONE");
  
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
