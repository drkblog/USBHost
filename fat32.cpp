#include <fat32.h>

uint8_t FAT32::Init() 
{
  uint8_t r;
  
  bootsec = (fat32_boot_t*)malloc(sizeof(fat32_boot_t));
  Serial.print("BOOT: ");
  Serial.println((int)bootsec);
  Serial.println((int)sizeof(fat32_boot_t));

  if (!bootsec)
    return FAT32_ERR_NOT_ENOUGH_MEMORY;
  
  // Read MBR
  HexDumper<USBReadParser, uint16_t, uint16_t> hexDump;
  r = bulk->Read(0, sizeof(fat32_boot_t), (uint8_t*)bootsec, 0);
  if (r) {
    free(bootsec);
    sub_error = r;
    return FAT32_ERR_LOW_LEVEL;
  }
  
  hexDump.Parse(512, (uint8_t*)bootsec, 512);
  
  // Validate
  if (bootsec->bootSectorSig0 != BOOTSIG0 || bootsec->bootSectorSig1 != BOOTSIG1) {
    free(bootsec);
    return FAT32_ERR_NOT_FAT32;
  }

  return FAT32_ERR_SUCCESS;
}

FAT32::~FAT32()
{
  if (bootsec)
    free(bootsec);
}

void FAT32::dump()
{
  uint8_t r;


  Serial.println("FAT32: ");
  Serial.print("OEM ID: ");
  Serial.println(bootsec->oemId);
  Serial.print("Bytes per sector: ");
  Serial.println(bootsec->bytesPerSector);
  Serial.print("Sectors per cluster: ");
  Serial.println(bootsec->sectorsPerCluster);
  Serial.print("Reserved sector count: ");
  Serial.println(bootsec->reservedSectorCount);
  Serial.print("Total sectors: ");
  Serial.println(bootsec->totalSectors32);
  Serial.print("Sectors per FAT: ");
  Serial.println(bootsec->sectorsPerFat32);
  Serial.print("Flags: ");
  Serial.println(bootsec->fat32Flags);
  Serial.print("Version: ");
  Serial.println(bootsec->fat32Version);
  Serial.print("Root directory first cluster: ");
  Serial.println(bootsec->fat32RootCluster);
  Serial.print("FSINFO sector: ");
  Serial.println(bootsec->fat32FSInfo);
  Serial.print("Boot record backup sector: ");
  Serial.println(bootsec->fat32BackBootBlock);
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
