#pragma once

#include "Basic/Block.h"
#include <QPoint>
#include <QString>
#include <vector>

namespace Lin::Edit {

struct Token {
  QString text;
  std::vector<QPoint> positions;

  QPoint startPos() const { return positions.front(); }
  QPoint endPos() const { return positions.back(); }
};

class Recognizer {
public:
  virtual ~Recognizer() = default;
  virtual std::vector<Token> tokenize(const Block &block) const = 0;
};

class SimpleRecognizer final : public Recognizer {
public:
  std::vector<Token> tokenize(const Block &block) const override;

private:
  bool isSeparator(Char ch) const;
};

} // namespace Lin::Edit
