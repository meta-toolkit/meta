from score import *

def main():
  rater_a = [12, 3, 1, 4, 5, 7, 8]
  rater_b = [12, 3, 1, 4, 5, 7, 2]
  print(quadratic_weighted_kappa(rater_a, rater_b))

main()
