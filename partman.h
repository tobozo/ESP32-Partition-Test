#include <FS.h>
#include <SD.h>

#include <esp_partition.h>
extern "C" {
  #include "esp_ota_ops.h"
  #include "esp_image_format.h"
}


// gather image partition info (only applies to OTA0 and OTA1)
static esp_image_metadata_t getSketchMeta( const esp_partition_t* running ) {
  esp_image_metadata_t data;
  if ( !running ) return data;
  const esp_partition_pos_t running_pos  = {
    .offset = running->address,
    .size = running->size,
  };
  data.start_addr = running_pos.offset;
  esp_image_verify( ESP_IMAGE_VERIFY, &running_pos, &data );
  return data;
}


// from https://github.com/lovyan03/M5Stack_LovyanLauncher
bool comparePartition(const esp_partition_t* src1, const esp_partition_t* src2, size_t length)
{
  size_t lengthLeft = length;
  const size_t bufSize = SPI_FLASH_SEC_SIZE;
  std::unique_ptr<uint8_t[]> buf1(new uint8_t[bufSize]);
  std::unique_ptr<uint8_t[]> buf2(new uint8_t[bufSize]);
  uint32_t offset = 0;
  size_t i;
  while( lengthLeft > 0) {
    size_t readBytes = (lengthLeft < bufSize) ? lengthLeft : bufSize;
    if (!ESP.flashRead(src1->address + offset, reinterpret_cast<uint32_t*>(buf1.get()), (readBytes + 3) & ~3)
     || !ESP.flashRead(src2->address + offset, reinterpret_cast<uint32_t*>(buf2.get()), (readBytes + 3) & ~3)) {
        return false;
    }
    for (i = 0; i < readBytes; ++i) if (buf1[i] != buf2[i]) return false;
    lengthLeft -= readBytes;
    offset += readBytes;
  }
  return true;
}

// from https://github.com/lovyan03/M5Stack_LovyanLauncher
bool copyPartition(File* fs, const esp_partition_t* dst, const esp_partition_t* src, size_t length)
{
  size_t lengthLeft = length;
  const size_t bufSize = SPI_FLASH_SEC_SIZE;
  std::unique_ptr<uint8_t[]> buf(new uint8_t[bufSize]);
  uint32_t offset = 0;
  uint32_t progress = 0, progressOld = 0;
  while( lengthLeft > 0) {
    size_t readBytes = (lengthLeft < bufSize) ? lengthLeft : bufSize;
    if (!ESP.flashRead(src->address + offset, reinterpret_cast<uint32_t*>(buf.get()), (readBytes + 3) & ~3)
     || !ESP.flashEraseSector((dst->address + offset) / bufSize)
     || !ESP.flashWrite(dst->address + offset, reinterpret_cast<uint32_t*>(buf.get()), (readBytes + 3) & ~3)) {
        return false;
    }
    if (fs) fs->write(buf.get(), (readBytes + 3) & ~3);
    lengthLeft -= readBytes;
    offset += readBytes;
  }
  return true;
}


void copyPartition( const char* binfilename )
{
  const esp_partition_t *running = esp_ota_get_running_partition();
  const esp_partition_t *nextupdate = esp_ota_get_next_update_partition(NULL);

  size_t sksize = ESP.getSketchSize();
  bool flgSD = SD.begin();
  File dst;
  if (flgSD) {
    dst = (SD.open(binfilename, FILE_WRITE ));
    Serial.printf("Overwriting %s\n", binfilename );
  }
  if (copyPartition( flgSD ? &dst : NULL, nextupdate, running, sksize)) {
    Serial.println("Done");
  }
  if (flgSD) dst.close();
}



// from https://github.com/lovyan03/M5Stack_LovyanLauncher
void checkStickyPartition( const char* binfilename )
{
  const esp_partition_t* running = esp_ota_get_running_partition();
  const esp_partition_t* nextupdate = esp_ota_get_next_update_partition(NULL);
  if (!running) {
    log_e( "Can't fetch running partition info !!" );
    return;
  }
  if (!nextupdate) {
    log_e( "Can't fetch nextupdate partition info !!" );
    return;
  }
  if (!nextupdate) {
    Serial.println("! ERROR ! No OTA partitions");
    return;
  }
  if (running && running->label[3] == '0' && nextupdate->label[3] == '1') {
    Serial.println("Sketch is currently running on partition OTA0");
    Serial.printf("Comparing '%s' and '%s' partitions\n", nextupdate->label, running->label );
    size_t sksize = ESP.getSketchSize();
    if (!comparePartition(running, nextupdate, sksize)) {
      bool flgSD = SD.begin( /*TFCARD_CS_PIN, SPI, 40000000*/ );
      Serial.printf("Partitions differ, will initiate copy\n");
      File dst;
      if (flgSD) {
        dst = (SD.open(binfilename, FILE_WRITE ));
        Serial.printf("[SD] Opening %s\n", binfilename );
      }
      Serial.printf("Copying partition '%s' to '%s' (flash) and %s (SD)\n", running->label, nextupdate->label, binfilename );
      if (copyPartition( flgSD ? &dst : NULL, nextupdate, running, sksize)) {
        Serial.println("Done");
      }
      if (flgSD) dst.close();
    }
    Serial.printf("Now Using rollback to switch to '%s' partition\n", nextupdate->label);
    if (Update.canRollBack()) {
      Update.rollBack();
      ESP.restart();
    } else {
      Serial.println("! WARNING !\r\nUpdate.rollBack() failed.");
      //log_e("Failed to rollback after copy");
    }
  } else {
    Serial.println("Sketch is currently running on partition OTA1, propagation isn't needed");
  }
}


// convert 32 bytes digest to hex string
struct {
  String toString( uint8_t dig[32] ) {
    static String digest;
    digest = "";
    char hex[3] = {0};
    for(int i=0;i<32;i++) {
      snprintf( hex, 3, "%02x", dig[i] );
      digest += String(hex);
    }
    return digest;
  }
} digest;



void lsPart()
{
  esp_partition_iterator_t pi = esp_partition_find(ESP_PARTITION_TYPE_ANY, ESP_PARTITION_SUBTYPE_ANY, NULL);

  Serial.println("Partition  Type   Subtype    Address   PartSize   ImgSize   Digest");
  Serial.println("---------+------+---------+----------+----------+---------+--------");

  while(pi != NULL) {
    const esp_partition_t* part = esp_partition_get(pi);
    esp_image_metadata_t meta;
    bool isOta = (part->label[3]=='1' || part->label[3] == '0');
    if( isOta ) meta  = getSketchMeta( part );
    Serial.printf("%-8s   0x%02x      0x%02x   0x%06x   %8d  %8s   %s\n",
      String( part->label ),
      part->type,
      part->subtype,
      part->address,
      part->size,
      isOta ? String(meta.image_len) : "n/a",
      isOta ? digest.toString( meta.image_digest ).c_str() : "n/a"
    );
    pi = esp_partition_next( pi );
  }
  esp_partition_iterator_release(pi);
}




