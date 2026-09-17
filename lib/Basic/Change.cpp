#include "Basic/Change.h"
#include <QJsonArray>

namespace Lin {

QJsonObject EncodeChange(const Change &ch) {
  QJsonObject obj;
  obj["op"] = ch.op;

  if (ch.oldBlock.has_value()) {
    obj["oldBlock"] = Encode(ch.oldBlock.value());
  }
  if (ch.newBlock.has_value()) {
    obj["newBlock"] = Encode(ch.newBlock.value());
  }

  if (ch.op == Change::Batch) {
    QJsonArray batchArray;
    for (const auto &change : ch.changes) {
      batchArray.append(EncodeChange(change));
    }
    obj["batch"] = batchArray;
  }

  return obj;
}

Change DecodeChange(const QJsonObject &obj) {
  Change ch;
  ch.op = static_cast<Change::Operate>(obj["op"].toInt());

  if (obj.contains("oldBlock")) {
    ch.oldBlock = Decode(obj["oldBlock"].toObject());
  }
  if (obj.contains("newBlock")) {
    ch.newBlock = Decode(obj["newBlock"].toObject());
  }

  if (ch.op == Change::Batch && obj.contains("batch")) {
    QJsonArray batchArray = obj["batch"].toArray();
    for (const auto &val : batchArray) {
      ch.changes.push_back(DecodeChange(val.toObject()));
    }
  }

  return ch;
}

} // namespace Lin
