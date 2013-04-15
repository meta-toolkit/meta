import subprocess
from score import *

def main():
  prefix = "/home/sean/Dropbox/results/"
  files = subprocess.check_output(["ls", prefix]).decode("utf-8").split()
  files = [f for f in files if "kaggle" in f]
  for f in files:
    lines = open(prefix + f).readlines()
    rater_a, rater_b = [], []
    try:
      for line in lines:
        nums = line.split()
        rater_a.append(int(nums[0]))
        rater_b.append(int(nums[1]))
      print(str(quadratic_weighted_kappa(rater_a, rater_b)) + " " + f)
    except:
      print("error on " + f)

main()
