from PyQt5 import QtWidgets, uic
from PyQt5.QtSerialPort import QSerialPort, QSerialPortInfo
from PyQt5.QtCore import QIODevice
from pyqtgraph import PlotWidget
import pyqtgraph as pg
import sys

# https://stackoverflow.com/questions/35932660/qcombobox-click-event Atnaujiname paspaudus

app = QtWidgets.QApplication([])
ui = uic.loadUi("design.ui")
ui.setWindowTitle("SerialGUI")

serial = QSerialPort()
serial.setBaudRate(115200)
portList = []
ports = QSerialPortInfo().availablePorts()
for port in ports:
    portList.append(port.portName())
ui.comL.addItems(portList)

posX = 200
posY = 100
listX = []
for x in range(250): listX.append(x)
listY = []
for x in range(250): listY.append(0)


def onRead():
    if not serial.canReadLine(): return     # Išeiname jeigu nėra ką skaityti
    rx = serial.readLine()
    rxs = str(rx, 'utf-8').strip()
    data = rxs.split(',')
    if data[0] == '0':
        ui.lcdN.display(data[1])
        ui.tempB.setValue(int(float(data[3]) * 10))
        ui.tempL.setText(data[3])
        global listX
        global listY
        listY = listY[1:]
        listY.append(int(data[2]))
        ui.graph.clear()
        ui.graph.plot(listX, listY)


#def rutuliukasControl():
    if data[0] == '1':
        if data[1] == '0':
            ui.rutuliukas.setChecked(True)
        else:
            ui.rutuliukas.setChecked(False)

    if data[0] == '2':
        global posX
        global posY
        posX += int((int(data[1]) - 512) / 100)
        posY += int((int(data[2]) - 512) / 100)
        ui.rutuliukas.setGeometry(posX, posY, 20, 20)


def onOpen():
    serial.setPortName(ui.comL.currentText())
    serial.open(QIODevice.ReadWrite)


def serialSend(data):
    txs = ""
    for val in data:
        txs += str(val)
        txs += ','
    txs = txs[:-1]
    txs += ';'
    serial.write(txs.encode())


def onClose():
    serial.close()


def ledControl(val):
    if val == 2: val = 1;
    serialSend([0, val])


def fenControl(val):
    if val == 2: val = 1;
    serialSend([3, val])


def releControl(val):
    if val == 2: val = 1;
    serialSend([4, val])


def RGBcontrol():
    serialSend([1, ui.RS.value(), ui.GS.value(), ui.BS.value()])


def servoControl(val):
    serialSend([2, val])


def sendText():
    txs = "5,"
    txs += ui.textF.displayText()
    txs += ';'
    serial.write(txs.encode())


serial.readyRead.connect(onRead)
ui.openB.clicked.connect(onOpen)
ui.closeB.clicked.connect(onClose)
#ui.rutuliukas.clicked.connect(rutuliukasControl)
ui.led.stateChanged.connect(ledControl)
ui.fen.stateChanged.connect(fenControl)
ui.rele.stateChanged.connect(releControl)
ui.RS.valueChanged.connect(RGBcontrol)
ui.GS.valueChanged.connect(RGBcontrol)
ui.BS.valueChanged.connect(RGBcontrol)
ui.servoK.valueChanged.connect(servoControl)
ui.sendB.clicked.connect(sendText)

ui.show()
app.exec()
