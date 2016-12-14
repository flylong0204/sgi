// kate: syntax C++11;
// SGI - Copyright (C) 2012-2015 FAST Protect, Andreas Roth

#include <QTreeWidget>
#include <QHeaderView>
#include <QMenu>

#include <sgi/plugins/ObjectTreeImpl>
#include <sgi/plugins/ContextMenu>
#include <sgi/helpers/qt>
#include <sgi/plugins/SGIHostItemInternal.h>
#include <sgi/SGIItemInternal>

#include "ObjectTreeImplProxy.h"
#include "ObjectTreeImplProxy_p.h"
#include <ObjectTreeImplProxy.moc>

namespace sgi {

using namespace qt_helpers;

ObjectTreeImplProxyPrivate::ObjectTreeImplProxyPrivate(QTreeWidget * widget, IObjectTreeImpl * impl, SGIPluginHostInterface * hostInterface)
    : _widget(widget)
    , _impl(impl)
    , _hostInterface(hostInterface)
    , _selectedTreeItem(NULL)
{
}

ObjectTreeImplProxyPrivate::~ObjectTreeImplProxyPrivate()
{
}

ObjectTreeImplProxy::ObjectTreeImplProxy(QTreeWidget * widget, IObjectTreeImpl * impl, SGIPluginHostInterface * hostInterface)
    : QObject(widget)
    , d_ptr(new ObjectTreeImplProxyPrivate(widget, impl, hostInterface))
{
    Q_D(ObjectTreeImplProxy);
    d->_widget->setContextMenuPolicy(Qt::CustomContextMenu);
    QObject::connect(d->_widget, &QTreeWidget::itemExpanded, this, &ObjectTreeImplProxy::onItemExpanded);
    QObject::connect(d->_widget, &QTreeWidget::itemCollapsed, this, &ObjectTreeImplProxy::onItemCollapsed);
    QObject::connect(d->_widget, &QTreeWidget::itemActivated, this, &ObjectTreeImplProxy::onItemActivated);
    QObject::connect(d->_widget, &QTreeWidget::customContextMenuRequested, this, &ObjectTreeImplProxy::onItemContextMenu);
    QObject::connect(d->_widget, &QTreeWidget::itemClicked, this, &ObjectTreeImplProxy::onItemClicked);
    QObject::connect(d->_widget, &QTreeWidget::itemSelectionChanged, this, &ObjectTreeImplProxy::onItemSelectionChanged);
}

ObjectTreeImplProxy::~ObjectTreeImplProxy()
{
    delete d_ptr;
}

IObjectTreeItem * ObjectTreeImplProxy::selectedTreeItem()
{
    Q_D(ObjectTreeImplProxy);
    return d->_selectedTreeItem.get();
}

bool ObjectTreeImplProxy::buildTree(IObjectTreeItem * treeItem, SGIItemBase * item, bool addInternal)
{
    Q_D(ObjectTreeImplProxy);
    bool ret = d->_hostInterface->objectTreeBuildTree(treeItem, item);

    if(addInternal)
    {
        // in any case mark the item as populated and add the internal item as well.
        // We ignore the result from any plugin here, because some plugin be not be
        // functioning as expected or no plugin might be available for this item. The
        // internal item is useful especially in these cases to investigate why the
        // call to build tree failed.
        InternalItemData internalItemData(item);;
        SGIHostItemInternal hostItemInternal(new ReferencedInternalItemData(internalItemData));
        treeItem->addChild("Internal", &hostItemInternal);
    }

    QTreeWidgetItem * treeItemQt = static_cast<ObjectTreeItem*>(treeItem)->treeItem();
    QtSGIItem itemData = treeItemQt->data(0, Qt::UserRole).value<QtSGIItem>();
    itemData.markAsPopulated();
    treeItemQt->setData(0, Qt::UserRole, QVariant::fromValue(itemData));

    // but return the result of the buildTree call to the user
    return ret;
}

void ObjectTreeImplProxy::onItemExpanded(QTreeWidgetItem * item)
{
    Q_D(ObjectTreeImplProxy);
    QtSGIItem itemData = item->data(0, Qt::UserRole).value<QtSGIItem>();
    if(!itemData.isPopulated() && itemData.hasItem())
    {
        // we are going to re-populate the item with new data,
        // so first remove the old dummy child item.
        QList<QTreeWidgetItem *> children = item->takeChildren();
        Q_FOREACH(QTreeWidgetItem * child, children)
        {
            delete child;
        }
        ObjectTreeItem treeItem(item, d->_hostInterface);
        buildTree(&treeItem, itemData.item());
        d->_impl->itemExpanded(&treeItem);
    }
    else
    {
        ObjectTreeItem treeItem(item, d->_hostInterface);
        d->_impl->itemExpanded(&treeItem);
    }
}

void ObjectTreeImplProxy::onItemCollapsed(QTreeWidgetItem * item)
{
    Q_D(ObjectTreeImplProxy);
    ObjectTreeItem treeItem(item, d->_hostInterface);
    d->_impl->itemCollapsed(&treeItem);
}

void ObjectTreeImplProxy::onItemClicked(QTreeWidgetItem * item, int column)
{
    Q_D(ObjectTreeImplProxy);
    ObjectTreeItem treeItem(item, d->_hostInterface);
    d->_impl->itemClicked(&treeItem);
}

void ObjectTreeImplProxy::onItemActivated(QTreeWidgetItem * item, int column)
{
    Q_D(ObjectTreeImplProxy);
    ObjectTreeItem treeItem(item, d->_hostInterface);
    d->_impl->itemActivated(&treeItem);
}

void ObjectTreeImplProxy::onItemContextMenu(const QPoint & pt)
{
    Q_D(ObjectTreeImplProxy);
    QTreeWidgetItem * item = d->_widget->itemAt (pt);
    QtSGIItem itemData;
    if (item)
    {
        itemData = item->data(0, Qt::UserRole).value<QtSGIItem>();

        QMenu * contextMenu = NULL;
        ObjectTreeItem treeItem(item, d->_hostInterface);

        IContextMenuPtr objectMenu;
        d->_impl->itemContextMenu(&treeItem, objectMenu);

        if (objectMenu.valid())
            contextMenu = objectMenu->getMenu();

        if (contextMenu)
        {
            QPoint tmp = pt;
            tmp.ry() += d->_widget->header()->height();
            QPoint globalPos = d->_widget->mapToGlobal(tmp);
            contextMenu->popup(globalPos);
        }
    }
}

void ObjectTreeImplProxy::onItemSelectionChanged()
{
    Q_D(ObjectTreeImplProxy);
    QTreeWidgetItem * item = d->_widget->currentItem();
    IObjectTreeItemPtr oldItem = d->_selectedTreeItem;
    if(item)
    {
        QtSGIItem itemData = item->data(0, Qt::UserRole).value<QtSGIItem>();
        d->_selectedTreeItem = new ObjectTreeItem(item, d_ptr->_hostInterface);
    }
    else
        d->_selectedTreeItem = NULL;
    d->_impl->itemSelected(oldItem.get(), d->_selectedTreeItem.get());
}

void ObjectTreeImplProxy::reloadSelectedItem()
{
    Q_D(ObjectTreeImplProxy);
    QTreeWidgetItem * item = d->_selectedTreeItem.valid()?((ObjectTreeItem*)d->_selectedTreeItem.get())->treeItem():NULL;
    if(item)
    {
        QtSGIItem itemData = item->data(0, Qt::UserRole).value<QtSGIItem>();
        if(itemData.hasItem())
        {
            // we are going to re-populate the item with new data,
            // so first remove the old dummy child item.
            QList<QTreeWidgetItem *> children = item->takeChildren();
            Q_FOREACH(QTreeWidgetItem * child, children)
            {
                delete child;
            }
        }
        ObjectTreeItem treeItem(item, d->_hostInterface);
        buildTree(&treeItem, itemData.item());
    }
}

ObjectTreeItem::ObjectTreeItem (QTreeWidget * widget, IObjectTreeImpl * impl, SGIPluginHostInterface * hostInterface)
    : IObjectTreeItem()
    , _hostInterface(hostInterface)
    , _item(widget->invisibleRootItem())
{
    new ObjectTreeImplProxy(widget, impl, hostInterface);
}

ObjectTreeItem::ObjectTreeItem (QTreeWidgetItem * item, SGIPluginHostInterface * hostInterface)
    : IObjectTreeItem()
    , _hostInterface(hostInterface)
    , _item(item)
{
}

ObjectTreeItem::ObjectTreeItem(const ObjectTreeItem & item)
    : IObjectTreeItem(item)
    , _hostInterface(item._hostInterface)
    , _item(item._item)
{
}

ObjectTreeItem::~ObjectTreeItem()
{
}


IObjectTreeItem * ObjectTreeItem::root()
{
    QTreeWidget * widget = _item->treeWidget();
    if(widget)
        return new ObjectTreeItem(widget->invisibleRootItem(), _hostInterface);
    else
        return NULL;
}

IObjectTreeItem * ObjectTreeItem::parent()
{
    QTreeWidgetItem * parent = _item->parent();
    if(parent)
        return new ObjectTreeItem(parent, _hostInterface);
    else
        return NULL;
}

void ObjectTreeItem::clear()
{
    QList<QTreeWidgetItem*> childs = _item->takeChildren();
    foreach(QTreeWidgetItem* child, childs)
        delete child;
}

IObjectTreeItem * ObjectTreeItem::addChild(const std::string & name, SGIItemBase * item)
{
    return addChildImpl(name, item);
}

IObjectTreeItem * ObjectTreeItem::addChild(const std::string & name, const SGIHostItemBase * hostitem)
{
    osg::ref_ptr<SGIItemBase> item;
    if(_hostInterface->generateItem(item, hostitem))
        return addChildImpl(name, item.get());
    else
        return NULL;
}
IObjectTreeItem * ObjectTreeItem::findChild(const std::string & name)
{
    QString qname = fromLocal8Bit(name);
    QTreeWidgetItem * retitem = NULL;
    for(int n = _item->childCount() - 1; !retitem && n >= 0; n--)
    {
        QTreeWidgetItem * child = _item->child(n);
        if(child->text(0) == qname)
            retitem = child;
    }
    if(retitem)
        return new ObjectTreeItem(retitem, _hostInterface);
    else
        return NULL;
}

IObjectTreeItem * ObjectTreeItem::addChildIfNotExists(const std::string & name, SGIItemBase * item)
{
    IObjectTreeItem * ret = findChild(name);
    if(!ret)
        ret = addChild(name, item);
    return ret;
}

IObjectTreeItem * ObjectTreeItem::addChildIfNotExists(const std::string & name, const SGIHostItemBase * hostitem)
{
    IObjectTreeItem * ret = findChild(name);
    if(!ret)
        ret = addChild(name, hostitem);
    return ret;
}

void ObjectTreeItem::setSelected(bool select)
{
    _item->setSelected(select);
}
bool ObjectTreeItem::isSelected() const
{
    return _item->isSelected();
}

SGIItemBase * ObjectTreeItem::item()
{
    QtSGIItem itemData = _item->data(0, Qt::UserRole).value<QtSGIItem>();
    return itemData.item();
}
void ObjectTreeItem::expand()
{
    _item->setExpanded(true);
}

void ObjectTreeItem::collapse()
{
    _item->setExpanded(false);
}

QTreeWidgetItem * ObjectTreeItem::treeItem() { return _item; }
const QTreeWidgetItem * ObjectTreeItem::treeItem() const { return _item; }

IObjectTreeItem * ObjectTreeItem::addChildImpl(const std::string & name, SGIItemBase * item)
{
    QTreeWidgetItem * newItem = new QTreeWidgetItem;
    QtSGIItem itemData(item);
    QString itemText;
    QString itemTypeText;
    if(name.empty() && item)
    {
        std::string displayName;
        _hostInterface->getObjectDisplayName(displayName, item);
        itemText = fromLocal8Bit(displayName);
    }
    else
        itemText = fromLocal8Bit(name);
    std::string typeName;
    if(item)
    {
        _hostInterface->getObjectTypename(typeName, item);
        itemTypeText = fromLocal8Bit(typeName);
    }

    newItem->setText(0, itemText);
    newItem->setText(1, itemTypeText);
    newItem->setData(0, Qt::UserRole, QVariant::fromValue(itemData));
    addDummyChild(newItem);
    if(_item->childCount() == 1)
    {
        // if there's a dummy item in place at parent remove it first
        QTreeWidgetItem * firstChild = _item->child(0);
        if(firstChild)
        {
            QtSGIItem itemData = firstChild->data(0, Qt::UserRole).value<QtSGIItem>();
            if(itemData.type() == SGIItemTypeDummy)
                _item->removeChild(firstChild);
        }
    }
    // ... and finally add the new item to the tree
    _item->addChild(newItem);
    return new ObjectTreeItem(newItem, _hostInterface);
}

void ObjectTreeItem::addDummyChild(QTreeWidgetItem * itemParent)
{
    QTreeWidgetItem * dummyChild = new QTreeWidgetItem;
    QtSGIItem dummyData(SGIItemTypeDummy);
    dummyChild->setText(0, "dummy");
    dummyChild->setData(0, Qt::UserRole, QVariant::fromValue(dummyData));
    itemParent->addChild(dummyChild);
}

void ObjectTreeItem::children(IObjectTreeItemPtrList & children)
{
    int numChilds = _item->childCount();
    children.resize(numChilds);
    for(int i = 0; i < numChilds; i++)
    {
        QTreeWidgetItem * child = _item->child(i);
        children[i] = new ObjectTreeItem(child, _hostInterface);
    }
}

IObjectTreeItem * ObjectTreeItem::selectedItem()
{
    if(!_item)
        return NULL;
    QTreeWidgetItem * selitem = _item->treeWidget()->currentItem();
    return new ObjectTreeItem(selitem, _hostInterface);
}

void ObjectTreeItem::reload()
{
    if(_item)
    {
        QtSGIItem itemData = _item->data(0, Qt::UserRole).value<QtSGIItem>();
        if(itemData.hasItem())
        {
            // we are going to re-populate the item with new data,
            // so first remove the old dummy child item.
            QList<QTreeWidgetItem *> children = _item->takeChildren();
            Q_FOREACH(QTreeWidgetItem * child, children)
            {
                delete child;
            }
        }
        itemData.markAsUnpopulated();
        _item->setData(0, Qt::UserRole, QVariant::fromValue(itemData));
        _item->setExpanded(false);
        addDummyChild(_item);
    }
}

void ObjectTreeItem::setName(const std::string & name)
{
    _item->setText(0, fromLocal8Bit(name));
}

void ObjectTreeItem::updateName()
{
    std::string displayName;
    _hostInterface->getObjectDisplayName(displayName, item());
    _item->setText(0, fromLocal8Bit(displayName));
}

} // namespace sgi
