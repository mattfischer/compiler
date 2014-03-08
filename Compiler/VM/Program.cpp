#include "VM/Program.h"

#include <iostream>
#include <iomanip>

namespace VM {

void Program::print()
{
	for(unsigned int i = 0; i < instructions.size(); i+=4) {
		VM::Instruction instr;
		for(std::map<std::string, int>::iterator it=symbols.begin(); it != symbols.end(); it++) {
			if(i == it->second) {
				std::cout << it->first << ":" << std::endl;
				break;
			}
		}

		std::cout << "  ";
		for(int j=0; j<4; j++) {
			int d = 0;
			if(i + j < instructions.size()) {
				d = instructions[i + j];
			}
			std::cout << std::setw(2) << std::setfill('0') << std::setbase(16) << d;
		}
		std::cout << std::setbase(10);
		std::memcpy(&instr, &instructions[i], 4);
		std::cout << "  " << instr << std::endl;
	}
}

}