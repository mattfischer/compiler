#ifndef IR_PROCEDURE_H
#define IR_PROCEDURE_H

#include <string>
#include <list>
#include <vector>

namespace Front {
	class Type;
}

namespace IR {
	class Block;
	class Symbol;
	class Entry;

	class Procedure {
	public:
		Procedure(const std::string &name);

		void print(const std::string &prefix = "") const;
		
		Block *start() const { return mStart; }
		Block *end() const { return mEnd; }
		const std::string &name() const { return mName; }
		std::vector<Block*> &blocks() { return mBlocks; }

		void removeBlock(Block *block);
		void replaceEnd(Block *block);

		typedef std::list<Symbol*> SymbolList;
		SymbolList &symbols() { return mSymbols; }
		Symbol *newTemp(Front::Type *type);
		Symbol *addSymbol(const std::string &name, Front::Type *type);
		void addSymbol(Symbol *symbol);
		Symbol *findSymbol(const std::string &name);
		Block *newBlock();

		void setCurrentBlock(Block *block);

		void emit(Entry *entry);

	private:
		std::string mName;
		SymbolList mSymbols;
		std::vector<Block*> mBlocks;
		Block *mStart;
		Block *mEnd;

		int mNextTemp;

		Block *mCurrentBlock;
	};
}

#endif