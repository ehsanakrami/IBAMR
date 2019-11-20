// ---------------------------------------------------------------------
//
// Copyright (c) 2014 - 2019 by the IBAMR developers
// All rights reserved.
//
// This file is part of IBAMR.
//
// IBAMR is free software and is distributed under the 3-clause BSD
// license. The full text of the license can be found in the file
// COPYRIGHT at the top level directory of IBAMR.
//
// ---------------------------------------------------------------------

/////////////////////////////// INCLUDES /////////////////////////////////////

#include "ibtk/SideSynchCopyFillPattern.h"
#include "ibtk/namespaces.h" // IWYU pragma: keep

#include "Box.h"
#include "BoxGeometry.h"
#include "BoxList.h"
#include "BoxOverlap.h"
#include "Index.h"
#include "IntVector.h"
#include "SideGeometry.h"
#include "SideOverlap.h"
#include "tbox/Pointer.h"
#include "tbox/Utilities.h"

#include <ostream>
#include <string>

/////////////////////////////// NAMESPACE ////////////////////////////////////

namespace IBTK
{
/////////////////////////////// STATIC ///////////////////////////////////////

namespace
{
static const std::string PATTERN_NAME = "SIDE_SYNCH_COPY_FILL_PATTERN";
}

/////////////////////////////// PUBLIC ///////////////////////////////////////

Pointer<BoxOverlap<NDIM> >
SideSynchCopyFillPattern::calculateOverlap(const BoxGeometry<NDIM>& dst_geometry,
                                           const BoxGeometry<NDIM>& src_geometry,
                                           const Box<NDIM>& /*dst_patch_box*/,
                                           const Box<NDIM>& src_mask,
                                           const bool overwrite_interior,
                                           const IntVector<NDIM>& src_offset) const
{
    Pointer<SideOverlap<NDIM> > box_geom_overlap =
        dst_geometry.calculateOverlap(src_geometry, src_mask, overwrite_interior, src_offset);
#if !defined(NDEBUG)
    TBOX_ASSERT(box_geom_overlap);
#endif
    if (box_geom_overlap->isOverlapEmpty()) return box_geom_overlap;

    auto const t_dst_geometry = dynamic_cast<const SideGeometry<NDIM>*>(&dst_geometry);
#if !defined(NDEBUG)
    TBOX_ASSERT(t_dst_geometry);
#endif
    BoxList<NDIM> dst_boxes[NDIM];
    for (unsigned int axis = 0; axis < NDIM; ++axis)
    {
        bool skip = false;
        for (unsigned int d = 0; d < NDIM && !skip; ++d)
        {
            if (d != axis)
            {
                skip = skip || (src_offset(d) != 0);
            }
        }
        if (!skip)
        {
            // Determine the stencil box.
            const Box<NDIM>& dst_box = t_dst_geometry->getBox();
            Box<NDIM> stencil_box = SideGeometry<NDIM>::toSideBox(dst_box, axis);
            stencil_box.lower(axis) = stencil_box.upper(axis);

            // Intersect the original overlap boxes with the stencil box.
            const BoxList<NDIM>& box_geom_overlap_boxes = box_geom_overlap->getDestinationBoxList(axis);
            for (BoxList<NDIM>::Iterator it(box_geom_overlap_boxes); it; it++)
            {
                const Box<NDIM> overlap_box = stencil_box * it();
                if (!overlap_box.empty()) dst_boxes[axis].appendItem(overlap_box);
            }
        }
    }
    return new SideOverlap<NDIM>(dst_boxes, src_offset);
} // calculateOverlap

IntVector<NDIM>&
SideSynchCopyFillPattern::getStencilWidth()
{
    return d_stencil_width;
} // getStencilWidth

const std::string&
SideSynchCopyFillPattern::getPatternName() const
{
    return PATTERN_NAME;
} // getPatternName

/////////////////////////////// PROTECTED ////////////////////////////////////

/////////////////////////////// PRIVATE //////////////////////////////////////

/////////////////////////////// NAMESPACE ////////////////////////////////////

} // namespace IBTK

//////////////////////////////////////////////////////////////////////////////
