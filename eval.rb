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
  `./learn evalConfig.ini > evalOutput`
  result = `#{liblinear} #{svmMethod} #{opts} evalOutput`
  puts "#{/[0-9.%]+/.match(result)[0]}\t#{config}"
end

def main()
 
  dataset = "ceeaus"
  config = {"quiet" => "yes", "prefix" => dataset, "parallel" => "no", "method" => "ngram"}

  config["ngramOpt"] = "Word"
  config["ngram"] = 1
  createConfigFile(config)
  runTest(config)
  config["parallel"] = "yes"
  createConfigFile(config)
  runTest(config)

  exit
  
  for type in ["Word", "POS", "FW"]
    config["ngramOpt"] = type
    for n in (1..6)
      config["ngram"] = n
      createConfigFile(config)
      runTest(config)
    end
  end

  config.delete("ngramOpt")
  config.delete("ngram")
  config["method"] = "tree"

  for treeMethod in ["Subtree", "Branch", "Tag", "Depth", "Skeleton", "SemiSkeleton", "Multi"]
    config["treeOpt"] = treeMethod
    createConfigFile(config)
    runTest(config)
  end

  for method in ["Word", "POS", "FW"]
    for n in (1..3)
      config["method"] = "both"
      config["ngramOpt"] = method
      config["ngram"] = n
      config["treeOpt"] = "Subtree"
      createConfigFile(config)
      runTest(config)
      config["treeOpt"] = "Multi"
      createConfigFile(config)
      runTest(config)
    end
  end

  `rm evalOutput evalConfig.ini`
end

main()
