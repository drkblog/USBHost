#include <fat32.h>

void FAT32::dump()
{
  
  /*
  const InquiryResponse &inq = bulk->GetInquiry();
  Serial.println((char*)inq.VendorID);
  Serial.println((char*)inq.ProductID);
  
  delay(50);
  
  uint8_t r = bulk->ReadCapacity(mlun);
  Serial.print("ERROR: ");
  Serial.println(r);
  
  Serial.print("Capacity: ");
  Serial.print(bulk->GetCapacity());
  Serial.println(" MBytes");
  */
  
  HexDumper<USBReadParser, uint16_t, uint16_t>		HexDump;
  uint8_t r = bulk->Read(0, 512, &HexDump);
  if (r) {
    Serial.print("ERROR: ");
    Serial.println(r);
    const SenseData s = bulk->GetLastSenseData();
    Serial.print("Sense: ");
    Serial.println(s.ErrorCode);
    Serial.println(s.AdditionalSenseCode);
    Serial.println(s.AdditionalSenseCodeQualifier);
  }  
}
