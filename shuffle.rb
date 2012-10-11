#!/usr/bin/env ruby

path = ARGV[0]
numTraining = ARGV[1].to_i
numTesting = -2

# if no arg for num testing, make it the rest of the files
numTesting = ARGV[2].to_i unless ARGV.size != 3

prefix = "/home/sean/projects/senior-thesis-data/"
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
