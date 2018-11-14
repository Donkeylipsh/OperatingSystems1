#!/usr/bin/python

######################################################################################
# Author: Colin Van Overschelde
# Date: 5/20/2018
# Assignment: Project Py
# Description: Project Py demonstrates using a Python script to create 3 files containing
#	a string of random lowercase integers. As well as multiplying random integers
######################################################################################

import sys	# Required for output
import random	# Required to generate random numbers

# Seed the random generator
random.seed()

# Create 3 files, each containing 3 strings of 10 random lowercase characters
fileCount = 1
while(fileCount < 4):
    # Create string of 10 random lowercase characters
    letterCount = 0
    myString = ""
    while(letterCount < 10):
	# Get random ascii code
	n = random.randint(97, 122)
	## Add teh character equivilent to myString
	myString = myString + str(unichr(n))
	letterCount = letterCount + 1
    myString = myString + "\n"
    # Write myString to stdout and to file
    sys.stdout.write(myString)
    f = open("file" + str(fileCount) + ".txt", "w+")
    f.write(myString)
    fileCount = fileCount + 1

#Generate two random integers between 1 and 42 inclusive
num1 = random.randint(1, 42)
sys.stdout.write(str(num1) + "\n")
num2 = random.randint(1, 42)
sys.stdout.write(str(num2) + "\n")
# Display their product
num3 = num1 * num2
sys.stdout.write(str(num3) + "\n")
