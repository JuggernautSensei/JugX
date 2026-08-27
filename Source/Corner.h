#pragma once

namespace jug
{

enum class eCorner
{
    LeftBottomNear,    // -X, -Y, -Z
    LeftBottomFar,     // -X, -Y, +Z
    LeftTopNear,       // -X, +Y, -Z
    LeftTopFar,        // -X, +Y, +Z
    RightBottomNear,   // +X, -Y, -Z
    RightBottomFar,    // +X, -Y, +Z
    RightTopNear,      // +X, +Y, -Z
    RightTopFar,       // +X, +Y, +Z
};

}   // namespace jug