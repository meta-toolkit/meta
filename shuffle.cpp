#include <algorithm>
#include <cstddef>
#include <fstream>
#include <iostream>
#include <random>
#include <string>
#include <unordered_map>
#include <vector>

// ./shuffle ../meta-data/ceeaus ceeaus-nationality
void print_usage( const std::string & name ) {
    std::cout << "Usage: " << name << " filename\n"
        << "\tShuffles the given filename into two equal halves\n" 
        << "Usage: " << name << " filename train test\n"
        << "\tShuffles the given filename into train training examples"
        << " and test testing examples" << std::endl;
}

void even_split( const std::string & prefix,
                 const std::string & list ) {
    std::ifstream file{ prefix + "/" + list + "-full-corpus.txt" };
    std::ofstream train{ prefix + "/" + list + "-train.txt" };
    std::ofstream test{ prefix + "/" + list + "-test.txt" };
    std::vector<std::string> lines;
    std::string line;
    while( std::getline( file, line ) )
        lines.push_back( line );
    std::random_device d;
    std::mt19937 g{ d() };
    std::shuffle( lines.begin(), lines.end(), g );
    auto part_end = lines.begin() + ( lines.end() - lines.begin() ) / 2;
    std::for_each( lines.begin(), part_end, 
                   [&]( const std::string & line ){ 
        train << line << '\n';
    });
    std::for_each( part_end, lines.end(),
                   [&]( const std::string & line ) {
        test << line << '\n';
    });
    std::cout << "Training on " << part_end - lines.begin() << " documents\n"
        << "Testing on " << lines.end() - part_end << " documents" << std::endl;
}

void partition( const std::string & prefix,
                const std::string & list,
                size_t num_training,
                size_t num_testing ) {
    std::ifstream file{ prefix + "/" + list + "-full-corpus.txt" };
    std::ofstream train{ prefix + "/" + list + "-train.txt" };
    std::ofstream test{ prefix + "/" + list + "-test.txt" };
    
    std::unordered_map<std::string, std::vector<std::string>> cats;
    std::string line;
    while( std::getline( file, line ) ) {
        size_t end_class = line.find_first_of( " " );
        std::string label = line.substr( 0, end_class );
        cats[ label ].push_back( line.substr( end_class + 1 ) );
    }
    std::random_device d;
    std::mt19937 g{ d() };
    for( auto & p : cats ) {
        std::string label = p.first;
        auto & docs = p.second;
        if( num_training + num_testing > docs.size() ) {
            throw std::runtime_error{ 
                "Inadequate data for requested partition size" };
        }
        std::shuffle( docs.begin(), docs.end(), g );
        auto part_end = docs.begin() + num_training;
        std::for_each( docs.begin(), part_end,
                       [&]( const std::string & line ) {
            train << label << " " << line << '\n';
        });
        std::for_each( part_end, part_end + num_testing,
                       [&]( const std::string & line ) {
            test << label << " " << line << '\n';
        });
    }
    
    std::cout << "Found " << cats.size() << " categories\n"
        << "Training on " << cats.size() * num_training << " documents\n"
        << "Testing on " << cats.size() * num_testing << " documents" 
        << std::endl;
}

int main( int argc, char ** argv ) {
    std::vector<std::string> args( argv, argv + argc );
    if( args.size() < 3 ) {
        print_usage( args[0] );
        return 1;
    }
    if( args.size() == 3 ) {
        even_split( args[1], args[2] );
    } else {
        partition( args[1], args[2], std::stoi( args[3] ), std::stoi( args[4] ) );
    }
}
