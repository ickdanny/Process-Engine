#pragma once

#include "Logging.h"
#include "Lexer.h"
#include "Parser.h"
#include "Interpreter.h"

#include "Logging.h"

namespace darkness{
	
	//todo: temp for testing, from
	//todo: https://stackoverflow.com/questions/20902945/reading-a-string-from-file-c
	std::string read_string_from_file(const std::string &file_path) {
		const std::ifstream input_stream(file_path, std::ios_base::binary);
		
		if (input_stream.fail()) {
			throw std::runtime_error("Failed to open file");
		}
		
		std::stringstream buffer;
		buffer << input_stream.rdbuf();
		
		return buffer.str();
	}
	
	class TestInterpreter : public Interpreter<>{
	public:
		TestInterpreter(){
			addNativeFunction("print", print);
			addNativeFunction("staller", staller);
		}
		
		static void test(){
			auto input{ read_string_from_file("res\\test.dk") };
			Lexer lexer{};
			auto tokens{ lexer.lex(input) };
			Parser parser{};
			auto script{ parser.parse(tokens) };
			TestInterpreter interpreter{};
			auto scriptState{ interpreter.runScript(script) };
			if( scriptState.stalled ) {
				while( (scriptState = interpreter.resumeScript(script, scriptState)).stalled );
			}
		}
		
	private:
		static DataType print(const std::vector<DataType>& parameters){
			for(const DataType& data : parameters){
				switch(data.index()){
					case boolIndex:
						if(std::get<bool>(data)){
							wasp::debug::log("true");
						}
						else{
							wasp::debug::log("false");
						}
						break;
					case intIndex:
						wasp::debug::log(std::to_string(std::get<int>(data)));
						break;
					case floatIndex:
						wasp::debug::log(std::to_string(std::get<float>(data)));
						break;
					case stringIndex:
						wasp::debug::log(std::get<std::string>(data));
						break;
					case functionIndex:
						throw std::runtime_error("native func print cannot print a function");
					default:
						throw std::runtime_error("native func print bad type");
				}
			}
			return false;
		}
		
		static DataType staller(const std::vector<DataType>& parameters){
			static int i{ 0 };
			wasp::debug::log(std::to_string(i));
			if(i < 100){
				++i;
				throw StallFlag{};
			}
			else{
				i = 0;
				return 12345;
			}
		}
	};
	
	void test() {
		TestInterpreter::test();
		wasp::debug::log("\nend of script");
	}
}
