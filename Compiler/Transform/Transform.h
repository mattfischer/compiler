#ifndef TRANSFORM_TRANSFORM_H
#define TRANSFORM_TRANSFORM_H

#include <string>

#include "Analysis/Analysis.h"

#include "IR/Procedure.h"

/*!
 * \brief A collection of classes for transforming an IR procedure in various ways
 */
namespace Transform {
	class Transform {
	public:
		/*!
		 * \brief Transform a procedure
		 * \param procedure Procedure to transform
		 * \return True if the transform modified the procedure, false otherwise
		 */
		virtual bool transform(IR::Procedure *procedure, Analysis::Analysis &analysis) = 0;

		/*!
		 * \brief Transformation name
		 * \return Name of transformation
		 */
		virtual std::string name() = 0;
	};
}
#endif