#ifndef TRANSFORM_TRANSFORM_H
#define TRANSFORM_TRANSFORM_H

namespace IR {
	class Procedure;
}

namespace Analysis {
	class Analysis;
}

namespace Transform {
	class Transform {
	public:
		virtual void transform(IR::Procedure *procedure, Analysis::Analysis &analysis) = 0;
	};
}
#endif