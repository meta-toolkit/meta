# McNemar test with 1 degree of freedom

s = ARGV[0].to_f
f = ARGV[1].to_f

result =(((s - f).abs - 1) ** 2) / (s + f)

ok = "no!"
ok = ".05" if result >= 3.84
ok = ".01" if result >= 6.64
ok = ".001" if result >= 10.83

puts "#{ok} #{result}"
