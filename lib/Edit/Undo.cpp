#include "Basic/Canvas.h"
#include "Edit/Core.h"

namespace Lin::Edit {

void Core::undo() {
  Canvas &c = fileManager.getCurrentCanvas();
  if (c.canUndo())
    c.undo();
}

void Core::redo() {
  Canvas &c = fileManager.getCurrentCanvas();
  if (c.canRedo())
    c.redo();
}

} // namespace Lin::Edit
