#!/usr/bin/python3
# -*- coding: utf-8 -*-

"""
ZetCode PyQt5 tutorial 
Website: zetcode.com 
"""

import sys
from PyQt5.QtWidgets import (QWidget, QToolTip, 
    QPushButton, QApplication, QTextEdit, QLabel)
from PyQt5.QtGui import (QFont)  

from PyQt5.QtCore import pyqtSlot


class CraneTest(QWidget):
    
    def __init__(self):
        super().__init__()
        
        self.initUI()
        
        
    def initUI(self):
        
        QToolTip.setFont(QFont('SansSerif', 10))
        
        self.setToolTip('Pooya\'s custom test widget.')
        
        self.btn = QPushButton('Start', self)
        # self.btn2 = QPushButton('St', self)
        self.btn.setToolTip('Start the <b>motor</b>.')
        self.btn.resize(self.btn.sizeHint())
        self.btn.move(20, 100)
        self.btn.clicked.connect(self.on_click)

        # x_pos = QLabel('X-Position')
        # y_pos = QLabel('Y-Position')

        # titleEdit = QLineEdit()
        x_label = QLabel('X-Position', self)
        x_label.move(10, 18)
        self.x_pos_input = QTextEdit('', self)
        self.x_pos_input.setPlaceholderText("0.0")
        self.x_pos_input.resize(100,25)
        self.x_pos_input.move(80, 15)
        y_label = QLabel('Y-Position', self)
        y_label.move(10, 55)
        self.y_pos_input = QTextEdit('', self)
        self.y_pos_input.setPlaceholderText("0.0")
        self.y_pos_input.resize(100,25)
        self.y_pos_input.move(80, 52)
        # authorEdit = QLineEdit()
        # reviewEdit = QTextEdit()

        # grid = QGridLayout()
        # grid.setSpacing(10)

        # grid.addWidget(title, 1, 0)
        # grid.addWidget(titleEdit, 1, 1)
        
        self.setGeometry(300, 300, 400, 300)
        self.setWindowTitle('Tooltips')    
        self.show()


    @pyqtSlot()
    def on_click(self):
        print(self.x_pos_input.toPlainText())
        # print(self.y_pos_input.text())
        text = self.btn.text()
        if text == "Start":
            self.btn.setText("Stop")
        if text == "Stop":
            self.btn.setText("Start")



if __name__ == '__main__':
    print("running app...")

    app = QApplication(sys.argv)

    # w = QWidget()
    # w.resize(250, 150)
    # w.move(300, 300)
    # w.setWindowTitle('Kitau Test App')
    # w.show()

    ex = CraneTest()
    
    sys.exit(app.exec_())