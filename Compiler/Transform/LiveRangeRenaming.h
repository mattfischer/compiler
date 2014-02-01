#ifndef TRANSFORM_LIVE_RANGE_RENAMING_H
#define TRANSFORM_LIVE_RANGE_RENAMING_H

#include "Transform/Transform.h"

namespace Transform {

/*!
 * \brief Perform live range renaming on a procedure
 *
 * If a symbol in a procedure is used in multiple regions of a procedure, but is dead in between
 * them, then its disjoint live ranges should be renamed to different symbols, so that register
 * allocation can assign them to different registers if necessary.
 */
class LiveRangeRenaming : public Transform
{
public:
	virtual bool transform(IR::Procedure *procedure, Analysis::Analysis &analysis);
	virtual std::string name() { return "LiveRangeRenaming"; }

	static LiveRangeRenaming *instance();
};

}
#endif