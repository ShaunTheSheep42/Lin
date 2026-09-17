#include "Edit/File.h"
#include "Basic/Canvas.h"
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <fcntl.h>
#include <unistd.h>

namespace Lin {

Canvas &FileManager::getCurrentCanvas() {
  if (currentId == -1)
    qFatal() << "Not Current Canvas can used, access error";

  return canvass.at(currentId);
}

Canvas &FileManager::getCanvas(int id) {
  if (auto it = canvass.find(id); it == canvass.end())
    qFatal() << "Canvas does not exist";

  return canvass.at(id);
}

int FileManager::pathToId(QString path) {
  auto order = [&](const auto &p) { return p.second.getPath() == path; };

  auto it = std::find_if(canvass.begin(), canvass.end(), order);
  if (it == canvass.end())
    qFatal() << "Can't find canvas from path" << path;

  return it->first; // Id
}

int FileManager::getPrevId(int id) {
  auto it = std::find(openingOrder.begin(), openingOrder.end(), id);
  if (it == openingOrder.end())
    qFatal() << "Can't find canvas id: " << id;

  if (it == openingOrder.begin())
    return openingOrder.back();

  return (--it) == openingOrder.begin() ? openingOrder.back() : *(it);
}

int FileManager::getNextId(int id) {
  auto it = std::find(openingOrder.begin(), openingOrder.end(), id);
  if (it == openingOrder.end())
    qFatal() << "Can't find canvas id: " << id;

  return (++it) == openingOrder.end() ? openingOrder.front() : *(it);
}

int FileManager::openFile(QString path, bool isCurrent) {
  canvass.emplace(++fileCount, loadFromFile(path));

  openingOrder.push_back(fileCount);

  if (isCurrent) {
    // Load and open as the current file
    currentId = fileCount;
  }

  return fileCount;
}

void FileManager::closeFile(int id) {
  saveToFile(canvass.at(id));
  canvass.erase(id);

  auto it = std::find(openingOrder.begin(), openingOrder.end(), id);
  if (it == openingOrder.end())
    qFatal() << "Can't find canvas id: " << id;

  if (id != currentId) {
    openingOrder.erase(it);
    return;
  }

  if (++it != openingOrder.end()) {
    currentId = *it;
    --it;
    openingOrder.erase(it);
    return;
  }

  if (--it == openingOrder.begin()) {
    currentId = -1;
    openingOrder.erase(it);
    return;
  }

  openingOrder.erase(it);
  currentId = openingOrder.back();
}

void FileManager::closeAllFile() {
  while (!openingOrder.empty())
    closeFile(openingOrder.front());
}

void FileManager::emergencySave() {
  saveToFile(tmpCanvas);

  for (auto &[_, c] : canvass)
    saveToFile(c);
}

void FileManager::saveToTmpFile(Canvas &c) {
  QString tmpPath = c.getPath() + ".tmp.lin";

  QFile::remove(tmpPath);

  QFile tmp(tmpPath);
  if (!tmp.open(QIODevice::WriteOnly | QIODevice::Text))
    qFatal() << "Can't open tmp file" << tmpPath;

  QString jsonStr = Encode(c);

  QTextStream out(&tmp);
  out << jsonStr;
  out.flush();
  if (::fsync(tmp.handle()) != 0)
    qFatal() << "Failed to flush tmp file to disk" << tmpPath;
  tmp.close();
}

void FileManager::saveToFile(Canvas &c) {
  saveToTmpFile(c);

  QString tmpPath = c.getPath() + ".tmp.lin";
  QString finalPath = c.getPath();

  const QByteArray tmpName = QFile::encodeName(tmpPath);
  const QByteArray finalName = QFile::encodeName(finalPath);
  if (::rename(tmpName.constData(), finalName.constData()) != 0)
    qFatal() << "Failed to rename tmp file to" << finalPath;

  const QByteArray directory = QFile::encodeName(
      QFileInfo(finalPath).absolutePath());
  const int directoryFd = ::open(directory.constData(), O_RDONLY | O_DIRECTORY);
  if (directoryFd >= 0) {
    if (::fsync(directoryFd) != 0)
      qFatal() << "Failed to flush directory to disk" << directory;
    ::close(directoryFd);
  }
}

Canvas FileManager::loadFromFile(QString path) {
  QFile file(path);
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    qFatal() << "Can't Open file" << path;

  QTextStream in(&file);
  QString jsonStr = in.readAll();
  file.close();

  if (jsonStr.isEmpty())
    return Canvas(path);

  auto res = Decode(jsonStr);
  if (!res.has_value())
    qFatal() << "Decode failed file " << path;

  Canvas canvas = std::move(res.value());
  canvas.setPath(path);
  return canvas;
}

} // namespace Lin
