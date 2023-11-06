/*
Cardputer Simple Launcher
@shikarunochi
*/
#include "M5Cardputer.h"
#include "M5GFX.h"
#include <SD.h>

#include <M5StackUpdater.h>
SPIClass SPI2;

#define MAX_FILES 256
File fileRoot;
String fileList[MAX_FILES];
String fileDir = "/";

M5Canvas canvas(&M5Cardputer.Display);

int fileListCount;
int startIndex;
int endIndex;
int dispfileCount = 8;
int selectIndex;
boolean needRedraw;
void setup()
{
  auto cfg = M5.config();
  M5Cardputer.begin(cfg, true);
  SPI2.begin(
      M5.getPin(m5::pin_name_t::sd_spi_sclk),
      M5.getPin(m5::pin_name_t::sd_spi_miso),
      M5.getPin(m5::pin_name_t::sd_spi_mosi),
      M5.getPin(m5::pin_name_t::sd_spi_ss));
  while (false == SD.begin(M5.getPin(m5::pin_name_t::sd_spi_ss), SPI2))
  {
    delay(500);
  }

  M5Cardputer.Display.setColorDepth(8);
  M5Cardputer.Display.setRotation(1);
  
  //ちらつき防止のため Canvasに描画してpushSpriteする
  canvas.setTextSize(2);
  canvas.setColorDepth(8);
  canvas.createSprite(240, 135);
  canvas.fillSprite(BLACK);

  fileRoot = SD.open(fileDir);
  fileListCount = 0;
  while (1)
  {
    File entry = fileRoot.openNextFile();
    if (!entry)
    { // no more files
      break;
    }
    // ファイルのみ取得
    if (!entry.isDirectory())
    {
      String fullFileName = entry.name();
      String fileName = fullFileName.substring(fullFileName.lastIndexOf("/") + 1);
      String ext = fileName.substring(fileName.lastIndexOf(".") + 1);
      ext.toUpperCase();
      if (ext.equals("BIN") == false)
      {
        continue;
      }
      fileList[fileListCount] = fileName;
      fileListCount++;
    }
    entry.close();
  }

  fileRoot.close();
  sortList(fileList, fileListCount);

  startIndex = 0;
  endIndex = startIndex + 10;
  if (endIndex >= fileListCount)
  {
    endIndex = fileListCount - 1;
  }

  needRedraw = true;
  selectIndex = 0;
}

void loop()
{

  if (needRedraw == true)
  {
    canvas.setCursor(0, 0);
    startIndex = selectIndex - 5;
    if (startIndex < 0)
    {
      startIndex = 0;
    }
    endIndex = startIndex + dispfileCount;
    if (endIndex >= fileListCount)
    {
      endIndex = fileListCount - 1;
      // startIndex = endIndex - 12;
      startIndex = endIndex - dispfileCount;
      if (startIndex < 0)
      {
        startIndex = 0;
      }
    }

    canvas.fillScreen(BLACK);

    for (int index = startIndex; index <= endIndex; index++)
    {
      if (index == selectIndex)
      {
        canvas.setTextColor(GREEN);
      }
      else
      {
        canvas.setTextColor(WHITE);
      }
      canvas.println(" " + fileList[index].substring(0, 16));
    }
    canvas.pushSprite(0, 0);
    needRedraw = false;
  }
  M5Cardputer.update();
  if (M5Cardputer.Keyboard.isChange())
  {
    if (M5Cardputer.Keyboard.isPressed())
    {
      Keyboard_Class::KeysState status = M5Cardputer.Keyboard.keysState();
      if (status.enter)
      {
        updateFromFS(SD, "/" + fileList[selectIndex]);
        ESP.restart();
      }
      for (auto i : status.hid_keys)
      {
        switch (i)
        {
        case 0x33: //";" UpArrow
          selectIndex--;
          if (selectIndex < 0)
          {
            selectIndex = fileListCount - 1;
          }
          needRedraw = true;
          break;   //;
        case 0x37: //"." DownArrow
          selectIndex++;
          if (selectIndex >= fileListCount)
          {
            selectIndex = 0;
          }
          needRedraw = true;
          break;
        }
      }
    }
  }
  delay(10);
}

// https://github.com/tobozo/M5Stack-SD-Updater/blob/master/examples/M5Stack-SD-Menu/M5Stack-SD-Menu.ino
void sortList(String fileList[], int fileListCount)
{
  bool swapped;
  String temp;
  String name1, name2;
  do
  {
    swapped = false;
    for (int i = 0; i < fileListCount - 1; i++)
    {
      name1 = fileList[i];
      name1.toUpperCase();
      name2 = fileList[i + 1];
      name2.toUpperCase();
      if (name1.compareTo(name2) > 0)
      {
        temp = fileList[i];
        fileList[i] = fileList[i + 1];
        fileList[i + 1] = temp;
        swapped = true;
      }
    }
  } while (swapped);
}