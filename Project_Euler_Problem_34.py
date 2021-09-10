# Project Euler Problem 34:
# "Find the sum of all numbers which are 
# equal to the sum of the factorial of 
# their digits.
#
# For example, 145 = 1! + 4! + 5!
#
# Note: As 1! = 1 and 2! = 2 are not sums,
# they are not included."

import math

# I define a function to check if a number
# is the sum of the factorial of its digits. 
def isfactsum(n):
  a = str(n)
  b = 0
  for i in range(0,len(a)):
    b = b + math.factorial(int(a[i]))
  if b == n:
    return 1
  else:
    return 0

# I determine the highest number this 
# could possibly be true for.
b = math.factorial(9)*7
factsum = 0

# I test all numbers lower than my
# calculated upper bound and add them
# to my running sum, also printing each
# valid value as a sanity check.
for i in range(10,b):
  if isfactsum(i):
    factsum = factsum + i
    print(i)

# I print the result
print(factsum)