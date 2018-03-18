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
#ifndef SOLARUSEDITOR_EDITOR_H
#define SOLARUSEDITOR_EDITOR_H

class QUndoCommand;
class QUndoStack;

#include "view_settings.h"
#include <QIcon>
#include <QWidget>
#include <memory>

namespace SolarusEditor {

class Quest;
class QuestDatabase;
class Refactoring;

/**
 * \brief Abstract class for a widget that can edit something in Solarus.
 *
 * It has to support common operations like saving and closing.
 */
class Editor : public QWidget {
  Q_OBJECT

public:

  Editor(Quest& quest, const QString& path, QWidget* parent = nullptr);

  const Quest& get_quest() const;
  Quest& get_quest();
  const QuestDatabase& get_database() const;
  QuestDatabase& get_database();
  QString get_file_path() const;
  QString get_file_name() const;
  QString get_file_name_without_extension() const;
  QString get_title() const;
  QIcon get_icon() const;
  const QUndoStack& get_undo_stack() const;
  QUndoStack& get_undo_stack();
  const QMap<QString, QAction*>& get_common_actions() const;
  void set_common_actions(const QMap<QString, QAction*>& common_actions);
  bool has_unsaved_changes() const;
  bool confirm_before_closing();

  bool is_select_all_supported() const;
  bool is_find_supported() const;
  bool is_zoom_supported() const;
  bool is_grid_supported() const;
  bool is_layer_supported() const;
  void get_layers_supported(int& min_layer, int& max_layer) const;
  bool is_traversables_visibility_supported() const;
  bool is_obstacles_visibility_supported() const;
  bool is_entity_type_visibility_supported() const;
  bool is_export_to_image_supported() const;
  const ViewSettings& get_view_settings() const;
  ViewSettings& get_view_settings();

  virtual void save() = 0;
  virtual bool can_cut() const;
  virtual void cut();
  virtual bool can_copy() const;
  virtual void copy();
  virtual bool can_paste() const;
  virtual void paste();
  virtual void select_all();
  virtual void unselect_all();
  virtual void find();
  virtual void export_to_image();
  virtual void reload_settings();

signals:

  void can_cut_changed(bool can_cut);
  void can_copy_changed(bool can_copy);
  void can_paste_changed(bool can_paste);
  void open_file_requested(Quest& quest, const QString& path);
  void refactoring_requested(const Refactoring& refactoring);

public slots:

  void undo();

protected:

  void set_title(const QString& title);
  void set_icon(const QIcon& icon);
  void set_select_all_supported(bool select_all_supported);
  void set_find_supported(bool find_supported);
  void set_zoom_supported(bool zoom_supported);
  void set_grid_supported(bool grid_supported);
  void set_layers_supported(int min_layer, int max_layer);
  void set_traversables_visibility_supported(bool supported);
  void set_obstacles_visibility_supported(bool supported);
  void set_entity_type_visibility_supported(bool supported);
  void set_export_to_image_supported(bool export_to_image_supported);

  void focusInEvent(QFocusEvent* event) override;
  virtual void editor_made_visible();

  QString get_close_confirm_message() const;
  void set_close_confirm_message(const QString& message);

  bool try_command(QUndoCommand* command);

private slots:

  void application_state_changed(Qt::ApplicationState state);

private:

  Quest& quest;                             /**< The quest the edited file belongs to. */
  QString file_path;                        /**< Path of the edited file. */
  QString title;                            /**< Title of the file. */
  QIcon icon;                               /**< Icon representing the file. */
  QString close_confirm_message;            /**< Message proposing to save changes when closing. */
  QUndoStack* undo_stack;                   /**< The undo/redo history of editing this file. */
  QMap<QString, QAction*> common_actions;   /**< Actions available to all editors. */
  bool select_all_supported;                /**< Whether the editor supports selecting all. */
  bool find_supported;                      /**< Whether the editor supports finding. */
  bool zoom_supported;                      /**< Whether the editor supports zooming. */
  bool grid_supported;                      /**< Whether the editor supports showing/hiding a grid. */
  int min_layer_supported;                  /**< Lowest layer if the editor supports showing/hiding layers. */
  int max_layer_supported;                  /**< Highest layer if the editor supports showing/hiding layers,
                                             * -1 otherwise. */
  bool traversables_visibility_supported;   /**< Whether the editor supports showing/hiding traversables. */
  bool obstacles_visibility_supported;      /**< Whether the editor supports showing/hiding obstacles. */
  bool entity_type_visibility_supported;    /**< Whether the editor supports showing/hiding entity types. */
  bool export_to_image_supported;           /**< Whether the editor supports exporting to an image. */
  ViewSettings view_settings;               /**< What is shown and how. */

};

}

#endif
