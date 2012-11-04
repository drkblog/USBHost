#include <fat32.h>

void FAT32::dump()
{
  uint8_t r;


  Serial.print("Capacity: ");
  Serial.print(bulk->GetCapacityMB());
  Serial.println(" MBytes");
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
  */
  
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
}
