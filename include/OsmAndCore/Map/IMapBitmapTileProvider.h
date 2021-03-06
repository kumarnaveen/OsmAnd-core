/**
 * @file
 *
 * @section LICENSE
 *
 * OsmAnd - Android navigation software based on OSM maps.
 * Copyright (C) 2010-2013  OsmAnd Authors listed in AUTHORS file
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef __I_MAP_BITMAP_TILE_PROVIDER_H_
#define __I_MAP_BITMAP_TILE_PROVIDER_H_

#include <stdint.h>
#include <memory>
#include <functional>

#include <OsmAndCore.h>
#include <OsmAndCore/CommonTypes.h>
#include <OsmAndCore/Map/IMapTileProvider.h>

namespace OsmAnd {

    class OSMAND_CORE_API IMapBitmapTileProvider : public IMapTileProvider
    {
    public:
        enum BitmapFormat
        {
            RGBA_8888,
            RGBA_4444,
            RGB_565,
        };

        class OSMAND_CORE_API Tile : public IMapTileProvider::Tile
        {
        private:
        protected:
            Tile(const void* data, size_t rowLength, uint32_t width, uint32_t height, const BitmapFormat& format);
        public:
            virtual ~Tile();

            const BitmapFormat format;
        };
    private:
    protected:
        IMapBitmapTileProvider();
    public:
        virtual ~IMapBitmapTileProvider();

        virtual float getTileDensity() const = 0;
    };

}

#endif // __I_MAP_BITMAP_TILE_PROVIDER_H_
