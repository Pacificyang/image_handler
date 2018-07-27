import QtQuick 2.6
import QtQuick.Window 2.2
import QtQuick.Controls 2.1
import QtQuick.Dialogs 1.2
import QtQml 2.2
import an.qt.ImageHandler 1.0

Window {
    visible: true
    width: 640
    height: 480
    title: qsTr("image handler")

    MainForm {
        anchors.fill: parent
        mouseArea.onClicked: {
           // console.log(qsTr('Clicked on ba ckground. Text: "' + textEdit.text + '"'))
        }


        ImageHandler {
                    id: imagehandler
                    path:""
                }


        Button {
                text: "Open";
                anchors.centerIn: parent;
                onClicked: {
                    fileDialog.selectedNameFilter = fileDialog.nameFilters[0];
                    fileDialog.open();
                }
            }

            // 文件对话框
            //-------------------------------------
            FileDialog {
                id: fileDialog;
                title: qsTr("Please choose an image file");
                nameFilters: [
                    "Image Files (*.jpg *.png *.gif *.bmp *.ico)",
                    "Text Files (*.txt *.ini *.log *.c *.h *.java *.cpp *.html *.xml)",
                    "Video Files (*.ts *.mp4 *.avi *.flv *.mkv *.3gp)",
                    "Audio Files (*.mp3 *.ogg *.wav *.wma *.ape *.ra)",
                    "*.*"
                ];
                onAccepted: {
                    var filepath = new String(fileUrl);
                    console.log(filepath.slice(8));
                    if(Qt.platform.os == "windows"){
                        imagehandler.handle(filepath.slice(8))
                    }
                }
            }


//        Button {
//                text: "Quit";
//                anchors.centerIn: parent;
//                onClicked: {
//                    Qt.quit();
//                }
//            }


    }
}
