#pragma once
#include <qdockwidget.h>
#include <string>

struct AssetDock : public QDockWidget
{
    struct QFileSystemModel* fileSystemModel;
    struct QTreeView* assetTreeView;
    struct QListWidget* assetIcons;

    AssetDock();
    void AssetItemClicked();
    void AssetFolderClicked();
    void SpawnMeshActor(std::string meshFilename);
};
