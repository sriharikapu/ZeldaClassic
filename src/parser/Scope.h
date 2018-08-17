#ifndef ZSCRIPT_SCOPE_H
#define ZSCRIPT_SCOPE_H

#include <map>
#include <string>
#include <vector>
#include "CompilerUtils.h"

class CompileErrorHandler;

// Forward declarations from AST.h
class AST;

namespace ZScript
{
	// Forward declarations from ZScript.h
	class Script;
	class Datum;
	class Function;
	class FunctionSignature;

	// Forward declarations from Types.h
	class DataType;
	class TypeStore;

	// Forward declarations from CompileOption.
	class CompileOption;

	// Local forward declarations
	class FunctionScope;
	class ZClass;

	class Scope : private NoCopy
	{
		// So Datum classes can only be generated in tandem with a scope.
		friend class Datum;
		
	public:
		Scope(TypeStore&);
		Scope(TypeStore&, std::string const& name);

		// Accessors
		TypeStore const& getTypeStore() const {return typeStore;}
		TypeStore& getTypeStore() {return typeStore;}
		optional<std::string> const& getName() const {return name;}
		optional<std::string>& getName() {return name;}

		// Scope Type
		virtual bool isGlobal() const {return false;}
		virtual bool isScript() const {return false;}
		virtual bool isFunction() const {return false;}

		// Inheritance
		virtual Scope* getParent() const = 0;
		virtual Scope* getChild(std::string const& name) const = 0;
		virtual std::vector<Scope*> getChildren() const = 0;
	
		// Lookup Local
		virtual DataType const* getLocalType(
				std::string const& name) const = 0;
		virtual ZClass* getLocalClass(std::string const& name) const = 0;
		virtual Datum* getLocalDatum(std::string const& name) const = 0;
		virtual Function* getLocalGetter(std::string const& name) const = 0;
		virtual Function* getLocalSetter(std::string const& name) const = 0;
		virtual Function* getLocalFunction(
				FunctionSignature const& signature) const = 0;
		virtual std::vector<Function*> getLocalFunctions(
				std::string const& name) const = 0;
		virtual optional<long> getLocalOption(CompileOption option) const = 0;
	
		// Get All Local.
		virtual std::vector<Datum*> getLocalData() const = 0;
		virtual std::vector<Function*> getLocalFunctions() const = 0;
		virtual std::vector<Function*> getLocalGetters() const = 0;
		virtual std::vector<Function*> getLocalSetters() const = 0;
		virtual std::map<CompileOption, long> getLocalOptions() const = 0;

		// Add
		virtual Scope* makeChild() = 0;
		virtual Scope* makeChild(std::string const& name) = 0;
		virtual FunctionScope* makeFunctionChild(Function& function) = 0;
		virtual DataType const* addType(
				std::string const& name, DataType const* type, AST* node)
		= 0;
		//virtual ZClass* addClass(string const& name, AST* node) = 0;
		virtual Function* addGetter(
				DataType const* returnType, std::string const& name,
				std::vector<DataType const*> const& paramTypes,
				AST* node = NULL)
		= 0;
		virtual Function* addSetter(
				DataType const* returnType, std::string const& name,
				std::vector<DataType const*> const& paramTypes,
				AST* node = NULL)
		= 0;
		virtual Function* addFunction(
				DataType const* returnType, std::string const& name,
				std::vector<DataType const*> const& paramTypes,
				AST* node = NULL)
		= 0;
		virtual void setOption(CompileOption option, long value) = 0;

		////////////////
		// Stack

		// If this scope starts a new stack frame, return its total stack
		// size.
		virtual optional<int> getRootStackSize() const {return nullopt;}

		// Let this scope know that it needs to recalculate the stack size.
		virtual void invalidateStackSize();
		
		// Get the depth of the stack for this scope, not considering its
		// children.
		virtual int getLocalStackDepth() const {return 0;}

		// Get the stack offset for this local datum.
		virtual optional<int> getLocalStackOffset(Datum const&) const {
			return nullopt;}
		
		bool varDeclsDeprecated;

	protected:
		TypeStore& typeStore;
		optional<std::string> name;

	private:
		// Add the datum to this scope, returning if successful. Called by
		// the Datum classes' ::create functions.
		virtual bool add(ZScript::Datum&, CompileErrorHandler*) = 0;
	};

	////////////////
	// Inheritance

	// Repeatedly get a child namespace with the names in order. Fail if any
	// name does not resolve.
	Scope* getDescendant(
			Scope const&, std::vector<std::string> const& names);

	// Find a scope with the given name in this scope.
	Scope* lookupScope(Scope const&, std::string const& name);

	// Find first scope with the given ancestry in this scope.
	Scope* lookupScope(Scope const&, std::vector<std::string> const& names);

	// Find all scopes with the given ancestry in this scope. Note than an
	// empty name list will the current scope and its ancestry.
	std::vector<Scope*> lookupScopes(
			Scope const&, std::vector<std::string> const& names);
	
	////////////////
	// Lookup

	// Attempt to resolve name to a type id under scope.
	DataType const* lookupType(Scope const&, std::string const& name);
	
	// Attempt to resolve name to a class id under scope.
	ZClass* lookupClass(Scope const&, std::string const& name);

	// Attempt to resolve name to a variable under scope.
	Datum* lookupDatum(Scope const&, std::string const& name);
	Datum* lookupDatum(Scope const&, std::vector<std::string> const& name);
	
	// Attempt to resolve name to a getter under scope.
	Function* lookupGetter(Scope const&, std::string const& name);

	// Attempt to resolve name to a setter under scope.
	Function* lookupSetter(Scope const&, std::string const& name);

	// Attempt to resolve signature to a function under scope.
	Function* lookupFunction(Scope const&, FunctionSignature const&);
	
	// Attempt to resolve name to possible functions under scope.
	std::vector<Function*> lookupFunctions(
			Scope const&, std::string const& name);
	std::vector<Function*> lookupFunctions(
			Scope const&, std::vector<std::string> const& name);

	// Resolve an option value under the scope. Will only return empty if
	// the provided option is invalid. If the option is valid but not set,
	// returns the default value for it.
	optional<long> lookupOption(Scope const&, CompileOption);

	////////////////
	// Stack

	// Does this scope start a new stack frame?
	bool isStackRoot(Scope const&);

	// Get the stack offset for a datum, checking parents until we hit a
	// root.
	optional<int> lookupStackOffset(Scope const&, Datum const&);

	// Find the total size of the stack scope is in.
	optional<int> lookupStackSize(Scope const&);
	
	// Lookup the stack offset and then subtract it from the root stack
	// size.
	optional<int> lookupStackPosition(Scope const&, Datum const&);
	
	////////////////
	// Get all in branch

	// Recursively get all of something for a scope and its children.
	// usage: getInBranch<Function*>(scope, &Scope::getLocalFunctions)
	template <typename Element>
	std::vector<Element> getInBranch(
			Scope const& scope,
			std::vector<Element> (Scope::* call)() const)
	{
		std::vector<Element> results = (scope.*call)();
		std::vector<Scope*> children = scope.getChildren();
		for (std::vector<Scope*>::const_iterator it = children.begin();
		     it != children.end(); ++it)
		{
			std::vector<Element> subResults = getInBranch(**it, call);
			results.insert(results.end(),
			               subResults.begin(), subResults.end());
		}
		return results;
	}

	std::vector<Function*> getFunctionsInBranch(Scope const& scope);
	
	////////////////////////////////////////////////////////////////
	// BasicScope - Primary Scope implementation.
	
	class FunctionScope;
	class BasicScope : public Scope
	{
	public:
		BasicScope(Scope* parent);
		BasicScope(Scope* parent, std::string const& name);
		virtual ~BasicScope();

		// Inheritance
		Scope* getParent() const {return parent;}
		Scope* getChild(std::string const& name) const;
		std::vector<Scope*> getChildren() const;
	
		// Lookup Local
		DataType const* getLocalType(std::string const& name) const;
		ZClass* getLocalClass(std::string const& name) const;
		Datum* getLocalDatum(std::string const& name) const;
		Function* getLocalGetter(std::string const& name) const;
		Function* getLocalSetter(std::string const& name) const;
		Function* getLocalFunction(FunctionSignature const& signature)
				const;
		std::vector<Function*> getLocalFunctions(std::string const& name)
				const;
		optional<long> getLocalOption(CompileOption option) const;
		
		// Get All Local
		std::vector<ZScript::Datum*> getLocalData() const;
		std::vector<ZScript::Function*> getLocalFunctions() const;
		std::vector<ZScript::Function*> getLocalGetters() const;
		std::vector<ZScript::Function*> getLocalSetters() const;
		std::map<CompileOption, long> getLocalOptions() const;

		// Add
		Scope* makeChild();
		Scope* makeChild(std::string const& name);
		FunctionScope* makeFunctionChild(Function& function);
		DataType const* addType(
				std::string const& name, DataType const* type,
				AST* node = NULL);
		Function* addGetter(
				DataType const* returnType, std::string const& name,
				std::vector<DataType const*> const& paramTypes,
				AST* node = NULL);
		Function* addSetter(
				DataType const* returnType, std::string const& name,
				std::vector<DataType const*> const& paramTypes,
				AST* node = NULL);
		Function* addFunction(
				DataType const* returnType, std::string const& name,
				std::vector<DataType const*> const& paramTypes,
				AST* node = NULL);
		void setOption(CompileOption option, long value);
		
		// Stack
		int getLocalStackDepth() const {return stackDepth;}
		optional<int> getLocalStackOffset(Datum const& datum) const;
		
	protected:
		Scope* parent;
		std::map<std::string, Scope*> children;
		std::vector<Scope*> anonymousChildren;
		std::map<std::string, DataType const*> types;
		std::map<std::string, ZClass*> classes;
		std::vector<Datum*> anonymousData;
		std::map<std::string, Datum*> namedData;
		std::map<Datum*, int> stackOffsets;
		int stackDepth;
		std::map<std::string, Function*> getters;
		std::map<std::string, Function*> setters;
		std::map<std::string, std::vector<Function*> > functionsByName;
		std::map<FunctionSignature, Function*> functionsBySignature;
		std::map<CompileOption, long> options;

		BasicScope(TypeStore&);
		BasicScope(TypeStore&, std::string const& name);

	private:
		// Disabled since it's easy to call by accident instead of the Scope*
		// constructor.
		BasicScope(BasicScope const& base);

		bool add(Datum&, CompileErrorHandler*);
	};

	class ScriptScope;
	class GlobalScope : public BasicScope
	{
	public:
		// Creates the starting global scope.
		GlobalScope(TypeStore&);
		
		bool isGlobal() const {return true;}
		ScriptScope* makeScriptChild(Script& script);
		optional<int> getRootStackSize() const;

	private:
		mutable optional<int> stackSize;
	};

	class ScriptScope : public BasicScope
	{
	public:
		ScriptScope(GlobalScope* parent, Script& script);
		bool isScript() const {return true;}
		Script& script;
	};

	class FunctionScope : public BasicScope
	{
	public:
		FunctionScope(Scope* parent, Function& function);
		bool isFunction() const {return true;}
		Function& function;
		optional<int> getRootStackSize() const;
	private:
		mutable optional<int> stackSize;
	};

	enum ZClassIdBuiltin
	{
		ZCLASSID_START = 0,
		ZCLASSID_GAME = 0,
		ZCLASSID_LINK,
		ZCLASSID_SCREEN,
		ZCLASSID_FFC,
		ZCLASSID_ITEM,
		ZCLASSID_ITEMCLASS,
		ZCLASSID_NPC,
		ZCLASSID_LWPN,
		ZCLASSID_EWPN,
		ZCLASSID_NPCDATA,
		ZCLASSID_DEBUG,
		ZCLASSID_AUDIO,
		ZCLASSID_COMBOS,
		ZCLASSID_SPRITEDATA,
		ZCLASSID_GRAPHICS,
		ZCLASSID_TEXT,
		ZCLASSID_INPUT,
		ZCLASSID_MAPDATA,
		ZCLASSID_DMAPDATA,
		ZCLASSID_ZMESSAGE,
		ZCLASSID_SHOPDATA,
		ZCLASSID_DROPSET,
		ZCLASSID_PONDS,
		ZCLASSID_WARPRING,
		ZCLASSID_DOORSET,
		ZCLASSID_ZUICOLOURS,
		ZCLASSID_RGBDATA,
		ZCLASSID_PALETTE,
		ZCLASSID_TUNES,
		ZCLASSID_PALCYCLE,
		ZCLASSID_GAMEDATA,
		ZCLASSID_CHEATS,
		ZCLASSID_END
	};

	class ZClass : public BasicScope
	{
	public:
		ZClass(TypeStore&, std::string const& name, int id);
		std::string const name;
		int const id;
	};

};
#endif
