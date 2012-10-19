#!/usr/bin/env ruby
# evaluates classification using liblinear with various features

def createConfigFile(config)
  configFile = File.open("evalConfig.ini", "w")
  config.each do |opt, val|
    configFile << opt << " " << val << "\n"
  end
  configFile.close()
end

def runTest(config)
  liblinear = "lib/train"
  svmMethod = "-s 2"
  opts = "-v 5 -q"
  `./liblinear evalConfig.ini > evalOutput`
  result = `#{liblinear} #{svmMethod} #{opts} evalOutput`
  puts "#{config}\t#{/[0-9.%]+/.match(result)[0]}"
end

def main()
  dataset = "ceeaus"
  config = {"quiet" => "yes", "prefix" => dataset}
  for n in (1..6)
    config["method"] = "ngram"
    config["ngramOpt"] = "POS"
    config["ngram"] = n
    createConfigFile(config)
    runTest(config)
  end
  config.delete("ngramOpt")
  config.delete("ngram")
  for treeMethod in ["Subtree", "Branch", "Tag", "Depth"]
    config["method"] = "tree"
    config["treeOpt"] = treeMethod
    createConfigFile(config)
    runTest(config)
  end
  `rm evalOutput evalConfig.ini`
end

main()
