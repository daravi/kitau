#!/usr/bin/python3
# -*- coding: utf-8 -*-

"""
ZetCode PyQt5 tutorial 
Website: zetcode.com 
"""

import sys
from PyQt5.QtWidgets import (QWidget, QToolTip, 
    QPushButton, QApplication)
from PyQt5.QtGui import QFont    

from PyQt5.QtCore import pyqtSlot


class CraneTest(QWidget):
    
    def __init__(self):
        super().__init__()
        
        self.initUI()
        
        
    def initUI(self):
        
        QToolTip.setFont(QFont('SansSerif', 10))
        
        self.setToolTip('Pooya\'s custom test widget.')
        
        btn = QPushButton('Start', self)
        btn.setToolTip('Start the <b>motor</b>.')
        btn.resize(btn.sizeHint())
        btn.move(50, 50)
        btn.clicked.connect(self.on_click)
        
        self.setGeometry(300, 300, 300, 200)
        self.setWindowTitle('Tooltips')    
        self.show()


    @pyqtSlot()
    def on_click(self):
        btn.setText("Stop");
        print('PyQt5 button click')

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