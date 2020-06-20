/* AnyMeal recipe database software
   Copyright (C) 2020 Jan Wedekind

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>. */
#include <cstdint>
#include <sstream>
#include <QtCore/QCoreApplication>
#include "html.hh"
#include "ingredient_model.hh"


using namespace std;

IngredientModel::IngredientModel(QObject *parent, vector<Ingredient> &ingredients, vector<pair<int, string>> &sections):
  QAbstractItemModel(parent), m_ingredients(ingredients), m_sections(sections)
{
}

string IngredientModel::translate(const char *context, const char *text) {
  return QCoreApplication::translate(context, text).toUtf8().constData();
}

int IngredientModel::columnCount(const QModelIndex &) const {
  return 3;
}

QVariant IngredientModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
    switch (section) {
      case 0:
        return QVariant(tr("amount"));
      case 1:
        return QVariant(tr("unit"));
      case 2:
        return QVariant(tr("ingredient"));
    };
  };
  return QVariant();
}

int IngredientModel::rowCount(const QModelIndex &parent) const {
  if (parent.column() > 0)
    return 0;
  if (!parent.isValid())
    return m_sections.size() + 1; // Number of sections plus one for main section.
  if (!is_ingredient(parent)) {
    // Return number of ingredients in a section.
    unsigned int section = parent.row();
    int a = section == 0 ? 0 : m_sections[section - 1].first;
    int b = section == m_sections.size() ? m_ingredients.size() : m_sections[section].first;
    return b - a;
  };
  return 0;
}

QVariant IngredientModel::data(const QModelIndex &index, int role) const {
  if (!index.isValid())
    return QVariant();
  if (role != Qt::DisplayRole)
    return QVariant();
  if (!is_ingredient(index)) {
    if (index.column() == 0) {
      if (index.row() == 0) {
        return QVariant(tr("<Main>"));
      } else {
        return QVariant(m_sections[index.row() - 1].second.c_str());
      };
    } else
      return QVariant();
  };
  Ingredient ingredient = get_ingredient(index);
  switch (index.column()) {
    case 0:
      return QVariant(html_amount(ingredient).c_str());
    case 1:
      return QVariant(html_unit(ingredient.unit(), &translate).c_str());
    case 2:
      return QVariant(ingredient.text().c_str());
  };
  return QVariant();
}

QModelIndex IngredientModel::index(int row, int column, const QModelIndex &parent) const
{
  if (!parent.isValid()) {
    return createIndex(row, column, (row + 1) * 10000);
  };
  if (!is_ingredient(parent)) {
    intptr_t id = (intptr_t)parent.internalPointer();
    return createIndex(row, column, id + row + 1);
  };
  return QModelIndex();
}

QModelIndex IngredientModel::parent(const QModelIndex &index) const
{
  if (!index.isValid())
    return QModelIndex();
  intptr_t id = (intptr_t)index.internalPointer();
  if (!is_ingredient(index)) {
    // Parent of section is root node.
    return QModelIndex();
  };
  // Parent of ingredient is ingredient section.
  int section = id / 10000 - 1;
  return createIndex(section, 0, (section + 1) * 10000);
}

bool IngredientModel::is_ingredient(const QModelIndex &index) const {
  intptr_t id = (intptr_t)index.internalPointer();
  return id % 10000 != 0;
}

int IngredientModel::ingredient_index(const QModelIndex &index) const {
  intptr_t id = (intptr_t)index.internalPointer();
  int section = id / 10000 - 1;
  int offset = section == 0 ? 0 : m_sections[section - 1].first;
  return offset + index.row();
}

Ingredient IngredientModel::get_ingredient(const QModelIndex &index) const {
  return m_ingredients[ingredient_index(index)];
}

void IngredientModel::set_ingredient(const QModelIndex &index, Ingredient &ingredient) {
  m_ingredients[ingredient_index(index)] = ingredient;
  int row = index.row();
  emit dataChanged(index.sibling(row, 0), index.sibling(row, 2));
}

QModelIndex IngredientModel::add_ingredient(const QModelIndex &idx, Ingredient &ingredient) {
  int row;
  int pos;
  QModelIndex p;
  if (!idx.isValid())
    return idx;
  if (is_ingredient(idx)) {
    p = parent(idx);
    pos = ingredient_index(idx) + 1;
    row = idx.row() + 1;
  } else {
    p = idx.sibling(idx.row(), 0);
    pos = idx.row() == 0 ? 0 : m_sections[idx.row() - 1].first;
    row = 0;
  };
  beginInsertRows(p, row, row);
  m_ingredients.insert(m_ingredients.begin() + pos, ingredient);
  for (int i=1; i!=(signed)m_sections.size() + 1; i++) {
    if (i > p.row()) {
      m_sections[i - 1].first++;
    };
  };
  endInsertRows();
  return index(row, 0, p);
}

QModelIndex IngredientModel::delete_ingredient(const QModelIndex &idx) {
  if (!idx.isValid())
    return idx;
  QModelIndex p = parent(idx);
  if (is_ingredient(idx)) {
    // Remove ingredient line.
    beginRemoveRows(p, idx.row(), idx.row());
    int pos = ingredient_index(idx);
    int count = rowCount(p) - 1;
    m_ingredients.erase(m_ingredients.begin() + pos);
    for (int i=1; i!=(signed)m_sections.size() + 1; i++) {
      if (i > p.row()) {
        m_sections[i - 1].first--;
      };
    };
    endRemoveRows();
    if (idx.row() > 0)
      return index(idx.row() - 1, 0, p);
    else if (count > 0) {
      return index(0, 0, p);
    } else
      return p;
  } else {
    unsigned int section = idx.row();
    if (section > 0) {
      // Remove an ingredient section.
      beginRemoveRows(p, section, section);
      int a = m_sections[section - 1].first;
      int b = section == m_sections.size() ? m_ingredients.size() : m_sections[section].first;
      m_ingredients.erase(m_ingredients.begin() + a, m_ingredients.begin() + b);
      m_sections.erase(m_sections.begin() + section - 1);
      for (unsigned int i=1; i!=m_sections.size() + 1; i++) {
        if (i >= section) {
          m_sections[i - 1].first -= b - a;
        };
      };
      endRemoveRows();
      return index(idx.row() - 1, 0, p);
    };
  };
  return QModelIndex();
}

string IngredientModel::get_ingredient_section(const QModelIndex &index) const {
  int section = index.row();
  if (section == 0)
    return tr("<Main>").toUtf8().constData();
  else
    return m_sections[section - 1].second;
}

void IngredientModel::set_ingredient_section(const QModelIndex &index, const char *text) {
  int section = index.row();
  if (section > 0) {
    m_sections[section - 1].second = text;
    emit dataChanged(index.sibling(section, 0), index.sibling(section, 2));
  };
}
