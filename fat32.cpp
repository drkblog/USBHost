#include <fat32.h>

uint8_t FAT32::Init() 
{
  uint8_t r;
  
  FAT32BootSectorParser bootp;
 
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
  

  return FAT32_ERR_SUCCESS;
}

FAT32::~FAT32()
{
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
