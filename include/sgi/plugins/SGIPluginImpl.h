// kate: syntax C++11;
// SGI - Copyright (C) 2012-2015 FAST Protect, Andreas Roth

#pragma once

#include <osgDB/ReaderWriter>
#include <osgDB/FileNameUtils>
#include <osgDB/Registry>

#include <sgi/details/type_list>
#include <sgi/helpers/preprocessor>

#include "SGIPluginInterface.h"
#include "SGIHostItemBase.h"
#include "GenerateItemImpl"
#include "GetObjectInfoImpl"
#include "ObjectTreeImpl"
#include "ContextMenuImpl"
#include "ActionHandlerImpl"
#include "SettingsDialogImpl"
#include "WritePrettyHTMLImpl"
#include "ObjectLoggerImpl"
#include "GuiAdapterImpl"
#include "ConvertToImageImpl"

namespace sgi {

class Registry;
class SGIPluginHostInterface;

#define SGIITEMTYPE_NAME(__enum) \
    registerItemType(__enum, #__enum)

#define GENERATE_IMPL_TEMPLATE() \
    template<typename T> \
    class generateItemAcceptImpl { \
    public: \
        typedef sgi::details::constexpr_true accept; \
        typedef sgi::details::constexpr_enum<SGIItemType, SGIItemTypeObject> ItemType; \
    };

#define GENERATE_IMPL_DECLARE_EX(__type, __accept, __itemType) \
    template<> \
    class generateItemAcceptImpl<__type> { \
    public: \
        typedef sgi::details::constexpr_##__accept accept; \
        typedef sgi::details::constexpr_enum<SGIItemType, __itemType> ItemType; \
    };

#define GENERATE_IMPL_DECLARE(__type, __accept) \
    GENERATE_IMPL_DECLARE_EX(__type, __accept, SGIItemTypeObject)

#define GENERATE_IMPL_NO_ACCEPT(__type) \
    GENERATE_IMPL_DECLARE_EX(__type, false, SGIItemTypeObject)

class defaultPluginGenerateItemImpl {
public:
    static bool generate(const SGIHostItemBase * /*object*/, SGIItemBasePtr & /*item*/)
    {
        return false;
    }
};

////////--------------

template<typename T>
class defaultPluginWritePrettyHTMLImpl {
public:
    typedef sgi::details::constexpr_false accept;
    defaultPluginWritePrettyHTMLImpl(SGIPluginHostInterface * hostInterface=NULL, const SGIItemBase * item=NULL, bool table=true) {}
    bool process(std::basic_ostream<char>& os) { return false; }
};

//---------------------------
template<typename T>
class defaultPluginGetObjectInfoStringImpl {
public:
    typedef sgi::details::constexpr_false accept;
    defaultPluginGetObjectInfoStringImpl(SGIPluginHostInterface * hostInterface=NULL, const SGIItemBase * item=NULL)
        {}
    std::string process() { return std::string(); }
};

///----------_#########################-----------------

template<typename T>
class defaultPluginGetObjectPathImpl {
public:
    typedef sgi::details::constexpr_false accept;
    defaultPluginGetObjectPathImpl(SGIPluginHostInterface * hostInterface=NULL, const SGIItemBase * item=NULL)
        {}
    SGIItemBasePtrPath process() { return SGIItemBasePtrPath(); }
};

template<typename T>
class defaultPluginGetObjectHasCallbackImpl {
public:
    typedef sgi::details::constexpr_false accept;
    defaultPluginGetObjectHasCallbackImpl(SGIPluginHostInterface * hostInterface=NULL, const SGIItemBase * item=NULL)
        {}
    bool process() { return false; }
};

template<typename T>
class defaultPluginGetObjectFilenameFiltersImpl {
public:
    typedef sgi::details::constexpr_false accept;
    defaultPluginGetObjectFilenameFiltersImpl(SGIPluginHostInterface * hostInterface=NULL, const SGIItemBase * item=NULL)
        {}
    std::vector<std::string> process() { return std::vector<std::string>(); }
};

template<typename T>
class defaultPluginWriteObjectFileImpl {
public:
    typedef sgi::details::constexpr_false accept;
    defaultPluginWriteObjectFileImpl(SGIPluginHostInterface * hostInterface=NULL, const SGIItemBase * item=NULL)
        {}
    bool process(const std::string & filename, const SGIItemBase* options=NULL) { return false; }
};


///----------_#########################-----------------
template<typename T>
class defaultPluginObjectTreeBuildImpl{
public:
    typedef sgi::details::constexpr_false accept;
    defaultPluginObjectTreeBuildImpl(SGIPluginHostInterface * hostInterface=NULL, SGIItemBase * item=NULL)
        {}
    bool build(IObjectTreeItem * treeItem) { return false; }
};


///----------_#########################-----------------

template<typename T>
class defaultPluginContextMenuPopulateImpl {
public:
    typedef sgi::details::constexpr_false accept;
    defaultPluginContextMenuPopulateImpl(SGIPluginHostInterface * hostInterface=NULL, SGIItemBase * item=NULL)
        {}
    bool populate(IContextMenuItem * menuItem) { return false; }
};

///----------_#########################-----------------
typedef unsigned int defaultContextMenuActionId;

template<defaultContextMenuActionId T>
class defaultPluginActionHandlerImpl {
public:
    defaultPluginActionHandlerImpl(SGIPluginHostInterface * hostInterface=NULL, const SGIItemBase * item=NULL, IContextMenuAction * menuAction=NULL)
        {}
    bool execute() { return false; }
};

//---------------------

template<unsigned Id>
class defaultPluginSettingsDialogCreateImpl {
public:
    typedef sgi::details::constexpr_false accept;
    defaultPluginSettingsDialogCreateImpl(SGIPluginHostInterface * hostInterface=NULL, SGIItemBase * item=NULL, QWidget *parent=NULL, ISettingsDialogInfo * info=NULL) {}
    bool execute(ISettingsDialogPtr & dialog) { return false; }
};

///-----------------------
template<typename T>
class defaultPluginGuiAdapterParentWidgetImpl {
public:
    typedef sgi::details::constexpr_false accept;
    defaultPluginGuiAdapterParentWidgetImpl(SGIPluginHostInterface * hostInterface=NULL, QWidgetPtr * widget=NULL, SGIItemBase * item=NULL) {}
    bool execute(QWidgetPtr * /*widget*/) { return false; }
};
///-----------------------
template<typename T>
class defaultPluginGuiAdapterSetViewImpl {
public:
    typedef sgi::details::constexpr_false accept;
    defaultPluginGuiAdapterSetViewImpl(SGIPluginHostInterface * hostInterface=NULL, const SGIItemBase * item=NULL) {}
    bool execute(osg::View * /*view*/, double /*animationTime*/) { return false; }
};

///-----------------------
template<typename T>
class defaultPluginGetOrCreateLoggerImpl {
public:
    typedef sgi::details::constexpr_false accept;
    defaultPluginGetOrCreateLoggerImpl(SGIPluginHostInterface * hostInterface=NULL, const SGIItemBase * item=NULL, bool createIfNotExists=false) {}
    bool execute(IObjectLoggerPtr & logger) { return false; }
};

///-----------------------
template<typename T>
class defaultPluginConvertToImageConvertImpl {
public:
    typedef sgi::details::constexpr_false accept;
    defaultPluginConvertToImageConvertImpl(SGIPluginHostInterface * hostInterface=NULL, const SGIItemBase * item=NULL) {}
    bool convert(ImagePtr & /*image*/) { return false; }
};

template<   typename pluginGenerateItemImpl=defaultPluginGenerateItemImpl,
            template<typename> class pluginWritePrettyHTMLImpl=defaultPluginWritePrettyHTMLImpl,
            template<typename> class pluginGetObjectNameImpl=defaultPluginGetObjectInfoStringImpl,
            template<typename> class pluginGetObjectDisplayNameImpl=defaultPluginGetObjectInfoStringImpl,
            template<typename> class pluginGetObjectTypenameImpl=defaultPluginGetObjectInfoStringImpl,
            template<typename> class pluginGetObjectPathImpl=defaultPluginGetObjectPathImpl,
            template<typename> class pluginGetObjectSuggestedFilenameExtensionImpl=defaultPluginGetObjectInfoStringImpl,
            template<typename> class pluginGetObjectSuggestedFilenameImpl=defaultPluginGetObjectInfoStringImpl,
            template<typename> class pluginGetObjectFilenameFiltersImpl=defaultPluginGetObjectFilenameFiltersImpl,
            template<typename> class pluginGetObjectHasCallbackImpl=defaultPluginGetObjectHasCallbackImpl,
            template<typename> class pluginWriteObjectFileImpl=defaultPluginWriteObjectFileImpl,
            template<typename> class pluginObjectTreeBuildImpl=defaultPluginObjectTreeBuildImpl,
            template<typename> class pluginObjectTreeBuildRootImpl=defaultPluginObjectTreeBuildImpl,
            template<typename> class pluginContextMenuPopulateImpl=defaultPluginContextMenuPopulateImpl,
            template<unsigned> class pluginActionHandlerImpl=defaultPluginActionHandlerImpl,
            template<unsigned> class pluginSettingsDialogCreateImpl=defaultPluginSettingsDialogCreateImpl,
            template<typename> class pluginGuiAdapterParentWidgetImpl=defaultPluginGuiAdapterParentWidgetImpl,
            template<typename> class pluginGuiAdapterSetViewImpl=defaultPluginGuiAdapterSetViewImpl,
            template<typename> class pluginGetOrCreateLoggerImpl=defaultPluginGetOrCreateLoggerImpl,
            template<typename> class pluginConverToImageConvertImpl=defaultPluginConvertToImageConvertImpl
            >
class SGIPluginImplementationT : public SGIPluginInterface
{
public:
    SGIPluginImplementationT(SGIPluginHostInterface * hostInterface=NULL, unsigned requiredInterfaceVersion=SGIPLUGIN_HOSTINTERFACE_CURRENT_VERSION)
        : SGIPluginInterface(hostInterface)
         , _requiredInterfaceVersion(requiredInterfaceVersion)
         , _writePrettyHTML(hostInterface)
         , _objectInfo(hostInterface)
         , _objectTree(hostInterface)
         , _objectLogger(hostInterface)
         , _contextMenu(hostInterface)
         , _settingsDialog(hostInterface)
         , _guiAdapter(hostInterface)
         , _convertToImage(hostInterface)
    {
    }
    SGIPluginImplementationT(const SGIPluginImplementationT & rhs, const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY)
        : SGIPluginInterface(rhs, copyop)
         , _requiredInterfaceVersion(rhs._requiredInterfaceVersion)
         , _writePrettyHTML(rhs._hostInterface)
         , _objectInfo(rhs._hostInterface)
         , _objectTree(rhs._hostInterface)
         , _objectLogger(rhs._hostInterface)
         , _contextMenu(rhs._hostInterface)
         , _settingsDialog(rhs._hostInterface)
         , _guiAdapter(rhs._hostInterface)
         , _convertToImage(rhs._hostInterface)
    {
    }
    virtual unsigned getRequiredInterfaceVersion()
    {
        return _requiredInterfaceVersion;
    }
    virtual unsigned getPluginScore()
    {
        return 0;
    }
    virtual bool generateItem(const SGIHostItemBase * object, SGIItemBasePtr & item)
    {
        return pluginGenerateItemImpl::generate(object, item);
    }
    virtual void shutdown()
    {
    }

    class WritePrettyHTMLImpl : public WritePrettyHTML
    {
    public:
        WritePrettyHTMLImpl(SGIPluginHostInterface * hostInterface)
            : _hostInterface(hostInterface)
        {
        }
        virtual bool writePrettyHTML(std::basic_ostream<char>& os, const SGIItemBase * object, bool table)
        {
            return writePrettyHTMLImplBaseT<pluginWritePrettyHTMLImpl>::call(_hostInterface, object, os, table);
        }
    private:
        SGIPluginHostInterface * _hostInterface;
    };

    virtual WritePrettyHTML * getWritePrettyHTML()
    {
        return &_writePrettyHTML;
    }
    class ObjectInfoImpl : public ObjectInfo
    {
    public:
        ObjectInfoImpl(SGIPluginHostInterface * hostInterface)
            : _hostInterface(hostInterface)
        {
        }
        virtual bool getObjectName(std::string & name, const SGIItemBase * object, bool full)
        {
            return getObjectInfoFullStringImplBaseT<pluginGetObjectNameImpl>::call(_hostInterface, object, name, full);
        }
        virtual bool getObjectDisplayName(std::string & name, const SGIItemBase * object, bool full)
        {
            return getObjectInfoFullStringImplBaseT<pluginGetObjectDisplayNameImpl>::call(_hostInterface, object, name, full);
        }
        virtual bool getObjectTypename(std::string & name, const SGIItemBase * object, bool full)
        {
            return getObjectInfoFullStringImplBaseT<pluginGetObjectTypenameImpl>::call(_hostInterface, object, name, full);
        }
        virtual bool getObjectPath(SGIItemBasePtrPath & path, const SGIItemBase * object)
        {
            return getObjectPathImplBaseT<pluginGetObjectTypenameImpl>::call(_hostInterface, object, path);
        }
        virtual bool getObjectSuggestedFilename(std::string & name, const SGIItemBase * object)
        {
            return getObjectInfoStringImplBaseT<pluginGetObjectSuggestedFilenameImpl>::call(_hostInterface, object, name);
        }
        virtual bool getObjectSuggestedFilenameExtension(std::string & name, const SGIItemBase * object)
        {
            return getObjectInfoStringImplBaseT<pluginGetObjectSuggestedFilenameExtensionImpl>::call(_hostInterface, object, name);
        }
        virtual bool getObjectFilenameFilters(std::vector<std::string> & filters, const SGIItemBase * object)
        {
            return getObjectFilenameFiltersImplBaseT<pluginGetObjectFilenameFiltersImpl>::call(_hostInterface, object, filters);
        }
        virtual bool hasCallback(bool & result, const SGIItemBase * object)
        {
            return hasCallbackImplBaseT<pluginGetObjectHasCallbackImpl>::call(_hostInterface, object, result);
        }
        virtual bool writeObjectFile(bool & result, SGIItemBase * object, const std::string & filename, const SGIItemBase* options)
        {
            return writeObjectFileImplBaseT<pluginWriteObjectFileImpl>::call(_hostInterface, object, filename, options, result);
        }
    private:
        SGIPluginHostInterface * _hostInterface;
    };
    virtual ObjectInfo * getObjectInfo()
    {
        return &_objectInfo;
    }

    class ObjectTreeImpl : public ObjectTree
    {
    public:
        ObjectTreeImpl(SGIPluginHostInterface * hostInterface)
            : _hostInterface(hostInterface)
        {
        }
        virtual bool buildRootTree(IObjectTreeItem * treeItem, SGIItemBase * object)
        {
            return objectTreeBuildRootImplBaseT<pluginObjectTreeBuildRootImpl>::call(_hostInterface, object, treeItem);
        }
        virtual bool buildTree(IObjectTreeItem * treeItem, SGIItemBase * object)
        {
            return objectTreeBuildImplBaseT<pluginObjectTreeBuildImpl>::call(_hostInterface, object, treeItem);
        }
    private:
        SGIPluginHostInterface * _hostInterface;
    };
    virtual ObjectTree * getObjectTree()
    {
        return &_objectTree;
    }

    class ObjectLoggerImpl : public ObjectLogger
    {
    public:
        ObjectLoggerImpl(SGIPluginHostInterface * hostInterface)
            : _hostInterface(hostInterface)
        {
        }
        virtual bool createObjectLogger(IObjectLoggerPtr & logger, SGIItemBase * object)
        {
            return getOrCreateObjectLoggerImplBaseT<pluginGetOrCreateLoggerImpl>::call(_hostInterface, object, logger, true);
        }
        virtual bool getObjectLogger(IObjectLoggerPtr & logger, SGIItemBase * object)
        {
            return getOrCreateObjectLoggerImplBaseT<pluginGetOrCreateLoggerImpl>::call(_hostInterface, object, logger, false);
        }
        virtual bool getOrCreateObjectLogger(IObjectLoggerPtr & logger, SGIItemBase * object)
        {
            return getOrCreateObjectLoggerImplBaseT<pluginGetOrCreateLoggerImpl>::call(_hostInterface, object, logger, true);
        }

    private:
        SGIPluginHostInterface * _hostInterface;
    };
    virtual ObjectLogger * getObjectLogger()
    {
        return &_objectLogger;
    }

    class ContextMenuImpl : public ContextMenu
    {
    public:
        ContextMenuImpl(SGIPluginHostInterface * hostInterface)
            : _hostInterface(hostInterface)
        {
        }

        virtual bool populate(IContextMenuItem * menuItem, SGIItemBase * object)
        {
            return contextMenuPopulateImplBaseT<pluginContextMenuPopulateImpl>::call(_hostInterface, object, menuItem);
        }
        virtual bool execute(IContextMenuAction * menuAction, SGIItemBase * object)
        {
            return actionHandlerImplBaseT<pluginActionHandlerImpl>::call(_hostInterface, object, menuAction);
            //return CommandItem<pluginContextMenuActionId, pluginActionHandlerImpl>::execute(_hostInterface, object, menuAction);
        }
    private:
        SGIPluginHostInterface * _hostInterface;
    };
    virtual ContextMenu * getContextMenu()
    {
        return &_contextMenu;
    }


    class SettingsDialogImpl : public SettingsDialog
    {
    public:
        SettingsDialogImpl(SGIPluginHostInterface * hostInterface)
            : _hostInterface(hostInterface)
        {
        }
        virtual bool create(ISettingsDialogPtr & dialog, SGIItemBase * object, ISettingsDialogInfo * info)
        {
            return settingsDialogCreateImplBaseT<pluginSettingsDialogCreateImpl>::call(_hostInterface, object, info, dialog);
        }

    private:
        SGIPluginHostInterface * _hostInterface;
    };

    virtual SettingsDialog * getSettingsDialog()
    {
        return &_settingsDialog;
    }

    class GUIAdapterImpl : public GUIAdapter
    {
    public:
        GUIAdapterImpl(SGIPluginHostInterface * hostInterface)
            : _hostInterface(hostInterface)
        {
        }
        virtual bool parentWidget(QWidgetPtr & widget, SGIItemBase * item)
        {
            return guiAdapterParentWidgetImplBaseT<pluginGuiAdapterParentWidgetImpl>::call(_hostInterface, &widget, item);
        }
        virtual bool setView(SGIItemBase * view, const SGIItemBase * object, double animationTime)
        {
            return guiAdapterSetViewImplBaseT<pluginGuiAdapterSetViewImpl>::call(_hostInterface, view, object, animationTime);
        }
        virtual bool repaint()
        {
            return false;
        }
    private:
        SGIPluginHostInterface * _hostInterface;
    };

    virtual GUIAdapter * getGUIAdapter()
    {
        return &_guiAdapter;
    }
    class ConvertToImageImpl : public ConvertToImage
    {
    public:
        ConvertToImageImpl(SGIPluginHostInterface * hostInterface)
            : _hostInterface(hostInterface)
        {
        }
        virtual bool convert(ImagePtr & image, const SGIItemBase * object)
        {
            return convertToImageConvertImplBaseT<pluginConverToImageConvertImpl>::call(_hostInterface, image, object);
        }
    private:
        SGIPluginHostInterface * _hostInterface;
    };
    virtual ConvertToImage * getConvertToImage()
    {
        return &_convertToImage;
    }

protected:
    template<typename T>
    bool registerNamedEnum(const std::string & description=std::string(), bool bitmask=false)
    {
        const std::string& value_type = details::StaticTypeName<T>::name();
        return _hostInterface->registerNamedEnum(value_type, description, bitmask);
    }
    template<typename T>
    bool registerNamedEnumValue(int value, const std::string & valuename)
    {
        const std::string& value_type = details::StaticTypeName<T>::name();
        return _hostInterface->registerNamedEnumValue(value_type, value, valuename);
    }
    template<typename T>
    bool registerNamedEnumValues(int value, const std::map<int, std::string> & values)
    {
        const std::string& value_type = details::StaticTypeName<T>::name();
        return _hostInterface->registerNamedEnumValues(value_type, values);
    }
    template<typename SGIITEMTYPE_ENUM>
    bool registerItemType(SGIITEMTYPE_ENUM value, const std::string & valuename)
    {
        const std::string& value_type = details::StaticTypeName<sgi::SGIItemType>::name();
        return _hostInterface->registerNamedEnumValue(value_type, value, valuename);
    }

private:
    unsigned _requiredInterfaceVersion;
    WritePrettyHTMLImpl _writePrettyHTML;
    ObjectInfoImpl _objectInfo;
    ObjectTreeImpl _objectTree;
    ObjectLoggerImpl _objectLogger;
    ContextMenuImpl _contextMenu;
    SettingsDialogImpl _settingsDialog;
    GUIAdapterImpl _guiAdapter;
    ConvertToImageImpl _convertToImage;
};


class SGIPluginReaderWriter: public osgDB::ReaderWriter
{
protected:
    virtual Features supportedFeatures() const
    {
        // we only use readObject so let osgDB know this
        return FEATURE_READ_OBJECT;
    }
    virtual ~SGIPluginReaderWriter()
    {
    }
};

#define SGI_PLUGIN_IMPLEMENT(plugin_name) \
    class SGIPlugin##plugin_name##ReaderWriter : public sgi::SGIPluginReaderWriter { \
        public: \
            SGIPlugin##plugin_name##ReaderWriter() \
                : sgi::SGIPluginReaderWriter() \
            { \
                setName("SGI Plugin " #plugin_name); \
                supportsExtension( SGI_QUOTE(sgi_##plugin_name##_plugin), "SGI Plugin " #plugin_name ); \
            } \
            virtual ~SGIPlugin##plugin_name##ReaderWriter() {} \
            virtual const char* libraryName() { return SGI_QUOTE(sgi_##plugin_name##_plugin); } \
            virtual const char* className() { return SGI_QUOTE(SGIPlugin##plugin_name##ReaderWriter); } \
            virtual osgDB::ReaderWriter::ReadResult readObject(const std::string& file_name, const Options* options) const \
            { \
                if ( !acceptsExtension(osgDB::getLowerCaseFileExtension( file_name ))) \
                    return osgDB::ReaderWriter::ReadResult::FILE_NOT_HANDLED; \
                const unsigned * hostInterfaceVersion = reinterpret_cast<const unsigned*>(options->getPluginData("hostInterfaceVersion")); \
                if(!hostInterfaceVersion || *hostInterfaceVersion != SGIPLUGIN_HOSTINTERFACE_CURRENT_VERSION) {\
                    return osgDB::ReaderWriter::ReadResult::ERROR_IN_READING_FILE; \
                } \
                const sgi::SGIPluginHostInterface * hostInterface = reinterpret_cast<const sgi::SGIPluginHostInterface *>(options->getPluginData("hostInterface")); \
                return osgDB::ReaderWriter::ReadResult( new sgi::SGIPlugin_##plugin_name##_Implementation(const_cast<sgi::SGIPluginHostInterface *>(hostInterface)) ); \
            } \
    }; \
    sgi::SGIPluginHostInterface * sgi::SGIPluginInterface::_hostInterface = NULL; \
    REGISTER_OSGPLUGIN(sgi_##plugin_name, SGIPlugin##plugin_name##ReaderWriter)

} // namespace sgi
