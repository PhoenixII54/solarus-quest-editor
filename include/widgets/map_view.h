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
#ifndef SOLARUSEDITOR_MAP_VIEW_H
#define SOLARUSEDITOR_MAP_VIEW_H

#include "entities/entity_traits.h"
#include <QGraphicsView>
#include <QPointer>

class QActionGroup;

namespace SolarusEditor {

class MapModel;
class MapScene;
class ViewSettings;

/**
 * @brief Graphical view of the map image, allowing to manage entities.
 */
class MapView : public QGraphicsView {
  Q_OBJECT

public:

  /**
   * @brief Indicates what the user is currently doing on the map view.
   */
  class State {

  public:

    State(MapView& view);
    virtual ~State() = default;

    const MapView& get_view() const;
    MapView& get_view();
    const MapScene& get_scene() const;
    MapScene& get_scene();
    const MapModel& get_map() const;
    MapModel& get_map();
    QPoint to_map_point(const QMouseEvent& mouse_event) const;

    virtual void start();
    virtual void stop();
    virtual void cancel();

    virtual void mouse_pressed(const QMouseEvent& event);
    virtual void mouse_released(const QMouseEvent& event);
    virtual void mouse_moved(const QMouseEvent& event);
    virtual void context_menu_requested(const QPoint& where);
    virtual void tileset_selection_changed();

  private:

    MapView& view;

  };

  MapView(QWidget* parent = nullptr);

  MapModel* get_map();
  MapScene* get_scene();
  void set_map(MapModel* map);
  const ViewSettings* get_view_settings() const;
  void set_view_settings(ViewSettings& view_settings);
  void export_to_image(const QString& file_name);
  const QMap<QString, QAction*>* get_common_actions() const;
  void set_common_actions(const QMap<QString, QAction*>* common_actions);

  // Selection.
  bool is_selection_empty() const;
  int get_num_selected_entities() const;
  EntityIndexes get_selected_entities() const;
  void set_selected_entities(const EntityIndexes& indexes);
  void set_only_selected_entity(const EntityIndex& index);
  void select_entity(const EntityIndex& index, bool selected);
  EntityModels clone_selected_entities() const;

  // Information about entities.
  EntityIndex get_entity_index_under_cursor() const;

  // State of the view.
  void start_state_doing_nothing();
  void start_state_drawing_rectangle(const QPoint& initial_point);
  void start_state_moving_entities(const QPoint& initial_point);
  void start_state_resizing_entities();
  void start_state_adding_entities(EntityModels&& entities, bool use_layer_under_mouse);
  void start_adding_entities_from_tileset_selection();

  bool are_entities_resizable(const EntityIndexes& indexes) const;

  // Actions.
  QMenu* create_context_menu();

signals:

  void mouse_map_coordinates_changed(const QPoint& xy);
  void mouse_left();
  void stopped_state();

  void undo_requested();
  void edit_entity_requested(const EntityIndex& index,
                             EntityModelPtr& values);
  void move_entities_requested(
      const EntityIndexes& indexes,
      const QPoint& translation,
      bool allow_merge_to_previous);
  void resize_entities_requested(
      const QMap<EntityIndex, QRect>& indexes,
      bool allow_merge_to_previous);
  void convert_tiles_requested(
      const EntityIndexes& indexes);
  void change_tiles_pattern_requested(
      const EntityIndexes& indexes);
  void set_entities_direction_requested(
      const EntityIndexes& indexes,
      int direction);
  void set_entities_layer_requested(
      const EntityIndexes& indexes,
      int layer);
  void increase_entities_layer_requested(
      const EntityIndexes& indexes);
  void decrease_entities_layer_requested(
      const EntityIndexes& indexes);
  void bring_entities_to_front_requested(
      const EntityIndexes& indexes);
  void bring_entities_to_back_requested(
      const EntityIndexes& indexes);
  void add_entities_requested(
      AddableEntities& entities,
      bool replace_selection);
  void remove_entities_requested(const EntityIndexes& indexes);

public slots:

  void cut();
  void copy();
  void paste();
  void update_zoom();
  void zoom_in();
  void zoom_out();
  void mouse_coordinates_changed(const QPoint& xy);
  void update_grid_visibility();
  void update_layer_visibility(int layer);
  void update_layer_locking(int layer);
  void update_traversables_visibility();
  void update_obstacles_visibility();
  void update_entity_type_visibility(EntityType type);
  void tileset_selection_changed();
  void tileset_id_changed(const QString& tileset_id);
  void tileset_reloaded();

  void cancel_state_requested();
  void undo_last_command();
  void edit_selected_entity();
  void move_selected_entities(const QPoint& translation, bool allow_merge_to_previous);
  void resize_entities(const QMap<EntityIndex, QRect>& boxes, bool allow_merge_to_previous);
  void convert_selected_tiles();
  void change_pattern_of_similar_tiles();
  void add_border_to_selection();
  void remove_selected_entities();

protected:

  void paintEvent(QPaintEvent* event) override;

  void keyPressEvent(QKeyEvent* event) override;
  void mousePressEvent(QMouseEvent* event) override;
  void mouseReleaseEvent(QMouseEvent* event) override;
  void mouseMoveEvent(QMouseEvent* event) override;
  void mouseDoubleClickEvent(QMouseEvent* event) override;
  void contextMenuEvent(QContextMenuEvent* event) override;

private:

  void build_context_menu_actions();
  void build_context_menu_layer_actions();
  QMenu* create_direction_context_menu(const EntityIndexes& indexes);
  void set_state(std::unique_ptr<State> state);

  QPointer<MapModel> map;          /**< The map model. */
  MapScene* scene;                 /**< The scene viewed. */
  QPointer<ViewSettings>
      view_settings;               /**< What is displayed in the view. */
  double zoom;                     /**< Zoom factor currently applied. */
  std::unique_ptr<State> state;    /**< Current state of the view. */

  // Actions of the context menu.
  const QMap<QString, QAction*>*
      common_actions;              /**< Common actions for the context menu. */
  QAction* edit_action;            /**< Action of editing the selected entity. */
  QAction* resize_action;          /**< Action of resizing the selected entities. */
  QAction* convert_tiles_action;   /**< Action of converting tiles to/from dynamic ones. */
  QAction* change_pattern_action;  /**< Action of changing the pattern of selected tiles. */
  QAction*
      change_pattern_all_action;   /**< Action of changing the pattern of all tiles that
                                    * have the same pattern as the selected ones. */
  QAction* add_border_action;      /**< Action of adding border tiles to the selection. */
  QList<QAction*>
      set_layer_actions;           /**< Actions of changing the layer of the selected entities. */
  QActionGroup*
      set_layer_actions_group;     /**< Groups of exclusive actions to change the layer. */
  QAction* up_one_layer_action;    /**< Action of putting selecting entities one layer up. */
  QAction* down_one_layer_action;  /**< Action of putting selecting entities one layer down. */
  QAction* bring_to_front_action;  /**< Action of bringing selected entities to front. */
  QAction* bring_to_back_action;   /**< Action of bringing selected entities to back. */
  QAction* remove_action;          /**< Action of deleting selected entities. */
  QAction* cancel_action;          /**< Action of stopping the current state. */

};

}

#endif
