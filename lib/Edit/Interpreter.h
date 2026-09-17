#pragma once

#include "Edit/Key.h"
// #include "Edit/Recognize.h"
#include "Edit/Region.h"

namespace Lin::Edit {

class Core;

class Interpreter {
public:
  Interpreter(Core &core);
  void translate(QKeyEvent *eK, QInputMethodEvent *e);

private:
  void done();
  void normal(Key k);
  void insert(QString ss);
  void visual(Key k);
  void operatorMotion(Key k);

  void parseToCmd(Key k);
  void parseToMotion(Qt::Key k);

  Core &core;
  Region r;
  int count;
  std::vector<Key> inputList;

  std::function<void(Key)> next;
  enum class Operator { None, Delete, Yank };
  Operator pendingOperator = Operator::None;
  QPoint operatorStart;

  // Recognizer &recognizer; // Default SimpleRecognizer
};

} // namespace Lin::Edit
