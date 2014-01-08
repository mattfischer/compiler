#ifndef TRANSFORM_LIVE_RANGE_RENAMING_H
#define TRANSFORM_LIVE_RANGE_RENAMING_H

#include "Transform/Transform.h"

namespace Transform {

class LiveRangeRenaming : public Transform
{
public:
	virtual bool transform(IR::Procedure *procedure);
	virtual std::string name() { return "LiveRangeRenaming"; }

	static LiveRangeRenaming *instance();
};

}
#endif