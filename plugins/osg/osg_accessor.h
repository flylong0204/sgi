#pragma once
#include <osg/ShaderComposer>

namespace sgi {
namespace osg_plugin {

class NodeAccess : public osg::Node
{
public:
    bool                            isBoundingSphereComputed() const { return _boundingSphereComputed; }

    /** Get the bounding sphere of node.
        Using lazy evaluation computes the bounding sphere if it is 'dirty'.*/
    inline const osg::BoundingSphere& getBoundNoCompute() const
    {
        if(_boundingSphereComputed)
	        return _boundingSphere;
		else
			return _initialBound;
    }

    void    setNumUpdateTraversal(unsigned num) { osg::Node::setNumChildrenRequiringUpdateTraversal(num); }
    void    setNumEventTraversal(unsigned num) { osg::Node::setNumChildrenRequiringEventTraversal(num); }
};

class ClipNodeAccess : public osg::ClipNode
{
public:
    osg::StateAttribute::GLModeValue value() const { return _value; }
};

class ViewerBaseAccess : public osgViewer::ViewerBase
{
public:
    bool requestRedraw() const { return _requestRedraw; }
    bool requestContinousUpdate() const { return _requestContinousUpdate; }
};

class ShaderComposerAccess : public osg::ShaderComposer
{
public:
    typedef std::map< osg::ShaderComponents, osg::ref_ptr<osg::Program> > ProgramMap;
    typedef std::map< Shaders, osg::ref_ptr<osg::Shader> > ShaderMainMap;
    
    const ProgramMap & programMap() const { return _programMap; }
    const ShaderMainMap & shaderMainMap() const { return _shaderMainMap; }
};

class StatsAccess : public osg::Stats
{
public:
    void getCollectStats(CollectMap & c)
    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);
        c = _collectMap;
    }
};

} // namespace osg_plugin
} // namespace sgi

