import matplotlib.pyplot as plt
import matplotlib.animation as animation
import time
import os
import sys
import easygui as eg
from os import listdir
from os.path import isfile, join

CountersFolder = "samples"
class test:
    sub_plot = None
    file_name = ""

def ShowOptions(option_list, title, header):
    choice = eg.multchoicebox(header , title, option_list)
    return choice

def ShowMenuOptions():
    option_list = ["exit", "graphs"]
    choice = ShowOptions(option_list, "Dashboard", "MainMenu")
    print choice
    if choice[0] == "exit":
        os.system("killall -9 tcp_stack");
        sys.exit(0)
    if choice[0] == "graphs":
        ShowGraphOptions()

def ShowGraphOptions():
    files = [f for f in listdir(CountersFolder) if isfile(join(CountersFolder, f))]
    listOfOptions = ShowOptions(files, "Tcp_stack", "Graph")
    ShowGraph(listOfOptions)

def ShowGraph(graph_list):
    length = len(graph_list)
    fig = plt.figure()
    obj_list = []
    fig.suptitle('Counter Plotter', fontsize=10, fontweight='bold')
    i = 0
    for files in graph_list:
        sub_plot = fig.add_subplot(length,1,i)
        obj = test()
        obj.sub_plot = sub_plot;
        obj.file_name = files
        obj_list.append(obj)
        i += 1
    ani = animation.FuncAnimation(fig, animate, interval=1000, fargs=(obj_list, ))
    plt.show()

def animate(i, obj_list):
    i = 0
    xar = []
    yar = []
    for objects in obj_list:
        pullData = open("samples/" + objects.file_name,"r").read()
        dataArray = pullData.split('\n')
        xa = []
        ya = []
        length = len(dataArray)
        if length < 10:
            start = 0
        else:
            start = length - 10
        for i in range(start, length):
            eachLine = dataArray[i]
            if len(eachLine)>1:
                 x,y = eachLine.split(',')
            xa.append(int(x))
            ya.append(int(y))
            i += 1
        xar.append(xa)
        yar.append(ya)
    i = 0
    for objects in obj_list:
        objects.sub_plot.clear()
    for objects in obj_list:
        objects.sub_plot.set_ylabel(objects.file_name)
        objects.sub_plot.plot(xar[i],yar[i])
        i += 1

while True:
    ShowMenuOptions()
