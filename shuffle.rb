path = ARGV[0]
puts "Examining collection located at \"#{path}\""

numTraining = ARGV[1].to_i
numTesting = -2

# if no arg for num testing, make it the rest of the files
if ARGV.size == 3
    numTesting = ARGV[2].to_i
end

corpus = File.open(path + "/full-corpus.txt", "r")
entries = corpus.readlines
puts "Found #{entries.length} documents"

Dir.chdir(path)
categories = Dir["*"].reject{ |f| not File.directory? (f) }
puts "Found #{categories.length} categories"

# split categories into testing and training
train = File.open(path + "/train.txt", "w")
test = File.open(path + "/test.txt", "w")

catMap = {}
for cat in categories

    # gather all same categories
    catMap[cat] = []
    for entry in entries
        if entry.include? cat
            catMap[cat] << entry
        end
    end

    puts "  - #{catMap[cat].size} documents in #{cat}"

    # randomize order
    catMap[cat].shuffle!
    i = 0

    # add to training...
    for doc in catMap[cat][0..(numTraining - 1)]
        train << doc
    end

    # ...and the remaining to testing
    for doc in catMap[cat][numTraining..(numTesting + 1)]
        test << doc
    end
end
