#pragma once

#include <sgi/plugins/WritePrettyHTMLImpl>

namespace osgEarth {
    class ElevationQuery;
    class TileKey;
} // namespace osgEarth

namespace sgi {
namespace osgearth_plugin {

typedef ReferencedRawPtrT<osgEarth::ElevationQuery> ElevationQueryReferenced;
typedef ReferencedDataT<osgEarth::TileKey> TileKeyReferenced;
struct TileSourceTileKeyData;
typedef ReferencedDataT<TileSourceTileKeyData> TileSourceTileKey;
class TileSourceInfo;

WRITE_PRETTY_HTML_IMPL_TEMPLATE()

} // namespace osgearth_plugin
} // namespace sgi
