#pragma once

#include "Basic/Direction.h"
#include "Edit/File.h"
#include "Edit/Interpreter.h"
#include "Edit/Mode.h"
#include "Edit/Region.h"
#include "Edit/Register.h"
#include "Lin/Config.h"
#include "Lin/RenderData.h"
#include <QDir>
#include <QKeyEvent>
#include <QObject>
#include <QString>
#include <QThread>
#include <QVector>
#include <optional>

namespace Lin::Edit {

class Core : public QObject {
  Q_OBJECT
public:
  explicit Core(Config &cfg, QObject *parent = nullptr);

  void exit();
  void openFile(QString path, bool isCurrent);
  void saveFile();
  void closeFile(int id);

  const Mode &getMode() const { return mode; }
  QPoint cursorPos() { return fileManager.getCurrentCanvas().getCursorPos(); }
  void setMode(Mode m);

  void executeCommand(QString cmd);

  // Normal
  void copy(Region r);
  void paste(bool newGroup = false);
  void undo();
  void redo();
  // void doDelete(int id);

  void moveCursorByOffSet(int offsetX, int offsetY);
  void moveCursorToPos(QPoint p);
  void moveCursorToNextChar();
  void moveCursorToPrevChar();
  void moveCursorWord(int direction);
  void copyMotion(QPoint start, QPoint end);
  void deleteMotion(QPoint start, QPoint end, bool includeEnd = true);
  void mirrorMove(bool isVertical = false);
  void moveVisualCursor(int offsetX, int offsetY);
  void moveVisualWord(int direction);
  void moveSelectedBlocks(int offsetX, int offsetY);
  // void moveCursorForward();
  // void moveCursorEnd();
  // void moveCursorBackward();

  // Insert
  void enterInsert(EditPosition ed, bool isNewGroup, Direction d = Left2Right);
  void exitInsert();
  void insertChar(Char ch);
  void deleteChar();

  // Visual
  void enterVisualBlock();
  void enterVisualGroup();
  void enterVisualRectangle();
  void exitVisual();
  void visualCopy();
  void visualDelete();

  // Lin
  void enterLin();
  void enterLinBlock();
  void enterLinGroup();
  void linCopy();
  void linDelete();
  void confirmLin();
  void cancelLin();

  Q_SLOT void handleEvent(QKeyEvent *eK, QInputMethodEvent *e);
  Q_SIGNAL void update(RenderData rd);

  std::function<void()> quit;

private:
  void doUpdate();

  Config &cfg;

  Mode mode;

  FileManager fileManager;
  RegisterManager registerManager;
  Interpreter interpreter;

  struct {
    Canvas *c;
  } normalContext;

  struct {
    Canvas *c;
    QPoint anchor;
    std::optional<Block> b = std::nullopt;
    EditPosition ep = Forward;
    quint64 groupId = -1;
    Direction direction = Left2Right;
  } insertContext;

  struct {
    Canvas *c;
    Region r;
    QPoint anchor;
    bool visualRectangle = false;
    std::vector<Block> moveOriginal;
  } visualContext;

  struct {
    Canvas *c = nullptr;
    std::vector<Block> original;
    bool tmpPrepared = false;
  } linContext;
};

} // namespace Lin::Edit
