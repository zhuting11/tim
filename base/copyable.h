#ifndef TIM_BASE_COPYABLE_H
#define TIM_BASE_COPYABLE_H

namespace tim
{

/// A tag class emphasises the objects are copyable.
/// The empty base class optimization applies.
/// Any derived class of copyable should be a value type.
class copyable
{
};

};

#endif  // TIM_BASE_COPYABLE_H
