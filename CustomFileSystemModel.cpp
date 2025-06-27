#include "CustomFileSystemModel.h"

void CustomFileSystemModel::refresh(const QModelIndex &index) {
    if (!index.isValid()) return;

    // Emit dataChanged for children of this index to force UI refresh
    int rowCount = this->rowCount(index);
    if (rowCount > 0) {
        QModelIndex topLeft = this->index(0, 0, index);
        QModelIndex bottomRight = this->index(rowCount - 1, 0, index);
        emit dataChanged(topLeft, bottomRight);
        emit layoutChanged();
    }
}
