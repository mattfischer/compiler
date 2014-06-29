#include "VM/Program.h"

#include <iostream>
#include <iomanip>

namespace VM {

void Program::print(std::ostream &o)
{
	for(unsigned int i = 0; i < instructions.size(); i+=4) {
		VM::Instruction instr;
		for(std::map<std::string, int>::iterator it=symbols.begin(); it != symbols.end(); it++) {
			if(i == it->second) {
				o << it->first << ":" << std::endl;
				break;
			}
		}

		int addressWidth = 0;
		int size = instructions.size();
		while(size > 0) {
			size >>= 4;
			addressWidth++;
		}

		o << "  0x" << std::setw(addressWidth) << std::setfill('0') << std::setbase(16) << i << ": ";
		for(int j=0; j<4; j++) {
			int d = 0;
			if(i + j < instructions.size()) {
				d = instructions[i + j];
			}
			o << std::setw(2) << std::setfill('0') << std::setbase(16) << d;
		}
		o << std::setbase(10);
		std::memcpy(&instr, &instructions[i], 4);
		o << "  " << instr << std::endl;
	}
}

}