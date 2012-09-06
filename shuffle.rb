#!/usr/bin/env ruby

path = ARGV[0]
numTraining = ARGV[1].to_i
numTesting = -2

# if no arg for num testing, make it the rest of the files
numTesting = ARGV[2].to_i unless ARGV.size != 3

corpus = File.open(path + "/full-corpus.txt", "r")
entries = corpus.readlines
puts "Found #{entries.length} documents"

Dir.chdir(path)
categories = Dir["*"].reject{ |f| not File.directory? (f) }
puts "Found #{categories.length} categories"
puts "Training on #{categories.length * numTraining} documents"
puts "Testing #{categories.length * numTesting} documents"

# split categories into testing and training
train = File.open(path + "/train.txt", "w")
test = File.open(path + "/test.txt", "w")

catMap = {}
categories.each { |cat|

  # gather all same categories
  catMap[cat] = []
  entries.each { |entry| catMap[cat] << entry if entry.include? cat }
  puts "  - #{catMap[cat].size} documents in #{cat}"

  # randomize order
  catMap[cat].shuffle!

  # add to training...
  catMap[cat][0..(numTraining - 1)].each { |doc| train << doc }

  # ...and the remaining to testing
  catMap[cat][numTraining..(numTesting + numTraining - 1)].each { |doc| test << doc }

}
