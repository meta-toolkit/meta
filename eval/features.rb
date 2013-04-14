def get_f1(str)
  /f1:([^ ]*) /.match(str)[1]
end

def get_acc(str)
  /acc:([^ ]*) /.match(str)[1]
end

filename = ARGV[0]
file = File.open(filename)
text = file.readlines
file.close()

svm = Array.new(50, text.shift)
svm.map! {|str| get_acc(str) }

scores = [svm, [], [], [], []]

idx = 1
for line in text
  if line[0] != 'U'
    scores[idx] << get_acc(line)
    idx += 1
    idx = 1 if idx >= scores.size
  end
end

output = File.open(filename + ".plot", 'w')
idx = 0
while idx < scores[0].size
  output << scores[0][idx] << " "   # svm
  output << scores[1][idx] << " "   # 
  output << scores[2][idx] << " "   #
  output << scores[3][idx] << " "   #
  output << scores[4][idx] << "\n"  # sLDA
  idx += 1
end
output.close()

`cp #{filename + ".plot"} input.dat`
`R plot-features.R`
