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
#ifndef SOLARUSEDITOR_MAP_MODEL_H
#define SOLARUSEDITOR_MAP_MODEL_H

#include "entities/entity_model.h"
#include "sprite_model.h"
#include <array>
#include <memory>

namespace SolarusEditor {

struct AddableEntity;
class Quest;
class TilesetModel;
class ViewSettings;

using AddableEntities = std::deque<AddableEntity>;

/**
 * @brief Model that wraps a map.
 *
 * It makes the link between the editor and the map data of the
 * Solarus library.
 * Signals are sent when something changes in the wrapped map.
 */
class MapModel : public QObject {
  Q_OBJECT

public:

  static constexpr int NO_FLOOR = Solarus::MapData::NO_FLOOR;

  // Creation.
  MapModel(Quest& quest, const QString& map_id, QObject* parent = nullptr);

  const Quest& get_quest() const;
  Quest& get_quest();
  QString get_map_id() const;

  // Map properties.
  QSize get_size() const;
  void set_size(const QSize& size);
  int get_min_layer() const;
  AddableEntities set_min_layer(int min_layer);
  int get_max_layer() const;
  AddableEntities set_max_layer(int max_layer);
  bool is_valid_layer(int layer) const;
  bool has_world() const;
  QString get_world() const;
  void set_world(const QString& world);
  bool has_floor() const;
  int get_floor() const;
  void set_floor(int floor);
  QPoint get_location() const;
  void set_location(const QPoint& location);
  TilesetModel* get_tileset_model() const;
  QString get_tileset_id() const;
  void set_tileset_id(const QString& tileset_id);
  void reload_tileset();
  QString get_music_id() const;
  void set_music_id(const QString& music_id);
  QString get_current_border_set_id();
  void set_current_border_set_id(const QString& current_border_set_id);

  // Entities.
  int get_num_entities() const;
  int get_num_entities(int layer) const;
  int get_num_tiles(int layer) const;
  int get_num_dynamic_entities(int layer) const;
  bool entity_exists(const EntityIndex& index) const;
  EntityType get_entity_type(const EntityIndex& index) const;
  QString get_entity_type_name(const EntityIndex& index) const;
  bool is_common_type(const EntityIndexes& indexes, EntityType& type) const;
  bool are_tiles(const EntityIndexes& indexes) const;
  EntityIndexes find_entities_of_type(EntityType type) const;
  EntityIndex find_default_destination_index() const;
  QString get_entity_name(const EntityIndex& index) const;
  bool set_entity_name(const EntityIndex& index, const QString& name);
  bool entity_name_exists(const QString& name) const;
  EntityIndex find_entity_by_name(const QString& name) const;
  QMap<QString, EntityIndex> get_named_entities() const;
  int get_entity_layer(const EntityIndex& index) const;
  EntityIndex set_entity_layer(const EntityIndex& index_before, int layer_after);
  bool is_common_layer(const EntityIndexes& indexes, int& layer) const;
  EntityIndexes set_entities_layer(const EntityIndexes& indexes_before, const QList<int>& layer_after);
  void undo_set_entities_layer(const EntityIndexes& indexes_after, const EntityIndexes& indexes_before);
  void set_entity_order(const EntityIndex& index_before, int order_after);
  EntityIndex bring_entity_to_front(const EntityIndex& index_before);
  EntityIndex bring_entity_to_back(const EntityIndex& index_before);
  QPoint get_entity_xy(const EntityIndex& index) const;
  void set_entity_xy(const EntityIndex& index, const QPoint& xy);
  void add_entity_xy(const EntityIndex& index, const QPoint& translation);
  QPoint get_entity_top_left(const EntityIndex& index) const;
  void set_entity_top_left(const EntityIndex& index, const QPoint& top_left);
  QPoint get_entity_origin(const EntityIndex& index) const;
  QSize get_entity_size(const EntityIndex& index) const;
  void set_entity_size(const EntityIndex& index, const QSize& size);
  QSize get_entity_closest_base_size_multiple(const EntityIndex& index) const;
  QSize get_entity_closest_base_size_multiple(const EntityIndex& index, const QSize& size) const;
  bool is_entity_size_valid(const EntityIndex& index) const;
  bool is_entity_size_valid(const EntityIndex& index, const QSize& size) const;
  QSize get_entity_valid_size(const EntityIndex& index) const;
  QRect get_entity_bounding_box(const EntityIndex& index) const;
  void set_entity_bounding_box(const EntityIndex& index, const QRect& bounding_box);
  bool has_entity_direction_field(const EntityIndex& index) const;
  bool is_entity_no_direction_allowed(const EntityIndex& index) const;
  QString get_entity_no_direction_text(const EntityIndex& index) const;
  int get_entity_num_directions(const EntityIndex& index) const;
  bool is_common_direction_rules(const EntityIndexes& indexes, int& num_directions, QString& no_direction_text) const;
  int get_entity_direction(const EntityIndex& index) const;
  void set_entity_direction(const EntityIndex& index, int direction);
  bool is_common_direction(const EntityIndexes& indexes, int& direction) const;
  int get_entity_user_property_count(const EntityIndex& index) const;
  QPair<QString, QString> get_entity_user_property(const EntityIndex& index, int property_index) const;
  void set_entity_user_property(const EntityIndex& index, int property_index, const QPair<QString, QString>& property);
  void add_entity_user_property(const EntityIndex& index, const QPair<QString, QString>& property);
  void remove_entity_user_property(const EntityIndex& index, int property_index);
  bool has_entity_field(const EntityIndex& index, const QString& key) const;
  QVariant get_entity_field(const EntityIndex& index, const QString& key) const;
  void set_entity_field(const EntityIndex& index, const QString& key, const QVariant& value);
  void add_entities(AddableEntities&& entities);
  AddableEntities remove_entities(const EntityIndexes& indexes);

  const Solarus::EntityData& get_internal_entity(const EntityIndex& index) const;
  Solarus::EntityData& get_internal_entity(const EntityIndex& index);

  const EntityModel& get_entity(const EntityIndex& index) const;
  EntityModel& get_entity(const EntityIndex& index);

signals:

  void size_changed(const QSize& size);
  void layer_range_changed(int min_layer, int max_layer);
  void world_changed(const QString& world);
  void floor_changed(int floor);
  void location_changed(const QPoint& location);
  void tileset_id_changed(const QString& tileset_id);
  void tileset_reloaded();
  void music_id_changed(const QString& music_id);

  void entities_about_to_be_added(const EntityIndexes& indexes);
  void entities_added(const EntityIndexes& indexes);
  void entities_about_to_be_removed(const EntityIndexes& indexes);
  void entities_removed(const EntityIndexes& indexes);
  void entity_layer_changed(const EntityIndex& index_before, const EntityIndex& index_after);
  void entity_order_changed(const EntityIndex& index_before, int order_after);
  void entity_name_changed(const EntityIndex& index, const QString& name);
  void entity_xy_changed(const EntityIndex& index, const QPoint& xy);
  void entity_size_changed(const EntityIndex& index, const QSize& size);
  void entity_direction_changed(const EntityIndex& index, int direction);
  void entity_user_property_changed(const EntityIndex& index, int property_index, const QPair<QString, QString>& property);
  void entity_user_property_added(const EntityIndex& index, int property_index, const QPair<QString, QString>& property);
  void entity_user_property_removed(const EntityIndex& index, int property_index);
  void entity_field_changed(const EntityIndex& index, const QString& key, const QVariant& value);

public slots:

  void save() const;

private:

  void rebuild_entity_indexes(int layer);

  Quest& quest;                   /**< The quest the tileset belongs to. */
  const QString map_id;           /**< Id of the map. */
  Solarus::MapData map;           /**< Map data wrapped by this model. */
  TilesetModel* tileset_model;    /**< Tileset of this map. nullptr if not set. */
  std::map<int, EntityModels>
      entities;                   /**< All entities by layer. */
  QString current_border_set_id;  /**< Border set currently selected by the user. */

};

/**
 * @brief Wraps an entity ready to be added to a map and its future index.
 */
struct AddableEntity {

public:

  AddableEntity(EntityModelPtr&& entity, const EntityIndex& index) :
    entity(std::move(entity)),
    index(index) {
  }

  // Comparison operators useful to sort lists.
  bool operator<=(const AddableEntity& other) const { return index <= other.index; }
  bool operator<(const AddableEntity& other) const  { return index < other.index; }
  bool operator>=(const AddableEntity& other) const { return index >= other.index; }
  bool operator>(const AddableEntity& other) const  { return index > other.index; }

  EntityModelPtr entity;
  EntityIndex index;

};

}

#endif
