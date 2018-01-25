/*
 * Copyright (C) 2014-2018 Christopho, Solarus - http://www.solarus-games.org
 *
 * Solarus Quest Editor is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Solarus Quest Editor is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#include "widgets/pattern_chooser.h"
#include "tileset_model.h"
#include <QInputDialog>

namespace SolarusEditor {

/**
 * @brief Creates a pattern picker.
 * @param parent The parent widget.
 */
PatternChooser::PatternChooser(QWidget *parent) :
  QPushButton(parent),
  tileset() {

  setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  setIconSize(QSize(32, 32));
  update_icon();

  connect(this, SIGNAL(clicked()), this, SLOT(pick_pattern_requested()));
}

/**
 * @brief Sets the tileset where patterns should come from in this chooser.
 * @param tileset The tileset or nullptr.
 */
void PatternChooser::set_tileset(const TilesetModel* tileset) {

  this->tileset = tileset;
}

/**
 * @brief Returns the current pattern of the picker.
 * @return Id of the current pattern.
 */
QString PatternChooser::get_pattern_id() const {

  return text();
}

/**
 * @brief Sets the pattern of the picker.
 *
 * Emits pattern_id_changed() if there is a change.
 *
 * @param pattern_id The pattern id to set.
 */
void PatternChooser::set_pattern_id(const QString& pattern_id) {

  if (pattern_id == get_pattern_id()) {
    return;
  }

  setText(pattern_id);
  update_icon();

  emit pattern_id_changed(pattern_id);
}

/**
 * @brief Makes the button icon show the current pattern.
 */
void PatternChooser::update_icon() {

  static QIcon default_icon = QIcon(":/images/entity_tile.png");

  if (tileset == nullptr) {
    // No tileset: use the generic tile icon.
    setIcon(default_icon);
    return;
  }

  const int pattern_index = tileset->id_to_index(get_pattern_id());
  if (pattern_index == -1) {
    setIcon(default_icon);
    return;
  }

  setIcon(tileset->get_pattern_icon(pattern_index));
}

/**
 * @brief Slot called when the user want to pick another pattern.
 */
void PatternChooser::pick_pattern_requested() {

  // TODO open a dialog with a tileset view instead
  bool ok = false;
  QString pattern_id = QInputDialog::getText(
        this,
        tr("Pattern"),
        tr("Pattern id:"),
        QLineEdit::Normal,
        "",
        &ok);

  if (ok && !pattern_id.isEmpty()) {
    set_pattern_id(pattern_id);
  }
}

}