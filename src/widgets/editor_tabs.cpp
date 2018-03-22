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
#include "widgets/closable_tab_bar.h"
#include "widgets/editor_tabs.h"
#include "widgets/gui_tools.h"
#include "widgets/map_editor.h"
#include "widgets/text_editor.h"
#include "widgets/tileset_editor.h"
#include "widgets/sprite_editor.h"
#include "widgets/quest_properties_editor.h"
#include "widgets/strings_editor.h"
#include "widgets/dialogs_editor.h"
#include "editor_exception.h"
#include "editor_settings.h"
#include "quest.h"
#include <QFileInfo>
#include <QKeyEvent>
#include <QSet>
#include <QUndoGroup>
#include <QUndoStack>

namespace SolarusEditor {

/**
 * @brief Creates an editor tab widget.
 * @param parent The parent object or nullptr.
 */
EditorTabs::EditorTabs(QWidget* parent):
  QTabWidget(parent),
  undo_group(new QUndoGroup(this)) {

  ClosableTabBar* tab_bar = new ClosableTabBar();
  setTabBar(tab_bar);

  setMovable(true);
  setFocusPolicy(Qt::StrongFocus);

  connect(tab_bar, SIGNAL(tabCloseRequested(int)),
          this, SLOT(close_file_requested(int)));
  connect(tab_bar, SIGNAL(currentChanged(int)),
          this, SLOT(current_editor_changed(int)));
  connect(tab_bar, SIGNAL(tabMoved(int, int)),
          this, SLOT(update_recent_files_list()));
}

/**
 * @brief Returns the undo/redo group of all open files.
 * @return The undo/redo group of all open files.
 */
QUndoGroup& EditorTabs::get_undo_group() {
  return *undo_group;
}

/**
 * @brief Shows a tab to edit the specified resource element.
 * @param quest A Solarus quest.
 * @param resource_type A type of resource.
 * @param id Id of the resource element to open in this quest.
 */
void EditorTabs::open_resource(
    Quest& quest, ResourceType resource_type, const QString& id) {

  switch (resource_type) {

  case ResourceType::MAP:
    // Open the map data file.
    open_map_editor(quest, quest.get_map_data_file_path(id));
    break;

  case ResourceType::TILESET:
    // Open the tileset data file.
    open_tileset_editor(quest, quest.get_tileset_data_file_path(id));
    break;

  case ResourceType::LANGUAGE:
    // Open the dialogs file.
    open_dialogs_editor(quest, id);
    break;

  case ResourceType::SPRITE:
    // Open the sprite file.
    open_sprite_editor(quest, quest.get_sprite_path(id));
    break;

  case ResourceType::ITEM:
    // Open the item script.
    open_text_editor(quest, quest.get_item_script_path(id));
    break;

  case ResourceType::ENEMY:
    // Open the enemy script.
    open_text_editor(quest, quest.get_enemy_script_path(id));
    break;

  case ResourceType::ENTITY:
    // Open the custom entity model script.
    open_text_editor(quest, quest.get_entity_script_path(id));
    break;

  case ResourceType::SHADER:
    // Open the shader file.
    // TODO shader editor
    open_text_editor(quest, quest.get_shader_path(id));
    break;

  case ResourceType::MUSIC:
  case ResourceType::SOUND:
  case ResourceType::FONT:
    // These resource types cannot be edited.
    break;
  }
}

/**
 * @brief Shows a tab to edit the quest properties.
 * @param quest A Solarus quest.
 */
void EditorTabs::open_quest_properties_editor(Quest& quest) {

  // Find the existing tab if any.
  int index = find_editor(quest.get_properties_path());
  if (index != -1) {
    // Already open.
    setCurrentIndex(index);
    return;
  }

  try {
    add_editor(std::unique_ptr<Editor>(new QuestPropertiesEditor(quest)));
  }
  catch (const EditorException& ex) {
    ex.show_dialog();
  }
}

/**
 * @brief Opens a file with a text editor in a new tab.
 *
 * The file may be a Lua script.
 *
 * @param quest A Solarus quest.
 * @param path Path of the Lua file to open.
 */
void EditorTabs::open_text_editor(
    Quest& quest, const QString& path) {

  if (!quest.is_in_root_path(path)) {
    // Not a file of this quest.
    return;
  }

  // Find the existing tab if any.
  int index = find_editor(path);
  if (index != -1) {
    // Already open.
    setCurrentIndex(index);
    return;
  }

  try {
    add_editor(std::unique_ptr<Editor>(new TextEditor(quest, path)));
  }
  catch (const EditorException& ex) {
    ex.show_dialog();
  }
}

/**
 * @brief Opens a file with a map editor in a new tab.
 * @param quest A Solarus quest.
 * @param path Path of the map data file to open.
 */
void EditorTabs::open_map_editor(
    Quest& quest, const QString& path) {

  if (!quest.is_in_root_path(path)) {
    // Not a file of this quest.
    return;
  }

  // Find the existing tab if any.
  int index = find_editor(path);
  if (index != -1) {
    // Already open.
    setCurrentIndex(index);
    return;
  }

  try {
    add_editor(std::unique_ptr<Editor>(new MapEditor(quest, path)));
  }
  catch (const EditorException& ex) {
    ex.show_dialog();
  }
}

/**
 * @brief Opens a file with a tileset editor in a new tab.
 * @param quest A Solarus quest.
 * @param path Path of the tileset data file to open.
 */
void EditorTabs::open_tileset_editor(
    Quest& quest, const QString& path) {

  if (!quest.is_in_root_path(path)) {
    // Not a file of this quest.
    return;
  }

  // Find the existing tab if any.
  int index = find_editor(path);
  if (index != -1) {
    // Already open.
    setCurrentIndex(index);
    return;
  }

  try {
    add_editor(std::unique_ptr<Editor>(new TilesetEditor(quest, path)));
  }
  catch (const EditorException& ex) {
    ex.show_dialog();
  }
}

/**
 * @brief Opens a file with a sprite editor in a new tab.
 * @param quest A Solarus quest.
 * @param path Path of the sprite data file to open.
 */
void EditorTabs::open_sprite_editor(
    Quest& quest, const QString& path) {

  if (!quest.is_in_root_path(path)) {
    // Not a file of this quest.
    return;
  }

  // Find the existing tab if any.
  int index = find_editor(path);
  if (index != -1) {
    // Already open.
    setCurrentIndex(index);
    return;
  }

  try {
    add_editor(std::unique_ptr<Editor>(new SpriteEditor(quest, path)));
  }
  catch (const EditorException& ex) {
    ex.show_dialog();
  }
}

/**
 * @brief Opens a file with a language dialogs editor in a new tab.
 * @param quest A Solarus quest.
 * @param path Path of the dialogs file to open.
 */
void EditorTabs::open_dialogs_editor(Quest& quest, const QString& language_id) {

  // Get the strings file path.
  QString path = quest.get_dialogs_path(language_id);

  if (!quest.is_in_root_path(path)) {
    // Not a file of this quest.
    return;
  }

  // Find the existing tab if any.
  int index = find_editor(path);
  if (index != -1) {
    // Already open.
    setCurrentIndex(index);
    return;
  }

  try {
    add_editor(std::unique_ptr<Editor>(new DialogsEditor(quest, language_id)));
  }
  catch (const EditorException& ex) {
    ex.show_dialog();
  }
}

/**
 * @brief Opens a file with a language strings list editor in a new tab.
 * @param quest A Solarus quest.
 * @param language_id Language id of the strings file to open.
 */
void EditorTabs::open_strings_editor(
    Quest& quest, const QString& language_id) {

  // Get the strings file path.
  QString path = quest.get_strings_path(language_id);

  if (!quest.is_in_root_path(path)) {
    // Not a file of this quest.
    return;
  }

  // Find the existing tab if any.
  int index = find_editor(path);
  if (index != -1) {
    // Already open.
    setCurrentIndex(index);
    return;
  }

  try {
    add_editor(std::unique_ptr<Editor>(new StringsEditor(quest, language_id)));
  }
  catch (const EditorException& ex) {
    ex.show_dialog();
  }
}

/**
 * @brief Creates a new tab and shows it.
 * @param editor The editor to put in the new tab.
 */
void EditorTabs::add_editor(std::unique_ptr<Editor> editor) {
  insert_editor(std::move(editor), count());
  setCurrentIndex(count() - 1);
}

/**
 * @brief Creates a new tab.
 * @param editor The editor to put in the new tab.
 * @param index Index of the tab to add.
 */
void EditorTabs::insert_editor(std::unique_ptr<Editor> editor, int index) {

  QUndoStack* undo_stack = &editor->get_undo_stack();
  undo_group->addStack(undo_stack);

  QString path = editor->get_file_path();
  insertTab(index, editor.get(), editor->get_icon(), editor->get_title());
  setTabToolTip(index, editor->get_file_path());

  // Show an asterisk in tab title when a file is modified.
  connect(undo_stack, SIGNAL(cleanChanged(bool)),
          this, SLOT(current_editor_modification_state_changed(bool)));

  connect(editor.get(), SIGNAL(open_file_requested(Quest&, QString)),
          this, SLOT(open_file_requested(Quest&, QString)));
  connect(editor.get(), SIGNAL(refactoring_requested(Refactoring)),
          this, SIGNAL(refactoring_requested(Refactoring)));

  editors.emplace(path, std::move(editor));
}

/**
 * @brief Closes the editor at the specified index without confirmation.
 * @param index An editor index.
 */
void EditorTabs::remove_editor(int index) {

  Editor* editor = get_editor(index);
  QString path = editor->get_file_path();

  undo_group->removeStack(&editor->get_undo_stack());

  removeTab(index);
  editors.erase(path);
}

/**
 * @brief Returns the editor at the specified index.
 * @param index An editor index.
 * @return The editor at this index in the tab bar or nullptr.
 */
Editor* EditorTabs::get_editor(int index) {

  if (index == -1 || index >= count()) {
    return nullptr;
  }

  return static_cast<Editor*>(widget(index));
}

/**
 * @brief Returns the editor currently open if any
 * @return The current editor.
 * Returns nullptr if there is no tab.
 */
Editor* EditorTabs::get_editor() {

  int index = currentIndex();
  if (index == -1) {
    return nullptr;
  }

  return get_editor(index);
}

/**
 * @brief Returns the index of an editor in the tabs.
 * @param path Path of a file to find the editor of.
 * @return The index of the editor or -1 if the file is not open.
 */
int EditorTabs::find_editor(const QString& path) {

  auto it = editors.find(path);
  if (it == editors.end()) {
    return -1;
  }

  Editor* editor = it->second.get();
  return indexOf(editor);
}

/**
 * @brief If the specified file is open in a tab, sets it as the current tab.
 * @param path Path of a file to show the editor of.
 * @return @c true if the file is open in an editor.
 */
bool EditorTabs::show_editor(const QString& path) {

  auto it = editors.find(path);
  if (it == editors.end()) {
    return -1;
  }

  Editor* editor = it->second.get();
  setCurrentWidget(editor);
  return true;
}

/**
 * @brief Slot called when the user attempts to save a file.
 * @param index Index of the tab to save.
 * @return @c true in case of success.
 */
bool EditorTabs::save_file_requested(int index) {

  Editor* editor = get_editor(index);
  if (editor == nullptr) {
    return false;
  }

  try {
    editor->save();
    editor->get_undo_stack().setClean();
    modification_state_changed(index, true);
  }
  catch (const EditorException& ex) {
    ex.show_dialog();
    return false;
  }

  return true;
}

/**
 * @brief Slot called when the user attempts to save all tabs.
 * @return @c true in case of success.
 */
bool EditorTabs::save_all_files_requested() {

  bool success = true;
  for (int i = 0; i < count(); ++i) {
    success = success && save_file_requested(i);
  }
  return success;
}

/**
 * @brief Slot called when the user attempts to open a file.
 * @param quest The quest that holds this file.
 * @param path Path of the file to open.
 */
void EditorTabs::open_file_requested(Quest& quest, const QString& path) {

  if (path.isEmpty()) {
    return;
  }

  QFileInfo file_info(path);
  QString canonical_path = file_info.canonicalFilePath();
  if (!quest.is_in_root_path(canonical_path)) {
    // Not a file of this quest.
    return;
  }

  ResourceType resource_type;
  QString element_id;
  if (quest.is_resource_element(canonical_path, resource_type, element_id)) {
    // A resource element declared in the quest.
    // Possibly a map data file, an enemy Lua script,
    // a language directory, etc.
    open_resource(quest, resource_type, element_id);
  }
  else if (quest.is_dialogs_file(canonical_path, element_id)) {
    open_dialogs_editor(quest, element_id);
  }
  else if (quest.is_strings_file(canonical_path, element_id)) {
    open_strings_editor(quest, element_id);
  }
  else if (quest.is_script(canonical_path)) {
    // A Lua script that is not a resource element.
    open_text_editor(quest, canonical_path);
  }
  else if (quest.is_properties_path(canonical_path)) {
    // Opening quest.dat directly.
    open_quest_properties_editor(quest);
  }
  else if (quest.is_data_path(canonical_path)) {
    // Opening the quest root also opens quest.dat.
    open_quest_properties_editor(quest);
  }
}

/**
 * @brief Slot called when the user attempts to close a tab.
 * @param index Index of the tab to close.
 */
void EditorTabs::close_file_requested(int index) {

  Editor* editor = get_editor(index);
  if (editor != nullptr && editor->confirm_before_closing()) {
    remove_editor(index);
  }
}

/**
 * @brief Slot called when the user attempts to close all tabs.
 */
void EditorTabs::close_all_files_requested() {

  if (confirm_before_closing()) {
    close_without_confirmation();
  }
}

/**
 * @brief Slot called when the user wants to reload the file of a tab.
 * @param index Index of the tab to reload.
 */
void EditorTabs::reload_file_requested(int index) {

  int active_editor_index = currentIndex();

  Editor* editor = get_editor(index);
  if (editor == nullptr) {
    return;
  }
  Quest& quest = editor->get_quest();
  QString path = editor->get_file_path();

  close_file_requested(index);
  open_file_requested(quest, path);

  editor = get_editor(count() - 1);

  removeTab(count() - 1);
  insertTab(index, editor, editor->get_icon(), editor->get_title());
  setTabToolTip(index, editor->get_file_path());

  setCurrentIndex(active_editor_index);
}

/**
 * @brief Slot called when a quest file of has been renamed.
 *
 * The corresponding tab is closed if any.
 *
 * @param old_path Old path of the file.
 * @param new_path New path after renaming.
 */
void EditorTabs::file_renamed(const QString& old_path, const QString& new_path) {

  Q_UNUSED(new_path);

  if (get_editor() == nullptr) {
    return;
  }

  Quest& quest = get_editor()->get_quest();
  ResourceType resource_type;
  QString language_id;

  QString path = old_path;
  if (quest.is_potential_resource_element(path, resource_type, language_id) &&
      resource_type == ResourceType::LANGUAGE) {

    int index = find_editor(quest.get_strings_path(language_id));
    if (index != -1) {
      remove_editor(index);
    }
    path = quest.get_dialogs_path(language_id);
  }

  int index = find_editor(path);
  if (index != -1) {
    remove_editor(index);
  }
}

/**
 * @brief Slot called when a quest file of has been deleted.
 *
 * The corresponding tab is closed if any.
 *
 * @param path Path of the deleted file.
 */
void EditorTabs::file_deleted(const QString& path) {

  int index = find_editor(path);
  if (index != -1) {
    remove_editor(index);
  }
}

/**
 * @brief Function called when the user wants to close all editors.
 *
 * This function does not close any editor, it only lets the user save them.
 *
 * @return @c false to cancel the closing operation.
 */
bool EditorTabs::confirm_before_closing() {

  for (int i = 0; i < count(); ++i) {

    Editor* editor = get_editor(i);
    if (!editor->confirm_before_closing()) {
      return false;
    }
  }

  return true;
}

/**
 * @brief Returns whether at least one editor has unsaved modifications.
 * @return @c true if there are unsaved tabs.
 */
bool EditorTabs::has_unsaved_files() {

  return has_unsaved_files_other_than(QSet<QString>());
}

/**
 * @brief Returns whether at least one editor has unsaved modifications
 * and is not in the given list.
 * @param ignored_paths File paths to ignore in this check.
 * @return @c true if at least an open file not in the list is unsaved.
 */
bool EditorTabs::has_unsaved_files_other_than(const QSet<QString>& ignored_paths) {

  for (int i = 0; i < count(); ++i) {
    const Editor* editor = get_editor(i);
    if (ignored_paths.contains(editor->get_file_path())) {
      continue;
    }
    if (!editor->get_undo_stack().isClean()) {
      return true;
    }
  }

  return false;
}

/**
 * @brief Returns the paths of unsaved files in open editors.
 * @return The unsaved file paths.
 */
QStringList EditorTabs::get_unsaved_files() {

  QStringList unsaved_paths;
  for (int i = 0; i < count(); ++i) {
    const Editor* editor = get_editor(i);
    if (!editor->get_undo_stack().isClean()) {
      unsaved_paths << editor->get_file_path();
    }
  }

  return unsaved_paths;
}

/**
 * @brief Closes all editors without confirmation.
 */
void EditorTabs::close_without_confirmation() {

  for (int i = count() - 1; i >= 0; --i) {
    remove_editor(i);
  }
}

/**
 * @brief Reloads settings of all editors.
 */
void EditorTabs::reload_settings() {

  for (int i = 0; i < count(); ++i) {
    get_editor(i)->reload_settings();
  }
}

/**
 * @brief Slot called when the current tab changes.
 * @param index Index of the new current tab.
 */
void EditorTabs::current_editor_changed(int /* index */) {

  Editor* editor = get_editor();
  if (editor == nullptr) {
    get_undo_group().setActiveStack(nullptr);
    emit can_cut_changed(false);
    emit can_copy_changed(false);
    emit can_paste_changed(false);
  }
  else {
    get_undo_group().setActiveStack(&editor->get_undo_stack());
    connect(editor, &Editor::can_cut_changed, [this, editor](bool can_cut) {
      if (get_editor() == editor) {
        emit can_cut_changed(can_cut);
      }
    });
    connect(editor, &Editor::can_copy_changed, [this, editor](bool can_copy) {
      if (get_editor() == editor) {
        emit can_copy_changed(can_copy);
      }
    });
    connect(editor, &Editor::can_paste_changed, [this, editor](bool can_paste) {
      if (get_editor() == editor) {
        emit can_paste_changed(can_paste);
      }
    });
    emit can_cut_changed(editor->can_cut());
    emit can_copy_changed(editor->can_copy());
    emit can_paste_changed(editor->can_paste());

    editor->setFocus();
  }

  // Remember the current active tab.
  QString file_path = (editor == nullptr) ? QString() : editor->get_file_path();
  EditorSettings settings;
  settings.set_value(EditorSettings::last_file, file_path);
}

/**
 * @brief Saves the list of open tabs.
 */
void EditorTabs::update_recent_files_list() {

  EditorSettings settings;
  QStringList last_files;
  for (int i = 0; i < count(); ++i) {

    Editor* editor = get_editor(i);
    last_files << editor->get_file_path();
  }

  settings.set_value(EditorSettings::last_files, last_files);
}

/**
 * @brief Slot called when the is-modified state of the current tab has changed.
 * @param clean @c true if the file is now clean, @c false if it is now
 * modified.
 */
void EditorTabs::current_editor_modification_state_changed(bool clean) {

  modification_state_changed(currentIndex(), clean);
}

/**
 * @brief Slot called when the is-modified state of a tab has changed.
 * @param index Index of a tab.
 * @param clean @c true if the file is now clean, @c false if it is now
 * modified.
 */
void EditorTabs::modification_state_changed(int index, bool clean) {

  Editor* editor = get_editor(index);
  if (editor == nullptr) {
    return;
  }

  QString title = editor->get_title();
  if (!clean) {
    title += '*';
  }
  setTabText(index, title);
}

/**
 * @brief Receives a key press event.
 *
 * Reimplemented to switch tabs with Ctrl+Tab and Ctrl+PageUp or
 * Ctrl+Shift+Tab and Ctrl+PageDown.
 *
 * @param event The event to handle.
 */
void EditorTabs::keyPressEvent(QKeyEvent* event) {

  const bool control = event->modifiers() & Qt::ControlModifier;
  const bool shift = event->modifiers() & Qt::ShiftModifier;
  const int key = event->key();

  if (count() > 1) {
    if ((control && key == Qt::Key_PageDown) ||
        (control && key == Qt::Key_Tab)) {
      setCurrentIndex((currentIndex() + 1) % count());
      return;
    }

    if ((control && key == Qt::Key_PageUp) ||
      (control && shift && key == Qt::Key_Tab)) {
      setCurrentIndex((currentIndex() + count() - 1) % count());
      return;
    }
  }

  QTabWidget::keyPressEvent(event);
}

/**
 * @brief Function called when a tab is inserted.
 * @param index Index of the inserted tab.
 */
void EditorTabs::tabInserted(int index) {

  Q_UNUSED(index);
  update_recent_files_list();
}

/**
 * @brief Function called when a tab is removed.
 * @param index Index of the removed tab.
 */
void EditorTabs::tabRemoved(int index) {

  Q_UNUSED(index);
  update_recent_files_list();
}

}
