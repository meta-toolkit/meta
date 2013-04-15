#!/usr/bin/env ruby

def even_split(prefix, path)
  corpus = File.open(prefix + path + "/full-corpus.txt", "r")
  entries = corpus.readlines
  puts "Found #{entries.length} documents"
  entries.shuffle!
  train = File.open(prefix + path + "/train.txt", "w")
  test = File.open(prefix + path + "/test.txt", "w")
  for i in 0..(entries.size/2)
    train << entries[i]
  end
  for i in (entries.size/2 + 1)..entries.size
    test << entries[i]
  end
end

def partition(prefix, path, numTraining, numTesting)

  corpus = File.open(prefix + path + "/full-corpus.txt", "r")
  entries = corpus.readlines
  puts "Found #{entries.length} documents"

  # split categories into testing and training
  train = File.open(prefix + path + "/train.txt", "w")
  test = File.open(prefix + path + "/test.txt", "w")

  catMap = {}

  # gather all same categories
  entries.each { |entry|
    # extract 2nd to last directory (the category)
    curCat = entry.match(/^(.+)\/(.+)$/)[1]
    catMap[curCat] = [] if catMap[curCat] == nil
    catMap[curCat] << entry
  }

  catMap.each { |cat, files|
    puts "  - #{files.size} documents in #{cat}"
    # randomize order
    files.shuffle!

    # add to training...
    files[0..(numTraining - 1)].each { |doc| train << doc }

    # ...and the remaining to testing
    files[numTraining..(numTesting + numTraining - 1)].each { |doc| test << doc }
  }

  puts "Found #{catMap.length} categories"
  puts "Training on #{catMap.length * numTraining} documents"
  puts "Testing #{catMap.length * numTesting} documents"
end

def main()
  path = ARGV[0]
  prefix = "/home/sean/projects/senior-thesis-data/"
  if ARGV.size == 1
    even_split(prefix, path)
  else
    numTraining = ARGV[1].to_i
    numTesting = ARGV[2].to_i
    partition(prefix, path, numTraining, numTesting)
  end
end

main()
