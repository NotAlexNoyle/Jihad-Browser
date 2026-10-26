/* @@@LICENSE
*
*      Copyright (c) 2012 Hewlett-Packard Development Company, L.P.
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*
LICENSE@@@ */

/* Modified by the Jihad Browser project (2026), per Apache-2.0 section 4(b): this file differs from
 * HP's isis-project original. The changes are this repository's git history for this file. */

#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <linux/fb.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <string.h>
#include <stdint.h>   // T-513: uint64_t (windowBytes)
#include <stdio.h>
#include <unistd.h>
#include <assert.h>

#include "BrowserOffscreen.h"
#include "BrowserOffscreenCalculations.h"
#include "BrowserRect.h"

static const float kOffscreenSizeAsScreenSizeMultiplier = 4.0f;

// Used only for desktop builds
static const int kDefaultScreenWidth = 1024;
static const int kDefaultScreenHeight = 768;

static double kDoubleZeroTolerance = 0.0001;

static bool PrvGetScreenDimensions(int& width, int& height)
{
#if defined(TARGET_DESKTOP)
    width = 1024;
    height = 768;
    return true;
#endif

    int fd = ::open("/dev/fb0", O_RDONLY);
    if (fd < 0)
        return false;

    struct fb_var_screeninfo varinfo;
    ::memset(&varinfo, 0, sizeof(varinfo));

    if (::ioctl(fd, FBIOGET_VSCREENINFO, &varinfo) == -1) {
        ::close(fd);
        return false;
    }

    width = varinfo.xres;
    height = varinfo.yres;
    ::close(fd);

    return true;
}

static inline bool PrvIsEqual(double a, double b)
{
    return (fabs(a-b) < kDoubleZeroTolerance);
}

BrowserOffscreen* BrowserOffscreen::create()
{
    int screenWidth, screenHeight;
    if (!PrvGetScreenDimensions(screenWidth, screenHeight)) {
        screenWidth = kDefaultScreenWidth;
        screenHeight = kDefaultScreenHeight;
    }

    int bufferSize = screenWidth *
                     screenHeight *
                     sizeof(unsigned int) *
                     kOffscreenSizeAsScreenSizeMultiplier;

    IpcBuffer* buffer = IpcBuffer::create(bufferSize + sizeof(BrowserOffscreenInfo));
    if (!buffer) {
        return 0;
    }
    // T-517: this adapter reads frame windows (JihadFrameWindow.h); the daemon moves them only then
    if (JihadFrameTrailer* t = jihadFrameTrailer((unsigned char*)buffer->buffer(), buffer->size())) {
        ::memset(t, 0, sizeof(*t));
        t->adapterCaps = kJihadWindowCaps;
    }
    return new BrowserOffscreen(buffer);
}

uint32_t BrowserOffscreen::frameSeq() const
{
    const JihadFrameTrailer* t = jihadFrameTrailer((unsigned char*)m_ipcBuffer->buffer(), m_ipcBuffer->size());
    return (t && t->magic == kJihadWindowMagic) ? t->dirtySeq : 0;
}

bool BrowserOffscreen::dirtyRect(uint32_t* base, int* left, int* top, int* right, int* bottom) const
{
    const JihadFrameTrailer* t = jihadFrameTrailer((unsigned char*)m_ipcBuffer->buffer(), m_ipcBuffer->size());
    if (!t || t->magic != kJihadWindowMagic || t->dirtyMagic != kJihadDirtyMagic)
        return false;
    *base = t->dirtyBase;
    *left = t->dirtyLeft; *top = t->dirtyTop; *right = t->dirtyRight; *bottom = t->dirtyBottom;
    return true;
}

int BrowserOffscreen::stuckRects(int* viewX, int* viewY, int (*rects)[4], int max) const
{
    unsigned char* seg = (unsigned char*)m_ipcBuffer->buffer();
    const JihadFrameTrailer* t = jihadFrameTrailer(seg, m_ipcBuffer->size());
    const JihadStuckBlock* b = jihadStuckBlock(seg, m_ipcBuffer->size());
    if (!t || !b || t->magic != kJihadWindowMagic || b->magic != kJihadStuckMagic || b->seq != t->dirtySeq)
        return 0;
    int n = b->count < max ? b->count : max;
    if (n < 0)
        n = 0;
    *viewX = b->viewX; *viewY = b->viewY;
    for (int k = 0; k < n; ++k)
        for (int c = 0; c < 4; ++c)
            rects[k][c] = b->rects[k][c];
    return n;
}

BrowserOffscreen* BrowserOffscreen::attach(int key, int size)
{
    IpcBuffer* buffer = IpcBuffer::attach(key, size);
    if (!buffer)
        return 0;

    return new BrowserOffscreen(buffer);
}

BrowserOffscreen::BrowserOffscreen(IpcBuffer* ipcBuffer)
    : m_ipcBuffer(ipcBuffer)
    , m_buffer((unsigned char*)ipcBuffer->buffer() + sizeof(BrowserOffscreenInfo))
    , m_header((BrowserOffscreenInfo*)m_ipcBuffer->buffer())
{
    resetBuffer();
}

BrowserOffscreen::~BrowserOffscreen()
{
    delete m_ipcBuffer;
}

void BrowserOffscreen::clear()
{
    ::memset(m_buffer, 0xFF, rasterSize());
}

void BrowserOffscreen::copyFrom(BrowserOffscreen* other,  BrowserRect* r)
{
    // Check if we can actually copy from this buffer
    if (m_header->bufferWidth  != other->m_header->bufferWidth ||
            m_header->bufferHeight != other->m_header->bufferHeight ||
            !PrvIsEqual(m_header->contentZoom, other->m_header->contentZoom))
        return;

    BrowserRect myRect(m_header->renderedX,
                       m_header->renderedY,
                       m_header->renderedWidth,
                       m_header->renderedHeight);

    BrowserRect otherRect(other->m_header->renderedX,
                          other->m_header->renderedY,
                          other->m_header->renderedWidth,
                          other->m_header->renderedHeight);

    if (!myRect.intersects(otherRect))
        return;

    myRect.intersect(otherRect);
    if (r) {
        if (!myRect.intersects(*r)) return;
        myRect.intersect(*r);
    }

    unsigned int* src = (unsigned int*) other->rasterBuffer();
    unsigned int* dst = (unsigned int*) rasterBuffer();
    int stride        = m_header->renderedWidth;

    src += (myRect.y() - other->m_header->renderedY) * stride +
           (myRect.x() - other->m_header->renderedX);
    dst += (myRect.y() - m_header->renderedY) * stride +
           (myRect.x() - m_header->renderedX);

    for (int j = 0; j < myRect.h(); j++) {
        ::memcpy(dst, src, myRect.w() * sizeof(unsigned int));
        src += stride;
        dst += stride;
    }
}

QImage BrowserOffscreen::surface()
{
    QImage result;

    if ((m_header->renderedWidth > 0) && (m_header->renderedHeight > 0))
        result = QImage(rasterBuffer(), m_header->renderedWidth, m_header->renderedHeight, QImage::Format_ARGB32_Premultiplied);

    return result;
}

// T-517: the frame's window, when the daemon wrote one that fits the pixels (else the raster's start)
size_t BrowserOffscreen::windowBytes() const
{
    JihadFrameTrailer* t = jihadFrameTrailer((unsigned char*)m_ipcBuffer->buffer(), m_ipcBuffer->size());
    if (!t || t->magic != kJihadWindowMagic)
        return 0;
    const int rows = t->windowRows;   // read once: the checks and the offset are of the same value
    if (rows <= 0 || m_header->renderedWidth <= 0 || m_header->renderedHeight <= 0)
        return 0;
    // T-513 (review): no product that can wrap: on the device size_t is 32 bits, and even in 64 a
    // row count, a height and a width from the header can multiply past it (Codex 2026-10-24). The
    // rows the window and the frame take are held against what the raster has room for, by division
    // (LunaSysMgr's only guard against blitting outside the segment)
    const uint64_t rowBytes = (uint64_t)m_header->renderedWidth * 4;
    const uint64_t roomRows = (uint64_t)rasterSize() / rowBytes;
    if ((uint64_t)rows + (uint64_t)m_header->renderedHeight > roomRows)
        return 0;
    return (size_t)((uint64_t)rows * rowBytes);   // <= rasterSize()
}

void BrowserOffscreen::resetBuffer()
{
    ::memset(m_header, 0, sizeof(BrowserOffscreenInfo));
}

bool BrowserOffscreen::matchesParams(BrowserOffscreenCalculations* calc) const
{
    return
        m_header->bufferWidth  == calc->bufferWidth &&
        m_header->bufferHeight == calc->bufferHeight &&
        m_header->renderedX == calc->renderX &&
        m_header->renderedY == calc->renderY &&
        m_header->renderedWidth == calc->renderWidth &&
        m_header->renderedHeight == calc->renderHeight &&
        PrvIsEqual(m_header->contentZoom, calc->contentZoom);
}

void BrowserOffscreen::updateParams(BrowserOffscreenCalculations* calc)
{
    resetBuffer();

    m_header->bufferWidth  = calc->bufferWidth;
    m_header->bufferHeight = calc->bufferHeight;

    m_header->contentZoom = calc->contentZoom;

    m_header->renderedX = calc->renderX;
    m_header->renderedY = calc->renderY;
    m_header->renderedWidth = calc->renderWidth;
    m_header->renderedHeight = calc->renderHeight;
}

bool BrowserOffscreen::matchesParams(BrowserOffscreen* other) const
{
    return
        m_header->bufferWidth  == other->m_header->bufferWidth &&
        m_header->bufferHeight == other->m_header->bufferHeight &&
        m_header->renderedX == other->m_header->renderedX &&
        m_header->renderedY == other->m_header->renderedY &&
        m_header->renderedWidth == other->m_header->renderedWidth &&
        m_header->renderedHeight == other->m_header->renderedHeight &&
        PrvIsEqual(m_header->contentZoom, other->m_header->contentZoom);
}

int BrowserOffscreen::rasterSize() const
{
    return m_ipcBuffer->size() - sizeof(BrowserOffscreenInfo) - kJihadTrailerBytes;   // T-517: the trailer after it
}

