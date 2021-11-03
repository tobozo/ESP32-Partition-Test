#include <Update.h>
#include <rom/rtc.h>
#include "../partman.h" // partitions manager
#define resetReason (int)rtc_get_reset_reason(0)

#define BIN_NAME_PIO     "/part_pio_test.bin"
#define BIN_NAME_ARDUINO "/part_arduino_test.bin"

// 'USE_PIO' is defined in platformio.ini as additional flag
#if defined USE_PIO
  const char* sketchFrom   = "platformio";
  const char* thisBinName  = BIN_NAME_PIO;
  const char* otherBinName = BIN_NAME_ARDUINO;
#else
  const char* sketchFrom   = "Arduino IDE";
  const char* thisBinName  = BIN_NAME_ARDUINO;
  const char* otherBinName = BIN_NAME_PIO;
#endif


bool UpdateFromSD( const char* fileName )
{
  File updateBin = SD.open( fileName );
  if ( !updateBin ) {
    Serial.printf( "Error: Could not load %s binary from sd root\n", fileName );
    return false;
  }
  size_t updateSize = updateBin.size();
  Serial.printf("File %s exists (%d bytes)\n", fileName, updateSize );
  if (Update.begin( updateSize )) {
    size_t written = Update.writeStream( updateBin );
    if ( written == updateSize ) {
      Serial.println( "Written : " + String(written) + " successfully" );
    } else {
      Serial.println( "Written only : " + String(written) + "/" + String(updateSize) + ". Retry?" );
    }
    if ( Update.end() ) {
      Serial.println( "OTA done!" );
      if ( Update.isFinished() ) {
        Serial.println( "Update successfully completed. Rebooting." );
        updateBin.close();
        ESP.restart();
      } else {
        Serial.println( "Update not finished? Something went wrong!" );
      }
    } else {
      Serial.println( "Update failed. Error #: " + String( Update.getError() ) );
    }
  } else {
    Serial.println( "Not enough space to begin OTA" );
  }
  updateBin.close();
  return false;
}



void setup()
{
  Serial.begin(115200);
  SD.begin( 4, SPI, 25000000 );
  delay(1000);
  Serial.println();
  Serial.printf("************* Welcome to %s sketch ****************\n", sketchFrom );

  lsPart(); // list partitions

  if( resetReason != SW_CPU_RESET ) {
    // auto-propagate after flashing
    // 1) compare self to next OTA partition
    // 2) if partitions differ, copy self to both next OTA partition and SD then restart
    // 3) else continue
    checkStickyPartition( thisBinName );
  }
}


void loop() {

  if( !Serial.available() ) return;
  String command = Serial.readStringUntil('\n');
  command.trim();
  if( command == "" ) return;

  Serial.printf("Command: %s\n", command.c_str() );

  if( command == "propagate" ) {
    checkStickyPartition( thisBinName );
    Serial.printf("This sketch is now on both OTA partitions, and on the SD Card as %s\n", thisBinName );
  }

  if( command == "rollback" ) {
    if (Update.canRollBack()) {
      Update.rollBack();
      ESP.restart();
    } else {
      Serial.println("Error, rollback not possible");
    }
  }

  if( command == "copytosd" ) {
    // Copy self to SD Card
    copyPartition( thisBinName );
  }

  if( command == "sdload-piosketch" ) {
    // Explicit: flash the ESP with SD binary from platformio
    UpdateFromSD( BIN_NAME_PIO );
  }

  if( command == "sdload-arduinosketch" ) {
    // Explicit: flash the ESP with SD binary from arduino
    UpdateFromSD( BIN_NAME_ARDUINO );
  }

  if( command == "sdload-this" ) {
    // Contextual: flash the ESP with SD binary from the **same** platform
    UpdateFromSD( thisBinName );
  }

  if( command == "sdload-other" ) {
    // Contextual: flash the ESP with SD binary from the **other** platform
    UpdateFromSD( otherBinName );
  }

}

#if !defined ARDUINO
extern "C" {
  void loopTask(void*)
  {
    setup();
    for(;;) {
      loop();
    }
  }
  void app_main()
  {
    xTaskCreatePinnedToCore( loopTask, "loopTask", 8192, NULL, 1, NULL, 1 );
  }

}
#endif

