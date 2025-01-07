//******************************************
// CP System III Module
// Cartridge
// 32/64/128 Mbits SIMM
// Tested with  HW5
// https://github.com/herzmx/CPS3-OSCR-Adapter
//******************************************
#if (defined(ENABLE_CPS3) && defined(ENABLE_FLASH8) && defined(ENABLE_FLASH16)) 
/******************************************
   Variables
 *****************************************/
uint32_t cartRegion = 0;
uint32_t cartCD = 0;
uint32_t cartCDPatch = 0;
uint32_t multiCart = 0;
uint32_t multiCartPatch1 = 0;
uint32_t multiCartPatch2 = 0;
uint32_t cartRegionOffset = 0;
uint32_t cartnocdOffset = 0;
uint16_t flashids[8];

/******************************************
   Menu
 *****************************************/
// CPS3 start menu options
static const char cpsMenuItem1[] PROGMEM = "CPS3 cartridge";
static const char cpsMenuItem2[] PROGMEM = "32/128 Mbits SIMM";
static const char cpsMenuItem3[] PROGMEM = "64 Mbits SIMM";
static const char cpsMenuItem4[] PROGMEM = "32/128 MBytes SIMM";
static const char cpsMenuItem5[] PROGMEM = "64 MBytes SIMM";
static const char* const menuOptionsCPS[] PROGMEM = { cpsMenuItem1, cpsMenuItem2, cpsMenuItem3, cpsMenuItem4, cpsMenuItem5, FSTRING_RESET };

// Cartridge region options
static const char cpsCartRegion0[] PROGMEM = "No Patch";
static const char cpsCartRegion1[] PROGMEM = "JAP";
static const char cpsCartRegion2[] PROGMEM = "ASIA";
static const char cpsCartRegion3[] PROGMEM = "EUR";
static const char cpsCartRegion4[] PROGMEM = "USA";
static const char cpsCartRegion5[] PROGMEM = "HISPANIC";
static const char cpsCartRegion6[] PROGMEM = "BRAZIL";
static const char cpsCartRegion7[] PROGMEM = "OCEANIA";
static const char cpsCartRegion8[] PROGMEM = "ASIA NCD";
static const char* const menuOptionsCartRegion[] PROGMEM = { cpsCartRegion0, cpsCartRegion1, cpsCartRegion2, cpsCartRegion3, cpsCartRegion4, cpsCartRegion5, cpsCartRegion6, cpsCartRegion7, cpsCartRegion8, FSTRING_RESET };

// Cartridge cd options
static const char cpsCDItem1[] PROGMEM = "CD";
static const char cpsCDItem2[] PROGMEM = "NOCD";
static const char* const menuOptionsCartCD[] PROGMEM = { cpsCartRegion0, cpsCDItem1, cpsCDItem2, FSTRING_RESET };

// CPS3 start menu
void cpsMenu() {
  // create menu with title and 6 options to choose from
  unsigned char cpsType;
  convertPgm(menuOptionsCPS, 6);
  cpsType = question_box(FS(FSTRING_SELECT_CART_TYPE), menuOptions, 6, 0);

  // wait for user choice to come back from the question box menu
  switch (cpsType) {
    case 0:
      display_Clear();
      display_Update();
      mode = CORE_CPS3_CART;
      setup_CPS3();
      id_Flash8();
      wait();
      break;
    case 1:
      display_Clear();
      display_Update();
      mode = CORE_CPS3_128SIMM;
      setup_CPS3();
      id_SIMM2x8();
      wait();
      break;
    case 2:
      display_Clear();
      display_Update();
      mode = CORE_CPS3_64SIMM;
      setup_CPS3();
      id_SIMM4x8();
      wait();
      break;
    case 3:
      display_Clear();
      display_Update();
      mode = CORE_CPS3_01SIMM;
      setup_CPS3();
      id_SIMM16();
      wait();
      break;
    case 4:
      display_Clear();
      display_Update();
      mode = CORE_CPS3_512SIMM;
      setup_CPS3();
      id_SIMM2x16();
      wait();
      break;
    case 5:
      resetArduino();
      break;
    default:
      print_MissingModule();  // does not return
  }
}

// CPS3 Cartridge menu
void flashromCPS_Cartridge() {
  // create menu with title and 7 options to choose from
  unsigned char mainMenu;
  // Copy menuOptions out of progmem
  convertPgm(menuOptionsFLASH8, 7);
  mainMenu = question_box(F("CPS3 Cartdrige Writer"), menuOptions, 7, 0);

  // wait for user choice to come back from the question box menu
  switch (mainMenu) {
    case 0:
      display_Clear();
      println_Msg(F("Blankcheck"));
      display_Update();
      time = millis();
      resetFlash8();
      blankcheck_Flash();
      break;

    case 1:
      if (flashromType != 0) {
        display_Clear();
        println_Msg(F("Warning: This will erase"));
        println_Msg(F("your CPS3 Cartdrige BIOS"));
        print_STR(press_button_STR, 1);
        display_Update();
        wait();
        rgbLed(black_color);
        println_Msg(FS(FSTRING_EMPTY));
        println_Msg(F("Please wait..."));
        display_Update();
        time = millis();

        switch (flashromType) {
          case 1: eraseFlash29F032(); break;
          case 2: eraseFlash29F1610(); break;
          case 3: eraseFlash28FXXX(); break;
        }

        println_Msg(F("CPS3 Cartdrige BIOS erased"));
        display_Update();
        resetFlash8();
      } else {
        readOnlyMode();
      }
      break;

    case 2:
      time = millis();
      resetFlash8();
      readCartridge();
      break;

    case 3:
      if (flashromType != 0) {
        // Set cartridge region
        filePath[0] = '\0';
        sd.chdir("/");
        fileBrowser(FS(FSTRING_SELECT_FILE));
        // Calculate CRC32 of BIOS
        display_Clear();
        println_Msg("Looking BIOS patch");
        println_Msg(FS(FSTRING_EMPTY));
        println_Msg(F("Please wait..."));
        display_Update();
        char crcStr[9];
        sprintf(crcStr, "%08lX", calculateCRC(fileName, filePath, 0));
        setCartridgePatchData(crcStr);
        display_Clear();
        time = millis();

        switch (flashromType) {
          case 1:
            break;
          case 2:
            if ((flashid == 0x0458) || (flashid == 0x0158) || (flashid == 0x01AB) || (flashid == 0x0422) || (flashid == 0x0423))
              writeCartridge();
            else if (flashid == 0x0)  // Manual flash config, pick most common type
              writeFlash29LV640();
            break;
          case 3:
            break;
        }

        delay(100);

        // Reset twice just to be sure
        resetFlash8();
        resetFlash8();

        verifyCartridge();
      } else {
        readOnlyMode();
      }
      break;

    case 4:
      time = 0;
      display_Clear();
      resetFlash8();
      println_Msg(F("ID Flashrom"));
      switch (flashromType) {
        case 0: break;
        case 1: idFlash29F032(); break;
        case 2: idFlash29F1610(); break;
        case 3: idFlash28FXXX(); break;
      }
      println_Msg(FS(FSTRING_EMPTY));
      printFlash(40);
      println_Msg(FS(FSTRING_EMPTY));
      display_Update();
      resetFlash8();
      break;

    case 5:
      time = 0;
      display_Clear();
      println_Msg(F("Print first 70Bytes"));
      display_Update();
      resetFlash8();
      printFlash(70);
      break;

    case 6:
      time = 0;
      display_Clear();
      display_Update();
      resetFlash8();
      resetArduino();
      break;
  }
  if (time != 0) {
    print_Msg(F("Operation took : "));
    print_Msg((millis() - time) / 1000, DEC);
    println_Msg(F("s"));
    display_Update();
  }
  // Prints string out of the common strings array either with or without newline
  print_STR(press_button_STR, 0);
  display_Update();
  wait();
}

// CPS3 Cartridge region patch menu
void cpsCartRegionMenu() {
  cartRegion = pageMenu(F("Cartridge Region Patch"), menuOptionsCartRegion, 10);
  if (cartRegion < 9) {
    display_Clear();
    display_Update();
  } else {
    time = 0;
    display_Clear();
    display_Update();
    resetFlash8();
    resetArduino();
  }
}

// CPS3 Cartridge CD patch menu
void cpsCartCDMenu() {
  // Copy menuOptions out of progmem
  convertPgm(menuOptionsCartCD, 4);
  cartCD = question_box(F("Cartridge CD Patch"), menuOptions, 4, 0);

  // wait for user choice to come back from the question box menu
  if (cartCD < 3) {
    display_Clear();
    display_Update();
  } else {
    time = 0;
    display_Clear();
    display_Update();
    resetFlash8();
    resetArduino();
  }
}

// CPS3 32/128MB SIMM menu
void flashromCPS_SIMM16() {
  // create menu with title and 7 options to choose from
  unsigned char mainMenu;
  // Copy menuOptions out of progmem
  convertPgm(menuOptionsFLASH8, 7);
  mainMenu = question_box(F("CPS3 SIMM 32/128 Writer"), menuOptions, 7, 0);

  // wait for user choice to come back from the question box menu
  switch (mainMenu) {
    case 0:
      display_Clear();
      println_Msg(F("Blankcheck"));
      display_Update();
      time = millis();
      resetSIMM16();
      blankcheckSIMM16();
      break;

    case 1:
      if (flashromType != 0) {
        display_Clear();
        println_Msg(F("Warning: This will erase"));
        println_Msg(F("your CPS3 SIMM 32/128"));
        print_STR(press_button_STR, 1);
        display_Update();
        wait();
        rgbLed(black_color);
        println_Msg(FS(FSTRING_EMPTY));
        println_Msg(F("Please wait..."));
        display_Update();
        time = millis();

        switch (flashromType) {
          case 1:
            if (flashid == 0x04AD)
              eraseSIMM2x8();
            break;
          case 2:
            if (flashid == 0x017E || flashid == 0x897E)
              eraseFlash16();
            break;
          case 3:
            break;
        }
        println_Msg(F("CPS3 SIMM 32/128 erased"));
        display_Update();
        resetSIMM16();
      } else {
        readOnlyMode();
      }
      break;

    case 2:
      time = millis();
      resetSIMM16();
      readSIMM16();
      break;

    case 3:
      if (flashromType != 0) {
        // Set cartridge region
        filePath[0] = '\0';
        sd.chdir("/");
        fileBrowser(FS(FSTRING_SELECT_FILE));
        display_Clear();
        time = millis();

        switch (flashromType) {
          case 1:
            if (flashid == 0x04AD)
              writeSIMM2x8();
            break;
          case 2:
            if (flashid == 0x017E || flashid == 0x897E)
              writeSIMM16(sectorSize, bufferSize);
            break;
          case 3:
            break;
        }

        delay(100);

        // Reset twice just to be sure
        resetSIMM16();
        resetSIMM16();

        verifySIMM16();
      } else {
        readOnlyMode();
      }
      break;

    case 4:
      time = 0;
      display_Clear();
      resetSIMM16();
      println_Msg(F("ID Flashrom"));
      switch (flashromType) {
        case 1:
          if (flashid == 0x04AD)
            idFlash2x8(0x0);
          break;
        case 2:
          if (flashid == 0x017E || flashid == 0x897E)
            idFlash16();
          break;
        case 3:
          break;
      }
      println_Msg(FS(FSTRING_EMPTY));
      printFlash16(40);
      println_Msg(FS(FSTRING_EMPTY));
      display_Update();
      resetSIMM16();
      break;

    case 5:
      time = 0;
      display_Clear();
      println_Msg(F("Print first 70Bytes"));
      display_Update();
      resetSIMM16();
      printFlash16(70);
      break;

    case 6:
      time = 0;
      display_Clear();
      display_Update();
      resetSIMM16();
      resetArduino();
      break;
  }
  if (time != 0) {
    print_Msg(F("Operation took : "));
    print_Msg((millis() - time) / 1000, DEC);
    println_Msg(F("s"));
    display_Update();
  }
  // Prints string out of the common strings array either with or without newline
  print_STR(press_button_STR, 0);
  display_Update();
  wait();
}

// CPS3 64MB SIMM menu
void flashromCPS_SIMM2x16() {
  // create menu with title and 7 options to choose from
  unsigned char mainMenu;
  // Copy menuOptions out of progmem
  convertPgm(menuOptionsFLASH8, 7);
  mainMenu = question_box(F("CPS3 SIMM 64 Writer"), menuOptions, 7, 0);

  // wait for user choice to come back from the question box menu
  switch (mainMenu) {
    case 0:
      display_Clear();
      println_Msg(F("Blankcheck"));
      display_Update();
      time = millis();
      resetSIMM2x16();
      blankcheckSIMM2x16();
      break;

    case 1:
      if (flashromType != 0) {
        display_Clear();
        println_Msg(F("Warning: This will erase"));
        println_Msg(F("your CPS3 SIMM 64"));
        print_STR(press_button_STR, 1);
        display_Update();
        wait();
        rgbLed(black_color);
        println_Msg(FS(FSTRING_EMPTY));
        println_Msg(F("Please wait..."));
        display_Update();
        time = millis();

        switch (flashromType) {
          case 1:
            if (flashid == 0x04AD)
              eraseSIMM4x8();
            break;
          case 2:
            if (flashid == 0x017E || flashid == 0x897E)
              eraseSIMM2x16();
            break;
          case 3:
            break;
        }

        println_Msg(F("CPS3 SIMM 64 erased"));
        display_Update();
        resetSIMM2x16();
      } else {
        readOnlyMode();
      }
      break;

    case 2:
      time = millis();
      resetSIMM2x16();
      readSIMM2x16();
      break;

    case 3:
      if (flashromType != 0) {
        // Set cartridge region
        filePath[0] = '\0';
        sd.chdir("/");
        fileBrowser(FS(FSTRING_SELECT_FILE));
        display_Clear();
        time = millis();

        switch (flashromType) {
          case 1:
            if (flashid == 0x04AD)
              writeSIMM4x8();
            break;
          case 2:
            if (flashid == 0x017E || flashid == 0x897E)
              writeSIMM2x16(sectorSize, bufferSize);
            break;
          case 3:
            break;
        }

        delay(100);

        // Reset twice just to be sure
        resetSIMM2x16();
        resetSIMM2x16();

        verifySIMM2x16();
      } else {
        readOnlyMode();
      }
      break;

    case 4:
      time = 0;
      display_Clear();
      resetFlash8();
      println_Msg(F("ID Flashrom"));
      enable64MLSB();
      switch (flashromType) {
        case 1:
          if (flashid == 0x04AD)
            idFlash2x8(0x0);
          break;
        case 2:
          if (flashid == 0x017E || flashid == 0x897E)
            idSIMM16();
          break;
        case 3:
          break;
      }
      println_Msg(FS(FSTRING_EMPTY));
      printSIMM2x16(40);
      println_Msg(FS(FSTRING_EMPTY));
      display_Update();
      resetSIMM2x16();
      break;

    case 5:
      time = 0;
      display_Clear();
      println_Msg(F("Print first 70Bytes"));
      display_Update();
      resetSIMM2x16();
      printSIMM2x16(70);
      break;

    case 6:
      time = 0;
      display_Clear();
      display_Update();
      resetSIMM2x16();
      resetArduino();
      break;
  }
  if (time != 0) {
    print_Msg(F("Operation took : "));
    print_Msg((millis() - time) / 1000, DEC);
    println_Msg(F("s"));
    display_Update();
  }
  // Prints string out of the common strings array either with or without newline
  print_STR(press_button_STR, 0);
  display_Update();
  wait();
}

/******************************************
   Setup
 *****************************************/
void setup_CPS3() {
  // Set Address Pins to Output
  //A0-A7
  DDRF = 0xFF;
  //A8-A15
  DDRK = 0xFF;
  //A16-A23
  DDRL = 0xFF;
  //A24(PJ0), A25(PJ1)
  DDRJ |= (1 << 0) | (1 << 1);

  // Set Control Pins to Output
  // RST(PH0) OE(PH1) BYTE(PH3) WE(PH4) CKIO_CPU(PH5) CE/CER_CST(PH6)
  DDRH |= (1 << 0) | (1 << 1) | (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6);
  // CE64LSB(PE3) CE128(PE4)
  DDRE |= (1 << 3) | (1 << 4);
  // RST_CPUC(PG1) CE64MSB(PG5)
  DDRG |= (1 << 1) | (1 << 5);

  // Set Data Pins (D0-D15) to Input
  DDRC = 0x00;
  DDRA = 0x00;
  // Disable Internal Pullups
  PORTC = 0x00;
  PORTA = 0x00;

  // Setting A24(PJ0), A25(PJ1) LOW
  setA24A25(0);

  // Setting RST(PH0) OE(PH1) WE(PH4) CKIO_CPU(PH5) HIGH
  PORTH |= (1 << 0) | (1 << 1) | (1 << 4) | (1 << 5);
  // Setting CE64LSB(PE3) CE128(PE4) HIGH
  PORTE |= (1 << 3) | (1 << 4);
  // Setting CE64MSB(PG5) HIGH
  PORTG |= (1 << 5);
  // Setting CE_CART/CER_CST(PH6) HIGH
  PORTH |= (1 << 6);
  // Setting BYTE(PH3) LOW
  PORTH &= ~(1 << 3);
  // Setting RST_CPUC(PG1) LOW
  PORTG &= ~(1 << 1);

  if (mode == CORE_CPS3_128SIMM || mode == CORE_CPS3_01SIMM) {
    // Setting CE128(PE4) LOW
    PORTE &= ~(1 << 4);
  } else if (mode == CORE_CPS3_CART) {
    // Setting CE_CART/CER_CST(PH6) LOW
    PORTH &= ~(1 << 6);
  }

  byteCtrl = 1;
}

/******************************************
   Low level functions
 *****************************************/
void setA24A25(unsigned long myAddress) {
  switch ((myAddress >> 24) & 0xFF) {
    case 0x0:
      // Setting A24(PJ0), A25(PJ1) LOW
      PORTJ &= ~((1 << 0) | (1 << 1));
      break;
    case 0x1:
      // Setting A24(PJ0) HIGH
      PORTJ |= (1 << 0);
      // Setting A25(PJ0) LOW
      PORTJ &= ~(1 << 1);
      break;
    case 0x2:
      // Setting A24(PJ0) LOW
      PORTJ &= ~(1 << 0);
      // Setting A25(PJ0) HIGH
      PORTJ |= (1 << 1);
      break;
    case 0x3:
      // Setting A24(PJ0), A25(PJ1) HIGH
      PORTJ |= (1 << 0) | (1 << 1);
      break;
  }
}

void enable64MLSB() {
  // Setting CE64LSB(PE3) LOW
  PORTE &= ~(1 << 3);
  // Setting CE64MSB(PG5) LOW
  PORTG &= ~(1 << 5);
  // Wait till output is stable
  NOP;
  NOP;
}

void enable64MSB() {
  // Setting CE64LSB(PE3) HIGH
  PORTE |= (1 << 3);
  // Setting CE64MSB(PG5) LOW
  PORTG &= ~(1 << 5);
  // Wait till output is stable
  NOP;
  NOP;
}

void enable64LSB() {
  // Setting CE64MSB(PG5) HIGH
  PORTG |= (1 << 5);
  // Setting CE64LSB(PE3) LOW
  PORTE &= ~(1 << 3);
  // Wait till output is stable
  NOP;
  NOP;
}

byte readByte_SIMM(unsigned long myAddress) {
  // A0-A7
  PORTF = myAddress & 0xFF;
  // A8-A15
  PORTK = (myAddress >> 8) & 0xFF;
  // A16-A23
  PORTL = (myAddress >> 16) & 0xFF;

  // Arduino running at 16Mhz -> one nop = 62.5ns
  NOP;

  // Setting OE(PH1)
  PORTH &= ~(1 << 1);

  NOP;
  NOP;

  // Read
  byte tempByte = PINC;

  PORTH |= (1 << 1);
  NOP;
  

  return tempByte;
}

/******************************************
  helper functions
*****************************************/
uint32_t char2int(char hex) {
  uint32_t byte = 0;
  // transform hex character to the 4bit equivalent number, using the ascii table indexes
  if (hex >= '0' && hex <= '9')
    byte = hex - 48;
  else if (hex >= 'A' && hex <= 'F')
    byte = hex - 55;
  return byte;
}

/******************************************
  Command functions
*****************************************/
void writeByteCommand_Flash2x8(uint32_t bank, byte command) {
  writeWord_Flash((bank << 21) | 0x555, 0xaaaa);
  writeWord_Flash((bank << 21) | 0x2aa, 0x5555);
  writeWord_Flash((bank << 21) | 0x555, command << 8 | command);
}

/******************************************
  Cartridge functions
*****************************************/
void readCartridge() {
  // Reset to root directory
  sd.chdir("/");

  createFolderAndOpenFile("CPS3", "Cartridge", "29f400", "u2");

  //Initialize progress bar
  uint32_t processedProgressBar = 0;
  uint32_t totalProgressBar = flashSize;
  draw_progressbar(0, totalProgressBar);

  for (unsigned long currByte = 0; currByte < flashSize; currByte += 512) {
    for (int c = 0; c < 512; c++) {
      sdBuffer[c] = readByte_Flash(currByte + c);
    }
    myFile.write(sdBuffer, 512);
    // Update progress bar
    processedProgressBar += 512;
    draw_progressbar(processedProgressBar, totalProgressBar);
    // Blink led
    if (currByte % 25600 == 0)
      blinkLED();
  }

  // Close the file:
  myFile.close();
  // Compare dump CRC with db values
  compareCRC("cps3.txt", 0, 1, 0);
  println_Msg(F("Finished reading"));
  display_Update();
}

void writeCartridge() {
  if (openFlashFile()) {
    // Set data pins to output
    dataOut();

    // Initialize progress bar
    uint32_t processedProgressBar = 0;
    uint32_t totalProgressBar = (uint32_t)fileSize;
    draw_progressbar(0, totalProgressBar);

    // Fill sdBuffer
    for (unsigned long currByte = 0; currByte < fileSize; currByte += 512) {
      myFile.read(sdBuffer, 512);
      // Blink led
      if (currByte % 2048 == 0)
        blinkLED();

      for (int c = 0; c < 512; c++) {
        // Write command sequence
        writeByteCommandShift_Flash(0xa0);

        // Write patch to avoid region menu in multi
        if ((currByte + c) == 0x405F && cartRegion > 0 && multiCart == 1) {
          writeByte_Flash(currByte + c, multiCartPatch1 & 0xFF);
          busyCheck29F032(currByte + c, multiCartPatch1 & 0xFF);
          continue;
        }
        if ((currByte + c) == 0x20746 && cartRegion > 0 && multiCart == 1) {
          writeByte_Flash(currByte + c, multiCartPatch2 & 0xFF);
          busyCheck29F032(currByte + c, multiCartPatch2 & 0xFF);
          continue;
        }
        // Write cartridge region
        if ((currByte + c) == cartRegionOffset && cartRegion > 0) {
          writeByte_Flash(currByte + c, cartRegion & 0xFF);
          busyCheck29F032(currByte + c, cartRegion & 0xFF);
          continue;
        }
        // Write cartridge cd
        if ((currByte + c) == cartnocdOffset && cartCD > 0) {
          writeByte_Flash(currByte + c, cartCDPatch & 0xFF);
          busyCheck29F032(currByte + c, cartCDPatch & 0xFF);
          continue;
        }
        // Write current byte
        writeByte_Flash(currByte + c, sdBuffer[c]);
        busyCheck29F032(currByte + c, sdBuffer[c]);
      }
      // update progress bar
      processedProgressBar += 512;
      draw_progressbar(processedProgressBar, totalProgressBar);
    }

    // Set data pins to input again
    dataIn8();

    // Close the file:
    myFile.close();
  }
}

void verifyCartridge() {
  if (openVerifyFlashFile()) {
    blank = 0;

    //Initialize progress bar
    uint32_t processedProgressBar = 0;
    uint32_t totalProgressBar = fileSize;
    draw_progressbar(0, totalProgressBar);

    for (unsigned long currByte = 0; currByte < fileSize; currByte += 512) {
      //fill sdBuffer
      myFile.read(sdBuffer, 512);
      for (int c = 0; c < 512; c++) {
        // Verify patch to avoid region menu
        if ((currByte + c) == 0x405F && cartRegion > 0 && multiCart == 1 && readByte_Flash(currByte + c) == multiCartPatch1 & 0xFF) {
          continue;
        } else if ((currByte + c) == 0x20746 && cartRegion > 0 && multiCart == 1 && readByte_Flash(currByte + c) == multiCartPatch2 & 0xFF) {
          continue;
          // Verify cartridge region
        } else if ((currByte + c) == cartRegionOffset && cartRegion > 0 && readByte_Flash(currByte + c) == cartRegion & 0xFF) {
          continue;
          // Verify cartridge cd
        } else if ((currByte + c) == cartnocdOffset && cartCD > 0 && readByte_Flash(currByte + c) == cartCDPatch & 0xFF) {
          continue;
        } else if (readByte_Flash(currByte + c) != sdBuffer[c]) {
          blank++;
        }
      }
      // Update progress bar
      processedProgressBar += 512;
      draw_progressbar(processedProgressBar, totalProgressBar);
      // Blink led
      if (currByte % 25600 == 0)
        blinkLED();
    }
    if (blank == 0) {
      println_Msg(F("Cart BIOS verified OK"));
      display_Update();
    } else {
      print_STR(error_STR, 0);
      print_Msg(blank);
      print_STR(_bytes_STR, 1);
      print_Error(did_not_verify_STR);
    }
    // Close the file:
    myFile.close();
  }
}

void setCartridgePatchData(char crcStr[9]) {
  // Init vars
  cartRegionOffset = 0;
  //Search for CRC32 in file
  char crc_search[9];
  char tmpDWord[9];
  //go to root
  sd.chdir("/");
  if (myFile.open("cps3.txt", O_READ)) {
    while (myFile.available()) {
      // Skip first line with name
      skip_line(&myFile);
      // Read the CRC32 from database
      for (byte i = 0; i < 8; i++) {
        crc_search[i] = char(myFile.read());
      }
      crc_search[8] = '\0';
      //if checksum search successful, set patch data
      if (strcmp(crc_search, crcStr) == 0) {
        // Skip the , in the file
        myFile.seekCur(1);
        // Read region offset
        cartRegionOffset = char2int(myFile.read()) << 16 | char2int(myFile.read()) << 12 | char2int(myFile.read()) << 8 | char2int(myFile.read()) << 4 | char2int(myFile.read());
        // Skip the , in the file
        myFile.seekCur(1);
        if (cartRegionOffset != 0) {
          // Ask for cart region change
          cpsCartRegionMenu();
          if (cartRegion != 0) {
            uint8_t skipRegions = 8 - cartRegion;
            cartRegion -= 1;
            myFile.seekCur(cartRegion * 2);
            cartRegion = (char2int(myFile.read())) << 4 | char2int(myFile.read());
            myFile.seekCur(skipRegions * 2);
          } else {
            myFile.seekCur(16);
          }
        } else {
          myFile.seekCur(16);
        }
        // Skip the , in the file
        myFile.seekCur(1);
        cartnocdOffset = char2int(myFile.read()) << 16 | char2int(myFile.read()) << 12 | char2int(myFile.read()) << 8 | char2int(myFile.read()) << 4 | char2int(myFile.read());
        // Skip the , in the file
        myFile.seekCur(1);
        if (cartnocdOffset != 0) {
          // Ask for cart cd change
          cpsCartCDMenu();
          cartCDPatch = char2int(myFile.read()) << 4 | char2int(myFile.read());
          if (cartCD == 1) {
            cartCDPatch = char2int(myFile.read()) << 4 | char2int(myFile.read());
          } else {
            // Skip the byte
            myFile.seekCur(2);
          }
        } else {
          myFile.seekCur(4);
        }
        // Skip the , in the file
        myFile.seekCur(1);
        // Read multi cart dat from database
        multiCart = char2int(myFile.read());
        // Read multi cart patches
        if (multiCart == 1) {
          multiCartPatch1 = char2int(myFile.read()) << 4 | char2int(myFile.read());
          multiCartPatch2 = char2int(myFile.read()) << 4 | char2int(myFile.read());
        }
        break;
      }
      // If no match go to next entry
      else {
        // skip rest of line
        myFile.seekCur(42);
        // skip third empty line
        skip_line(&myFile);
      }
    }
    myFile.close();
  }
}

/******************************************
  2x8bit SIMM functions
*****************************************/
void id_SIMM2x8() {
  idFlash2x8(0x0);
  idFlash2x8(0x1);
  idFlash2x8(0x2);
  idFlash2x8(0x3);
  resetSIMM2x8();
  uint8_t ngFlash = 0;
  uint8_t okFlash = 0;

  if (flashids[0] == flashids[1]) {
    flashid = flashids[0];
    sprintf(flashid_str, "%04X", flashid);
    okFlash = 2;
    for (byte i = 2; i < 8; i++) {
      if (flashid == flashids[i])
        okFlash += 1;
      else
        ngFlash += 1;
    }
  }
  // Print start screen
  display_Clear();
  display_Update();
  println_Msg(F("SIMM Writer 2x8bit"));
  println_Msg("");
  println_Msg("");
  print_Msg(F("Flash ID: "));
  println_Msg(flashid_str);
  if (flashid == 0x04AD && okFlash == 2 && ngFlash == 6) {
    println_Msg(F("32 Mbit Fujitsu SIMM detected"));
    flashSize = 0x400000;
    flashromType = 1;
  } else if (flashid == 0x04AD && okFlash == 8) {
    println_Msg(F("128 Mbit Fujitsu SIMM detected"));
    flashSize = 0x1000000;
    flashromType = 1;
  } else if (flashid == 0x04AD && okFlash > 2) {
    println_Msg(F("?? Mbit Fujitsu SIMM detected"));
    println_Msg(F("With some bad flash"));
    flashSize = 0x1000000;
    flashromType = 0;
  } else {
    // ID not found
    flashSize = 0x1000000;
    flashromType = 0;
    display_Clear();
    println_Msg(F("SIMM Writer 2x8bit"));
    println_Msg("");
    print_Msg(F("ID Type 1: "));
    println_Msg(vendorID);
    print_Msg(F("ID Type 2: "));
    println_Msg(flashid_str);
    println_Msg("");
    println_Msg(F("UNKNOWN FLASHROM"));
    println_Msg("");
    // Prints string out of the common strings array either with or without newline
    print_Error(press_button_STR);
    display_Update();
    wait();
    // print first 40 bytes of flash
    display_Clear();
    println_Msg(F("First 40 bytes:"));
    println_Msg(FS(FSTRING_EMPTY));
    printFlash16(40);
    resetSIMM2x8();
  }
  println_Msg(FS(FSTRING_EMPTY));
  // Prints string out of the common strings array either with or without newline
  print_STR(press_button_STR, 1);
  display_Update();
  resetSIMM2x8();
}

void resetSIMM2x8() {
  resetFlash2x8(0x3);
  resetFlash2x8(0x2);
  resetFlash2x8(0x1);
  resetFlash2x8(0x0);
}

void blankcheckSIMM16() {

  //Initialize progress bar
  uint32_t processedProgressBar = 0;
  uint32_t totalProgressBar = flashSize;
  draw_progressbar(0, totalProgressBar);

  blank = 1;
  word d = 0;
  unsigned long simmAddress = 0;
  for (unsigned long currBuffer = 0; currBuffer < flashSize / 2; currBuffer += 256) {
    // Fill buffer
    for (int c = 0; c < 256; c++) {
      simmAddress = currBuffer + c;
      word currWord = readWord_Flash(simmAddress);
      // Read byte right
      sdBuffer[d + 1] = ((currWord >> 8) & 0xFF);
      // Read byte left
      sdBuffer[d] = (currWord & 0xFF);
      d += 2;
    }
    // Check if all bytes are 0xFF
    d = 0;
    for (unsigned long currByte = 0; currByte < 256; currByte++) {
      if (sdBuffer[d] != 0xFF || sdBuffer[d + 1] != 0xFF) {
        currByte = 256;
        currBuffer = flashSize / 2;
        blank = 0;
      }
      d += 2;
    }
    d = 0;
    // Update progress bar
    processedProgressBar += 512;
    draw_progressbar(processedProgressBar, totalProgressBar);
    // Blink led
    if (currBuffer % 25600 == 0)
      blinkLED();
    if (processedProgressBar % 0x2000000 == 0)
      setA24A25(simmAddress + 1);
  }
  setA24A25(0);
  if (blank) {
    println_Msg(F("SIMM is empty"));
    display_Update();
  } else {
    println_Msg(FS(FSTRING_EMPTY));
    print_Error(F("Error: Not blank"));
  }
}

// From readFlash16
void readSIMM16() {
  // Reset to root directory
  sd.chdir("/");

  createFolderAndOpenFile("CPS3", "SIMM", "128M", "bin");
  //Initialize progress bar
  uint32_t processedProgressBar = 0;
  uint32_t totalProgressBar = flashSize;
  draw_progressbar(0, totalProgressBar);

  word d = 0;
  unsigned long simmAddress = 0;
  for (unsigned long currByte = 0; currByte < flashSize / 2; currByte += 256) {
    for (word c = 0; c < 256; c++) {
      simmAddress = currByte + c;

      word currWord = readWord_Flash(simmAddress);
      // Split word into two bytes
      // Right
      sdBuffer[d + 1] = ((currWord >> 8) & 0xFF);
      // Left
      sdBuffer[d] = (currWord & 0xFF);
      d += 2;
    }
    myFile.write(sdBuffer, 512);
    d = 0;
    // Update progress bar
    processedProgressBar += 512;
    draw_progressbar(processedProgressBar, totalProgressBar);
    // Blink led
    if (currByte % 25600 == 0)
      blinkLED();
    if (processedProgressBar % 0x2000000 == 0)
      setA24A25(simmAddress + 1);
  }
  setA24A25(0);

  // Close the file:
  myFile.close();
  println_Msg(F("Finished reading."));
  display_Update();
}

void eraseSIMM2x8() {
  eraseFlash2x8(0x3);
  eraseFlash2x8(0x2);
  eraseFlash2x8(0x1);
  eraseFlash2x8(0x0);
}

// From writeFlash29F032
void writeSIMM2x8() {
  if (openFlashFile()) {
    // Set data pins to output
    dataOut16();

    //Initialize progress bar
    uint32_t processedProgressBar = 0;
    uint32_t totalProgressBar = (uint32_t)fileSize;
    draw_progressbar(0, totalProgressBar);

    // Fill sdBuffer LSB
    unsigned long simmAddress = 0;
    for (unsigned long currByte = 0; currByte < fileSize / 2; currByte += 256) {
      myFile.read(sdBuffer, 512);
      // Blink led
      if (currByte % 4096 == 0)
        blinkLED();

      noInterrupts();
      for (int c = 0; c < 256; c++) {
        simmAddress = currByte + c;
        word myWord = ((sdBuffer[(c * 2) + 1] & 0xFF) << 8) | (sdBuffer[c * 2] & 0xFF);
        // Skip if data exist in flash
        dataIn16();
        word wordFlash = readWord_Flash(simmAddress);
        dataOut16();
        if (wordFlash == myWord || myWord == 0xFFFF) {
          continue;
        }
        // Write command sequence
        writeByteCommand_Flash2x8(simmAddress >> 21, 0xa0);
        // Write current word
        writeWord_Flash(simmAddress, myWord);
        busyCheck2x8(simmAddress, myWord);
      }
      interrupts();
      // update progress bar
      processedProgressBar += 512;
      draw_progressbar(processedProgressBar, totalProgressBar);
    }

    // Set data pins to input again
    dataIn16();

    // Close the file:
    myFile.close();
  }
}

// From verifyFlash16
void verifySIMM16() {
  if (openVerifyFlashFile()) {
    blank = 0;
    word d = 0;
    unsigned long simmAddress = 0;
    //Initialize progress bar
    uint32_t processedProgressBar = 0;
    uint32_t totalProgressBar = fileSize;
    draw_progressbar(0, totalProgressBar);
    for (unsigned long currByte = 0; currByte < fileSize / 2; currByte += 256) {
      //fill sdBuffer
      myFile.read(sdBuffer, 512);
      for (int c = 0; c < 256; c++) {
        // Set address
        simmAddress = currByte + c;
        word currWord = ((sdBuffer[d + 1] << 8) | sdBuffer[d]);
        if (readWord_Flash(simmAddress) != currWord) {
          blank++;
        }
        d += 2;
      }
      d = 0;
      // Update progress bar
      processedProgressBar += 512;
      if (processedProgressBar % 0x2000000 == 0)
        setA24A25(simmAddress + 1);
      draw_progressbar(processedProgressBar, totalProgressBar);
      // Blink led
      if (currByte % 25600 == 0)
        blinkLED();
    }
    setA24A25(0);
    if (blank == 0) {
      println_Msg(F("SIMM verified OK"));
      display_Update();
    } else {
      print_STR(error_STR, 0);
      print_Msg(blank);
      print_STR(_bytes_STR, 1);
      print_Error(did_not_verify_STR);
    }
    // Close the file:
    myFile.close();
  }
}

/******************************************
  4x8bit SIMM functions
*****************************************/
void id_SIMM4x8() {
  enable64MSB();
  idFlash2x8(0x0);
  enable64LSB();
  idFlash2x8(0x1);
  resetSIMM4x8();
  uint8_t ngFlash = 0;
  uint8_t okFlash = 0;

  flashid = flashids[0];
  for (byte i = 0; i < 4; i++) {
    if (flashid == flashids[i])
      okFlash += 1;
    else
      ngFlash += 1;
  }
  sprintf(flashid_str, "%04X", flashid);

  // Print start screen
  display_Clear();
  display_Update();
  println_Msg(F("SIMM Writer 4x8bit"));
  println_Msg("");
  println_Msg("");
  print_Msg(F("Flash ID: "));
  println_Msg(flashid_str);
  if (flashid == 0x04AD && okFlash == 4) {
    println_Msg(F("64 Mbit Fujitsu SIMM detected"));
    flashSize = 0x800000;
    flashromType = 1;
  } else if (flashid == 0x04AD && okFlash < 4) {
    println_Msg(F("Fujitsu SIMM detected"));
    println_Msg(F("With some bad flash"));
    flashSize = 0x800000;
    flashromType = 0;
  } else {
    // ID not found
    display_Clear();
    println_Msg(F("SIMM Writer 4x8bit"));
    println_Msg("");
    print_Msg(F("ID Type 1: "));
    println_Msg(vendorID);
    print_Msg(F("ID Type 2: "));
    println_Msg(flashid_str);
    println_Msg("");
    println_Msg(F("UNKNOWN FLASHROM"));
    println_Msg("");
    // Prints string out of the common strings array either with or without newline
    print_Error(press_button_STR);
    display_Update();
    wait();
    // print first 40 bytes of flash
    display_Clear();
    println_Msg(F("First 40 bytes:"));
    println_Msg(FS(FSTRING_EMPTY));
    printFlash16(40);
    resetSIMM4x8();
  }
  println_Msg(FS(FSTRING_EMPTY));
  // Prints string out of the common strings array either with or without newline
  print_STR(press_button_STR, 1);
  display_Update();
  resetSIMM4x8();
}

void resetSIMM4x8() {
  enable64MSB();
  resetFlash2x8(0x0);
  enable64LSB();
  resetFlash2x8(0x0);
}

void blankcheckSIMM2x16() {

  //Initialize progress bar
  uint32_t processedProgressBar = 0;
  uint32_t totalProgressBar = flashSize;
  draw_progressbar(0, totalProgressBar);

  blank = 1;
  word d = 0;
  for (unsigned long currBuffer = 0; currBuffer < flashSize / 4; currBuffer += 128) {
    // Fill buffer
    for (int c = 0; c < 128; c++) {
      enable64MSB();
      word currWord = readWord_Flash(currBuffer + c);
      // Read byte right
      sdBuffer[d + 1] = ((currWord >> 8) & 0xFF);
      // Read byte left
      sdBuffer[d] = (currWord & 0xFF);
      enable64LSB();
      currWord = readWord_Flash(currBuffer + c);
      // Read byte right
      sdBuffer[d + 3] = ((currWord >> 8) & 0xFF);
      // Read byte left
      sdBuffer[d + 2] = (currWord & 0xFF);
      d += 4;
    }
    // Check if all bytes are 0xFF
    d = 0;
    for (unsigned long currByte = 0; currByte < 128; currByte++) {
      if (sdBuffer[d] != 0xFF || sdBuffer[d + 1] != 0xFF || sdBuffer[d + 2] != 0xFF || sdBuffer[d + 3] != 0xFF) {
        currByte = 128;
        currBuffer = flashSize / 4;
        blank = 0;
      }
      d += 4;
    }
    d = 0;
    // Update progress bar
    processedProgressBar += 512;
    draw_progressbar(processedProgressBar, totalProgressBar);
    // Blink led
    if (currBuffer % 25600 == 0)
      blinkLED();
  }
  if (blank) {
    println_Msg(F("SIMM is empty"));
    display_Update();
  } else {
    println_Msg(FS(FSTRING_EMPTY));
    print_Error(F("Error: Not blank"));
  }
}

void eraseSIMM4x8() {
  enable64MSB();
  eraseFlash2x8(0x0);
  enable64LSB();
  eraseFlash2x8(0x0);
}

// From readFlash16
void readSIMM2x16() {
  // Reset to root directory
  sd.chdir("/");

  createFolderAndOpenFile("CPS3", "SIMM", "64M", "bin");
  //Initialize progress bar
  uint32_t processedProgressBar = 0;
  uint32_t totalProgressBar = flashSize;
  draw_progressbar(0, totalProgressBar);

  word d = 0;
  for (unsigned long currByte = 0; currByte < flashSize / 4; currByte += 128) {
    for (word c = 0; c < 128; c++) {
      enable64MSB();
      word currWord = readWord_Flash(currByte + c);
      // Split word into two bytes
      // Right
      sdBuffer[d + 1] = ((currWord >> 8) & 0xFF);
      // Left
      sdBuffer[d] = (currWord & 0xFF);

      enable64LSB();
      currWord = readWord_Flash(currByte + c);
      // Split word into two bytes
      // Right
      sdBuffer[d + 3] = ((currWord >> 8) & 0xFF);
      // Left
      sdBuffer[d + 2] = (currWord & 0xFF);
      d += 4;
    }
    myFile.write(sdBuffer, 512);
    d = 0;
    // Update progress bar
    processedProgressBar += 512;
    draw_progressbar(processedProgressBar, totalProgressBar);
    // Blink led
    if (currByte % 12800 == 0)
      blinkLED();
  }

  // Close the file:
  myFile.close();
  println_Msg(F("Finished reading."));
  display_Update();
}

void writeSIMM4x8() {
  if (openFlashFile()) {
    // Set data pins to output
    dataOut16();

    //Initialize progress bar
    uint32_t processedProgressBar = 0;
    uint32_t totalProgressBar = (uint32_t)fileSize;
    draw_progressbar(0, totalProgressBar);

    // Fill sdBuffer
    unsigned long simmAddress = 0;
    for (unsigned long currByte = 0; currByte < fileSize / 4; currByte += 128) {
      myFile.read(sdBuffer, 512);
      // Blink led
      if (currByte % 2048 == 0)
        blinkLED();
      
      noInterrupts();
      for (int c = 0; c < 128; c++) {
        simmAddress = currByte + c;
        // 0600 0EA0
        enable64MSB();
        // 0006
        word myWordMSB = ((sdBuffer[(c * 4) + 1] & 0xFF) << 8) | (sdBuffer[(c * 4)] & 0xFF);
        dataIn16();
        word wordFlashMSB = readWord_Flash(simmAddress);
        dataOut16();
        if (wordFlashMSB != myWordMSB && myWordMSB != 0xFFFF) {
          // Write command sequence
          writeByteCommand_Flash2x8(0x0, 0xa0);
          // Write current word
          writeWord_Flash(simmAddress, myWordMSB);
          busyCheck2x8(simmAddress, myWordMSB);
        }
        enable64LSB();
        word myWordLSB = ((sdBuffer[(c * 4) + 3] & 0xFF) << 8) | (sdBuffer[(c * 4) + 2] & 0xFF);
        dataIn16();
        word wordFlashLSB = readWord_Flash(simmAddress);
        dataOut16();
        if (wordFlashLSB != myWordLSB && myWordLSB != 0xFFFF) {
          // Write command sequence
          writeByteCommand_Flash2x8(0x0, 0xa0);
          // Write current word
          writeWord_Flash(simmAddress, myWordLSB);
          busyCheck2x8(simmAddress, myWordLSB);
        }
      }
      interrupts();
      // update progress bar
      processedProgressBar += 512;
      draw_progressbar(processedProgressBar, totalProgressBar);
    }

    // Set data pins to input again
    dataIn16();

    // Close the file:
    myFile.close();
  }
}

void verifySIMM2x16() {
  if (openVerifyFlashFile()) {
    blank = 0;
    word d = 0;
    //Initialize progress bar
    uint32_t processedProgressBar = 0;
    uint32_t totalProgressBar = fileSize;
    draw_progressbar(0, totalProgressBar);

    for (unsigned long currByte = 0; currByte < fileSize / 4; currByte += 128) {
      //fill sdBuffer
      myFile.read(sdBuffer, 512);
      for (int c = 0; c < 128; c++) {
        enable64MSB();
        word currWord = ((sdBuffer[d + 1] << 8) | sdBuffer[d]);
        if (readWord_Flash(currByte + c) != currWord) {
          blank++;
        }
        enable64LSB();
        currWord = ((sdBuffer[d + 3] << 8) | sdBuffer[d + 2]);
        if (readWord_Flash(currByte + c) != currWord) {
          blank++;
        }
        d += 4;
      }
      d = 0;
      // Update progress bar
      if (currByte % 128 == 0) {
        processedProgressBar += 512;
        draw_progressbar(processedProgressBar, totalProgressBar);
      }
      // Blink led
      if (currByte % 25600 == 0)
        blinkLED();
    }

    if (blank == 0) {
      println_Msg(F("SIMM verified OK"));
      display_Update();
    } else {
      print_STR(error_STR, 0);
      print_Msg(blank);
      print_STR(_bytes_STR, 1);
      print_Error(did_not_verify_STR);
    }
    // Close the file:
    myFile.close();
  }
}

void printSIMM2x16(int numBytes) {
  /*
    right_byte = short_val & 0xFF;
    left_byte = ( short_val >> 8 ) & 0xFF
    short_val = ( ( left_byte & 0xFF ) << 8 ) | ( right_byte & 0xFF );
  */

  char buf[3];

  for (int currByte = 0; currByte <= ceil(numBytes / 4); currByte += 1) {
    enable64MSB();
    word currWord = readWord_Flash(currByte);
    sdBuffer[(currByte * 4)] = currWord & 0xFF;
    sdBuffer[(currByte * 4) + 1] = (currWord >> 8) & 0xFF;
    enable64LSB();
    currWord = readWord_Flash(currByte);
    sdBuffer[(currByte * 4) + 2] = currWord & 0xFF;
    sdBuffer[(currByte * 4) + 3] = (currWord >> 8) & 0xFF;
  }

  for (int currByte = 0; currByte < numBytes; currByte += 10) {
    for (int c = 0; c < 10; c++) {
      sprintf(buf, "%.2X", sdBuffer[currByte + c]);
      // Now print the significant bits
      print_Msg(buf);
    }
    println_Msg("");
  }
  display_Update();
}

/******************************************
  16bit SIMM functions
*****************************************/
void id_SIMM16() {
  idFlash16();
  resetFlash16();

  // Print start screen
  display_Clear();
  display_Update();
  println_Msg(F("SIMM Writer 16bit"));
  println_Msg("");
  println_Msg("");
  print_Msg(F("Flash ID: "));
  println_Msg(flashid_str);
  if (flashid == 0x017E) {
    idFlash16();
    if (readWord_Flash(0xE) == 0x2228) {
      println_Msg(F("S29GL01 detected"));
      flashSize = 0x8000000;
    }
    else if (readWord_Flash(0xE) == 0x2223) {
      println_Msg(F("S29GL512 detected"));
      flashSize = 0x4000000;
    }
    else if (readWord_Flash(0xE) == 0x2222) {
      println_Msg(F("S29GL256 detected"));
      flashSize = 0x2000000;
    }
    else if (readWord_Flash(0xE) == 0x2221) {
      println_Msg(F("S29GL128 detected"));
      flashSize = 0x1000000;
    }
    // 64KW
    sectorSize = 65536;
    // 256W = sdBuffer[512]
    bufferSize = 256;
    println_Msg(FS(ATTENTION_3_3V));
    flashromType = 2;
  } else if (flashid == 0x897E) {
    idFlash16();
    if (readWord_Flash(0xE) == 0x2228) {
      println_Msg(F("MT28EW01G detected"));
      flashSize = 0x8000000;
      //flashSize = 0x1000000;
    }
    else if (readWord_Flash(0xE) == 0x2223) {
      println_Msg(F("MT28EW512 detected"));
      flashSize = 0x4000000;
    }
    else if (readWord_Flash(0xE) == 0x2222) {
      println_Msg(F("MT28EW256 detected"));
      flashSize = 0x2000000;
    }
    else if (readWord_Flash(0xE) == 0x2221) {
      println_Msg(F("MT28EW128 detected"));
      flashSize = 0x1000000;
    }
    // 64KW
    sectorSize = 65536;
    // 256W = sdBuffer[512]
    bufferSize = 256;
    println_Msg(FS(ATTENTION_3_3V));
    flashromType = 2;
  } else {
    // ID not found
    flashSize = 0x1000000;
    flashromType = 0;
    display_Clear();
    println_Msg(F("SIMM Writer 16bit"));
    println_Msg("");
    print_Msg(F("ID Type 1: "));
    println_Msg(vendorID);
    print_Msg(F("ID Type 2: "));
    println_Msg(flashid_str);
    println_Msg("");
    println_Msg(F("UNKNOWN FLASHROM"));
    println_Msg("");
    // Prints string out of the common strings array either with or without newline
    print_Error(press_button_STR);
    display_Update();
    wait();
    // print first 40 bytes of flash
    display_Clear();
    println_Msg(F("First 40 bytes:"));
    println_Msg(FS(FSTRING_EMPTY));
    printFlash16(40);
    resetFlash16();
  }
  println_Msg(FS(FSTRING_EMPTY));
  // Prints string out of the common strings array either with or without newline
  print_STR(press_button_STR, 1);
  display_Update();
  resetFlash16();
}

void writeSIMM16(unsigned long sectorSize, uint16_t bufferSize) {
  if (openFlashFile()) {
    // Set data pins to output
    dataOut16();

    //Initialize progress bar
    uint32_t processedProgressBar = 0;
    uint32_t totalProgressBar = (uint32_t)fileSize / 2;
    draw_progressbar(processedProgressBar, totalProgressBar);
    for (unsigned long currSector = 0; currSector < fileSize / 2; currSector += sectorSize) {
      // Write to flashrom
      for (unsigned long currSdBuffer = 0; currSdBuffer < sectorSize; currSdBuffer += 256) {
        // Fill SD buffer
        myFile.read(sdBuffer, 512);
        // Write bufferSize words at a time
        for (int currWriteBuffer = 0; currWriteBuffer < 256; currWriteBuffer += bufferSize) {
          noInterrupts();
          // 2 unlock commands
          writeWord_Flash(0x555, 0xaa);
          writeWord_Flash(0x2aa, 0x55);
          // Write buffer load command at sector address
          writeWord_Flash(currSector + currSdBuffer + currWriteBuffer, 0x25);
          // Write word count (minus 1) at sector address
          writeWord_Flash(currSector + currSdBuffer + currWriteBuffer, bufferSize - 1);
          // Load words into buffer
          for (uint16_t currWord = 0; currWord < bufferSize; currWord++) {
            writeWord_Flash(currSector + currSdBuffer + currWriteBuffer + currWord, ((sdBuffer[((currWriteBuffer + currWord) * 2) + 1] & 0xFF) << 8) | (sdBuffer[((currWriteBuffer + currWord) * 2)] & 0xFF));
          }
          // Write Buffer to Flash
          writeWord_Flash(currSector + currSdBuffer + currWriteBuffer + bufferSize - 1, 0x29);
          // Read the status register at last written address
          dataIn16();
          byte statusReg = readByte_Flash(currSector + currSdBuffer + currWriteBuffer + bufferSize - 1);
          byte lastByte = sdBuffer[currWriteBuffer + (bufferSize * 2) - 2];
          while ((statusReg & 0x80) != (lastByte & 0x80)) {
            statusReg = readByte_Flash(currSector + currSdBuffer + currWriteBuffer + bufferSize - 1);
          }
          interrupts();
          // update progress bar
          processedProgressBar += bufferSize;
          draw_progressbar(processedProgressBar, totalProgressBar);
          // Blink led
          if (processedProgressBar % 2048 == 0)
            blinkLED();
          if (processedProgressBar % 0x1000000 == 0)
            setA24A25(currSector + currSdBuffer + currWriteBuffer + bufferSize + 1);

          dataOut16();
        }
      }
    }
    setA24A25(0);
    // Set data pins to input again
    dataIn16();
    // Close the file:
    myFile.close();
  }
}

/******************************************
  2x16bit SIMM functions
*****************************************/

void id_SIMM2x16() {
  enable64MSB();
  idSIMM16();
  flashids[0] = flashid;
  enable64LSB();
  idSIMM16();
  flashids[1] = flashid;
  resetSIMM2x16();

  sprintf(flashid_str, "%04X", flashid);

  // Print start screen
  display_Clear();
  display_Update();
  println_Msg(F("SIMM Writer 2x16bit"));
  println_Msg("");
  println_Msg("");
  print_Msg(F("Flash ID: "));
  println_Msg(flashid_str);
  if (flashid == 0x017E && flashids[0] == flashids[1]) {
    idFlash16();
    if (readWord_Flash(0xE) == 0x2228) {
      println_Msg(F("S29GL01 detected"));
      // flashSize = 0x8000000;
      flashSize = 0x2000000;
    }
    else if (readWord_Flash(0xE) == 0x2223) {
      println_Msg(F("S29GL512 detected"));
      // flashSize = 0x4000000;
      flashSize = 0x2000000;
    }    else if (readWord_Flash(0xE) == 0x2222) {
      println_Msg(F("S29GL256 detected"));
      flashSize = 0x2000000;
    }
    else if (readWord_Flash(0xE) == 0x2221) {
      println_Msg(F("S29GL128 detected"));
      flashSize = 0x1000000;
    }
    flashSize *= 2;
    // 64KW
    sectorSize = 65536;
    // 128DW = sdBuffer[512]
    bufferSize = 128;
    println_Msg(FS(ATTENTION_3_3V));
    flashromType = 2;
  } else if (flashid == 0x897E && flashids[0] == flashids[1]) {
    idFlash16();
    if (readWord_Flash(0xE) == 0x2228) {
      println_Msg(F("MT28EW01G detected"));
      // flashSize = 0x8000000;
      flashSize = 0x2000000;
    }
    else if (readWord_Flash(0xE) == 0x2223) {
      println_Msg(F("MT28EW512 detected"));
      // flashSize = 0x4000000;
      flashSize = 0x2000000;
    }
    else if (readWord_Flash(0xE) == 0x2222) {
      println_Msg(F("MT28EW256 detected"));
      flashSize = 0x2000000;
    }
    else if (readWord_Flash(0xE) == 0x2221) {
      println_Msg(F("MT28EW128 detected"));
      flashSize = 0x1000000;
    }
    flashSize *= 2;
    // 64KW
    sectorSize = 65536;
    // 128DW = sdBuffer[512]
    bufferSize = 128;
    println_Msg(FS(ATTENTION_3_3V));
    flashromType = 2;
  } else {
    // ID not found
    flashSize = 0x4000000;
    flashromType = 0;
    display_Clear();
    println_Msg(F("SIMM Writer 2x16bit"));
    println_Msg("");
    print_Msg(F("ID Type 1: "));
    println_Msg(vendorID);
    print_Msg(F("ID Type 2: "));
    println_Msg(flashid_str);
    println_Msg("");
    println_Msg(F("UNKNOWN FLASHROM"));
    println_Msg("");
    // Prints string out of the common strings array either with or without newline
    print_Error(press_button_STR);
    display_Update();
    wait();
    // print first 40 bytes of flash
    display_Clear();
    println_Msg(F("First 40 bytes:"));
    println_Msg(FS(FSTRING_EMPTY));
    printFlash16(40);
    resetSIMM4x8();
  }
  println_Msg(FS(FSTRING_EMPTY));
  // Prints string out of the common strings array either with or without newline
  print_STR(press_button_STR, 1);
  display_Update();
  resetSIMM4x8();
}

void writeSIMM2x16(unsigned long sectorSize, uint16_t bufferSize) {
  if (openFlashFile()) {
    // Set data pins to output
    dataOut16();

    //Initialize progress bar
    uint32_t processedProgressBar = 0;
    uint32_t totalProgressBar = (uint32_t)fileSize / 4;
    draw_progressbar(processedProgressBar, totalProgressBar);

    for (unsigned long currSector = 0; currSector < fileSize / 4; currSector += sectorSize) {
      // Blink led
      blinkLED();
      // Write to flashrom
      for (unsigned long currSdBuffer = 0; currSdBuffer < sectorSize; currSdBuffer += 128) {
        // Fill SD buffer
        myFile.read(sdBuffer, 512);
        // Write bufferSize words at a time
        for (int currWriteBuffer = 0; currWriteBuffer < 128; currWriteBuffer += bufferSize) {
          noInterrupts();
          // 2 unlock commands
          enable64MSB();
          writeWord_Flash(0x555, 0xaa);
          writeWord_Flash(0x2aa, 0x55);
          // Write buffer load command at sector address
          writeWord_Flash(currSector + currSdBuffer + currWriteBuffer, 0x25);
          // Write word count (minus 1) at sector address
          writeWord_Flash(currSector + currSdBuffer + currWriteBuffer, bufferSize - 1);

          // Load words into buffer
          for (uint16_t currWord = 0; currWord < bufferSize; currWord++) {
            word myWord = ((sdBuffer[((currWriteBuffer + currWord) * 4) + 1] & 0xFF) << 8) | (sdBuffer[((currWriteBuffer + currWord) * 4)] & 0xFF);
            writeWord_Flash(currSector + currSdBuffer + currWriteBuffer + currWord, myWord);
          }
          // Write Buffer to Flash
          writeWord_Flash(currSector + currSdBuffer + currWriteBuffer + bufferSize - 1, 0x29);
          // Read the status register at last written address
          dataIn16();
          byte statusReg = readByte_Flash(currSector + currSdBuffer + currWriteBuffer + bufferSize - 1);
          byte lastByte = sdBuffer[currWriteBuffer + (bufferSize * 4) - 4];
          while ((statusReg & 0x80) != (lastByte & 0x80)) {
            statusReg = readByte_Flash(currSector + currSdBuffer + currWriteBuffer + bufferSize - 1);
          }
          dataOut16();
          enable64LSB();
          writeWord_Flash(0x555, 0xaa);
          writeWord_Flash(0x2aa, 0x55);
          // Write buffer load command at sector address
          writeWord_Flash(currSector + currSdBuffer + currWriteBuffer, 0x25);
          // Write word count (minus 1) at sector address
          writeWord_Flash(currSector + currSdBuffer + currWriteBuffer, bufferSize - 1);
          // Load words into buffer
          for (uint16_t currWord = 0; currWord < bufferSize; currWord++) {
            word myWord = ((sdBuffer[((currWriteBuffer + currWord) * 4) + 3] & 0xFF) << 8) | (sdBuffer[((currWriteBuffer + currWord) * 4) + 2] & 0xFF);
            writeWord_Flash(currSector + currSdBuffer + currWriteBuffer + currWord, myWord);
          }
          // Write Buffer to Flash
          writeWord_Flash(currSector + currSdBuffer + currWriteBuffer + bufferSize - 1, 0x29);
          // Read the status register at last written address
          dataIn16();
          statusReg = readByte_SIMM(currSector + currSdBuffer + currWriteBuffer + bufferSize - 1);
          lastByte = sdBuffer[currWriteBuffer + (bufferSize * 4) - 2];
          while ((statusReg & 0x80) != (lastByte & 0x80)) {
            statusReg = readByte_SIMM(currSector + currSdBuffer + currWriteBuffer + bufferSize - 1);
          }
          interrupts();
          // update progress bar
          processedProgressBar += bufferSize;
          draw_progressbar(processedProgressBar, totalProgressBar);
          // Blink led
          if (processedProgressBar % 2048 == 0)
            blinkLED();
          dataOut16();
        }
      }
    }
    // Set data pins to input again
    dataIn16();
    // Close the file:
    myFile.close();
  }
}

/******************************************
  SIMM 2x8bit flashrom functions
*****************************************/

void resetFlash2x8(uint32_t bank) {
  // Set data pins to output
  dataOut16();

  // Reset command sequence
  writeByteCommand_Flash2x8(bank, 0xf0);

  // Set data pins to input again
  dataIn16();

  delay(500);
}

void idFlash2x8(uint32_t bank) {
  // Set data pins to output
  dataOut16();

  // ID command sequence
  writeByteCommand_Flash2x8(bank, 0x90);

  // Set data pins to input again
  dataIn16();

  // Read the two id bytes into a string
  flashids[(bank * 2)] = (readWord_Flash((bank << 21) | 0) >> 8) << 8;
  flashids[(bank * 2)] |= readWord_Flash((bank << 21) | 1) >> 8

  // Read the two id bytes into a string
  flashids[(bank * 2) + 1] = (readWord_Flash((bank << 21) | 0) & 0xFF) << 8;
  flashids[(bank * 2) + 1] |= readWord_Flash((bank << 21) | 1) & 0xFF;
}

// From eraseFlash29F032
void eraseFlash2x8(uint32_t bank) {
  // Set data pins to output
  dataOut16();

  // Erase command sequence
  writeByteCommand_Flash2x8(bank, 0x80);
  writeByteCommand_Flash2x8(bank, 0x10);

  // Set data pins to input again
  dataIn16();

  // Read the status register
  word statusReg = readWord_Flash((bank << 21) | 0);

  // After a completed erase D7 and D15 will output 1
  while ((statusReg & 0x8080) != 0x8080) {
    // Blink led
    blinkLED();
    delay(100);
    // Update Status
    statusReg = readWord_Flash((bank << 21) | 0);
  }
}

// From busyCheck29F032
int busyCheck2x8(uint32_t addr, word c) {
  int ret = 0;
  // Set data pins to input
  dataIn16();

  // Setting OE(PH1) LOW
  PORTH &= ~(1 << 1);
  // Setting WE(PH4) HIGH
  PORTH |= (1 << 4);
  NOP;
  NOP;

  //When the Embedded Program algorithm is complete, the device outputs the datum programmed to D7 and D15
  for (;;) {
    word d = readWord_Flash(addr);
    if ((d & 0x8080) == (c & 0x8080)) {
      break;
    }
    if ((d & 0x2020) == 0x2020) {
      // From the datasheet:
      // DQ 5 will indicate if the program or erase time has exceeded the specified limits (internal pulse count).
      // Under these conditions DQ 5 will produce a “1”.
      // This is a failure condition which indicates that the program or erase cycle was not successfully completed.
      // Note : DQ 7 is rechecked even if DQ 5 = “1” because DQ 7 may change simultaneously with DQ 5 .
      d = readWord_Flash(addr);
      if ((d & 0x8080) == (c & 0x8080)) {
        break;
      } else {
        ret = 1;
        break;
      }
    }
  }

  // Set data pins to output
  dataOut16();

  // Setting OE(PH1) HIGH
  PORTH |= (1 << 1);
  NOP;
  NOP;
  return ret;
}

/******************************************
  SIMM 16bit flashrom functions
*****************************************/
void idSIMM16() {
  // Set data pins to output
  dataOut16();

  // ID command sequence
  writeWordCommand_Flash(0x90);

  // Set data pins to input again
  dataIn16();

  // Read the two id bytes into a string
  flashid = (readWord_Flash(0) & 0xFF) << 8;
  flashid |= readWord_Flash(1) & 0xFF;
  sprintf(flashid_str, "%04X", flashid);
}

void resetSIMM16() {
  switch (flashromType) {
    case 1:
      if (flashid == 0x04AD)
        resetSIMM2x8();
      break;
    case 2:
      if (flashid == 0x017E || flashid == 0x897E)
        resetFlash16();
      break;
    case 3:
      break;
  }
}

/******************************************
  SIMM 2x16bit flashrom functions
*****************************************/
void resetSIMM2x16() {
  enable64MLSB();
  switch (flashromType) {
    case 1:
      if (flashid == 0x04AD)
         resetFlash2x8(0x0);
      break;
    case 2:
      if (flashid == 0x017E || flashid == 0x897E)
        resetFlash16();
      break;
    case 3:
      break;
  }
}

void eraseSIMM2x16() {
  enable64MLSB();
  eraseFlash16();
}

#endif