// kate: syntax C++11;
// SGI - Copyright (C) 2012-2018 FAST Protect, Andreas Roth

#include <sgi/plugins/SGIImage.h>
#include <sstream>
#include <cmath>
#include <cassert>
#include <memory.h>

#define ALIGN_SIZE(x,n)       ((size_t)((~(n-1))&((x)+(n-1))))
#define ALIGN_BY_16(x) ALIGN_SIZE(x, 16)
#define ALIGN_BY_8(x) ALIGN_SIZE(x, 8)
#define ALIGN_BY_4(x) ALIGN_SIZE(x, 4)

#define SGI_UNUSED(x) (void)x;

namespace sgi {

Image::Image(ImageFormat format, DataType type)
    : _format(format), _dataType(type), _origin(OriginDefault), _data(NULL), _length(0)
    , _width(0), _height(0), _depth(0)
    , _allocatedWidth(0), _allocatedHeight(0)
    , _pitch { 0, 0, 0, 0 }, _lines{ 0, 0, 0, 0 }, _planeOffset{0, 0, 0, 0}
    , _originalImage(NULL), _originalImageQt(NULL), _freeQt(NULL), _copyQt(NULL)
    , _allocated(false)
{
}

Image::Image(ImageFormat format, DataType type, Origin origin, void * data, size_t length,
        unsigned width, unsigned height, unsigned depth, unsigned bytesPerLine,
        const osg::Referenced * originalImage, bool copyData)
    : _format(format), _dataType(type), _origin(origin), _data(copyData ? malloc(length) : data), _length(length)
    , _width(width), _height(height), _depth(depth)
    , _allocatedWidth(width), _allocatedHeight(height)
    , _pitch { bytesPerLine, 0, 0, 0 }, _lines{ height, 0, 0, 0 }
    , _planeOffset{0, 0, 0, 0}
    , _originalImage(originalImage), _originalImageQt(NULL), _freeQt(NULL), _copyQt(NULL)
    , _allocated(copyData)
{
    if (copyData)
        memcpy(_data, data, length);
    loadPitchAndPlaneOffsets();
}

Image::Image(ImageFormat format, DataType type, void * data, size_t length, bool copyData)
    : _format(format), _dataType(type), _origin(OriginDefault), _data(copyData ? malloc(length) : data), _length(length)
    , _width(0), _height(0), _depth(0)
    , _allocatedWidth(0), _allocatedHeight(0)
    , _pitch{ 0, 0, 0, 0 }, _lines{ 0, 0, 0, 0 }, _planeOffset{ 0, 0, 0, 0 }
    , _originalImage(NULL), _originalImageQt(NULL), _freeQt(NULL), _copyQt(NULL)
    , _allocated(copyData)
{
    if (copyData)
        memcpy(_data, data, length);
    loadPitchAndPlaneOffsets();
}

Image::Image(const Image & rhs)
    : _format(rhs._format), _dataType(rhs._dataType), _origin(rhs._origin), _data(rhs._data), _length(rhs._length)
    , _width(rhs._width), _height(rhs._height), _depth(rhs._depth)
    , _allocatedWidth(rhs._allocatedWidth), _allocatedHeight(rhs._allocatedHeight)
    , _pitch { rhs._pitch[0], rhs._pitch[1], rhs._pitch[2], rhs._pitch[3] }
    , _lines{ rhs._lines[0], rhs._lines[1], rhs._lines[2], rhs._lines[3] }
    , _planeOffset { rhs._planeOffset[0], rhs._planeOffset[1], rhs._planeOffset[2], rhs._planeOffset[3] }
    , _originalImage(rhs._originalImage)
    , _originalImageQt( (rhs._originalImageQt && rhs._copyQt) ? (rhs.*rhs._copyQt)() : NULL)
    , _freeQt(rhs._freeQt), _copyQt(rhs._copyQt)
    , _allocated(false)
{
    //(this->*_freeQt)();
}

Image::~Image()
{
    free();
}

bool Image::empty() const
{
    return _format == ImageFormatInvalid || (_originalImageQt == nullptr && _data == nullptr);
}

Image & Image::operator=(const Image & rhs)
{
    free();
    _format = rhs._format;
    _dataType = rhs._dataType;
    _origin = rhs._origin;
    _data = rhs._data;
    _allocated = false;
    _length = rhs._length;
    _width = rhs._width;
    _height = rhs._height;
    _depth = rhs._depth;
    _allocatedWidth = rhs._allocatedWidth;
    _allocatedHeight = rhs._allocatedHeight;
    _pitch[0] = rhs._pitch[0];
    _pitch[1] = rhs._pitch[1];
    _pitch[2] = rhs._pitch[2];
    _pitch[3] = rhs._pitch[3];
    _lines[0] = rhs._lines[0];
    _lines[1] = rhs._lines[1];
    _lines[2] = rhs._lines[2];
    _lines[3] = rhs._lines[3];
    _planeOffset[0] = rhs._planeOffset[0];
    _planeOffset[1] = rhs._planeOffset[1];
    _planeOffset[2] = rhs._planeOffset[2];
    _planeOffset[3] = rhs._planeOffset[3];
    _originalImage = rhs._originalImage;
    if (rhs._originalImageQt && rhs._copyQt)
    {
        const Image * prhs = &rhs;
        // copy/clone the QImage instance using the rhs object with the copyQt function ptr from rhs object
        _originalImageQt = (rhs.*rhs._copyQt)();
    }
    else
        _originalImageQt = nullptr;
    _freeQt = rhs._freeQt;
    _copyQt = rhs._copyQt;
    return *this;
}

void Image::loadPitchAndPlaneOffsets()
{
    switch (_format)
    {
    default:
    case ImageFormatInvalid:
        assert(false); // "invalid frame format"
        break;
    case ImageFormatAutomatic:
        assert(false); // "invalid frame format, automatic"
        break;
    case ImageFormatRaw:
        assert(false); // "invalid frame format, raw"
        break;
    case ImageFormatRGB24:
    case ImageFormatBGR24:
    {
        _pitch[0] = _width * 3;
        _pitch[1] = _pitch[2] = _pitch[3] = 0;
        _lines[0] = _height;
        _lines[1] = _lines[2] = _lines[3] = 0;
        _planeOffset[0] = _planeOffset[1] = _planeOffset[2] = _planeOffset[3] = 0;
    }
    break;
    case ImageFormatRGB32:
    case ImageFormatRGBA32:
    case ImageFormatARGB32:
    case ImageFormatBGR32:
    case ImageFormatBGRA32:
    case ImageFormatABGR32:
    case ImageFormatFloat:
    case ImageFormatDepth:
    {
        _pitch[0] = _width * 4;
        _pitch[1] = _pitch[2] = _pitch[3] = 0;
        _lines[0] = _height;
        _lines[1] = _lines[2] = _lines[3] = 0;
        _planeOffset[0] = _planeOffset[1] = _planeOffset[2] = _planeOffset[3] = 0;
    }
    break;
    case ImageFormatFloat64:
    {
        _pitch[0] = _width * 8;
        _pitch[1] = _pitch[2] = _pitch[3] = 0;
        _lines[0] = _height;
        _lines[1] = _lines[2] = _lines[3] = 0;
        _planeOffset[0] = _planeOffset[1] = _planeOffset[2] = _planeOffset[3] = 0;
    }
    break;
    case ImageFormatYUV444:
    {
        _pitch[0] = _pitch[1] = _pitch[2] = _width * 3;
        _pitch[3] = 0;
        _lines[0] = _lines[1] = _lines[2] = _height;
        _lines[3] = 0;
        _planeOffset[0] = 0;
        _planeOffset[1] = _width * _height * 3;
        _planeOffset[2] = _planeOffset[1] << 1;
        _planeOffset[3] = 0;
    }
    break;
    case ImageFormatYUV422:
    {
        _pitch[0] = _width;
        _pitch[1] = _pitch[2] = _width / 2;
        _pitch[3] = 0;
        _lines[0] = _lines[1] = _lines[2] = _height;
        _lines[3] = 0;
        _planeOffset[0] = 0;
        _planeOffset[1] = _width * _height;
        _planeOffset[2] = _planeOffset[1] + (_planeOffset[1] >> 1);
        _planeOffset[3] = 0;
    }
    break;
    case ImageFormatYUV420:
    {
        _pitch[0] = _width;
        _pitch[1] = _pitch[2] = _width / 2;
        _pitch[3] = 0;
        _lines[0] = _height;
        _lines[1] = _lines[2] = _height / 2;
        _lines[3] = 0;
        _planeOffset[0] = 0;
        _planeOffset[1] = _width * _height;
        _planeOffset[2] = _planeOffset[1] + (_planeOffset[1] >> 2);
        _planeOffset[3] = 0;
    }
    break;

    case ImageFormatYUYV:
    case ImageFormatUYVY:
    {
        _pitch[0] = _width + _width;
        _pitch[1] = _pitch[2] = _pitch[3] = 0;
        _lines[0] = _height;
        _lines[1] = _lines[2] = _lines[3] = 0;
        _planeOffset[0] = _planeOffset[1] = _planeOffset[2] = _planeOffset[3] = 0;
    }
    break;
    case ImageFormatGray:
    case ImageFormatRed:
    case ImageFormatGreen:
    case ImageFormatBlue:
    case ImageFormatAlpha:
    case ImageFormatLuminance:
    case ImageFormatLuminanceAlpha:
    {
        // only one channel with 8-bit color data
        _pitch[0] = (_width * bitsForDataElement(_dataType)) / 8;
        _pitch[1] = _pitch[2] = _pitch[3] = 0;
        _lines[0] = _height;
        _lines[1] = _lines[2] = _lines[3] = 0;
        _planeOffset[0] = _planeOffset[1] = _planeOffset[2] = _planeOffset[3] = 0;
    }
    break;
    case ImageFormatDXT1:
    case ImageFormatDXT1Alpha:
    {
        // from http://www.gamedev.net/topic/615440-calculating-pitch-of-a-dxt-compressed-texture/
        _pitch[0] = ((_width + 3u) / 4u) * 8u;
        _pitch[1] = _pitch[2] = _pitch[3] = 0;
        _lines[0] = _height;
        _lines[1] = _lines[2] = _lines[3] = 0;
        _planeOffset[0] = _planeOffset[1] = _planeOffset[2] = _planeOffset[3] = 0;
    }
    break;
    case ImageFormatDXT3:
    case ImageFormatDXT5:
    {
        // from http://www.gamedev.net/topic/615440-calculating-pitch-of-a-dxt-compressed-texture/
        _pitch[0] = ((_width + 3u) / 4u) * 16u;
        _pitch[1] = _pitch[2] = _pitch[3] = 0;
        _lines[0] = _height;
        _lines[1] = _lines[2] = _lines[3] = 0;
        _planeOffset[0] = _planeOffset[1] = _planeOffset[2] = _planeOffset[3] = 0;
    }
    break;
    case ImageFormatMono:
    case ImageFormatMonoLSB:
    case ImageFormatIndexed8:
    {
        _pitch[0] = _width;
        _pitch[1] = _pitch[2] = _pitch[3] = 0;
        _lines[0] = _height;
        _lines[1] = _lines[2] = _lines[3] = 0;
        _planeOffset[0] = _planeOffset[1] = _planeOffset[2] = _planeOffset[3] = 0;
    }
    break;

    }
}

void Image::free()
{
    if(_originalImageQt && _freeQt)
        (this->*_freeQt)();
    if(_allocated && _data)
    {
        ::free(_data);
        _data = NULL;
    }
}


bool Image::allocate(unsigned width, unsigned height, ImageFormat format, Origin origin)
{
    bool ret = false;
    unsigned allocated_width = ALIGN_BY_16(width);
    unsigned allocated_height = ALIGN_BY_16(height);
    free();
    _length = 0;
    switch (format)
    {
    default:
    case ImageFormatInvalid:
        assert(false); // "invalid frame format"
        break;
    case ImageFormatOriginal:
        assert(false); // "invalid frame format, original"
        break;
    case ImageFormatAutomatic:
        assert(false); // "invalid frame format, automatic"
        break;
    case ImageFormatRaw:
        assert(false); // "invalid frame format, raw"
        break;
    case ImageFormatRGB24:
    case ImageFormatBGR24:
        _length = allocated_width * allocated_height * 3;
        break;
    case ImageFormatRGB32:
    case ImageFormatRGBA32:
    case ImageFormatARGB32:
    case ImageFormatBGR32:
    case ImageFormatBGRA32:
    case ImageFormatABGR32:
    case ImageFormatFloat:
        _length = allocated_width * allocated_height * 4;
        break;
    case ImageFormatFloat64:
        _length = allocated_width * allocated_height * 8;
        break;
    case ImageFormatYUV444:
        _length = allocated_width * allocated_height * 3;
        break;
    case ImageFormatYUV422:
        _length = allocated_width * allocated_height * 2;
        break;
    case ImageFormatYUV420:
        _length = allocated_width * allocated_height + (allocated_width / 2 * allocated_height/2);
        break;

    case ImageFormatYUYV:
    case ImageFormatUYVY:
        _length = (allocated_width + allocated_width) * allocated_height;
        break;
    case ImageFormatGray:
    case ImageFormatRed:
    case ImageFormatGreen:
    case ImageFormatBlue:
    case ImageFormatAlpha:
        // only one channel with 8-bit color data
        _length = allocated_width * allocated_height;
        break;
    }
    if(_length)
    {
        _data = malloc(_length);
        _width = width;
        _height = height;
        _depth = 1;
        _allocatedWidth = allocated_width;
        _allocatedHeight = allocated_height;
        _format = format;
        _dataType = DataTypeUnsignedByte;
        _origin = origin;
        loadPitchAndPlaneOffsets();
        ret = _data != NULL;
        _allocated = ret;
    }
    return ret;
}

bool Image::reinterpretFormat(ImageFormat targetFormat)
{
    bool ret = false;
    switch(_format)
    {
    case Image::ImageFormatRed:
    case Image::ImageFormatGreen:
    case Image::ImageFormatBlue:
    case Image::ImageFormatAlpha:
    case Image::ImageFormatGray:
        {
            // reinterpret a single channel image as a full color image
            switch(targetFormat)
            {
            case Image::ImageFormatYUV444:
                ret = (_height % 3) == 0;
                if(ret)
                {
                    unsigned planeHeight = _height / 3;
                    unsigned planeLength = planeHeight * _width;
                    // split single plane into three separate ones
                    _planeOffset[0] = 0;
                    _planeOffset[1] = planeLength;
                    _planeOffset[2] = planeLength + planeLength;
                    _planeOffset[3] = 0;
                    _pitch[0] = _width;
                    _pitch[1] = _width;
                    _pitch[2] = _width;
                    _pitch[3] = 0;
                    _height = planeHeight;
                }
                break;
            case Image::ImageFormatYUV422:
                ret = (_height % 2) == 0;
                if(ret)
                {
                    unsigned luma_planeHeight = _height / 2;
                    unsigned chroma_planeHeight = luma_planeHeight / 2;
                    unsigned luma_planeLength = luma_planeHeight * _width;
                    unsigned chroma_planeLength = chroma_planeHeight * _width;
                    // split single plane into three separate ones
                    _planeOffset[0] = 0;
                    _planeOffset[1] = luma_planeLength;
                    _planeOffset[2] = luma_planeLength + chroma_planeLength;
                    _planeOffset[3] = 0;
                    _pitch[0] = _width;
                    _pitch[1] = _width;
                    _pitch[2] = _width;
                    _pitch[3] = 0;
                    _height = luma_planeHeight;
                }
                break;
            case Image::ImageFormatYUV420:
                ret = ((_height+_height) % 3) == 0;
                if(ret)
                {
                    unsigned luma_planeHeight = (_height + _height)/ 3;
                    unsigned chroma_planeHeight = luma_planeHeight / 2;
                    unsigned luma_planeLength = luma_planeHeight * _width;
                    unsigned chroma_planeLength = chroma_planeHeight * (_width/2);
                    // split single plane into three separate ones
                    _planeOffset[0] = 0;
                    _planeOffset[1] = luma_planeLength;
                    _planeOffset[2] = luma_planeLength + chroma_planeLength;
                    _planeOffset[3] = 0;
                    _pitch[0] = _width;
                    _pitch[1] = _width / 2;
                    _pitch[2] = _width / 2;
                    _pitch[3] = 0;
                    _height = luma_planeHeight;
                }
                break;
            case Image::ImageFormatYUYV:
            case Image::ImageFormatUYVY:
                {
                    // it's a YUV422 color format all in one plane
                    ret = (_height % 2) == 0;
                    if(ret)
                    {
                        _planeOffset[0] = _planeOffset[1] = _planeOffset[2] = _planeOffset[3] = 0;
                        _pitch[0] = _width + _width;
                        _pitch[1] = _pitch[2] = _pitch[3] = 0;
                    }
                }
                break;
            }
        }
        break;
    case ImageFormatBGR32:
    case ImageFormatRGB32:
    case ImageFormatRGBA32:
    case ImageFormatARGB32:
    case ImageFormatBGRA32:
    case ImageFormatABGR32:
        // reinterpret a 32-bit image and reinterpret the colors (helpful for wrong color
        // display)
        switch(targetFormat)
        {
        case ImageFormatBGR32:
        case ImageFormatRGB32:
        case ImageFormatARGB32:
        case ImageFormatRGBA32:
        case ImageFormatABGR32:
        case ImageFormatBGRA32:
            // just accept this, no special handling required
            ret = true;
        }
        break;
    case ImageFormatBGR24:
    case ImageFormatRGB24:
    case ImageFormatYUV444:
        // reinterpret a 24-bit image and reinterpret the colors (helpful for wrong color
        // display)
        switch(targetFormat)
        {
        case ImageFormatBGR24:
        case ImageFormatRGB24:
        case ImageFormatYUV444:
            // just accept this, no special handling required
            ret = true;
        }
        break;
    }
    if(ret)
    {
        // we got the desired format, so take the targetFormat as new format
        _format = targetFormat;
    }
    return ret;
}

bool Image::reinterpret(const ImageSize & size)
{
    return reinterpret(size.format, size.width, size.height, size.depth);
}

bool Image::reinterpret(ImageFormat format, unsigned width, unsigned height, unsigned depth)
{
    _width = width;
    _height = height;
    _allocatedWidth = width;
    _allocatedHeight = height;
    _depth = depth;
    _format = format;
    loadPitchAndPlaneOffsets();
    return true;
}

bool Image::guessImageSizes(ImageSizeList & possibleSizes) const
{
    bool ret = false;
    possibleSizes.clear();
    size_t totalNumberOfPixels = 0;
    switch(_format)
    {
    case Image::ImageFormatARGB32:
    case Image::ImageFormatRGBA32:
    case Image::ImageFormatRGB32:
    case Image::ImageFormatABGR32:
    case Image::ImageFormatBGRA32:
    case Image::ImageFormatBGR32:
    case Image::ImageFormatFloat:
        ret = (_length % 4 == 0);
        if(ret)
            totalNumberOfPixels = _length / 4;
        break;
    case Image::ImageFormatFloat64:
        ret = (_length % 8 == 0);
        if(ret)
            totalNumberOfPixels = _length / 8;
        break;
    case Image::ImageFormatRGB24:
    case Image::ImageFormatBGR24:
    case Image::ImageFormatYUV444:
        ret = (_length % 3 == 0);
        if(ret)
            totalNumberOfPixels = _length / 3;
        break;
    case Image::ImageFormatYUV422:
    case Image::ImageFormatYUYV:
    case Image::ImageFormatUYVY:
        ret = (_length % 2 == 0);
        if(ret)
            totalNumberOfPixels = _length / 2;
        break;
    case Image::ImageFormatYUV420:
        ret = ((_length*2) % 3 == 0);
        if(ret)
            totalNumberOfPixels = (_length*2) / 3;
    }
    if(ret)
    {
        double d = std::sqrt(totalNumberOfPixels);
        if(fmod(d, 1.0) == 0)
        {
            // got a square image
            int width = std::floor(d);
            int height = width;
            possibleSizes.push_back(ImageSize(_format, width, height));
        }
        else
        {
            const int size_step_size = 8;
            const int min_height = 16;
            const int min_width = 16;
            int max_width = totalNumberOfPixels / min_height;
            int start_width = (int)std::floor(d)/size_step_size * size_step_size;
            // adjust to next matching 8-pixel width
            start_width = (start_width / size_step_size) * size_step_size;

            for(int width = start_width; width <= max_width; width += size_step_size)
            {
                int height_4_3 = (width * 3) / 4;
                int height_16_9 = (width * 9) / 16;
                int total_4_3 = width * height_4_3;
                int total_16_9 = width * height_16_9;
                if(total_4_3 == totalNumberOfPixels)
                {
                    possibleSizes.push_back(ImageSize(_format, width, height_4_3));
                }
                if(total_16_9 == totalNumberOfPixels)
                {
                    possibleSizes.push_back(ImageSize(_format, width, height_16_9));
                }
            }
            for(int width = start_width; width >= min_width; width -= size_step_size)
            {
                int height_4_3 = (width * 3) / 4;
                int height_16_9 = (width * 9) / 16;
                int total_4_3 = width * height_4_3;
                int total_16_9 = width * height_16_9;
                if(total_4_3 == totalNumberOfPixels)
                {
                    possibleSizes.push_back(ImageSize(_format, width, height_4_3));
                }
                if(total_16_9 == totalNumberOfPixels)
                {
                    possibleSizes.push_back(ImageSize(_format, width, height_16_9));
                }
            }

        }
    }
    return ret;
}

unsigned Image::bitsPerPixel() const
{
    unsigned ret = 0;
    switch (_format)
    {
    default:
    case ImageFormatInvalid: 
        ret = 0; break;
    case ImageFormatRGB24: 
    case ImageFormatBGR24:
        ret = 24; break;
    case ImageFormatARGB32:
    case ImageFormatRGB32: 
    case ImageFormatBGR32:
    case ImageFormatABGR32:
    case ImageFormatRGBA32:
    case ImageFormatBGRA32:
        ret = 32; break;
    case ImageFormatYUV420:
    case ImageFormatYUV422:
    case ImageFormatYUV444:
    case ImageFormatYUYV:
    case ImageFormatUYVY:
        ret = 16;
        break;
    case ImageFormatMono: 
    case ImageFormatMonoLSB: 
        ret = 1; 
        break;
    case ImageFormatIndexed8: 
        ret = 8;
        break;
    case ImageFormatFloat: 
        ret = 32; 
        break;
    case ImageFormatGray:
    case ImageFormatRed:
    case ImageFormatGreen:
    case ImageFormatBlue:
    case ImageFormatAlpha:
    case ImageFormatLuminance:
    case ImageFormatLuminanceAlpha:
    case ImageFormatDepth:
        ret = bitsForDataElement(_dataType);
        break;
    case ImageFormatDXT1: 
    case ImageFormatDXT1Alpha: 
        // 4x4 pixels go into 8 bytes
        ret = 4; 
        break;
    case ImageFormatDXT3:
    case ImageFormatDXT5:
        // 4x4 pixels go into 16 bytes
        ret = 8;
        break;
    case ImageFormatFloat64:
        ret = 64;
        break;
    }
    return ret;
}

std::string Image::imageFormatToString(ImageFormat format)
{
    std::string ret;
    switch(format)
    {
    case ImageFormatInvalid: ret = "Invalid"; break;
    case ImageFormatRGB24: ret = "RGB24"; break;
    case ImageFormatRGB32: ret = "RGB32"; break;
    case ImageFormatARGB32: ret = "ARGB32"; break;
    case ImageFormatMono: ret = "Mono"; break;
    case ImageFormatMonoLSB: ret = "MonoLSB"; break;
    case ImageFormatIndexed8: ret = "Indexed8"; break;
    case ImageFormatFloat: ret = "Float"; break;
    case ImageFormatBGR24: ret = "BGR24"; break;
    case ImageFormatBGR32: ret = "BGR32"; break;
    case ImageFormatABGR32: ret = "ABGR32"; break;
    case ImageFormatYUV420: ret = "YUV420"; break;
    case ImageFormatYUV422: ret = "YUV422"; break;
    case ImageFormatYUV444: ret = "YUV444"; break;
    case ImageFormatYUYV: ret = "YUYV"; break;
    case ImageFormatUYVY: ret = "UYVY"; break;
    case ImageFormatGray: ret = "Gray"; break;
    case ImageFormatRed: ret = "Red"; break;
    case ImageFormatGreen: ret = "Green"; break;
    case ImageFormatBlue: ret = "Blue"; break;
    case ImageFormatAlpha: ret = "Alpha"; break;
    case ImageFormatDepth: ret = "Depth"; break;
    case ImageFormatLuminance: ret = "Luminance"; break;
    case ImageFormatLuminanceAlpha: ret = "LuminanceAlpha"; break;
    case ImageFormatDXT1: ret = "DXT1"; break;
    case ImageFormatDXT1Alpha: ret = "DXT1a"; break;
    case ImageFormatDXT3: ret = "DXT3"; break;
    case ImageFormatDXT5: ret = "DXT5"; break;
    case ImageFormatRaw: ret = "raw"; break;
    case ImageFormatRGBA32: ret = "RGBA32"; break;
    case ImageFormatBGRA32: ret = "BGRA32"; break;
    case ImageFormatFloat64: ret = "Float64"; break;
    default:
        {
            std::stringstream ss;
            ss << "Unknown(" << (int)format << ')';
            ret = ss.str();
        }
        break;
    }
    return ret;
}

std::string Image::colorSpaceToString(ColorSpace colorspace)
{
    std::string ret;
    switch (colorspace)
    {
    case ColorSpaceInvalid: ret = "invalid"; break;
    case ColorSpaceAutomatic: ret = "auto"; break;
    case ColorSpaceRGB: ret = "RGB"; break;
    case ColorSpaceYUV_ITU_R_BT_601: ret = "YUV ITU-R BT.601"; break;
    case ColorSpaceYUV_ITU_R_BT_709: ret = "YUV ITU-R BT.709"; break;
    case ColorSpaceYUV_ITU_R_BT_2020: ret = "YUV ITU-R BT.2020"; break;
    case ColorSpaceCYMK: ret = "CYMK"; break;
    default:
    {
        std::stringstream ss;
        ss << "Unknown(" << (int)colorspace << ')';
        ret = ss.str();
    }
    break;
    }
    return ret;
}

std::string Image::dataTypeToString(DataType type)
{
    std::string ret;
    switch (type)
    {
    case DataTypeInvalid: ret = "invalid"; break;
    case DataTypeUnsignedByte: ret = "u8"; break;
    case DataTypeSignedByte: ret = "i8"; break;
    case DataTypeUnsignedShort: ret = "u16"; break;
    case DataTypeSignedShort: ret = "i16"; break;
    case DataTypeUnsignedInt: ret = "u32"; break;
    case DataTypeSignedInt: ret = "i32"; break;
    case DataTypeFloat32: ret = "f32"; break;
    case DataTypeFloat64: ret = "f64"; break;
    default:
    {
        std::stringstream ss;
        ss << "Unknown(" << (int)type << ')';
        ret = ss.str();
    }
    break;
    }
    return ret;
}

unsigned Image::bitsForDataElement(DataType type)
{
    unsigned ret = 0;
    switch (type)
    {
    case DataTypeUnsignedByte: 
    case DataTypeSignedByte: 
        ret = 8; break;
    case DataTypeUnsignedShort: 
    case DataTypeSignedShort: 
        ret = 16; break;
    case DataTypeUnsignedInt:
    case DataTypeSignedInt:
        ret = 32; break;
    case DataTypeFloat32:
        ret = 32; break;
    case DataTypeFloat64:
        ret = 64; break;
    default: ret = 0; break;
    }
    return ret;
}

std::string Image::originToString(Origin o)
{
    std::string ret;
    switch(o)
    {
    case OriginBottomLeft: ret = "bottom-left"; break;
    case OriginTopLeft: ret = "top-left"; break;
    default:
        {
            std::stringstream ss;
            ss << "Unknown(" << (int)o << ')';
            ret = ss.str();
        }
        break;
    }
    return ret;
}

unsigned Image::planeEndOffset(unsigned index) const
{ 
    unsigned ret = 0;
    if (index > 0)
        ret = planeOffset(index - 1);
    ret += _pitch[index] * _lines[index];
    return ret;
}

const void * Image::pixelPtr(unsigned x, unsigned y, unsigned z, unsigned plane) const
{
    const void * ret = nullptr;
    SGI_UNUSED(z);
    if (_data)
    {
        const uint8_t * src_data = reinterpret_cast<const uint8_t *>(_data);
        unsigned src_bits = bitsPerPixel();
        size_t src_offset = 0;
        switch(_origin)
        {
        case OriginTopLeft:
        default:
            src_offset = _planeOffset[plane] + ((y * _pitch[plane]) + (x * src_bits / 8));
            break;
        case OriginBottomLeft:
            src_offset = _planeOffset[plane] + (((_lines[plane] - y) * _pitch[plane]) + (x * src_bits / 8));
            break;
        }
        ret = src_data + src_offset;
    }
    return ret;
}

float Image::hscale() const
{
    return (_allocatedWidth > 0) ? ((float)_width / (float)_allocatedWidth) : 1.0f;
}

float Image::vscale() const
{
    return (_allocatedHeight > 0) ? ((float)_height / (float)_allocatedHeight) : 1.0f;
}

float Image::horizontalPixelSize() const
{
    return (_allocatedWidth > 0) ? (1.0f / (float)_allocatedWidth) : 0.0f;
}

float Image::verticalPixelSize() const
{
    return (_allocatedHeight > 0) ? (1.0f / (float)_allocatedHeight) : 0.0f;
}

} // namespace sgi
