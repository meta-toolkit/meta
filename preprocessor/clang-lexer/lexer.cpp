/**
 * @file preprocessor/clang-lexer/lexer.cpp
 * A simple clang-based lexer for C/C++ code, which simply outputs the
 * tokens it sees in all files in a directory.
 *
 * @see https://github.com/loarabia/Clang-tutorial/blob/master/CItutorial2.cpp
 */

#ifndef _DST_CLANG_LEXER_LEXER_H_
#define _DST_CLANG_LEXER_LEXER_H_

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include <llvm/Support/FileSystem.h>
#include <llvm/Support/PathV2.h>
#include <llvm/Support/Host.h>
#include <llvm/ADT/IntrusiveRefCntPtr.h>

#include <clang/Basic/DiagnosticOptions.h>
#include <clang/Frontend/TextDiagnosticPrinter.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Basic/TargetOptions.h>
#include <clang/Basic/TargetInfo.h>
#include <clang/Basic/FileManager.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Lex/Preprocessor.h>
#include <clang/Basic/Diagnostic.h>

void run_lexer_on_file( const std::string & filename ) {
    std::cout << "Parsing file " << filename << "..." << std::endl;

    llvm::SmallString<128> output_filename( filename );
    llvm::sys::path::replace_extension( output_filename, ".cpp.lex" );

    std::ofstream output( output_filename.c_str() );

    clang::CompilerInstance ci;
    ci.createDiagnostics(0, 0);
    
    clang::TargetOptions * targ_opts = new clang::TargetOptions();
    targ_opts->Triple = llvm::sys::getDefaultTargetTriple();
    clang::TargetInfo * targ = clang::TargetInfo::CreateTargetInfo( ci.getDiagnostics(), 
            *targ_opts );
    ci.setTarget( targ );
    
    ci.createFileManager();
    ci.createSourceManager( ci.getFileManager() );
    ci.createPreprocessor();
    const clang::FileEntry * file = ci.getFileManager().getFile( filename );
    ci.getSourceManager().createMainFileID( file );
    ci.getPreprocessor().EnterMainSourceFile();
    ci.getDiagnosticClient().BeginSourceFile( ci.getLangOpts(), &ci.getPreprocessor() );
    
    clang::Token tok;
    do {
        ci.getPreprocessor().Lex( tok );
        output << tok.getName() << '\n';
    } while( tok.isNot( clang::tok::eof ) );
    ci.getDiagnosticClient().EndSourceFile();
}

int run_lexer_on_directory( const std::string & directory ) {
    // iterate over the directory looking for all .cpp files
    llvm::error_code ec;
    for( llvm::sys::fs::directory_iterator dir_iter( directory, ec ), end;
            dir_iter != end;
            dir_iter.increment( ec ) ) {
        if( ".cpp" == llvm::sys::path::extension( dir_iter->path() ).str() )
            run_lexer_on_file( dir_iter->path() );
    }
    return 0;
}

int print_usage( const std::string & prog_name ) {
    std::cout << "Usage: " << prog_name << " directory\n"
        << "\tLexes each file in in the given directory, "
        "placing the output into a .lex file\n"
        "\te.g., filename.cpp -> filename.cpp.lex" << std::endl;
    return 1;
}

int main( int argc, char ** argv ) {
    std::vector<std::string> args( argv, argv + argc );
    if( args.size() < 2 )
        return print_usage( args[0] );
    return run_lexer_on_directory( args[1] );
}

#endif
