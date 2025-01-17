// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2023 Team Dissolve and contributors

#pragma once

#include "classes/species.h"
#include "templates/optionalRef.h"
#include <QAbstractTableModel>
#include <QModelIndex>

#include <vector>

class SpeciesModel : public QAbstractListModel
{
    Q_OBJECT

    private:
    // Source Species data
    OptionalReferenceWrapper<const std::vector<std::unique_ptr<Species>>> species_;
    // Vector containing checked items (if relevant)
    OptionalReferenceWrapper<std::vector<const Species *>> checkedItems_;
    // Return object represented by specified model index
    const Species *rawData(const QModelIndex &index) const;

    public:
    enum class SpeciesUserRole
    {
        RawData = Qt::UserRole,
        BondsCount,
        AnglesCount,
        TorsionsCount,
        ImpropersCount
    };
    Q_ENUM(SpeciesUserRole);

    public:
    // Set source Species data
    void setData(const std::vector<std::unique_ptr<Species>> &species);
    // Set vector containing checked items
    void setCheckStateData(std::vector<const Species *> &checkedItemsVector);
    // Refresh model data
    void reset();

    /*
     * QAbstractItemModel overrides
     */
    public:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
};
