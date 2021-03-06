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
#ifndef __BASE_ATLAS_MAP_RENDERER_H_
#define __BASE_ATLAS_MAP_RENDERER_H_

#include <stdint.h>
#include <memory>

#include <OsmAndCore.h>
#include <OsmAndCore/CommonTypes.h>
#include <IMapRenderer.h>

namespace OsmAnd {

    class OSMAND_CORE_API BaseAtlasMapRenderer : public virtual IMapRenderer
    {
    public:
        enum {
            TileSide3D = 100,
        };

    private:
    protected:
        BaseAtlasMapRenderer();
    public:
        virtual ~BaseAtlasMapRenderer();
    };

}

#endif // __BASE_ATLAS_MAP_RENDERER_H_