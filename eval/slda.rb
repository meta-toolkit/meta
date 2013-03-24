#!/usr/bin/env ruby

def createConfigFile(config)
  configFile = File.open("evalConfig.ini", "w")
  config.each do |opt, val|
    configFile << opt << " " << val << "\n"
  end
  configFile.close()
end

def runTest(config)
  createConfigFile(config)
  command = "../slda-test evalConfig.ini ../"
 #begin
    result = `#{command}`
    filename = "slda-#{config["prefix"]}-#{config["percent"]}-#{config["method"]}-#{config["ngramOpt"]}-#{config["ngram"]}-#{config["treeOpt"]}"
    filename.gsub!(/-[-]+/, "-")
    filename.gsub!(/-$/, "")
    File.open("../../senior-thesis-data/slda-runs/#{filename}.run", "w") { |f| f.write(result) }
    puts config
   #slda = /Average accuracy: (.*)/.match(result)[1].to_f.round(4)
    acc = result.scan(/acc:([0-9.]*)/)
   #puts "1.0 #{slda.to_s}"
    first = true 
    p = 1.0
    for a in acc
      puts p.round(4).to_s + " " + a[0].to_f.round(4).to_s
      if first
        p = -0.01
        first = false
      end
      p += 0.02 if not first
    end
 #rescue
 #  puts "ERROR"
 #end
end

def testCombined(config)
  for method in ["Word", "POS", "FW"]
    for n in (3..5)
      config["method"] = "both"
      config["ngramOpt"] = method
      config["ngram"] = n
      for treeMethod in ["Subtree", "Skel", "Semi"]
        config["treeOpt"] = treeMethod
        runTest(config)
      end
    end
  end
end

def testTree(config)
  config["method"] = "tree"
  for treeMethod in ["Subtree", "Skel", "Semi"]
    config["treeOpt"] = treeMethod
    runTest(config)
  end
end

def main()
  config = {"prefix" => "ceeaus"}
  config["method"] = "both"
  config["ngramOpt"] = "Word" 
  config["ngram"] = 4
  for method in ["Subtree", "Semi", "Skel"]
    config["treeOpt"] = method
    runTest(config)
  end
end

main()
