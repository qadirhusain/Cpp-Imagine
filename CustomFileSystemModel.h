#ifndef CUSTOMFILESYSTEMMODEL_H
#define CUSTOMFILESYSTEMMODEL_H

#include <QFileSystemModel>

class CustomFileSystemModel : public QFileSystemModel
{
    Q_OBJECT  // Required if you're emitting signals like layoutChanged

public:
    using QFileSystemModel::QFileSystemModel;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override {
        if (role == Qt::DecorationRole) {
            // Suppress default file/folder icons
            return QVariant();
        }
        return QFileSystemModel::data(index, role);
    }

    // Refreshes contents of a given index (e.g., the currently opened folder)
    void refresh(const QModelIndex &index = QModelIndex());
};

#endif // CUSTOMFILESYSTEMMODEL_H
