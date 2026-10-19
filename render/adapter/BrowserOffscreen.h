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

#ifndef BROWSEROFFSCREEN_H
#define BROWSEROFFSCREEN_H

#include "IpcBuffer.h"
#include "BrowserRect.h"
#include "BrowserOffscreenInfo.h"
#include "JihadFrameWindow.h"
#include <QImage>

class BrowserOffscreenCalculations;

class BrowserOffscreen
{
public:

    static BrowserOffscreen* create();
    static BrowserOffscreen* attach(int key, int size);
    ~BrowserOffscreen();

    inline IpcBuffer* ipcBuffer() const {
        return m_ipcBuffer;
    }
    inline int key() const {
        return m_ipcBuffer->key();
    }
    inline int size() const {
        return m_ipcBuffer->size();
    }
    inline BrowserOffscreenInfo* header() const {
        return m_header;
    }

    bool matchesParams(BrowserOffscreenCalculations* calc) const;
    void updateParams(BrowserOffscreenCalculations* calc);
    bool matchesParams(BrowserOffscreen* other) const;

    QImage surface();
    void clear();
    void copyFrom(BrowserOffscreen* other, BrowserRect* rect=NULL);

    // The frame's first row: the raster's start, or further in when the daemon moved the frame's
    // window (JihadFrameWindow.h, T-517); every reader of the frame goes through here
    unsigned char* rasterBuffer() const {
        return m_buffer + windowBytes();
    }
    int rasterSize() const;   // the pixels' bytes, without the window trailer

private:

    BrowserOffscreen(IpcBuffer* buffer);
    void resetBuffer();
    size_t windowBytes() const;   // T-517: where the frame starts in the raster

    IpcBuffer* m_ipcBuffer;
    unsigned char* m_buffer;
    BrowserOffscreenInfo* m_header;

    int m_contentWidth;
    int m_contentHeight;
    int m_viewportWidth;
    int m_viewportHeight;
};


#endif /* BROWSEROFFSCREEN_H */
