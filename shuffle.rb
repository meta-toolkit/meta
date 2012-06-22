file = File.open('full-corpus.txt', 'r')
entries = file.readlines
puts "Found #{entries.length} authors"

authors = [ "austen", "dickens", "doyle", "henry", "kipling",
    "poe", "rls", "shakespeare", "twain", "wells" ]

# split authors into testing and training
train = File.open('train.txt', 'w')
test = File.open('test.txt', 'w')

authorList = {}
for author in authors

    # gather all same authors
    authorList[author] = []
    for entry in entries
        if entry.include? author
            authorList[author] << entry
        end
    end

    # randomize order
    authorList[author].shuffle!
    i = 0

    puts "  - #{authorList[author].size} by #{author}"

    # add first 4 to training...
    for work in authorList[author][0..4]
        train << work
    end

    # ...and the remaining to testing
    for work in authorList[author][5..-1]
        test << work
    end
end
