#pragma once

#include "Basic/Canvas.h"
#include <QString>
#include <list>

namespace Lin {

class FileManager {
public:
  FileManager(QString tmpDir)
      : fileCount(0), tmpDir(tmpDir), tmpCanvas(tmpDir + "TmpCanvas.lin"),
        currentId(-1) {}

  int openFile(QString path, bool isCurrent = false);
  void closeFile(int id);
  void closeAllFile();

  void saveToFile(Canvas &canvas);
  void saveToTmpFile(Canvas &canvas);
  void emergencySave();

  int pathToId(QString path);
  int getCurrentId() { return currentId; }
  int getPrevId(int id);
  int getNextId(int id);

  Canvas &getCanvas(int id);
  Canvas &getCurrentCanvas();
  Canvas &getTmpCanvas() { return tmpCanvas; }

private:
  Canvas loadFromFile(QString path);

  int currentId; // Current canvas id, default -1 means no canvas now

  std::list<int> openingOrder; // Buffer opening order

  int fileCount; // Incrementing counter, used to set the id

  std::unordered_map<int, Canvas> canvass; // Multiple buffers, query by ID

  // Temporary  layer for insert & move
  // Overlay rendering on top layer
  Canvas tmpCanvas;

  QString tmpDir;
};

} // namespace Lin
