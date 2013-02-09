import subprocess
from score import *

def main():
  files = subprocess.check_output(["ls", "../runs"]).decode("utf-8").split()
  files = [f for f in files if "kaggle" in f]
  for f in files:
    lines = open("../runs/" + f).readlines()
    lines = lines[0:13] # only get confusion matrix part
    rater_a, rater_b = [], []
    pred = 0
    try:
      for line in lines:
        line = line.replace("[", "")
        line = line.replace("]", "")
        counts = [int(c) for c in line.split()]
        for actual in range(0, 13):
          rater_a.extend([pred] * counts[actual]) # guessed pred actual times
          rater_b.extend([actual] * counts[actual]) # actual "actual" times
        pred += 1
      print(str(quadratic_weighted_kappa(rater_a, rater_b)) + " " + f)
    except:
      print("error on " + f)

main()
