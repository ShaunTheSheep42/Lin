#include "Basic/Block.h"
#include <QTypeInfo>
#include <optional>
#include <vector>

namespace Lin {

struct Change {
  enum Operate { Update, Add, Delete, Batch } op;
  std::optional<Block> oldBlock; // For update and delete
  std::optional<Block> newBlock; // For add and update
  std::vector<Change> changes;   // For batch operations
};

QJsonObject EncodeChange(const Change &ch);
Change DecodeChange(const QJsonObject &obj);

} // namespace Lin
