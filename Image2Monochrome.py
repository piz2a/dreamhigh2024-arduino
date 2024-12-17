import sys
from PyQt5.QtWidgets import *
from PyQt5.QtCore import Qt
from PyQt5.QtGui import QPixmap, QIcon
import cv2
import urllib.request

class MyApp(QWidget):

    def __init__(self):
        super().__init__()
        self.initUI()


    def initUI(self):
        self.setWindowTitle('Image2Monochrome')
        self.move(300, 300)
        self.resize(500,300)
        self.label1 = QLabel("파일명은 알파벳과 숫자만 사용하고, 공백이 없도록 해주세요.", self)
        self.label2 = QLabel("", self)
        box = QVBoxLayout(self)
        try:
            urlString = "https://i.ibb.co/P9qFkcV/SADA-Logo.png"
            imageFromWeb = urllib.request.urlopen(urlString).read()
            qPixmapWebVar = QPixmap()
            qPixmapWebVar.loadFromData(imageFromWeb)
            self.setWindowIcon(QIcon(qPixmapWebVar))
            lbl_img = QLabel()
            lbl_img.setPixmap(qPixmapWebVar)
            box.addWidget(lbl_img, alignment=Qt.AlignCenter)
        except urllib.error.URLError:
            pass

        btn1 = QPushButton("Upload Image", self)
        box.setAlignment(Qt.AlignCenter)
        box.setContentsMargins(0, 0, 0, 0)
        box.addWidget(btn1, alignment=Qt.AlignCenter)
        box.addWidget(self.label1, alignment=Qt.AlignCenter)
        box.addWidget(self.label2, alignment=Qt.AlignCenter)
        box.setSpacing(20)
        btn1.clicked.connect(self.btn_fun_convert)

        self.show()

    def btn_fun_convert(self):
        fname = QFileDialog.getOpenFileName(self, '', '', 'Image(*.jpg *.png *.bmp *.webp);;All Files (*)')
        sys.stdout = open('array.txt', 'w')
        if not fname[0]:
            self.label2.setText("")
            return

        image = cv2.imread(fname[0], cv2.IMREAD_GRAYSCALE)
        if image is None:
            raise Exception("error")

        image_name = (fname[0].split('/')[-1].split('.')[:-1])[0]
        print(f"//image size: {image.shape}")
        print(f"const uint8_t {image_name}[] PROGMEM = {'{'}")
        (thresh, blackAndWhiteImage) = cv2.threshold(image, 127, 255, cv2.THRESH_BINARY)
        blackAndWhiteImage[blackAndWhiteImage == 255] = 1
        for i in range(blackAndWhiteImage.shape[0]):
            cnt = 0
            pixels = ''
            for j in range(blackAndWhiteImage.shape[1]):
                cnt += 1
                pixels += str(blackAndWhiteImage[i][j])
                if cnt % 8 == 0 or (i == blackAndWhiteImage.shape[0]-1 and j == blackAndWhiteImage.shape[1]-1):
                    print('0x'+str(hex(int(pixels, 2)))[2:].zfill(2), end=', ')
                    #print('0b'+pixels, end=', ')
                    pixels = ''
            print()
        print("};\n")

        sys.stdout.close()

        self.label2.setText(f"Converted '{image_name}'.")


if __name__ == '__main__':
   app = QApplication(sys.argv)
   ex = MyApp()
   sys.exit(app.exec_())