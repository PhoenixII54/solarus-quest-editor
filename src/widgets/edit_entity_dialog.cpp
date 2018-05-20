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
#include "entities/destination.h"
#include "widgets/edit_entity_dialog.h"
#include "widgets/gui_tools.h"
#include "widgets/new_entity_user_property_dialog.h"
#include "map_model.h"
#include "quest.h"
#include "tileset_model.h"
#include <QInputDialog>

namespace SolarusEditor {

namespace {

// Put field names in constants to avoid repeated identical literals.

const QString behavior_field_name = "behavior";
const QString breed_field_name = "breed";
const QString damage_on_enemies_field_name = "damage_on_enemies";
const QString destination_field_name = "destination";
const QString destination_map_field_name = "destination_map";
const QString destruction_sound_field_name = "destruction_sound";
const QString font_field_name = "font";
const QString ground_field_name = "ground";
const QString maximum_moves_field_name = "maximum_moves";
const QString model_field_name = "model";
const QString opening_method_field_name = "opening_method";
const QString opening_condition_field_name = "opening_condition";
const QString opening_condition_consumed_field_name = "opening_condition_consumed";
const QString pattern_field_name = "pattern";
const QString savegame_variable_field_name = "savegame_variable";
const QString sound_field_name = "sound";
const QString sprite_field_name = "sprite";
const QString starting_location_mode_field_name = "starting_location_mode";
const QString tileset_field_name = "tileset";
const QString transition_field_name = "transition";
const QString treasure_name_field_name = "treasure_name";
const QString treasure_variant_field_name = "treasure_variant";
const QString treasure_savegame_variable_field_name = "treasure_savegame_variable";
const QString weight_field_name = "weight";

}  // Anonymous namespace.

/**
 * @brief Creates an edit entity dialog.
 * @param entity_before The entity to edit. It may or may not already be on the map.
 * @param parent The parent widget or nullptr.
 */
EditEntityDialog::EditEntityDialog(EntityModel& entity_before, QWidget* parent) :
  QDialog(parent),
  entity_before(entity_before),
  resize_mode(entity_before.get_resize_mode()) {

  ui.setupUi(this);

  initialize();
}

/**
 * @brief Returns the quest the map belongs to.
 * @return The quest.
 */
Quest& EditEntityDialog::get_quest() const {
  return get_map().get_quest();
}

/**
 * @brief Returns the map the entity edited belongs to.
 * @return The map.
 */
MapModel& EditEntityDialog::get_map() const {
  return get_entity_before().get_map();
}

/**
 * @brief Returns the index of the entity being edited.
 * @return The entity index on the map before any change.
 */
EntityIndex EditEntityDialog::get_entity_index() const {
  return entity_before.get_index();
}

/**
 * @brief Returns the entity being edited, before any change.
 * @return The entity edited.
 */
EntityModel& EditEntityDialog::get_entity_before() const {
  return entity_before;
}

/**
 * @brief Creates and returns an entity representing the new input data.
 *
 * The created entity is not on the map.
 *
 * @return The new data.
 */
EntityModelPtr EditEntityDialog::get_entity_after() {

  entity_after = EntityModel::clone(get_map(), entity_before.get_index());
  apply();
  return std::move(entity_after);
}

/**
 * @brief Slot called when the user changes the width value.
 * @param width The new width value.
 */
void EditEntityDialog::width_changed(int width) {

  ui.size_field->blockSignals(true);

  if (resize_mode == ResizeMode::SQUARE) {
    ui.size_field->set_second_value(width);
  }
  else if (resize_mode == ResizeMode::SINGLE_DIMENSION) {
    ui.size_field->set_second_value(entity_before.get_base_size().height());
  }

  ui.size_field->blockSignals(false);
}

/**
 * @brief Slot called when the user changes the height value.
 * @param width The new height value.
 */
void EditEntityDialog::height_changed(int height) {

  ui.size_field->blockSignals(true);

  if (resize_mode == ResizeMode::SQUARE) {
    ui.size_field->set_first_value(height);
  }
  else if (resize_mode == ResizeMode::SINGLE_DIMENSION) {
    ui.size_field->set_first_value(entity_before.get_base_size().width());
  }

  ui.size_field->blockSignals(false);
}

/**
 * @brief Slot called when the user changes the direction value.
 */
void EditEntityDialog::direction_changed() {

  entity_after = EntityModel::clone(get_map(), entity_before.get_index());
  apply_direction();

  resize_mode = entity_after->get_resize_mode();
  update_size_constraints();

  if (!entity_after->is_size_valid(ui.size_field->get_size())) {
    ui.size_field->set_size(entity_after->get_valid_size());
  }

  entity_after.reset();
}

/**
 * @brief Slot called when the user wants add a new user property.
 */
void EditEntityDialog::add_user_property_requested() {

  QTreeWidgetItem* selected = ui.user_properties_table->currentItem();
  QString selected_key =
      selected != nullptr ? selected->data(0, 0).toString() : "";
  QString selected_value =
      selected != nullptr ? selected->data(1, 0).toString() : "";

  NewEntityUserPropertyDialog dialog(selected_key, selected_value, this);

  int result = dialog.exec();
  if (result != QDialog::Accepted) {
    return;
  }

  QPair<QString, QString> property = dialog.get_property();

  if (user_property_exists(property.first)) {
    GuiTools::error_dialog(
      tr("The property '%1' already exists").arg(property.first));
    return;
  }

  QTreeWidgetItem* item = new QTreeWidgetItem();
  item->setData(0, Qt::DisplayRole, property.first);
  item->setData(1, Qt::DisplayRole, property.second);
  ui.user_properties_table->addTopLevelItem(item);

  ui.user_properties_table->setCurrentItem(item);
}

/**
 * @brief Slot called when the user wants change a user property key.
 */
void EditEntityDialog::change_user_property_key_requested() {

  QTreeWidgetItem* selected = ui.user_properties_table->currentItem();
  if (selected == nullptr) {
    return;
  }

  QString old_key =selected->data(0, 0).toString();
  bool ok;
  QString new_key = QInputDialog::getText(
        this, tr("Change user property key"),
        tr("Change the key of the property '%1':").arg(old_key),
        QLineEdit::Normal, old_key, &ok);

  if (!ok || new_key == old_key) {
    return;
  }

  if (new_key.isEmpty()) {
    GuiTools::error_dialog(tr("The property key cannot be empty"));
    return;
  }

  if (user_property_exists(new_key)) {
    GuiTools::error_dialog(
      tr("The property '%1' already exists").arg(new_key));
    return;
  }

  if (!entity_before.is_valid_user_property_key(new_key)) {
    GuiTools::error_dialog(
      tr("The key '%1' is invalid").arg(new_key));
    return;
  }

  selected->setData(0, 0, new_key);
}

/**
 * @brief Slot called when the user wants delete a user property.
 */
void EditEntityDialog::delete_user_property_requested() {

  QTreeWidgetItem* item = ui.user_properties_table->currentItem();
  if (item != nullptr) {
    delete item;
    update_user_property_buttons();
  }
}

/**
 * @brief Slot called when the user wants move up a user property.
 */
void EditEntityDialog::move_up_user_property_requested() {

  int index = ui.user_properties_table->currentIndex().row();
  if (index <= 0) {
    return;
  }

  QTreeWidgetItem* item = ui.user_properties_table->takeTopLevelItem(index);
  ui.user_properties_table->insertTopLevelItem(index - 1, item);

  ui.user_properties_table->setCurrentItem(item);
}

/**
 * @brief Slot called when the user wants move down a user property.
 */
void EditEntityDialog::move_down_user_property_requested() {

  int index = ui.user_properties_table->currentIndex().row();
  int count = ui.user_properties_table->topLevelItemCount();
  if (index >= count - 1) {
    return;
  }

  QTreeWidgetItem* item = ui.user_properties_table->takeTopLevelItem(index);
  ui.user_properties_table->insertTopLevelItem(index + 1, item);

  ui.user_properties_table->setCurrentItem(item);
}

/**
 * @brief Slot called when the user double click on the user property table.
 */
void EditEntityDialog::user_property_double_clicked(
    QTreeWidgetItem* item, int column) {

  Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;
  if (column == 1) {
    flags |= Qt::ItemIsEditable;
  }

  item->setFlags(flags);
}

/**
 * @brief Slot called when the selected user property changes.
 */
void EditEntityDialog::update_user_property_buttons() {

  QTreeWidgetItem* item = ui.user_properties_table->currentItem();
  int index = ui.user_properties_table->currentIndex().row();
  int count = ui.user_properties_table->topLevelItemCount();
  bool exists = item != nullptr;

  ui.change_property_key_button->setEnabled(exists);
  ui.delete_property_button->setEnabled(exists);

  ui.move_property_up_button->setEnabled(exists && index > 0);
  ui.move_property_down_button->setEnabled(exists && index < count - 1);
}

/**
 * @brief Creates a validator for entity name fields.
 * @return A validator that checks entity name inputs.
 */
QValidator* EditEntityDialog::create_name_validator() {

  // Accept everthing except quotes, double quotes, backslashes and whitespaces.
  QRegularExpression regexp("^[^\"'\\\\ \t]*$");
  return new QRegularExpressionValidator(regexp);
}

/**
 * @brief Creates a validator for dialog id fields.
 * @return A validator that checks dialog id inputs.
 */
QValidator* EditEntityDialog::create_dialog_id_validator() {

  // Empty string or only ascii letters, digits, underscores and dots.
  // The first character must be a letter.
  QRegularExpression regexp("^$|^[a-zA-Z_][a-zA-Z0-9_\\.]*$");
  return new QRegularExpressionValidator(regexp);
}

/**
 * @brief Creates a validator for savegame variable fields.
 * @return A validator that checks savegame variable inputs.
 */
QValidator* EditEntityDialog::create_savegame_variable_validator() {

  // Empty string or only ascii letters, digits and underscores.
  // The first character must be a letter.
  QRegularExpression regexp("^$|^[a-zA-Z_][a-zA-Z0-9_]*$");
  return new QRegularExpressionValidator(regexp);
}

/**
 * @brief Fills the fields from the existing entity.
 */
void EditEntityDialog::initialize() {

  initialize_simple_booleans();
  initialize_simple_integers();
  initialize_simple_strings();

  initialize_behavior();
  initialize_breed();
  initialize_damage_on_enemies();
  initialize_destination();
  initialize_destination_map();
  initialize_direction();
  initialize_font();
  initialize_ground();
  initialize_layer();
  initialize_maximum_moves();
  initialize_model();
  initialize_name();
  initialize_opening_method();
  initialize_pattern();
  initialize_savegame_variable();
  initialize_size();
  initialize_sound();
  initialize_sprite();
  initialize_starting_location_mode();
  initialize_subtype();
  initialize_tileset();
  initialize_transition();
  initialize_treasure();
  initialize_type();
  initialize_weight();
  initialize_xy();
  initialize_user_properties();

  adjustSize();
}

/**
 * @brief Applies the data in the GUI to the entity.
 */
void EditEntityDialog::apply() {

  apply_simple_booleans();
  apply_simple_integers();
  apply_simple_strings();

  apply_behavior();
  apply_breed();
  apply_damage_on_enemies();
  apply_destination();
  apply_destination_map();
  apply_direction();
  apply_font();
  apply_ground();
  apply_layer();
  apply_maximum_moves();
  apply_model();
  apply_name();
  apply_opening_method();
  apply_pattern();  // Before applying the size.
  apply_savegame_variable();
  apply_size();
  apply_sound();
  apply_sprite();
  apply_starting_location_mode();
  apply_subtype();
  apply_tileset();
  apply_transition();
  apply_treasure();
  apply_type();
  apply_weight();
  apply_xy();
  apply_user_properties();
}

/**
 * @brief Sets up the behavior of a field with a checkbox or a label depending
 * on whether it is optional.
 * @param field_name The field to initialize.
 * @param label_layout Parent layout of the checkbox and label to handle
 * (one of them is removed). If nullptr, nothing is removed.
 * @param label A label to be used when the field is mandatory.
 * Can be nullptr.
 * @param checkbox A checkbox to be used instead of the label when the field
 * is optional. Can be nullptr.
 * @param field The field widget to disable when the checkbox is disabled.
 * Can be nullptr.
 */
void EditEntityDialog::initialize_possibly_optional_field(const QString& field_name,
                                                          QLayout* label_layout,
                                                          QWidget* label,
                                                          QCheckBox* checkbox,
                                                          QWidget* field) {

  if (!entity_before.is_field_optional(field_name)) {
    // Mandatory field: remove the checkbox if any, keep the label.
    if (label_layout != nullptr && checkbox != nullptr) {
      checkbox->setChecked(true);  // Make it check even if hidden to simplify apply_xxx() functions.
      checkbox->hide();
      label_layout->removeWidget(checkbox);
    }
    return;
  }

  // Optional field: remove the label if any, keep the checkbox.
  if (label_layout != nullptr && label != nullptr) {
    label->hide();
    label_layout->removeWidget(label);
  }

  bool unset = entity_before.is_field_unset(field_name);

  if (unset) {
    if (field != nullptr) {
      field->setEnabled(false);
    }
  }
  else {
    if (checkbox != nullptr) {
      checkbox->setChecked(true);
    }
  }
  if (checkbox != nullptr && field != nullptr) {
    connect(checkbox, SIGNAL(toggled(bool)),
            field, SLOT(setEnabled(bool)));
  }
}

/**
 * @brief Removes a row of the form layout.
 * @param label Label of the row.
 * @param field Field of the row.
 */
void EditEntityDialog::remove_field(QWidget* label, QWidget* field) {

  label->hide();
  ui.form_layout->removeWidget(label);
  field->hide();
  ui.form_layout->removeWidget(field);
}

/**
 * @brief Initializes the simple boolean fields.
 */
void EditEntityDialog::initialize_simple_booleans() {

  simple_boolean_fields <<
    SimpleBooleanField("enabled_at_start", tr("Initial state"), tr("Enabled at start")) <<
    SimpleBooleanField("default", tr("Default"), tr("Set as the default destination")) <<
    SimpleBooleanField("can_be_cut", tr("Cutting the object"), tr("Can be cut"), ui.damage_on_enemies_layout) <<
    SimpleBooleanField("can_explode", tr("Exploding"), tr("Can explode"), ui.damage_on_enemies_layout) <<
    SimpleBooleanField("can_regenerate", tr("Regeneration"), tr("Can regenerate"), ui.damage_on_enemies_layout) <<
    SimpleBooleanField("pushable", tr("Interactions"), tr("Can be pushed")) <<
    SimpleBooleanField("pullable", "", tr("Can be pulled")) <<
    SimpleBooleanField("needs_block", tr("Activation"), tr("Requires a block to be activated")) <<
    SimpleBooleanField("inactivate_when_leaving", tr("Leaving the switch"), tr("Deactivate when leaving")) <<
    SimpleBooleanField("stops_hero", tr("Hero"), tr("Obstacle for the hero")) <<
    SimpleBooleanField("stops_enemies", tr("Enemies"), tr("Obstacle for enemies")) <<
    SimpleBooleanField("stops_npcs", tr("NPCs"), tr("Obstacle for NPCs")) <<
    SimpleBooleanField("stops_blocks", tr("Blocks"), tr("Obstacle for blocks")) <<
    SimpleBooleanField("stops_projectiles", tr("Projectiles"), tr("Obstacle for projectiles")) <<
    SimpleBooleanField("allow_movement", tr("Movements"), tr("Allow to move")) <<
    SimpleBooleanField("allow_attack", tr("Sword"), tr("Allow to use the sword")) <<
    SimpleBooleanField("allow_item", tr("Items"), tr("Allow to use equipment items"));

  for (SimpleBooleanField& field : simple_boolean_fields) {
    if (entity_before.has_field(field.field_name)) {
      QLabel* label = new QLabel(field.label_text, this);
      QCheckBox* checkbox = new QCheckBox(field.checkbox_text, this);
      checkbox->setChecked(entity_before.get_field(field.field_name).toBool());
      field.checkbox = checkbox;
      if (field.before_widget != nullptr) {
        int row = 0;
        QFormLayout::ItemRole role;
        ui.form_layout->getWidgetPosition(field.before_widget, &row, &role);
        if (row != -1) {
          ui.form_layout->insertRow(row, label, checkbox);
        }
        else {
          // Widget not found.
          ui.form_layout->addRow(label, checkbox);
        }
      }
      else {
        ui.form_layout->addRow(label, checkbox);
      }
    }
  }
}

/**
 * @brief Updates the entity from the simple boolean fields.
 */
void EditEntityDialog::apply_simple_booleans() {

  for (const SimpleBooleanField& field : simple_boolean_fields) {
    if (entity_before.has_field(field.field_name) && field.checkbox != nullptr) {
      entity_after->set_field(field.field_name, field.checkbox->isChecked());
    }
  }
}

/**
 * @brief Initializes the simple integer fields.
 */
void EditEntityDialog::initialize_simple_integers() {

  simple_integer_fields <<
    SimpleIntegerField("price", tr("Price"), 0, 10, ui.font_field) <<
    SimpleIntegerField("jump_length", tr("Jump length"), 16, 8) <<
    SimpleIntegerField("speed", tr("Speed"), 0, 8);

  for (SimpleIntegerField& field : simple_integer_fields) {
    if (entity_before.has_field(field.field_name)) {
      QLabel* label = new QLabel(field.label_text, this);
      QSpinBox* spinbox = new QSpinBox(this);
      spinbox->setMinimum(field.minimum);
      spinbox->setMaximum(999999);
      spinbox->setValue(entity_before.get_field(field.field_name).toInt());
      spinbox->setSingleStep(field.step);
      spinbox->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
      field.spinbox = spinbox;
      if (field.before_widget != nullptr) {
        int row = 0;
        QFormLayout::ItemRole role;
        ui.form_layout->getWidgetPosition(field.before_widget, &row, &role);
        if (row != -1) {
          ui.form_layout->insertRow(row, label, spinbox);
        }
        else {
          // Widget not found.
          ui.form_layout->addRow(label, spinbox);
        }
      }
      else {
        ui.form_layout->addRow(label, spinbox);
      }
    }
  }
}

/**
 * @brief Updates the entity from the simple integer fields.
 */
void EditEntityDialog::apply_simple_integers() {

  for (const SimpleIntegerField& field : simple_integer_fields) {
    if (entity_before.has_field(field.field_name) && field.spinbox != nullptr) {
      entity_after->set_field(field.field_name, field.spinbox->value());
    }
  }
}

/**
 * @brief Initializes the simple string fields.
 */
void EditEntityDialog::initialize_simple_strings() {

  simple_string_fields <<
    SimpleStringField("cannot_open_dialog", tr("Show a dialog if fails to open"), create_dialog_id_validator()) <<
    SimpleStringField("dialog", tr("Description dialog id"), create_dialog_id_validator());

  for (SimpleStringField& field : simple_string_fields) {
    if (!entity_before.has_field(field.field_name)) {
      continue;
    }

    QWidget* left_widget = nullptr;
    QLineEdit* line_edit = new QLineEdit(this);
    line_edit->setText(entity_before.get_field(field.field_name).toString());
    if (field.validator != nullptr) {
      line_edit->setValidator(field.validator);
    }
    field.line_edit = line_edit;
    if (!entity_before.is_field_optional(field.field_name)) {
      left_widget = new QLabel(field.label_text, this);
    }
    else {
      field.checkbox = new QCheckBox(field.label_text, this);
      initialize_possibly_optional_field(
            field.field_name,
            nullptr,
            nullptr,
            field.checkbox,
            line_edit);
      left_widget = field.checkbox;
    }

    if (field.before_widget != nullptr) {
      int row = 0;
      QFormLayout::ItemRole role;
      ui.form_layout->getWidgetPosition(field.before_widget, &row, &role);
      if (row != -1) {
        ui.form_layout->insertRow(row, left_widget, line_edit);
      }
      else {
        // Widget not found.
        ui.form_layout->addRow(left_widget, line_edit);
      }
    }
    else {
      ui.form_layout->addRow(left_widget, line_edit);
    }
  }
}

/**
 * @brief Updates the entity from the simple string fields.
 */
void EditEntityDialog::apply_simple_strings() {

  for (const SimpleStringField& field : simple_string_fields) {
    if (entity_before.has_field(field.field_name) && field.line_edit != nullptr) {
      QString value;
      if (field.checkbox == nullptr || field.checkbox->isChecked()) {
        value = field.line_edit->text();
      }
      entity_after->set_field(field.field_name, value);
    }
  }
}

/**
 * @brief Initializes the behavior fields.
 */
void EditEntityDialog::initialize_behavior() {

  if (!entity_before.has_field(behavior_field_name)) {
    remove_field(ui.behavior_label, ui.behavior_layout);
    return;
  }

  ui.behavior_item_field->set_quest(get_quest());
  ui.behavior_item_field->set_resource_type(ResourceType::ITEM);

  ui.behavior_dialog_field->setEnabled(false);
  connect(ui.behavior_dialog_radio, SIGNAL(toggled(bool)),
          ui.behavior_dialog_field, SLOT(setEnabled(bool)));
  ui.behavior_item_field->setEnabled(false);
  connect(ui.behavior_item_radio, SIGNAL(toggled(bool)),
          ui.behavior_item_field, SLOT(setEnabled(bool)));

  QString behavior = entity_before.get_field(behavior_field_name).toString();
  // behavior can be one of:
  // - "map",
  // - "dialog#xxx" where xxx is a dialog id,
  // - "item#xxx" where xxx is an item id.

  QStringList parts = behavior.split('#');
  if (parts.size() == 2) {
    QString first = parts.at(0);
    if (first == "dialog") {
      // Show a dialog.
      QString dialog_id = parts.at(1);
      ui.behavior_dialog_radio->setChecked(true);
      ui.behavior_dialog_field->setText(dialog_id);
      ui.behavior_dialog_field->setEnabled(true);
    }
    else if (first == "item") {
      // Call an item script.
      QString item_id = parts.at(1);
      ui.behavior_item_radio->setChecked(true);
      ui.behavior_item_field->set_selected_id(item_id);
      ui.behavior_item_field->setEnabled(true);
    }
    else {
      // The field is invalid: initialize the dialog with "map".
      behavior = "map";
    }
  }
  else if (behavior != "map") {
    // The field is invalid: initialize the dialog with "map".
    behavior = "map";
  }

  if (behavior == "map") {
    // Call the map script.
    ui.behavior_map_radio->setChecked(true);
  }
}

/**
 * @brief Updates the entity from the behavior fields.
 */
void EditEntityDialog::apply_behavior() {

  if (!entity_before.has_field(behavior_field_name)) {
    return;
  }

  QString behavior = "map";
  if (ui.behavior_dialog_radio->isChecked()) {
    QString dialog_id = ui.behavior_dialog_field->text();
    // TODO check that dialog_id is valid
    behavior = QString("dialog#") + dialog_id;
  }
  else if (ui.behavior_item_radio->isChecked()) {
    QString item_id = ui.behavior_item_field->get_selected_id();
    behavior = QString("item#") + item_id;
  }
  entity_after->set_field(behavior_field_name, behavior);
}

/**
 * @brief Initializes the breed field.
 */
void EditEntityDialog::initialize_breed() {

  if (!entity_before.has_field(breed_field_name)) {
    remove_field(ui.breed_label, ui.breed_field);
    return;
  }

  ui.breed_field->set_resource_type(ResourceType::ENEMY);
  ui.breed_field->set_quest(get_quest());
  ui.breed_field->set_tileset_id(get_map().get_tileset_id());
  QString breed = entity_before.get_field(breed_field_name).toString();
  ui.breed_field->set_selected_id(breed);
}

/**
 * @brief Updates the entity from the breed field.
 */
void EditEntityDialog::apply_breed() {

  if (entity_after->has_field(breed_field_name)) {
    entity_after->set_field(breed_field_name, ui.breed_field->get_selected_id());
  }
}

/**
 * @brief Initializes the damage on enemies field.
 */
void EditEntityDialog::initialize_damage_on_enemies() {

  if (!entity_before.has_field(damage_on_enemies_field_name)) {
    remove_field(ui.damage_on_enemies_checkbox, ui.damage_on_enemies_layout);
    return;
  }

  int damage_on_enemies = entity_before.get_field(damage_on_enemies_field_name).toInt();
  ui.damage_on_enemies_field->setValue(damage_on_enemies);

  if (damage_on_enemies == 0) {
    ui.damage_on_enemies_layout->setEnabled(false);
  }
  else {
    ui.damage_on_enemies_checkbox->setChecked(true);
  }
  connect(ui.damage_on_enemies_checkbox, SIGNAL(toggled(bool)),
          ui.damage_on_enemies_layout, SLOT(setEnabled(bool)));
}

/**
 * @brief Updates the entity from the damage on enemies field.
 */
void EditEntityDialog::apply_damage_on_enemies() {

  if (entity_after->has_field(damage_on_enemies_field_name)) {
    entity_after->set_field(damage_on_enemies_field_name, ui.damage_on_enemies_checkbox->isChecked() ?
                              ui.damage_on_enemies_field->value() : 0);
  }
}

/**
 * @brief Initializes the destination field.
 */
void EditEntityDialog::initialize_destination() {

  if (!entity_before.has_field(destination_field_name)) {
    remove_field(ui.destination_label, ui.destination_field);
    return;
  }

  QString destination_map_id = entity_before.get_field(destination_map_field_name).toString();
  ui.destination_field->set_map_id(get_quest(), destination_map_id);
  ui.destination_field->set_filtered_by_entity_type(true);
  ui.destination_field->set_entity_type_filter(EntityType::DESTINATION);
  ui.destination_field->add_special_value("", tr("(Default destination)"));
  ui.destination_field->add_special_value("_same", tr("(Same point)"));
  ui.destination_field->add_special_value("_side", tr("(Side of the map)"));
  ui.destination_field->build();
  ui.destination_field->set_selected_name(entity_before.get_field(destination_field_name).toString());
}

/**
 * @brief Updates the entity from the destination field.
 */
void EditEntityDialog::apply_destination() {

  if (entity_before.has_field(destination_field_name)) {
    entity_after->set_field(destination_field_name, ui.destination_field->get_selected_name());
  }
}

/**
 * @brief Initializes the destination map field.
 */
void EditEntityDialog::initialize_destination_map() {

  if (!entity_before.has_field(destination_map_field_name)) {
    remove_field(ui.destination_map_label, ui.destination_map_field);
    return;
  }

  ui.destination_map_field->set_quest(get_quest());
  ui.destination_map_field->set_resource_type(ResourceType::MAP);
  ui.destination_map_field->set_selected_id(entity_before.get_field(destination_map_field_name).toString());

  connect(ui.destination_map_field, &QComboBox::currentTextChanged, [this](const QString&) {
    QString map_id = ui.destination_map_field->currentData().toString();
    ui.destination_field->set_map_id(get_quest(), map_id);
    ui.destination_field->build();
  });
}

/**
 * @brief Updates the entity from the destination map field.
 */
void EditEntityDialog::apply_destination_map() {

  if (entity_before.has_field(destination_map_field_name)) {
    entity_after->set_field(destination_map_field_name, ui.destination_map_field->get_selected_id());
  }
}

/**
 * @brief Initializes the direction field.
 */
void EditEntityDialog::initialize_direction() {

  if (!entity_before.has_direction_field()) {
    remove_field(ui.direction_label, ui.direction_layout);
    return;
  }

  int num_directions = entity_before.get_num_directions();
  if (entity_before.is_no_direction_allowed()) {
    ui.direction_field->addItem(entity_before.get_no_direction_text(), -1);
  }

  QStringList texts;
  if (num_directions == 4) {
    texts = QStringList{
      tr("Right"),
      tr("Up"),
      tr("Left"),
      tr("Down")
    };
  }
  else if (num_directions == 8) {
    texts = QStringList{
      tr("Right"),
      tr("Right-up"),
      tr("Up"),
      tr("Left-up"),
      tr("Left"),
      tr("Left-down"),
      tr("Down"),
      tr("Right-down"),
    };
  }
  else {
    for (int i = 0; i < num_directions; ++num_directions) {
      texts.append(QString::number(i));
    }
  }
  for (int i = 0; i < texts.size(); ++i) {
    ui.direction_field->addItem(texts[i], i);
  }

  int index = entity_before.get_direction();
  if (entity_before.is_no_direction_allowed()) {
    ++index;
  }
  ui.direction_field->setCurrentIndex(index);

  connect(ui.direction_field, SIGNAL(currentIndexChanged(int)),
          this, SLOT(direction_changed()));
}

/**
 * @brief Updates the entity from the direction field.
 */
void EditEntityDialog::apply_direction() {

  if (entity_after->has_direction_field()) {
    entity_after->set_direction(ui.direction_field->currentData().toInt());
  }
}

/**
 * @brief Initializes the font field.
 */
void EditEntityDialog::initialize_font() {

  if (!entity_before.has_field(font_field_name)) {
    remove_field(ui.font_label, ui.font_field);
    return;
  }

  ui.font_field->set_quest(get_quest());
  ui.font_field->set_resource_type(ResourceType::FONT);
  ui.font_field->add_special_value("", tr("(Default)"), 0);
  QString font = entity_before.get_field(font_field_name).toString();
  ui.font_field->set_selected_id(font);
}

/**
 * @brief Updates the entity from the font field.
 */
void EditEntityDialog::apply_font() {

  if (entity_after->has_field(font_field_name)) {
    entity_after->set_field(font_field_name, ui.font_field->get_selected_id());
  }
}

/**
 * @brief Initializes the ground field.
 */
void EditEntityDialog::initialize_ground() {

  if (!entity_before.has_field(ground_field_name)) {
    remove_field(ui.ground_checkbox, ui.ground_field);
    return;
  }

  initialize_possibly_optional_field(
        ground_field_name,
        nullptr,
        nullptr,
        ui.ground_checkbox,
        ui.ground_field);

  QString ground_name = entity_before.get_field(ground_field_name).toString();
  ui.ground_field->set_selected_value(GroundTraits::get_by_lua_name(ground_name));
}

/**
 * @brief Updates the entity from the ground field.
 */
void EditEntityDialog::apply_ground() {

  if (entity_after->has_field(ground_field_name)) {
    Ground ground = ui.ground_checkbox->isChecked() ? ui.ground_field->get_selected_value() : Ground::WALL;
    entity_after->set_field(ground_field_name, GroundTraits::get_lua_name(ground));
  }
}

/**
 * @brief Initializes the layer field.
 */
void EditEntityDialog::initialize_layer() {

  ui.layer_field->setMinimum(entity_before.get_map().get_min_layer());
  ui.layer_field->setMaximum(entity_before.get_map().get_max_layer());
  ui.layer_field->setValue(entity_before.get_layer());
}

/**
 * @brief Updates the entity from the layer field.
 */
void EditEntityDialog::apply_layer() {

  entity_after->set_layer(ui.layer_field->value());
}

/**
 * @brief Initializes the maximum moves field.
 */
void EditEntityDialog::initialize_maximum_moves() {

  if (!entity_before.has_field(maximum_moves_field_name)) {
    remove_field(ui.maximum_moves_label, ui.maximum_moves_field);
    return;
  }

  ui.maximum_moves_field->addItem(tr("Cannot move"), 0);
  ui.maximum_moves_field->addItem(tr("1 move only"), 1);
  ui.maximum_moves_field->addItem(tr("Unlimited"), 2);

  int value = entity_before.get_field(maximum_moves_field_name).toInt();
  int index = ui.maximum_moves_field->findData(value);
  if (index != -1) {
    ui.maximum_moves_field->setCurrentIndex(index);
  }
}

/**
 * @brief Updates the entity from the maximum moves field.
 */
void EditEntityDialog::apply_maximum_moves() {

  if (entity_after->has_field(maximum_moves_field_name)) {
    int value = ui.maximum_moves_field->currentData().toInt();
    entity_after->set_field(maximum_moves_field_name, value);
  }
}

/**
 * @brief Initializes the model field.
 */
void EditEntityDialog::initialize_model() {

  if (!entity_before.has_field(model_field_name)) {
    remove_field(ui.model_checkbox, ui.model_field);
    return;
  }

  initialize_possibly_optional_field(
        model_field_name,
        nullptr,
        nullptr,
        ui.model_checkbox,
        ui.model_field);
  ui.model_field->set_quest(get_quest());
  ui.model_field->set_resource_type(ResourceType::ENTITY);
  QString model = entity_before.get_field(model_field_name).toString();
  ui.model_field->set_selected_id(model);
}

/**
 * @brief Updates the entity from the model field.
 */
void EditEntityDialog::apply_model() {

  if (entity_after->has_field(model_field_name)) {
    entity_after->set_field(model_field_name, ui.model_checkbox->isChecked() ?
                              ui.model_field->get_selected_id() : "");
  }
}

/**
 * @brief Initializes the name field.
 */
void EditEntityDialog::initialize_name() {

  if (entity_before.get_type() != EntityType::DESTINATION) {
    ui.name_update_teletransporters_checkbox->setVisible(false);
  }

  if (!entity_before.is_dynamic()) {
    remove_field(ui.name_label, ui.name_field);
    return;
  }

  ui.name_field->setText(entity_before.get_name());
  ui.name_field->setValidator(create_name_validator());
}

/**
 * @brief Updates the entity from the name field.
 */
void EditEntityDialog::apply_name() {

  entity_after->set_name(ui.name_field->text());

  if (entity_after->get_type() == EntityType::DESTINATION) {
    const bool update_teletransporters = ui.name_update_teletransporters_checkbox->isChecked();
    static_cast<Destination&>(entity_before).set_update_teletransporters(update_teletransporters);
  }
}

/**
 * @brief Removes the widgets of opening methods that do not exist for this entity type.
 */
void EditEntityDialog::hide_unexisting_opening_methods() {

  // All opening methods are not available for all types of entities.
  QMap<EntityType, QSet<QString>> opening_methods_by_type = {
    { EntityType::DOOR, { "none", "interaction", "interaction_if_savegame_variable", "interaction_if_item", "explosion" } },
    { EntityType::CHEST, { "interaction", "interaction_if_savegame_variable", "interaction_if_item" } }
  };

  const QSet<QString>& opening_methods = opening_methods_by_type.value(entity_before.get_type());
  if (!opening_methods.contains("none")) {
    ui.opening_method_layout->layout()->removeWidget(ui.opening_method_none_radio);
    ui.opening_method_none_radio->hide();
  }
  if (!opening_methods.contains("interaction")) {
    ui.opening_method_layout->layout()->removeWidget(ui.opening_method_interaction_radio);
    ui.opening_method_interaction_radio->hide();
  }
  if (!opening_methods.contains("interaction_if_savegame_variable")) {
    ui.opening_method_layout->layout()->removeItem(ui.opening_condition_savegame_variable_layout);
    ui.opening_method_layout->layout()->removeItem(ui.opening_condition_savegame_variable_consumed_layout);
  }
  if (!opening_methods.contains("interaction_if_item")) {
    ui.opening_method_layout->layout()->removeItem(ui.opening_condition_item_layout);
    ui.opening_method_layout->layout()->removeItem(ui.opening_condition_item_consumed_layout);
  }
  if (!opening_methods.contains("explosion")) {
    ui.opening_method_layout->layout()->removeWidget(ui.opening_method_explosion_radio);
    ui.opening_method_explosion_radio->hide();
  }
}

/**
 * @brief Returns the radio button associated to each opening method value.
 * @return The opening method radio buttons.
 */
QMap<QString, QRadioButton*> EditEntityDialog::get_opening_method_radio_buttons() {

  const QMap<QString, QRadioButton*> buttons = {
    { "none", ui.opening_method_none_radio },
    { "interaction", ui.opening_method_interaction_radio },
    { "interaction_if_savegame_variable", ui.opening_method_savegame_variable_radio },
    { "interaction_if_item", ui.opening_method_item_radio },
    { "explosion", ui.opening_method_explosion_radio }
  };
  return buttons;
}

/**
 * @brief Returns the radio button associated to an opening method value.
 * @param opening_method An opening method.
 * @return The corresponding radio button or nullptr if the opening method is invalid.
 */
QRadioButton* EditEntityDialog::get_opening_method_radio_button(const QString& opening_method) {

  return get_opening_method_radio_buttons().value(opening_method);
}

/**
 * @brief Returns the opening method corresponding to the selected radio button.
 * @return The current opening method or an empty string if no radio button
 * is checked.
 */
QString EditEntityDialog::get_selected_opening_method() {

  const QMap<QString, QRadioButton*>& buttons = get_opening_method_radio_buttons();
  for (auto it = buttons.begin(); it != buttons.end(); ++it) {
    if (it.value()->isChecked()) {
      return it.key();
    }
  }
  return QString();
}

/**
 * @brief Initializes the opening method fields.
 */
void EditEntityDialog::initialize_opening_method() {

  if (!entity_before.has_field(opening_method_field_name) ||
      !entity_before.has_field(opening_condition_field_name) ||
      !entity_before.has_field(opening_condition_consumed_field_name)) {
    remove_field(ui.opening_method_label, ui.opening_method_layout);
    return;
  }

  // Some entity types don't have all values: remove such fields.
  hide_unexisting_opening_methods();

  // Initialize the item selector.
  ui.opening_condition_item_field->set_resource_type(ResourceType::ITEM);
  ui.opening_condition_item_field->set_quest(get_quest());

  // Put the current values into the widgets.
  // opening_method is how to open the chest or door,
  // opening_condition is the required savegame variable or item id depending on the method.
  QString opening_method = entity_before.get_field(opening_method_field_name).toString();
  QString opening_condition = entity_before.get_field(opening_condition_field_name).toString();
  bool opening_condition_consumed = entity_before.get_field(opening_condition_consumed_field_name).toBool();

  // Check the correct radio button.
  const QMap<QString, QRadioButton*>& radios = get_opening_method_radio_buttons();
  QRadioButton* radio = radios.value(opening_method);
  if (radio != nullptr) {
    radio->setChecked(true);
  }
  else {
    // Check a default radio button if the current value is invalid.
    ui.opening_method_interaction_radio->setChecked(true);
  }

  // Prepare the savegame variable fields.
  if (radio == ui.opening_method_savegame_variable_radio) {
    ui.opening_condition_savegame_variable_field->setText(opening_condition);
    ui.opening_condition_savegame_variable_consumed_checkbox->setChecked(opening_condition_consumed);
  }
  else {
    ui.opening_condition_savegame_variable_field->setEnabled(false);
    ui.opening_condition_savegame_variable_consumed_checkbox->setEnabled(false);
  }
  connect(ui.opening_method_savegame_variable_radio, SIGNAL(toggled(bool)),
          ui.opening_condition_savegame_variable_field, SLOT(setEnabled(bool)));
  connect(ui.opening_method_savegame_variable_radio, SIGNAL(toggled(bool)),
          ui.opening_condition_savegame_variable_consumed_checkbox, SLOT(setEnabled(bool)));

  // Prepare the item fields.
  if (radio == ui.opening_method_item_radio) {
    ui.opening_condition_item_field->set_selected_id(opening_condition);
    ui.opening_condition_item_consumed_checkbox->setChecked(opening_condition_consumed);
  }
  else {
    ui.opening_condition_item_field->setEnabled(false);
    ui.opening_condition_item_consumed_checkbox->setEnabled(false);
  }
  connect(ui.opening_method_item_radio, SIGNAL(toggled(bool)),
          ui.opening_condition_item_field, SLOT(setEnabled(bool)));
  connect(ui.opening_method_item_radio, SIGNAL(toggled(bool)),
          ui.opening_condition_item_consumed_checkbox, SLOT(setEnabled(bool)));
}

/**
 * @brief Updates the entity from the opening method fields.
 */
void EditEntityDialog::apply_opening_method() {

  if (!entity_before.has_field(opening_method_field_name) ||
      !entity_before.has_field(opening_condition_field_name) ||
      !entity_before.has_field(opening_condition_consumed_field_name)) {
    remove_field(ui.opening_method_label, ui.opening_method_layout);
    return;
  }

  entity_after->set_field(opening_method_field_name, get_selected_opening_method());

  if (ui.opening_method_savegame_variable_radio->isChecked()) {
    entity_after->set_field(opening_condition_field_name, ui.opening_condition_savegame_variable_field->text());
    entity_after->set_field(opening_condition_consumed_field_name, ui.opening_condition_savegame_variable_consumed_checkbox->isChecked());
  }
  else if (ui.opening_method_item_radio->isChecked()) {
    entity_after->set_field(opening_condition_field_name, ui.opening_condition_item_field->get_selected_id());
    entity_after->set_field(opening_condition_consumed_field_name, ui.opening_condition_item_consumed_checkbox->isChecked());
  }
}

/**
 * @brief Initializes the pattern field.
 */
void EditEntityDialog::initialize_pattern() {

  if (!entity_before.has_field(pattern_field_name)) {
    remove_field(ui.pattern_label, ui.pattern_field);
    return;
  }

  // Show the initial value.
  QString value = entity_before.get_field(pattern_field_name).toString();
  ui.pattern_field->set_pattern_id(value);
}

/**
 * @brief Updates the entity from the pattern field.
 */
void EditEntityDialog::apply_pattern() {

  if (!entity_before.has_field(pattern_field_name)) {
    return;
  }

  QString value = ui.pattern_field->get_pattern_id();
  entity_after->set_field(pattern_field_name, value);
}

/**
 * @brief Initializes the savegame variable field.
 */
void EditEntityDialog::initialize_savegame_variable() {

  if (!entity_before.has_field(savegame_variable_field_name)) {
    remove_field(ui.savegame_variable_checkbox, ui.savegame_variable_layout);
    return;
  }

  // Specific checkbox text for some types of entities.
  const QMap<EntityType, QString> checkbox_texts = {
    { EntityType::ENEMY, tr("Save the enemy state") },
    { EntityType::DOOR, tr("Save the door state") }
  };
  QString checkbox_text = checkbox_texts.value(entity_before.get_type());
  if (!checkbox_text.isEmpty()) {
    ui.savegame_variable_checkbox->setText(checkbox_text);
  }

  // Connect the checkbox to the field.
  initialize_possibly_optional_field(
        savegame_variable_field_name,
        nullptr,
        nullptr,
        ui.savegame_variable_checkbox,
        ui.savegame_variable_layout);

  // Only accept valid identifiers as savegame variable names.
  ui.savegame_variable_field->setValidator(create_savegame_variable_validator());

  // Show the initial value.
  QString value = entity_before.get_field(savegame_variable_field_name).toString();
  ui.savegame_variable_field->setText(value);
}

/**
 * @brief Updates the entity from the savegame variable field.
 */
void EditEntityDialog::apply_savegame_variable() {

  if (entity_before.has_field(savegame_variable_field_name)) {
    QString value = ui.savegame_variable_checkbox->isChecked() ?
          ui.savegame_variable_field->text() : "";
    entity_after->set_field(savegame_variable_field_name, value);
  }
}

/**
 * @brief Initializes the size fields.
 */
void EditEntityDialog::initialize_size() {

  if (!entity_before.has_size_fields()) {
    remove_field(ui.size_label, ui.size_field);
    return;
  }

  // Initialize spinboxes.
  ui.size_field->config("x", 8, 999999);

  // Show the current size in the spinboxes.
  ui.size_field->set_size(entity_before.get_size());

  // Tell spinboxes to only make multiples of the base size.
  const QSize& base_size = entity_before.get_base_size();
  ui.size_field->set_first_step(base_size.width());
  ui.size_field->set_first_min(base_size.width());
  ui.size_field->set_second_step(base_size.height());
  ui.size_field->set_second_min(base_size.height());

  // Apply the resize mode contraints.
  update_size_constraints();

  connect(ui.size_field, SIGNAL(first_value_changed(int)),
          this, SLOT(width_changed(int)));
  connect(ui.size_field, SIGNAL(second_value_changed(int)),
          this, SLOT(height_changed(int)));
}

/**
 * @brief Updates the entity from the size fields.
 */
void EditEntityDialog::apply_size() {

  if (entity_after->has_size_fields()) {
    QSize size = ui.size_field->get_size();

    // Round the size.
    size = entity_after->get_closest_base_size_multiple(size);

    // If the size is invalid, refuse the change.
    if (entity_after->is_size_valid(size)) {
      entity_after->set_size(size);
    }
  }
}

/**
 * @brief Initializes the sound field.
 */
void EditEntityDialog::initialize_sound() {

  QString field_name;
  if (entity_before.has_field(sound_field_name)) {
    field_name = sound_field_name;
  }
  else if (entity_before.has_field(destruction_sound_field_name)) {
    ui.sound_checkbox->setText(tr("Play a sound when destroyed"));
    field_name = destruction_sound_field_name;
  }
  else {
    remove_field(ui.sound_checkbox, ui.sound_field);
    return;
  }

  ui.sound_field->set_quest(get_quest());
  initialize_possibly_optional_field(
        field_name,
        nullptr,
        nullptr,
        ui.sound_checkbox,
        ui.sound_field);
  QString sound = entity_before.get_field(field_name).toString();
  ui.sound_field->set_selected_id(sound);
}

/**
 * @brief Updates the entity from the sound field.
 */
void EditEntityDialog::apply_sound() {

  QString field_name;
  if (entity_before.has_field(sound_field_name)) {
    field_name = sound_field_name;
  }
  else if (entity_before.has_field(destruction_sound_field_name)) {
    field_name = destruction_sound_field_name;
  }
  else {
    return;
  }

  entity_after->set_field(field_name, ui.sound_checkbox->isChecked() ?
                            ui.sound_field->get_selected_id() : "");
}

/**
 * @brief Initializes the sprite field.
 */
void EditEntityDialog::initialize_sprite() {

  if (!entity_before.has_field(sprite_field_name)) {
    remove_field(ui.sprite_label_checkbox, ui.sprite_field);
    return;
  }

  initialize_possibly_optional_field(
        sprite_field_name,
        ui.sprite_label_checkbox->layout(),
        ui.sprite_label,
        ui.sprite_checkbox,
        ui.sprite_field);
  ui.sprite_field->set_resource_type(ResourceType::SPRITE);
  ui.sprite_field->set_quest(get_quest());
  ui.sprite_field->set_tileset_id(get_map().get_tileset_id());
  QString sprite = entity_before.get_field(sprite_field_name).toString();
  ui.sprite_field->set_selected_id(sprite);
}

/**
 * @brief Updates the entity from the sprite field.
 */
void EditEntityDialog::apply_sprite() {

  if (entity_after->has_field(sprite_field_name)) {
    entity_after->set_field(sprite_field_name, ui.sprite_checkbox->isChecked() ?
                              ui.sprite_field->get_selected_id() : "");
  }
}

/**
 * @brief Initializes the starting location mode field.
 */
void EditEntityDialog::initialize_starting_location_mode() {

  if (!entity_before.has_field(starting_location_mode_field_name)) {
    remove_field(ui.starting_location_mode_label, ui.starting_location_mode_field);
    return;
  }

  QString starting_location_mode_name = entity_before.get_field(starting_location_mode_field_name).toString();
  ui.starting_location_mode_field->set_selected_value(StartingLocationModeTraits::get_by_lua_name(starting_location_mode_name));
}

/**
 * @brief Updates the entity from the starting location mode field.
 */
void EditEntityDialog::apply_starting_location_mode() {

  if (entity_after->has_field(starting_location_mode_field_name)) {
    entity_after->set_field(
          starting_location_mode_field_name,
          StartingLocationModeTraits::get_lua_name(ui.starting_location_mode_field->get_selected_value())
    );
  }
}

/**
 * @brief Initializes the subtype field.
 */
void EditEntityDialog::initialize_subtype() {

  if (!entity_before.has_subtype_field()) {
    remove_field(ui.subtype_label, ui.subtype_field);
    return;
  }

  int i = 0;
  const SubtypeList& subtypes = entity_before.get_existing_subtypes();
  for (const QPair<QString, QString>& subtype : subtypes) {
    ui.subtype_field->addItem(subtype.second, subtype.first);

    if (entity_before.get_subtype() == subtype.first) {
      ui.subtype_field->setCurrentIndex(i);
    }
    ++i;
  }
}

/**
 * @brief Updates the entity from the subtype field.
 */
void EditEntityDialog::apply_subtype() {

  if (entity_after->has_subtype_field()) {
    entity_after->set_subtype(ui.subtype_field->currentData().toString());
  }
}

/**
 * @brief Initializes the tileset field.
 */
void EditEntityDialog::initialize_tileset() {

  if (!entity_before.has_field(tileset_field_name)) {
    remove_field(ui.tileset_label, ui.tileset_layout);
    return;
  }

  ui.tileset_field->set_quest(get_quest());
  ui.tileset_field->set_resource_type(ResourceType::TILESET);

  QString tileset_id = entity_before.get_field(tileset_field_name).toString();
  if (tileset_id.isEmpty()) {
    ui.tileset_from_map_radio->setChecked(true);
    ui.tileset_field->setEnabled(false);
  }
  else {
    ui.tileset_other_radio->setChecked(true);
    ui.tileset_field->set_selected_id(tileset_id);
  }

  connect(ui.tileset_from_map_radio, &QRadioButton::clicked, [this]() {
    ui.tileset_field->setEnabled(false);
    update_pattern_chooser_tileset();
  });
  connect(ui.tileset_other_radio, &QRadioButton::clicked, [this]() {
    ui.tileset_field->setEnabled(true);
    update_pattern_chooser_tileset();
  });

  connect(ui.tileset_field, &QComboBox::currentTextChanged,
          this, &EditEntityDialog::update_pattern_chooser_tileset);

  update_pattern_chooser_tileset();
}

/**
 * @brief Updates the entity from the tileset field.
 */
void EditEntityDialog::apply_tileset() {

  if (entity_before.has_field(tileset_field_name)) {
    QString tileset_id;
    if (ui.tileset_other_radio->isChecked()) {
      tileset_id = ui.tileset_field->get_selected_id();
    }
    entity_after->set_field(tileset_field_name, tileset_id);
  }
}

/**
 * @brief Sets the tileset of the pattern chooser.
 */
void EditEntityDialog::update_pattern_chooser_tileset() {

  QString tileset_id;
  if (ui.tileset_other_radio->isChecked()) {
    tileset_id = ui.tileset_field->get_selected_id();
    if (!tileset_id.isEmpty()) {
      TilesetModel* tileset = get_quest().get_tileset(tileset_id);
      ui.pattern_field->set_tileset(tileset);
      return;
    }
  }

  // Use the tileset of the map otherwise.
  ui.pattern_field->set_tileset(get_map().get_tileset_model());
}

/**
 * @brief Initializes the transition field.
 */
void EditEntityDialog::initialize_transition() {

  if (!entity_before.has_field(transition_field_name)) {
    remove_field(ui.transition_label, ui.transition_field);
    return;
  }

  QString tileset_id = entity_before.get_field(tileset_field_name).toString();
  ui.tileset_field->set_selected_id(tileset_id);
}

/**
 * @brief Updates the entity from the transition field.
 */
void EditEntityDialog::apply_transition() {

  if (entity_after->has_field(transition_field_name)) {
    entity_after->set_field(transition_field_name, TransitionTraits::get_lua_name(ui.transition_field->get_selected_value()));
  }
}

/**
 * @brief Initializes the treasure field.
 */
void EditEntityDialog::initialize_treasure() {

  if (!entity_before.has_field(treasure_name_field_name) ||
      !entity_before.has_field(treasure_variant_field_name) ||
      !entity_before.has_field(treasure_savegame_variable_field_name)
  ) {
    remove_field(ui.treasure_label, ui.treasure_layout);
    return;
  }

  ui.treasure_name_field->set_resource_type(ResourceType::ITEM);
  ui.treasure_name_field->set_quest(get_quest());
  ui.treasure_name_field->set_tileset_id(get_map().get_tileset_id());
  ui.treasure_name_field->add_special_value("", tr("(None)"), 0);  // Add the special value "None".

  // Only accept valid identifiers as savegame variable names.
  ui.treasure_savegame_variable_field->setValidator(create_savegame_variable_validator());

  QString treasure_name = entity_before.get_field(treasure_name_field_name).toString();
  ui.treasure_name_field->set_selected_id(treasure_name);
  ui.treasure_variant_field->setValue(entity_before.get_field(treasure_variant_field_name).toInt());
  QString treasure_savegame_variable = entity_before.get_field(treasure_savegame_variable_field_name).toString();
  if (treasure_savegame_variable.isEmpty()) {
    ui.treasure_savegame_variable_label->setEnabled(false);
    ui.treasure_savegame_variable_field->setEnabled(false);
  }
  else {
    ui.treasure_savegame_variable_field->setText(treasure_savegame_variable);
    ui.save_treasure_checkbox->setChecked(true);
  }
  connect(ui.save_treasure_checkbox, SIGNAL(toggled(bool)),
          ui.treasure_savegame_variable_label, SLOT(setEnabled(bool)));
  connect(ui.save_treasure_checkbox, SIGNAL(toggled(bool)),
          ui.treasure_savegame_variable_field, SLOT(setEnabled(bool)));
}

/**
 * @brief Updates the entity from the tresaure fields.
 */
void EditEntityDialog::apply_treasure() {

  if (!entity_after->has_field(treasure_name_field_name) ||
      !entity_after->has_field(treasure_variant_field_name) ||
      !entity_after->has_field(treasure_savegame_variable_field_name)) {
    return;
  }

  entity_after->set_field(treasure_name_field_name, ui.treasure_name_field->get_selected_id());
  entity_after->set_field(treasure_variant_field_name, ui.treasure_variant_field->value());
  entity_after->set_field(treasure_savegame_variable_field_name, ui.save_treasure_checkbox->isChecked() ?
                            ui.treasure_savegame_variable_field->text() : "");
}

/**
 * @brief Initializes the type field.
 */
void EditEntityDialog::initialize_type() {

  ui.type_field->setText(EntityTraits::get_friendly_name(entity_before.get_type()));
}

/**
 * @brief Updates the entity from the type field.
 */
void EditEntityDialog::apply_type() {

  // Nothing to do: the type is a read-only field.
  Q_ASSERT(EntityTraits::get_friendly_name(entity_after->get_type()) == ui.type_field->text());
}

/**
 * @brief Initializes the weight field.
 */
void EditEntityDialog::initialize_weight() {

  if (!entity_before.has_field(weight_field_name)) {
    remove_field(ui.weight_checkbox, ui.weight_layout);
    return;
  }

  int weight = entity_before.get_field(weight_field_name).toInt();
  if (weight == -1) {
    ui.weight_layout->setEnabled(false);
    ui.weight_checkbox->setChecked(false);
  }
  else {
    ui.weight_layout->setEnabled(true);
    ui.weight_checkbox->setChecked(true);
    ui.weight_field->setValue(weight);
  }

  connect(ui.weight_checkbox, SIGNAL(toggled(bool)),
          ui.weight_layout, SLOT(setEnabled(bool)));
}

/**
 * @brief Updates the entity from the wight field.
 */
void EditEntityDialog::apply_weight() {

  if (entity_after->has_field(weight_field_name)) {
    entity_after->set_field(weight_field_name, ui.weight_checkbox->isChecked() ?
                              ui.weight_field->value() : -1);
  }
}

/**
 * @brief Initializes the position field.
 */
void EditEntityDialog::initialize_xy() {

  ui.xy_field->config(",", -99999, 999999, 8);
  ui.xy_field->set_point(entity_before.get_xy());
}

/**
 * @brief Updates the entity from the position fields.
 */
void EditEntityDialog::apply_xy() {

  entity_after->set_xy(ui.xy_field->get_point());
}

/**
 * @brief Initializes the user properties table.
 */
void EditEntityDialog::initialize_user_properties() {

  connect(ui.add_property_button, SIGNAL(clicked(bool)),
          this, SLOT(add_user_property_requested()));
  connect(ui.change_property_key_button, SIGNAL(clicked(bool)),
          this, SLOT(change_user_property_key_requested()));
  connect(ui.delete_property_button, SIGNAL(clicked(bool)),
          this, SLOT(delete_user_property_requested()));
  connect(ui.move_property_up_button, SIGNAL(clicked(bool)),
          this, SLOT(move_up_user_property_requested()));
  connect(ui.move_property_down_button, SIGNAL(clicked(bool)),
          this, SLOT(move_down_user_property_requested()));

  connect(ui.user_properties_table,
          SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)),
          this, SLOT(user_property_double_clicked(QTreeWidgetItem*, int)));

  connect(ui.user_properties_table->selectionModel(),
          SIGNAL(selectionChanged(QItemSelection, QItemSelection)),
          this, SLOT(update_user_property_buttons()));

  int count = entity_before.get_user_property_count();
  for (int i = 0; i < count; i++) {
    QPair<QString, QString> property = entity_before.get_user_property(i);
    QTreeWidgetItem* item = new QTreeWidgetItem();
    item->setData(0, Qt::DisplayRole, property.first);
    item->setData(1, Qt::DisplayRole, property.second);
    ui.user_properties_table->addTopLevelItem(item);
  }
}

/**
 * @brief Updates the entity from the user properties table.
 */
void EditEntityDialog::apply_user_properties() {

  int entity_count = entity_after->get_user_property_count();
  for (int i = 0; i < entity_count; ++i) {
    entity_after->remove_user_property(0);
  }

  int table_count = ui.user_properties_table->topLevelItemCount();
  for (int i = 0; i < table_count; ++i) {
    QTreeWidgetItem* item = ui.user_properties_table->topLevelItem(i);
    QPair<QString, QString> property = qMakePair(
      item->data(0, Qt::DisplayRole).toString(),
      item->data(1, Qt::DisplayRole).toString()
    );
    entity_after->add_user_property(property);
  }
}

/**
 * @brief Updates the size constraints with the current resize mode.
 */
void EditEntityDialog::update_size_constraints() {

  ui.size_field->set_first_enabled(
    resize_mode != ResizeMode::NONE &&
    resize_mode != ResizeMode::VERTICAL_ONLY);

  ui.size_field->set_second_enabled(
    resize_mode != ResizeMode::NONE &&
    resize_mode != ResizeMode::HORIZONTAL_ONLY);
}

/**
 * @brief Check if an user property with a given key exists.
 * @param key The key of the user property.
 * @return true if the user property exists, false otherwise.
 */
bool EditEntityDialog::user_property_exists(const QString &key) const {

  int count = ui.user_properties_table->topLevelItemCount();
  for (int i = 0; i < count; i++) {
    QTreeWidgetItem* item = ui.user_properties_table->topLevelItem(i);
    if (item->data(0, Qt::DisplayRole).toString() == key) {
      return true;
    }
  }

  return false;
}

}
