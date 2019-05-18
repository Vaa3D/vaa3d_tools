
import os
import cv2
import matplotlib.pyplot as plt
import shutil
import tkinter as tk
import tkinter.messagebox


def windows():
    window = tk.Tk()
    window.title('My Window')
    window.geometry('500x300')
    var = tk.StringVar()
    l = tk.Label(window, textvariable=var, bg='green', fg='white', font=('Arial', 12), width=30, height=2)
    l.pack()
    tk.Button(window, text='hit me', bg='green', font=('Arial', 14), command=hit_me).pack()
    window.mainloop()

def hit_me():
    tkinter.messagebox.showinfo(title='Hi', message='delete?')  # 提示信息对话窗

def selectFolder(infolderrbg,outfolderrgb,rawfolder,falsefolderraw):
    for file in os.listdir(infolderrbg):
        print(file)
        # renamefile = file.replace("1:1:e_","")
        # os.rename(infolder+"/"+file,infolder+"/"+renamefile)
        I = cv2.imread(infolderrbg+"/"+file)
        plt.imshow(I)
        plt.show()
        action=input("delete(Y/N)").upper();
        if action=='Y':
            shutil.move(infolderrbg+"/"+file,outfolderrgb+"/"+file)
            shutil.move(rawfolder+"/"+file.replace("_mask.png","_mask.tif"),falsefolderraw+"/"+file.replace("_mask.png","_mask.tif"))
            shutil.move(rawfolder + "/" + file.replace("_mask.png", ".tif"),falsefolderraw + "/" + file.replace("_mask.png", ".tif"))

if __name__=="__main__":
    infolder = "G:/samples_refined/raw_rgb"
    rawfolder = "G:/samples_refined/raw"
    falsefolderrgb = "G:/samples_refined/false_rgb"
    falsefolderraw="G:/samples_refined/false_raw"

    if not os.path.exists(falsefolderrgb):
        os.mkdir(falsefolderrgb)

    if not os.path.exists(falsefolderraw):
        os.mkdir(falsefolderraw)
    selectFolder(infolder,falsefolderrgb,rawfolder,falsefolderraw)

    # windows()
    # hit_me()