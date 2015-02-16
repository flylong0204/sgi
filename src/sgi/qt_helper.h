#pragma once

#include "sgi/plugins/SGIDataItemBase.h"

namespace sgi {

class SGIItemBase;
class SGIDataItemBase;

typedef osg::ref_ptr<SGIItemBase> SGIItemBasePtr;
typedef osg::ref_ptr<SGIDataItemBase> SGIDataItemBasePtr;

class QtSGIItem
{
public:
    QtSGIItem(SGIItemType type=SGIItemTypeInvalid, SGIItemBase * item=NULL, bool populated=false, osg::Referenced * userData=NULL)
        : _type(type), _item(item), _populated(populated), _userData(userData) {}
    QtSGIItem(SGIItemBase * item, bool populated=false, osg::Referenced * userData=NULL)
        : _type(item?item->type():SGIItemTypeInvalid), _item(item), _populated(populated), _userData(userData) {}
    QtSGIItem(const QtSGIItem & rhs)
        : _type(rhs._type), _item(rhs._item), _populated(rhs._populated), _userData(rhs._userData) {}
    ~QtSGIItem()
        { _item = NULL; }

    QtSGIItem & operator = (const QtSGIItem & rhs)
    {
        _type = rhs._type;
        _item = rhs._item;
        _populated = rhs._populated;
        _userData = rhs._userData;
        return *this;
    }
    SGIItemType type() const { return _type; }
    SGIItemBase * item() const { return _item.get(); }

    bool hasItem() const { return _item.valid(); }

    bool isPopulated() const { return _populated; }
    void markAsPopulated() { _populated = true; }

    const osg::Referenced * userData() const { return _userData.get(); }
    osg::Referenced * userData() { return _userData.get(); }
    void setUserData(osg::Referenced * userData) { _userData = userData; }

private:
    SGIItemType             _type;
    SGIItemBasePtr          _item;
    bool                    _populated;
    osg::ref_ptr<osg::Referenced>   _userData;
};

class QtMenuSGIItem : public QtSGIItem
{
public:
    QtMenuSGIItem(SGIItemType type=SGIItemTypeInvalid, SGIItemBase * item=NULL, bool populated=false, osg::Referenced * userData=NULL)
        : QtSGIItem(type, item, populated, userData), _actionId(0), _mode(0) {}
    QtMenuSGIItem(SGIItemBase * item, bool populated=false, osg::Referenced * userData=NULL)
        : QtSGIItem(item, populated, userData), _actionId(0), _mode(0) {}
    QtMenuSGIItem(const QtMenuSGIItem & rhs)
        : QtSGIItem(rhs), _actionId(rhs._actionId), _mode(rhs._mode) {}
    ~QtMenuSGIItem()
        { }

    QtMenuSGIItem & operator = (const QtMenuSGIItem & rhs)
    {
        QtSGIItem::operator=(rhs);
        _actionId = rhs._actionId;
        _mode = rhs._mode;
        return *this;
    }

    unsigned actionId() const { return _actionId; }
    void setActionId(unsigned actionId) { _actionId = actionId; }

    unsigned mode() const { return _mode; }
    void setMode(unsigned mode) { _mode = mode; }

private:
    unsigned                _actionId;
    unsigned                _mode;
};

class QtTableSGIItem
{
public:
    QtTableSGIItem(SGIDataItemBase * item=NULL, unsigned field=0)
        : _item(item), _field(field) {}
    QtTableSGIItem(const QtTableSGIItem & rhs)
        : _item(rhs._item), _field(rhs._field) {}
    ~QtTableSGIItem()
        { }

    QtTableSGIItem & operator = (const QtTableSGIItem & rhs)
    {
        _item = rhs._item;
        _field = rhs._field;
        return *this;
    }

    SGIDataItemBase * item() const { return _item.get(); }
    bool hasItem() const { return _item.valid(); }

private:
    SGIDataItemBasePtr  _item;
    unsigned            _field;
};

inline QString fromLocal8Bit(const std::string & str)
{
    return QString::fromLocal8Bit(str.data(), str.size());
}

inline std::string toLocal8Bit(const QString & str)
{
    QByteArray qba = str.toLocal8Bit();
    return std::string(qba.constData(), qba.size());
}

} // namespace sgi

Q_DECLARE_METATYPE (sgi::QtSGIItem);
Q_DECLARE_METATYPE (sgi::QtMenuSGIItem);
Q_DECLARE_METATYPE (sgi::QtTableSGIItem);