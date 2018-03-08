#!/usr/bin/python3
# -*- coding: utf-8 -*-

"""
ZetCode PyQt5 tutorial 
Website: zetcode.com 
"""

import sys
from PyQt5.QtWidgets import (QWidget, QToolTip, 
    QPushButton, QApplication, QTextEdit)
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
        self.x_pos_input = QTextEdit('0.0', self)    
        self.x_pos_input.resize(200,20)
        self.x_pos_input.move(20, 20)
        self.y_pos_input = QTextEdit('0.0', self)    
        self.y_pos_input.resize(200,20)
        self.y_pos_input.move(20, 50)
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