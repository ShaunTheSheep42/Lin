module;

#include <cstdint>
#include <deque>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include <glaze/core/context.hpp>

export module Basic.History;

import Basic.Block;
import Basic.Canvas;

export namespace Lin {

struct Change {
  enum class Operation { State, Batch };

  Operation operation = Operation::State;
  uint64_t blockId = 0;
  std::unique_ptr<Block> before;
  std::unique_ptr<Block> after;
  std::vector<Change> changes;

  static Change Added(const Block &block);
  static Change Deleted(const Block &block);
  static Change Updated(const Block &before, const Block &after);
  static Change Batch(std::vector<Change> changes);
};

class History {
public:
  void pushChange(Change change);
  bool undo(Canvas &canvas);
  bool redo(Canvas &canvas);
  bool canUndo() const;
  bool canRedo() const;
  void clear();

private:
  static void apply(Change &change, Canvas &canvas, bool forward);

  friend glz::error_ctx EncodeHistory(const History &, std::string &);
  friend glz::error_ctx DecodeHistory(std::string_view, History &);

  std::deque<Change> history;
  std::size_t index = 0;
  static constexpr std::size_t HistoryMaxSize = 1000;
};

glz::error_ctx EncodeHistory(const History &history, std::string &json);
glz::error_ctx DecodeHistory(std::string_view json, History &history);

} // namespace Lin
