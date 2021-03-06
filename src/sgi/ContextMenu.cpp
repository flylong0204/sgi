#include "stdafx.h"
#include "ContextMenu.h"

#include "sgi/plugins/ContextMenu"
#include "sgi/plugins/SGIPluginInterface.h"

#include "SGIPlugin.h"

#include <sgi/plugins/SGIHostItemQt.h>
#include <sgi/helpers/qt>

#include <QDebug>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace sgi {

using namespace qt_helpers;

ContextMenuBase::ContextMenuBase(QWidget *parent)
    : QMenu(parent)
{
}

ContextMenuBase::~ContextMenuBase()
{
}

void ContextMenuBase::hideEvent(QHideEvent *event)
{
    QMenu::hideEvent(event);
    emit hidden();
}

class ContextMenu::ContextMenuItem : public IContextMenuItem
{
public:
    ContextMenuItem(ContextMenu * contextMenu, QMenu * menu, QActionGroup * actionGroup=NULL)
        : _contextMenu(contextMenu), _menu(menu), _actionGroup(actionGroup)
    {
    }
    ContextMenuItem(const ContextMenuItem & item)
        : _contextMenu(item._contextMenu), _menu(item._menu), _actionGroup(item._actionGroup), _childs(item._childs)
    {
    }

    virtual ~ContextMenuItem()
    {
    }

    void clear()
    {
        _menu->clear();
        _childs.clear();
    }
    void addSeparator()
    {
        _menu->addSeparator();
    }
    bool addSimpleAction(unsigned actionId, const std::string & name, const SGIHostItemBase * hostitem, osg::Referenced * userData=NULL)
    {
        osg::ref_ptr<SGIItemBase> item;
        if(SGIPlugins::instance()->generateItem(item, hostitem))
            return addSimpleAction(actionId, name, item.get(), userData);
        else
            return false;
    }

    bool addSimpleAction(unsigned actionId, const std::string & name, SGIItemBase * item, osg::Referenced * userData=NULL)
    {
        QString itemText;
        if(name.empty())
        {
            std::string displayName;
            SGIPlugins::instance()->getObjectDisplayName(displayName, item);
            itemText = fromUtf8(displayName);
        }
        else
            itemText = fromUtf8(name);
        QtMenuSGIItem itemData(item);
        itemData.setActionId(actionId);
        itemData.setUserData(userData);

        QAction * action = new QAction(itemText, _menu);
        action->setData(QVariant::fromValue(itemData));
        _menu->addAction(action);
        connect(action, &QAction::triggered, _contextMenu, &ContextMenu::slotSimpleItemAction);
        return action;
    }
    bool addBoolAction(unsigned actionId, const std::string & name, SGIItemBase * item, bool state, osg::Referenced * userData=NULL)
    {
        QString itemText;
        if(name.empty())
        {
            std::string displayName;
            SGIPlugins::instance()->getObjectDisplayName(displayName, item);
            itemText = fromUtf8(displayName);
        }
        else
            itemText = fromUtf8(name);
        QtMenuSGIItem itemData(item);
        itemData.setActionId(actionId);
        itemData.setUserData(userData);

        QAction * action = new QAction(itemText, _menu);
        action->setData(QVariant::fromValue(itemData));
        action->setCheckable(true);
        action->setChecked(state);
        _menu->addAction(action);
        connect(action, &QAction::triggered, _contextMenu, &ContextMenu::slotBoolItemAction);
        return action;
    }

    bool addModeAction(const std::string & name, int mode, osg::Referenced * userData=NULL)
    {
        QString itemText = fromUtf8(name);
        QtMenuSGIItem menuItemData = _menu->menuAction()->data().value<QtMenuSGIItem>();
        QtMenuSGIItem itemData(menuItemData);
        int currentMode = menuItemData.mode();
        itemData.setActionId(menuItemData.actionId());
        itemData.setMode(mode);
        itemData.setUserData(userData);

        QAction * action = new QAction(itemText, _menu);
        action->setActionGroup(_actionGroup);
        action->setData(QVariant::fromValue(itemData));
        action->setCheckable(true);
        action->setChecked(mode == currentMode);
        _menu->addAction(action);
        return action;
    }

    IContextMenuItem * addMenu(const std::string & name, SGIItemBase * item, osg::Referenced * userData=NULL)
    {
        return addMenuImpl(name, item, userData);
    }

    IContextMenuItem * addMenu(const std::string & name, const SGIHostItemBase * hostitem, osg::Referenced * userData=NULL)
    {
        osg::ref_ptr<SGIItemBase> item;
        if(SGIPlugins::instance()->generateItem(item, hostitem))
            return addMenuImpl(name, item.get(), userData);
        else
            return NULL;
    }

    IContextMenuItem * addModeMenu(unsigned actionId, const std::string & name, SGIItemBase * item, int currentMode, osg::Referenced * userData=NULL)
    {
        return addModeMenuImpl(actionId, name, item, currentMode, userData);
    }

    IContextMenuItem * getOrCreateMenu(const std::string & name, osg::Referenced * userData=NULL)
    {
        QMenu * menu = getOrCreateNamedMenu(_menu, fromUtf8(name), userData);
        return addChild(new ContextMenuItem(_contextMenu, menu));
    }

    IContextMenu * menu() override
    {
        return _contextMenu->menuInterface();
    }

protected:
    IContextMenuItem * addChild(ContextMenuItem * child)
    {
        _childs.push_back(child);
        return child;
    }
    IContextMenuItem * addMenuImpl(const std::string & name, SGIItemBase * item, osg::Referenced * userData)
    {
        ContextMenuBase * newMenu = new ContextMenuBase(_menu);
        QtMenuSGIItem itemData(item);
        itemData.setUserData(userData);
        QString itemText;
        if(name.empty())
        {
            std::string displayName;
            SGIPlugins::instance()->getObjectDisplayName(displayName, item);
            itemText = fromUtf8(displayName);
        }
        else
            itemText = fromUtf8(name);

        newMenu->setTitle(itemText);
        newMenu->menuAction()->setData(QVariant::fromValue(itemData));
        Q_VERIFY(connect(newMenu, &QMenu::aboutToShow, _contextMenu, &ContextMenu::slotPopulateItemMenu));
        Q_VERIFY(connect(newMenu, &ContextMenuBase::hidden, _contextMenu, &ContextMenu::slotClearItemMenu, Qt::QueuedConnection));
        // ... and finally add the new sub-menu to the menu
        _menu->addMenu(newMenu);
        return addChild(new ContextMenuItem(_contextMenu, newMenu));
    }

    IContextMenuItem * addModeMenuImpl(unsigned actionId, const std::string & name, SGIItemBase * item, int currentMode, osg::Referenced * userData)
    {
        QMenu * newMenu = new QMenu(_menu);
        QtMenuSGIItem itemData(item);
        itemData.setActionId(actionId);
        itemData.setMode(currentMode);
        itemData.setUserData(userData);
        QString itemText;
        if(name.empty())
        {
            std::string displayName;
            SGIPlugins::instance()->getObjectDisplayName(displayName, item);
            itemText = fromUtf8(displayName);
        }
        else
            itemText = fromUtf8(name);

        QActionGroup * actionGroup = new QActionGroup(newMenu);
        actionGroup->setExclusive(true);
        Q_VERIFY(connect(actionGroup, &QActionGroup::triggered, _contextMenu, &ContextMenu::slotActionGroup));

        newMenu->setTitle(itemText);
        newMenu->menuAction()->setData(QVariant::fromValue(itemData));
        // ... and finally add the new sub-menu to the menu
        _menu->addMenu(newMenu);
        return addChild(new ContextMenuItem(_contextMenu, newMenu, actionGroup));
    }

    static QMenu * getOrCreateNamedMenu(QMenu * menu, const QString & name, osg::Referenced * userData=NULL)
    {
        QMenu * ret = NULL;
        QList<QAction*> actions = menu->actions();
        for(QAction * action : actions)
        {
            QMenu * submenu = action->menu();
            if(submenu && action->text() == name)
            {
                ret = submenu;
                break;
            }
        }
        if(!ret)
        {
            ret = menu->addMenu(name);
            if(ret)
            {
                QtMenuSGIItem itemData;
                itemData.setUserData(userData);
                ret->menuAction()->setData(QVariant::fromValue(itemData));
            }
        }
        return ret;
    }

private:
    ContextMenu * _contextMenu;
    QMenu * _menu;
    QActionGroup * _actionGroup;
    typedef std::vector<IContextMenuItemPtr> IContextMenuItemPtrVector;
    IContextMenuItemPtrVector _childs;
};

class ContextMenu::ContextMenuAction : public IContextMenuAction
{
public:
    ContextMenuAction(ContextMenu * contextMenu, QAction * action, bool state=false)
        : _contextMenu(contextMenu), _action(action), _state(state)
    {
    }
    ContextMenuAction(const ContextMenuAction & rhs)
        : _contextMenu(rhs._contextMenu), _action(rhs._action), _state(rhs._state)
    {
    }

    virtual SGIItemBase * item()
    {
        QtMenuSGIItem itemData = _action->data().value<QtMenuSGIItem>();
        return itemData.item();
    }
    virtual unsigned actionId()
    {
        QtMenuSGIItem itemData = _action->data().value<QtMenuSGIItem>();
        return itemData.actionId();
    }
    virtual unsigned mode()
    {
        QtMenuSGIItem itemData = _action->data().value<QtMenuSGIItem>();
        return itemData.mode();
    }

    virtual bool state()
    {
        return _state;
    }

    virtual IContextMenu * menu()
    {
        return _contextMenu->menuInterface();
    }
    virtual osg::Referenced * userData()
    {
        QtMenuSGIItem itemData = _action->data().value<QtMenuSGIItem>();
        return itemData.userData();
    }
    virtual const osg::Referenced * userData() const
    {
        QtMenuSGIItem itemData = _action->data().value<QtMenuSGIItem>();
        return itemData.userData();
    }
    virtual osg::Referenced * modeUserData()
    {
        QMenu * menu = dynamic_cast<QMenu*>(_action->parent());
        QAction * menuAction = menu?menu->menuAction():NULL;

        QtMenuSGIItem menuItemData;
        if(menuAction)
            menuItemData = menuAction->data().value<QtMenuSGIItem>();
        return menuItemData.userData();
    }
    virtual const osg::Referenced * modeUserData() const
    {
        QMenu * menu = dynamic_cast<QMenu*>(_action->parent());
        QAction * menuAction = menu?menu->menuAction():NULL;

        QtMenuSGIItem menuItemData;
        if(menuAction)
            menuItemData = menuAction->data().value<QtMenuSGIItem>();
        return menuItemData.userData();
    }

private:
    ContextMenu * _contextMenu;
    QAction * _action;
    bool _state;
};

class ContextMenu::ContextMenuImpl : public IContextMenu
{
public:
    ContextMenuImpl(ContextMenu * menu)
        : _menu(menu) {}
    ~ContextMenuImpl()
        {
            //qDebug() << "ContextMenuImpl dtor" << _menu;
            if(_menu)
            {
                _menu->_interface = NULL;
                delete _menu;
            }
        }

    virtual QWidget *               parentWidget() override { return _menu->parentWidget(); }
    virtual QMenu *                 getMenu() override { return _menu; }
    virtual void                    setObject(SGIItemBase * item, IHostCallback * callback=NULL) override { _menu->setObject(item, callback); }
    virtual void                    setObject(const SGIHostItemBase * item, IHostCallback * callback=NULL) override { _menu->setObject(item, callback); }
    virtual IHostCallback *         getHostCallback() override { return _menu->getHostCallback(); }
    virtual void                    popup(QWidget * parent, int x, int y) { emit _menu->triggerPopup(parent, x, y); }

    ContextMenu * _menu;
};

ContextMenu::ContextMenu(bool onlyRootItem, QWidget * parent)
    : ContextMenuBase(parent)
    , _interface(new ContextMenuImpl(this))
    , _item()
    , _hostCallback(NULL)
    , _onlyRootItem(onlyRootItem)
    , _donotClearItem(false)
{
    Q_VERIFY(connect(this, &QMenu::aboutToShow, this, &ContextMenu::slotPopulateItemMenu));
    Q_VERIFY(connect(this, &ContextMenuBase::hidden, this, &ContextMenu::slotClearItemMenu, Qt::QueuedConnection));
	Q_VERIFY(connect(this, &ContextMenu::triggerPopup, this, &ContextMenu::popup));
	Q_VERIFY(connect(this, &ContextMenu::triggerUpdateMenu, this, &ContextMenu::updateMenu));
}

ContextMenu::ContextMenu(SGIItemBase * item, IHostCallback* callback, bool onlyRootItem, QWidget *parent)
    : ContextMenuBase(parent)
    , _interface(new ContextMenuImpl(this))
    , _item(item)
    , _hostCallback(callback)
    , _onlyRootItem(onlyRootItem)
    , _donotClearItem(false)
{
    populate();
    Q_VERIFY(connect(this, &QMenu::aboutToShow, this, &ContextMenu::slotPopulateItemMenu));
    Q_VERIFY(connect(this, &ContextMenuBase::hidden, this, &ContextMenu::slotClearItemMenu, Qt::QueuedConnection));
	Q_VERIFY(connect(this, &ContextMenu::triggerPopup, this, &ContextMenu::popup));
	Q_VERIFY(connect(this, &ContextMenu::triggerUpdateMenu, this, &ContextMenu::updateMenu));
}

ContextMenu::~ContextMenu()
{
	_hostCallback = NULL;
    //qDebug() << "ContextMenu dtor" << _interface;
    if(_interface)
    {
        // tell interface that this instance is already gone, so no need to
        // delete again
        static_cast<ContextMenuImpl*>(_interface)->_menu = NULL;
        _interface = NULL;
    }
}

void ContextMenu::showSceneGraphDialog(SGIItemBase * item)
{
    if(_hostCallback)
        _hostCallback->showSceneGraphDialog(parentWidget(), item);
    else
        SGIPlugins::instance()->showSceneGraphDialog(parentWidget(), item, NULL);
}

void ContextMenu::populate()
{
    if(_item.valid())
    {
        std::string displayName;
        SGIPlugins::instance()->getObjectDisplayName(displayName, _item.get());
        setTitle(fromUtf8(displayName));

        QtMenuSGIItem data(_item);
        menuAction()->setData(QVariant::fromValue(data));
    }
    else
        menuAction()->setData(QVariant());
}

IHostCallback * ContextMenu::getHostCallback()
{
    return _hostCallback;
}

void ContextMenu::slotPopulateItemMenu()
{
    QMenu * menu = qobject_cast<QMenu *>(sender());
    if(menu)
    {
        QAction * action = menu->menuAction();
        QtMenuSGIItem itemData = action->data().value<QtMenuSGIItem>();
        if(itemData.hasItem())
        {
            ContextMenuItem menuItem(this, menu);
            menu->blockSignals(true);
            menu->clear();
            bool onlyRootItem = (menu == this)?_onlyRootItem:true;
            SGIPlugins::instance()->contextMenuPopulate(&menuItem, itemData.item(), onlyRootItem);
            menu->blockSignals(false);
        }
    }
}

void ContextMenu::slotClearItemMenu()
{
    QMenu * menu = qobject_cast<QMenu *>(sender());
    if(menu && qobject_cast<ContextMenu*>(menu) && !_donotClearItem)
    {
        //qDebug() << "slotClearItemMenu()" << this << menu;
        QAction * action = menu->menuAction();
        menu->clear();
        action->setData(QVariant());
        // release all references to the current item
        _item = NULL;
        _hostCallback = NULL;
    }
}

void ContextMenu::slotSimpleItemAction()
{
    QAction *action = qobject_cast<QAction *>(sender());
    if(action)
    {
        QtMenuSGIItem itemData = action->data().value<QtMenuSGIItem>();
        IContextMenuActionPtr menuAction = new ContextMenuAction(this, action);
        SGIPlugins::instance()->contextMenuExecute(menuAction, itemData.item());
    }
}

void ContextMenu::slotBoolItemAction(bool newState)
{
    QAction *action = qobject_cast<QAction *>(sender());
    if(action)
    {
        QtMenuSGIItem itemData = action->data().value<QtMenuSGIItem>();
        IContextMenuActionPtr menuAction = new ContextMenuAction(this, action, newState);
        SGIPlugins::instance()->contextMenuExecute(menuAction, itemData.item());
    }
}

void ContextMenu::slotActionGroup(QAction * action)
{
    if(action)
    {
        QtMenuSGIItem itemData = action->data().value<QtMenuSGIItem>();
        IContextMenuActionPtr menuAction = new ContextMenuAction(this, action);
        SGIPlugins::instance()->contextMenuExecute(menuAction, itemData.item());
    }
}

void ContextMenu::updateMenu()
{
	clear();
	populate();
}

void ContextMenu::setObject(SGIItemBase * item, IHostCallback * callback)
{
    _item = item;
    _hostCallback = callback;
	emit triggerUpdateMenu();
}

void ContextMenu::setObject(const SGIHostItemBase * hostitem, IHostCallback * callback)
{
    SGIItemBasePtr item;
    if(SGIPlugins::instance()->generateItem(item, hostitem))
        setObject(item.get(), callback);
}

void ContextMenu::popup(QWidget * parent, int x, int y)
{
    QPoint globalPos(x,y);
    if(parent)
    {
        globalPos = parent->mapToGlobal(globalPos);
    }
    else if(parentWidget())
    {
        globalPos = parentWidget()->mapToGlobal(globalPos);
    }
    QMenu::popup(globalPos);
}

class ContextMenuQt::ContextMenuQtImpl : public IContextMenuQt
{
public:
    ContextMenuQtImpl(ContextMenuQt * menu)
        : _menu(menu) {}
    virtual ~ContextMenuQtImpl()
        { 
			//qDebug() << "ContextMenuQtImpl::dtor" << _menu; 
            if (_menu)
            {
                _menu->_interface = NULL;
                delete _menu;
            }
		}

    virtual QMenu *                 getMenu() override { return _menu->getMenu(); }
    virtual IHostCallback *         getHostCallback() override { return _menu->getHostCallback(); }
    virtual void                    setObject(QObject * item, IHostCallback * callback=NULL) override { _menu->setObject(item, callback); }
    virtual QWidget *               parentWidget() override { return _menu->parentWidget(); }
    virtual void                    popup(QWidget * parent, int x, int y) override { emit _menu->popup(parent, x, y); }

    ContextMenuQt * _menu;
};

ContextMenuQt::ContextMenuQt(QObject * qobject, IHostCallback * callback, bool onlyRootItem, QWidget *parent)
    : QObject(parent)
    , _interface(new ContextMenuQtImpl(this))
    , _qobject(qobject)
    , _hostCallback(callback)
    , _onlyRootItem(onlyRootItem)
{
    SGIHostItemQt hostItem(qobject);
    _realMenu = callback->contextMenu(parent, &hostItem);
}

ContextMenuQt::~ContextMenuQt()
{
    //qDebug() << "ContextMenuQt dtor" << _interface;
    // release the real-menu @a ContextMenu object
    _realMenu = NULL;
	_hostCallback = NULL;
	if (_interface)
	{
		// tell interface that this instance is already gone, so no need to
		// delete again
		static_cast<ContextMenuQtImpl*>(_interface)->_menu = NULL;
		//qDebug() << "ContextMenuQt dtor delete " << _interface;
		delete _interface;
	}
}

IHostCallback * ContextMenuQt::getHostCallback()
{
    return _hostCallback;
}

QWidget * ContextMenuQt::parentWidget()
{
    if (_realMenu)
        return _realMenu->parentWidget();
    else
        return nullptr;
}

QMenu * ContextMenuQt::getMenu()
{
    if (_realMenu)
        return _realMenu->getMenu();
    else
        return nullptr;
}

void ContextMenuQt::setObject(QObject * qobject, IHostCallback * callback)
{
    _hostCallback = callback;
    if (_realMenu)
    {
        if (qobject)
        {
            SGIHostItemQt hostItem(qobject);
            _realMenu->setObject(&hostItem, callback);
        }
        else
            _realMenu->setObject((SGIItemBase*)NULL, callback);
    }
}

void ContextMenuQt::popup(QWidget * parent, int x, int y)
{
    //qDebug() << "ContextMenuQt::popup" << parent << x << y;
    if(_realMenu)
        _realMenu->popup(parent, x, y);
}

} // namespace sgi
