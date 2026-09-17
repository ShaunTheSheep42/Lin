#include "Edit/Register.h"
#include "Basic/Char.h"
#include <QClipboard>
#include <QGuiApplication>
#include <optional>

namespace Lin::Edit {

RegisterManager::RegisterManager() {
  payload.emplace(':', std::nullopt);
  payload.emplace(';', std::nullopt);
  for (int index = 0; index < 10; index++)
    payload.emplace(index, std::nullopt);
}

std::optional<Register> &RegisterManager::getRegister(int name) {
  return payload.at(name);
}

void RegisterManager::syncFromSystemClipboard() {
  QClipboard *clipboard = QGuiApplication::clipboard();
  Chars cs = StringToChars(clipboard->text());
  payload.insert_or_assign(';', cs);
}

void RegisterManager::syncToSystemClipboard() {
  const auto it = payload.find(';');
  if (it == payload.end() || !it->second.has_value())
    return;

  QString text;
  std::visit(
      [&](const auto &value) {
        using T = std::decay_t<decltype(value)>;
        if constexpr (std::is_same_v<T, Block>) {
          text = value.toString();
        } else if constexpr (std::is_same_v<T, Blocks>) {
          for (size_t i = 0; i < value.size(); ++i) {
            if (i != 0)
              text += QLatin1Char('\n');
            text += value[i].toString();
          }
        } else if constexpr (std::is_same_v<T, Chars>) {
          text = CharsToString(value);
        }
      },
      *it->second);

  if (QClipboard *clipboard = QGuiApplication::clipboard())
    clipboard->setText(text);
}

} // namespace Lin::Edit
