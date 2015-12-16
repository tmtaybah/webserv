#!/usr/bin/python
# my-histogram.cgi -- Prints out a histogram

import sys
import re
import subprocess
import HTMLParser

patterns = {}


def make_plot():
    proc = subprocess.Popen(['gnuplot', '-p'],
                            shell=True,
                            stdin=subprocess.PIPE,
                            )

    proc.stdin.write('set terminal jpeg truecolor; set autoscale\n')
    proc.stdin.write('set title "Histogram"; set xtic auto\n')
    proc.stdin.write('set style data histograms\n')
    proc.stdin.write('set output "histogram.jpeg"\n')
    proc.stdin.write('set xlabel "Patterns"; set ylabel "Frequency"\n')
    proc.stdin.write("plot 'histogram_data.dat' using 2:xtic(1) title 'Frequency'\n")
    proc.stdin.write('quit\n')  # close the gnuplot window

    # proc.kill()


def write_data():
    # Write to file
    f = open('histogram_data.dat', 'w')
    for pattern in patterns:
        f.write("%s %d\n" % (pattern, patterns[pattern]))
    f.close()


def calculate_histogram():
    h = HTMLParser.HTMLParser()

    # Process patterns into Dictionary
    for pattern in args[2:]:
        patterns[h.unescape(pattern)] = 0

    # Open and read file into text, and tokenize.
    filename = args[1]
    txt = open(filename).read().split(" ")

    # Calculate Histogram
    for word in txt:
        for pattern in patterns:
            result = re.match(pattern, word)
            if result is not None:
                patterns[pattern] += 1


def pretty_print_result():
    img = 'histogram.jpeg'
    title = "CS410 Webserver"

    print ("<!DOCTYPE html><html><head><title>%s</title><h1 style='color:red;font-size: 16pt;\
     text-align: center;'>%s</h1></head><body style='background-color:white;'><br style='height: 150px'>\
     <img src='%s'></body></html>\n" % (title, title, img))


if __name__ == "__main__":

    args = sys.argv

    print("Content-type: text/html\n\n")

    # Check if at lease a file name and a one pattern are provided
    if len(args) < 3:
        print("Must provide a file, and at least one pattern")

    calculate_histogram()
    write_data()
    make_plot()
    pretty_print_result()

    sys.exit(1)
