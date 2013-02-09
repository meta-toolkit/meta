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
  createConfigFile(config)
  liblinear = "lib/liblinear-1.92/train"
  svmMethod = "-s 2"
  opts = "-v 5 -q"
  `./learn evalConfig.ini > evalOutput`
  result = `#{liblinear} #{svmMethod} #{opts} evalOutput`
  filename = "#{config["prefix"]}-#{config["method"]}-#{config["ngramOpt"]}-#{config["ngram"]}-#{config["treeOpt"]}"
  filename.gsub!(/-[-]+/, "-")
  filename.gsub!(/-$/, "")
  File.open("runs/#{filename}.run", "w") { |f| f.write(result) }
  f1 = /^ f1:([0-9\.]+)/.match(result)[1].to_f
  acc = /^ f1:([0-9\.]+) acc:([0-9\.]+)/.match(result)[2].to_f
  puts "#{f1.round(4)}\t#{acc.round(4)}\t#{config}"
end

def main()
 
  config = {"prefix" => "sentiment"}

# config["method"] = "ngram"
# for type in ["Char", "Word", "POS", "FW"]
# for type in ["Char"]
#   config["ngramOpt"] = type
#   for n in (1..6)
#     config["ngram"] = n
#     runTest(config)
#   end
# end

# config.delete("ngramOpt")
# config.delete("ngram")
# config["method"] = "tree"

# for treeMethod in ["Subtree", "Branch", "Tag", "Depth", "Skeleton", "SemiSkeleton", "Multi"]
#   config["treeOpt"] = treeMethod
#   runTest(config)
# end

  #for method in ["Word", "POS", "FW", "Char"]
  for method in ["Char"]
    for n in (4..6)
      config["method"] = "both"
      config["ngramOpt"] = method
      config["ngram"] = n
      for treeMethod in ["Subtree", "Branch", "Tag", "Depth", "Skeleton", "SemiSkeleton", "Multi"]
        config["treeOpt"] = treeMethod
        runTest(config)
      end
    end
  end

  `rm evalOutput evalConfig.ini`
end

main()
