dataset = ARGV[0]
path = "../../senior-thesis-data/runs"
for file in `ls #{path} | grep #{dataset}`.split("\n")
  text = File.open("#{path}/#{file}", "r").read
  score = text.match(/^ f1:(.*)/)[1]
  score.gsub!(/f1:/, "")
  score.gsub!(/acc:/, "")
  score.gsub!(/p:/, "")
  score.gsub!(/r:/, "")
  puts "#{score} #{file.gsub(/.run/, '')}"
end
