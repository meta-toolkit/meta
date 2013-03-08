CLANG_LIBS =-lclangTooling\
				-lclangFrontendTool\
				-lclangFrontend\
				-lclangDriver\
				-lclangSerialization\
				-lclangCodeGen\
				-lclangParse\
				-lclangSema\
				-lclangStaticAnalyzerFrontend\
				-lclangStaticAnalyzerCheckers\
				-lclangStaticAnalyzerCore\
				-lclangAnalysis\
				-lclangARCMigrate\
				-lclangRewriteFrontend\
				-lclangRewriteCore\
				-lclangEdit\
				-lclangAST\
				-lclangLex\
				-lclangBasic 

CXXFLAGS = -Wall -O0 -g $(CLANG_LIBS) `llvm-config --libs --cxxflags --ldflags`

all: lexer

lexer: lexer.o
	$(CXX) -o $@ $^ $(CXXFLAGS)

lexer.o: lexer.cpp

clean:
	-rm -rf *.o lexer
