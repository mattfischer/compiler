#ifndef MIDDLE_ERROR_CHECK_H
#define MIDDLE_ERROR_CHECK_H

namespace IR {
	class Program;
}

namespace Middle {
	class ErrorCheck {
	public:
		static bool check(IR::Program *program);
	};
}
#endif