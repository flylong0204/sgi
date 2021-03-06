#pragma once

#include <QDialog>
#include <sgi/plugins/SGIPluginInterface.h>

QT_BEGIN_NAMESPACE
class Ui_TileInspectorDialog;
class QTreeWidgetItem;
QT_END_NAMESPACE

namespace osgEarth {
    class TileSource;
    class TerrainLayer;
    class TileKey;
}

namespace sgi {
class SGIPluginHostInterface;
class SGIItemOsg;

class IObjectTreeItem;
typedef osg::ref_ptr<IObjectTreeItem> IObjectTreeItemPtr;

class IObjectTreeImpl;
typedef osg::ref_ptr<IObjectTreeImpl> IObjectTreeImplPtr;
class IContextMenu;
typedef osg::ref_ptr<IContextMenu> IContextMenuPtr;

namespace osgearth_plugin {

typedef std::list<osgEarth::TileKey> TileKeyList;
typedef std::set<osgEarth::TileKey> TileKeySet;

class TileInspectorDialog : public QDialog
{
	Q_OBJECT

public:
                                TileInspectorDialog(QWidget * parent, SGIItemOsg * item, ISettingsDialogInfo * info=NULL, SGIPluginHostInterface * hostInterface=NULL);
	virtual				        ~TileInspectorDialog();

public:
    ISettingsDialog *           dialogInterface() { return _interface; }

public slots:
    void                        refresh();
    void                        layerChanged(int index);
    void                        layerSourceChanged(int index);
    void                        proxySaveScript();
    void                        reloadTree();
    void                        updateMetaData();
    void                        loadData();
    void                        reloadSelectedItem();
	void                        takePositionFromCamera();
    void                        loadFromFile();
    void                        takeFromDataExtents();
    void                        takeFromDataExtentsUnion();
    
public:
    enum LAYER_DATA_SOURCE
    {
        LayerDataSourceLayer,
        LayerDataSourceTileSource,
        LayerDataSourceCache,
    };
    enum NUM_NEIGHBORS
    {
        NUM_NEIGHBORS_NONE = 0,
        NUM_NEIGHBORS_CROSS,
        NUM_NEIGHBORS_IMMEDIATE,
		NUM_NEIGHBORS_PARENTAL,
		NUM_NEIGHBORS_PARENTAL_AND_CHILDS,
        NUM_NEIGHBORS_CHILDS,
    };
    void                    addTileKey(const osgEarth::TileKey & key);
    void                    addTileKeys(osgEarth::TileSource * tileSource, const TileKeyList & tiles, bool append=false);
    void                    addTileKeys(osgEarth::TileSource * tileSource, const TileKeySet & tiles, bool append=false);
    void                    addTileKeys(const TileKeyList & tiles, bool append=false);
    void                    addTileKeys(const TileKeySet & tiles, bool append=false);
protected:
    void                    triggerRepaint();
    SGIItemBase *           getView();

    void                    setNodeInfo(const SGIItemBase * item);

    void                    itemContextMenu(IObjectTreeItem * treeItem, IContextMenuPtr & contextMenu);

protected:
    class ObjectTreeImpl;

private:
	Ui_TileInspectorDialog *	    ui;
    SGIPluginHostInterface *        _hostInterface;
    ISettingsDialogPtr _interface;
	ISettingsDialogInfoPtr          _info;
    IObjectTreeItemPtr              _treeRoot;
    IObjectTreeImplPtr              _treeImpl;
    IContextMenuPtr                 _contextMenu;
    osg::ref_ptr<SGIItemOsg>        _item;
    SGIItemBasePtrVector            _tiles;
};

} // namespace osgearth_plugin
} // namespace sgi
