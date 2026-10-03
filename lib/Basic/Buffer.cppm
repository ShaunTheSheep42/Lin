module;

#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <string_view>

#include <glaze/core/context.hpp>

export module Basic.Buffer;

export import Basic.Block;
export import Basic.Canvas;
export import Basic.Geometry;
export import Basic.History;
export import Basic.ViewPort;
export import Support;

namespace Lin {

export class Buffer {
public:
  explicit Buffer(std::string path);

  const std::string &getPath() const { return path; }
  Version getVersion() const { return version; }
  const Canvas &getCanvas() const { return canvas; }
  ViewPort &getViewPort() { return viewPort; }
  const ViewPort &getViewPort() const { return viewPort; }

  void addBlock(std::unique_ptr<Block> block);
  void updateBlock(uint64_t id, std::unique_ptr<Block> block);
  bool editBlock(uint64_t id, const std::function<void(Block &)> &edit);
  void deleteBlock(uint64_t id);
  bool undo();
  bool redo();
  bool canUndo() const;
  bool canRedo() const;
  glz::error_ctx save() const;
  glz::error_ctx load();

private:
  friend glz::error_ctx EncodeBuffer(const Buffer &, std::string &);
  friend glz::error_ctx DecodeBuffer(std::string_view, Buffer &);

  Version version{1, 0, 0};
  Canvas canvas;
  ViewPort viewPort;
  History history;
  std::string path;
};

export glz::error_ctx EncodeBuffer(const Buffer &buffer, std::string &json);
export glz::error_ctx DecodeBuffer(std::string_view json, Buffer &buffer);

} // namespace Lin
