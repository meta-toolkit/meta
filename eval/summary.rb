dataset = ARGV[0]
for file in `ls ../runs | grep #{dataset}`.split("\n")
  text = File.open("../runs/#{file}", "r").read
  score = text.match(/^ f1:(.*)/)[1]
  score.gsub!(/f1:/, "")
  score.gsub!(/acc:/, "")
  score.gsub!(/p:/, "")
  score.gsub!(/r:/, "")
  puts "#{score} #{file.gsub(/.run/, '')}"
end
